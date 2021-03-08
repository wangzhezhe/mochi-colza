/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_PROVIDER_IMPL_H
#define __COLZA_PROVIDER_IMPL_H

#include "colza/Backend.hpp"
#include "colza/Exception.hpp"
#include "colza/ErrorCodes.hpp"
#include "SSGUtil.hpp"

#include <thallium.hpp>
#include <thallium/serialization/stl/string.hpp>
#include <thallium/serialization/stl/vector.hpp>
#include <mona.h>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <fstream>
#include <dlfcn.h>
#include <tuple>

#define FIND_PIPELINE(__var__) \
        std::shared_ptr<PipelineState> __var__;\
        do {\
            std::lock_guard<tl::mutex> lock(m_pipelines_mtx);\
            auto it = m_pipelines.find(pipeline_name);\
            if(it == m_pipelines.end()) {\
                result.success() = false;\
                result.error() = "Pipeline with name "s + pipeline_name + " not found";\
                result.value() = (int)ErrorCode::INVALID_PIPELINE_NAME;\
                req.respond(result);\
                spdlog::error("[provider:{}] Pipeline {} not found", id(), pipeline_name);\
                return;\
            }\
            __var__ = it->second;\
        } while(0)

namespace colza {

using namespace std::string_literals;
namespace tl = thallium;

struct PipelineState {
    std::shared_ptr<Backend> pipeline;
    bool                     active = false;
    uint64_t                 iteration = 0;
};

class ProviderImpl : public tl::provider<ProviderImpl> {

    auto id() const { return get_provider_id(); } // for convenience

    using json = nlohmann::json;

    public:

    // security
    std::string            m_token;
    // SSG
    tl::mutex              m_ssg_mtx;
    tl::condition_variable m_ssg_cv;
    ssg_group_id_t         m_gid;
    uint64_t               m_group_hash = 0;
    tl::pool               m_pool;
    // Mona
    tl::mutex              m_mona_mtx;
    tl::condition_variable m_mona_cv;
    mona_instance_t        m_mona;
    std::string            m_mona_self_addr;
    std::map<ssg_member_id_t, na_addr_t> m_mona_addresses;
    // Admin RPC
    tl::remote_procedure m_create_pipeline;
    tl::remote_procedure m_destroy_pipeline;
    // Client RPC
    tl::remote_procedure m_check_pipeline;
    tl::remote_procedure m_start;
    tl::remote_procedure m_stage;
    tl::remote_procedure m_execute;
    tl::remote_procedure m_cleanup;
    tl::remote_procedure m_abort;
    // Other RPCs
    tl::remote_procedure m_get_mona_addr;
    // Pipelines
    std::unordered_map<std::string, std::shared_ptr<PipelineState>> m_pipelines;
    size_t m_num_active_pipelines = 0;
    tl::mutex m_pipelines_mtx;

    ProviderImpl(const tl::engine& engine, ssg_group_id_t gid, mona_instance_t mona, uint16_t provider_id, const tl::pool& pool)
    : tl::provider<ProviderImpl>(engine, provider_id)
    , m_gid(gid)
    , m_pool(pool)
    , m_mona(mona)
    , m_create_pipeline(define("colza_create_pipeline", &ProviderImpl::createPipeline, pool))
    , m_destroy_pipeline(define("colza_destroy_pipeline", &ProviderImpl::destroyPipeline, pool))
    , m_check_pipeline(define("colza_check_pipeline", &ProviderImpl::checkPipeline, pool))
    , m_start(define("colza_start", &ProviderImpl::start, pool))
    , m_stage(define("colza_stage", &ProviderImpl::stage, pool))
    , m_execute(define("colza_execute", &ProviderImpl::execute, pool))
    , m_cleanup(define("colza_cleanup", &ProviderImpl::cleanup, pool))
    , m_abort(define("colza_abort", &ProviderImpl::abort, pool).disable_response())
    , m_get_mona_addr(define("colza_get_mona_addr", &ProviderImpl::getMonaAddress, pool))
    {
        m_group_hash = ComputeGroupHash(m_gid);
        {
            std::lock_guard<tl::mutex> lock(m_mona_mtx);
            na_addr_t my_mona_addr;
            na_return_t ret = mona_addr_self(m_mona, &my_mona_addr);
            if(ret != NA_SUCCESS)
                throw Exception(ErrorCode::MONA_ERROR,
                    "Could not get address from MoNA");
            char buf[256];
            na_size_t buf_size = 256;
            ret = mona_addr_to_string(m_mona, buf, &buf_size, my_mona_addr);
            mona_addr_free(m_mona, my_mona_addr);
            if(ret != NA_SUCCESS) {
                throw Exception(ErrorCode::MONA_ERROR,
                    "Could not serialize MoNA address");
            }
            m_mona_self_addr = buf;
        }
        m_mona_cv.notify_all();
        _resolveMonaAddresses();
        spdlog::trace("[provider:{0}] Registered provider with id {0}", id());
    }

    ~ProviderImpl() {
        spdlog::trace("[provider:{}] Deregistering provider", id());
        m_create_pipeline.deregister();
        m_destroy_pipeline.deregister();
        m_check_pipeline.deregister();
        m_stage.deregister();
        m_execute.deregister();
        m_cleanup.deregister();
        m_abort.deregister();
        m_pipelines.clear();
        {
            std::lock_guard<tl::mutex> lock(m_mona_mtx);
            for(auto& p : m_mona_addresses) {
                mona_addr_free(m_mona, p.second);
            }
            m_mona_addresses.clear();
        }
        spdlog::trace("[provider:{}]    => done!", id());
    }

    void processConfig(const std::string& config) {
        if(config.empty()) return;
        json json_config;
        try {
            json_config = json::parse(config);
        } catch(...) {
            throw Exception(ErrorCode::JSON_PARSE_ERROR,
                "Could not parse JSON configuration");
        }
        auto it = json_config.find("pipelines");
        if(it == json_config.end()) return;
        auto pipelines = *it;
        if(!pipelines.is_object()) {
            throw Exception(ErrorCode::JSON_CONFIG_ERROR,
                "'pipeline' entry should be an object");
        }
        for(auto& p : pipelines.items()) {
            std::string name = p.key();
            auto& pipeline = p.value();
            if(!pipeline.is_object()) {
                throw Exception(ErrorCode::JSON_CONFIG_ERROR,
                    "Pipeline '"s + name + "' should be an object");
            }
            std::string library = pipeline.value("library", std::string());
            json config = pipeline.value("config", json::object());
            std::string type = pipeline.value("type", std::string());
            if(type.empty()) {
                throw Exception(ErrorCode::JSON_CONFIG_ERROR,
                    "No type provided for pipeline '"s + name + "'");
            }
            _createPipeline(name, type, config, library);
        }
    }

    void _createPipeline(const std::string& name,
                         const std::string& type,
                         const json& config,
                         const std::string& library) {
        if(!library.empty()) {
            void* handle = dlopen(library.c_str(), RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);
            if(!handle) {
                throw Exception(ErrorCode::INVALID_LIBRARY, dlerror());
            }
        }

        std::unique_ptr<Backend> pipeline;
        try {
            PipelineFactoryArgs args;
            args.engine = get_engine();
            args.config = config;
            args.gid = m_gid;
            pipeline = PipelineFactory::createPipeline(type, args);
        } catch(const Exception& ex) {
            spdlog::error("[provider:{}] Error when creating pipeline {} of type {}:",
                    id(), name, type);
            spdlog::error("[provider:{}]    => {}", id(), ex.what());
            throw Exception(ex.code(), ex.what());
        } catch(const std::exception& ex) {
            spdlog::error("[provider:{}] Error when creating pipeline {} of type {}:",
                    id(), name, type);
            spdlog::error("[provider:{}]    => {}", id(), ex.what());
            throw Exception(ErrorCode::OTHER_ERROR, ex.what());
        }

        if(not pipeline) {
            spdlog::error("[provider:{}] Unknown pipeline type {} for pipeline {}",
                    id(), type, name);
            throw Exception(ErrorCode::PIPELINE_CREATE_ERROR,
                "Unknown pipeline type "s + type);
        }

        std::vector<na_addr_t> addresses;
        {
            std::lock_guard<tl::mutex> lock(m_mona_mtx);
            addresses.reserve(m_mona_addresses.size());
            for(const auto& p : m_mona_addresses)
                addresses.push_back(p.second);
        }
        pipeline->updateMonaAddresses(m_mona, addresses);

        {
            std::lock_guard<tl::mutex> lock(m_pipelines_mtx);
            auto state = std::make_shared<PipelineState>();
            state->pipeline = std::move(pipeline);
            m_pipelines[name] = std::move(state);
        }

        spdlog::trace("[provider:{}] Successfully created pipeline {} of type {}",
                id(), name, type);
    }

    void createPipeline(const tl::request& req,
                        const std::string& token,
                        const std::string& pipeline_name,
                        const std::string& pipeline_type,
                        const std::string& pipeline_config,
                        const std::string& library) {

        spdlog::trace("[provider:{}] Received createPipeline request", id());
        spdlog::trace("[provider:{}]    => type = {}", id(), pipeline_type);
        if(!pipeline_config.empty())
            spdlog::trace("[provider:{}]    => config = {}", id(), pipeline_config);
        if(!library.empty()) {
            spdlog::trace("[provider:{}]    => library = {}", id(), library);
        }

        RequestResult<int32_t> result;

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
            result.value() = (int)ErrorCode::INVALID_SECURITY_TOKEN;
            req.respond(result);
            spdlog::error("[provider:{}] Invalid security token {}", id(), token);
            return;
        }

        json json_config;
        try {
            if(!pipeline_config.empty()) {
                json_config = json::parse(pipeline_config);
            }
        } catch(json::parse_error& e) {
            result.error() = e.what();
            result.value() = (int)ErrorCode::JSON_PARSE_ERROR;
            result.success() = false;
            spdlog::error("[provider:{}] Could not parse pipeline configuration for pipeline {}",
                    id(), pipeline_name);
            req.respond(result);
            return;
        }

        try {
            _createPipeline(pipeline_name, pipeline_type, json_config, library);
        } catch(Exception& e) {
            result.error()   = e.what();
            result.success() = false;
            result.value()   = (int)e.code();
            req.respond(result);
            return;
        }
        result.success() = true;
        req.respond(result);
    }

    void destroyPipeline(const tl::request& req,
                         const std::string& token,
                         const std::string& pipeline_name) {
        RequestResult<int32_t> result;
        spdlog::trace("[provider:{}] Received destroyPipeline request for pipeline {}", id(), pipeline_name);

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
            result.value() = (int)ErrorCode::INVALID_SECURITY_TOKEN;
            req.respond(result);
            spdlog::error("[provider:{}] Invalid security token {}", id(), token);
            return;
        }

        {
            std::lock_guard<tl::mutex> lock(m_pipelines_mtx);

            if(m_pipelines.count(pipeline_name) == 0) {
                result.success() = false;
                result.error() = "Pipeline "s + pipeline_name + " not found";
                result.value() = (int)ErrorCode::INVALID_PIPELINE_NAME;
                req.respond(result);
                spdlog::error("[provider:{}] Pipeline {} not found", id(), pipeline_name);
                return;
            }

            auto state = m_pipelines[pipeline_name];
            if(state->active) {
                result.success() = false;
                result.error() = "Cannot destroy a pipeline while active";
                result.value() = (int)ErrorCode::PIPELINE_IS_ACTIVE;
                req.respond(result);
                spdlog::error("[provider:{}] Pipeline {} could not be destroyed "
                              "because it is active", id(), pipeline_name);
                return;
            }

            result = state->pipeline->destroy();
            m_pipelines.erase(pipeline_name);
        }

        req.respond(result);
        spdlog::trace("[provider:{}] Pipeline {} successfully destroyed", id(), pipeline_name);
    }

    void checkPipeline(const tl::request& req,
                       const std::string& pipeline_name) {
        spdlog::trace("[provider:{}] Received checkPipeline request for pipeline {}", id(), pipeline_name);
        RequestResult<int32_t> result;
        FIND_PIPELINE(state);
        result.success() = true;
        req.respond(result);
        spdlog::trace("[provider:{}] Code successfully executed on pipeline {}", id(), pipeline_name);
    }

    void start(const tl::request& req,
               uint64_t group_hash,
               const std::string& pipeline_name,
               uint64_t iteration) {
        spdlog::trace("[provider:{}] Received start request for pipeline {}", id(), pipeline_name);
        RequestResult<int32_t> result;
        FIND_PIPELINE(state);
        auto pipeline = state->pipeline;
        if(state->active) {
            result.value() = (int)ErrorCode::PIPELINE_IS_ACTIVE;
            result.success() = false;
            result.error() = "Pipeline is already active";
        } else if(state->iteration != 0 && state->iteration >= iteration) {
            result.value() = (int)ErrorCode::INVALID_ITERATION;
            result.success() = false;
            result.error() = "Pipeline cannot be started at an inferior iteration number";
        } else {
            {
                std::lock_guard<tl::mutex> lock(m_pipelines_mtx);
                m_num_active_pipelines += 1;
            }
            result = pipeline->start(iteration);
            spdlog::trace("[provider:{}] Pipeline {} successfuly started iteration {}",
                          id(), pipeline_name, iteration);
            if(result.success()) {
                state->iteration = iteration;
                state->active = true;
            } else {
                std::lock_guard<tl::mutex> lock(m_pipelines_mtx);
                m_num_active_pipelines -= 1;
            }
        }
        req.respond(result);
    }

    void stage(const tl::request& req,
               const std::string& pipeline_name,
               const std::string& sender_addr,
               const std::string& dataset_name,
               uint64_t iteration,
               uint64_t block_id,
               const std::vector<size_t>& dimensions,
               const std::vector<int64_t>& offsets,
               const Type& type,
               const thallium::bulk& data) {
        spdlog::trace("[provider:{}] Received stage request for pipeline {}", id(), pipeline_name);
        RequestResult<int32_t> result;
        FIND_PIPELINE(state);
        auto pipeline = state->pipeline;
        if(!state->active) {
            result.value() = (int)ErrorCode::PIPELINE_NOT_ACTIVE;
            result.success() = false;
            result.error() = "Pipeline is not active";
        } else if(state->iteration != iteration) {
            result.value() = (int)ErrorCode::INVALID_ITERATION;
            result.success() = false;
            result.error() = "Invalid iteration";
        } else {
            result = pipeline->stage(
                    sender_addr, dataset_name, iteration,
                    block_id, dimensions, offsets, type, data);
        }
        req.respond(result);
    }

    void execute(const tl::request& req,
                 const std::string& pipeline_name,
                 uint64_t iteration) {
        spdlog::trace("[provider:{}] Received execute request for pipeline {}", id(), pipeline_name);
        RequestResult<int32_t> result;
        FIND_PIPELINE(state);
        auto pipeline = state->pipeline;
        if(!state->active) {
            result.value() = (int)ErrorCode::PIPELINE_NOT_ACTIVE;
            result.success() = false;
            result.error() = "Pipeline is not active";
        } else if(state->iteration != iteration) {
            result.value() = (int)ErrorCode::INVALID_ITERATION;
            result.success() = false;
            result.error() = "Invalid iteration";
        } else {
            result = pipeline->execute(iteration);
        }
        req.respond(result);
    }

    void cleanup(const tl::request& req,
                 const std::string& pipeline_name,
                 uint64_t iteration) {
        spdlog::trace("[provider:{}] Received cleanup request for pipeline {}", id(), pipeline_name);
        RequestResult<int32_t> result;
        FIND_PIPELINE(state);
        auto pipeline = state->pipeline;
        if(!state->active) {
            result.value() = (int)ErrorCode::PIPELINE_NOT_ACTIVE;
            result.success() = false;
            result.error() = "Pipeline is not active";
        } else if(state->iteration != iteration) {
            result.value() = (int)ErrorCode::INVALID_ITERATION;
            result.success() = false;
            result.error() = "Invalid iteration";
        } else {
            result = pipeline->cleanup(iteration);
            if(result.success()) {
                std::lock_guard<tl::mutex> lock(m_pipelines_mtx);
                state->active = false;
                m_num_active_pipelines -= 1;
            }
        }
        req.respond(result);
    }

    void abort(const std::string& pipeline_name,
               uint64_t iteration) {
        spdlog::trace("[provider:{}] Received abort request for pipeline {}", id(), pipeline_name);
        RequestResult<int32_t> result;
        std::shared_ptr<PipelineState> state;
        {
            std::lock_guard<tl::mutex> lock(m_pipelines_mtx);
            auto it = m_pipelines.find(pipeline_name);
            if(it == m_pipelines.end()) {
                spdlog::error("[provider:{}] Pipeline {} not found", id(), pipeline_name);
                return;
            }
            state = it->second;
        }
        auto pipeline = state->pipeline;
        if(!state->active || state->iteration != iteration) {
            return;
        } else {
            pipeline->abort(iteration);
            state->active = false;
        }
    }

    void getMonaAddress(const tl::request& req) {
        spdlog::trace("[provider:{}] Received request for MoNA address", id());
        RequestResult<std::string> result;
        {
            std::unique_lock<tl::mutex> guard(m_mona_mtx);
            while(m_mona_self_addr.empty()) {
                m_mona_cv.wait(guard);
            }
            result.value() = m_mona_self_addr;
        }
        req.respond(result);
    }

    na_addr_t _requestMonaAddressFromSSGMember(ssg_member_id_t member_id) {
        hg_addr_t hg_addr = ssg_get_group_member_addr(m_gid, member_id);
        tl::provider_handle ph;
        try {
            ph = tl::provider_handle(get_engine(), hg_addr, get_provider_id(), false);
        } catch(const std::exception& e) {
            spdlog::critical("Could not create provider handle from address to member {}: {}",
                             member_id, e.what());
            throw;
        }
        RequestResult<std::string> result;
        bool ok = false;
        while(!ok) {
            try {
                result = m_get_mona_addr.on(ph)();
                ok = true;
            } catch(...) {
                // TODO improve that to retry only in relevant
                // cases and sleep for increasing amounts of time
                spdlog::trace("[provider:{}] Failed to get MoNA address of member {}, retrying...", id(), member_id);
                tl::thread::sleep(get_engine(), 100);
            }
        }
        na_addr_t addr = NA_ADDR_NULL;
        na_return_t ret = mona_addr_lookup(m_mona, result.value().c_str(), &addr);
        if(ret != NA_SUCCESS)
            throw Exception(ErrorCode::MONA_ERROR,
                "mona_addr_lookup failed with error code "s + std::to_string(ret));
        spdlog::trace("[provider:{}] Successfully obtained MoNA address of member {}", id(), member_id);
        return addr;
    }

    void _resolveMonaAddresses() {
        // TODO we could speed up this function if the getMonaAddress RPC
        // were to piggy-back the addresses it already knows
        spdlog::trace("[provider:{}] Resolving MoNA addressed of SSG group", id());
        auto self_id = ssg_get_self_id(get_engine().get_margo_instance());
        auto self_rank = ssg_get_group_member_rank(m_gid, self_id);
        auto group_size = ssg_get_group_size(m_gid);
        decltype(m_mona_addresses) addresses;
        for(int i = 0; i < group_size; i++) {
            int j = (self_rank + i) % group_size;
            ssg_member_id_t member_id = ssg_get_group_member_id_from_rank(m_gid, j);
            if(i == 0) {
                na_addr_t self_mona_addr;
                mona_addr_self(m_mona, &self_mona_addr);
                addresses[member_id] = self_mona_addr;
            } else {
                addresses[member_id] = _requestMonaAddressFromSSGMember(member_id);
            }
        }
        {
            std::lock_guard<tl::mutex> lock(m_mona_mtx);
            m_mona_addresses = addresses;
        }
        m_mona_cv.notify_all();
        spdlog::trace("[provider:{}] Done resolving MoNA addressed of SSG group", id());
        spdlog::trace("[provider:{}] {} addresses found in SSG group", id(), group_size);
    }

    void _membershipUpdate(ssg_member_id_t member_id,
                           ssg_member_update_type_t update_type) {
        spdlog::trace("[provider:{}] Member {} updated", id(), member_id);
        m_group_hash = UpdateGroupHash(m_group_hash, member_id);
        // TODO use the provider's pool instead of self ES
        tl::xstream::self().make_thread([this, member_id, update_type]() {

        if(update_type == SSG_MEMBER_JOINED) {
            spdlog::trace("[provider:{}] Member {} joined", id(), member_id);
            na_addr_t na_addr = _requestMonaAddressFromSSGMember(member_id);
            {
                std::lock_guard<tl::mutex> lock(m_mona_mtx);
                m_mona_addresses[member_id] = na_addr;
            }
            m_mona_cv.notify_all();
        } else {
            spdlog::trace("[provider:{}] Member {} left", id(), member_id);
            {
                std::lock_guard<tl::mutex> lock(m_mona_mtx);
                m_mona_addresses.erase(member_id);
            }
        }
        std::vector<na_addr_t> addresses;
        {
            std::lock_guard<tl::mutex> lock(m_mona_mtx);
            addresses.reserve(m_mona_addresses.size());
            for(const auto& p : m_mona_addresses)
                addresses.push_back(p.second);
        }
        {
            std::lock_guard<tl::mutex> lock(m_pipelines_mtx);
            for(auto& p : m_pipelines) {
                auto& state = p.second;
                state->pipeline->updateMonaAddresses(m_mona, addresses);
            }
        }

        }, tl::anonymous());
    }

    static void membershipUpdate(void* p, ssg_member_id_t member_id,
            ssg_member_update_type_t update_type) {
        auto provider = static_cast<Provider*>(p);
        provider->self->_membershipUpdate(member_id, update_type);
    }
};

}

#endif
