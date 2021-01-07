/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_PROVIDER_IMPL_H
#define __COLZA_PROVIDER_IMPL_H

#include "colza/Backend.hpp"
#include "colza/Exception.hpp"

#include <thallium.hpp>
#include <thallium/serialization/stl/string.hpp>
#include <thallium/serialization/stl/vector.hpp>
#include <mona.h>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <dlfcn.h>
#include <tuple>

#define FIND_PIPELINE(__var__) \
        std::shared_ptr<Backend> __var__;\
        do {\
            std::lock_guard<tl::mutex> lock(m_pipelines_mtx);\
            auto it = m_pipelines.find(pipeline_name);\
            if(it == m_pipelines.end()) {\
                result.success() = false;\
                result.error() = "Pipeline with name "s + pipeline_name + " not found";\
                req.respond(result);\
                spdlog::error("[provider:{}] Pipeline {} not found", id(), pipeline_name);\
                return;\
            }\
            __var__ = it->second;\
        } while(0)

namespace colza {

using namespace std::string_literals;
namespace tl = thallium;

class ProviderImpl : public tl::provider<ProviderImpl> {

    auto id() const { return get_provider_id(); } // for convenience

    using json = nlohmann::json;

    public:

    std::string          m_token;
    ssg_group_id_t       m_gid;
    tl::pool             m_pool;
    // Mona
    tl::mutex              m_mona_mtx;
    tl::condition_variable m_mona_cv;
    mona_instance_t        m_mona;
    std::string            m_mona_self_addr;
    std::vector<na_addr_t> m_mona_addresses;
    // Admin RPC
    tl::remote_procedure m_create_pipeline;
    tl::remote_procedure m_destroy_pipeline;
    // Client RPC
    tl::remote_procedure m_check_pipeline;
    tl::remote_procedure m_stage;
    tl::remote_procedure m_execute;
    tl::remote_procedure m_cleanup;
    // Other RPCs
    tl::remote_procedure m_get_mona_addr;
    // Backends
    std::unordered_map<std::string, std::shared_ptr<Backend>> m_pipelines;
    tl::mutex m_pipelines_mtx;

    ProviderImpl(const tl::engine& engine, ssg_group_id_t gid, mona_instance_t mona, uint16_t provider_id, const tl::pool& pool)
    : tl::provider<ProviderImpl>(engine, provider_id)
    , m_gid(gid)
    , m_pool(pool)
    , m_mona(mona)
    , m_create_pipeline(define("colza_create_pipeline", &ProviderImpl::createPipeline, pool))
    , m_destroy_pipeline(define("colza_destroy_pipeline", &ProviderImpl::destroyPipeline, pool))
    , m_check_pipeline(define("colza_check_pipeline", &ProviderImpl::checkPipeline, pool))
    , m_stage(define("colza_stage", &ProviderImpl::stage, pool))
    , m_execute(define("colza_execute", &ProviderImpl::execute, pool))
    , m_cleanup(define("colza_cleanup", &ProviderImpl::cleanup, pool))
    , m_get_mona_addr(define("colza_get_mona_addr", &ProviderImpl::getMonaAddress, pool))
    {
        {
            std::lock_guard<tl::mutex> lock(m_mona_mtx);
            na_addr_t my_mona_addr;
            na_return_t ret = mona_addr_self(m_mona, &my_mona_addr);
            if(ret != NA_SUCCESS)
                throw Exception("Could not get address from MoNA");
            char buf[256];
            na_size_t buf_size = 256;
            ret = mona_addr_to_string(m_mona, buf, &buf_size, my_mona_addr);
            mona_addr_free(m_mona, my_mona_addr);
            if(ret != NA_SUCCESS) {
                throw Exception("Could not serialize MoNA address");
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
        m_pipelines.clear();
        for(auto& addr : m_mona_addresses) {
            mona_addr_free(m_mona, addr);
        }
        spdlog::trace("[provider:{}]    => done!", id());
    }

    void processConfig(const std::string& config) {
        if(config.empty()) return;
        json json_config;
        try {
            json_config = json::parse(config);
        } catch(...) {
            throw Exception("Could not parse JSON configuration");
        }
        auto it = json_config.find("pipelines");
        if(it == json_config.end()) return;
        auto pipelines = *it;
        if(!pipelines.is_object()) {
            throw Exception("'pipeline' entry should be an object");
        }
        for(auto& p : pipelines.items()) {
            std::string name = p.key();
            auto& pipeline = p.value();
            if(!pipeline.is_object()) {
                throw Exception("Pipeline '"s + name + "' should be an object");
            }
            std::string library = pipeline.value("library", std::string());
            json config = pipeline.value("config", json::object());
            std::string type = pipeline.value("type", std::string());
            if(type.empty()) {
                throw Exception("No type provided for pipeline '"s + name + "'");
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
                throw Exception(dlerror());
            }
        }

        std::unique_ptr<Backend> pipeline;
        try {
            PipelineFactoryArgs args;
            args.engine = get_engine();
            args.config = config;
            args.gid = m_gid;
            pipeline = PipelineFactory::createPipeline(type, args);
        } catch(const std::exception& ex) {
            spdlog::error("[provider:{}] Error when creating pipeline {} of type {}:",
                    id(), name, type);
            spdlog::error("[provider:{}]    => {}", id(), ex.what());
            throw Exception(ex.what());
        }

        if(not pipeline) {
            spdlog::error("[provider:{}] Unknown pipeline type {} for pipeline {}",
                    id(), type, name);
            throw Exception("Unknown pipeline type "s + type);
        } else {
            std::lock_guard<tl::mutex> lock(m_pipelines_mtx);
            m_pipelines[name] = std::move(pipeline);
        }

        pipeline->updateMonaAddresses(m_mona, m_mona_addresses);

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

        RequestResult<bool> result;

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
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
            result.success() = false;
            spdlog::error("[provider:{}] Could not parse pipeline configuration for pipeline {}",
                    id(), pipeline_name);
            req.respond(result);
            return;
        }

        try {
            _createPipeline(pipeline_name, pipeline_type, json_config, library);
        } catch(Exception& e) {
            result.error() = e.what();
            result.success() = false;
            req.respond(result);
            return;
        }
        result.success() = true;
        req.respond(result);
    }

    void destroyPipeline(const tl::request& req,
                         const std::string& token,
                         const std::string& pipeline_name) {
        RequestResult<bool> result;
        spdlog::trace("[provider:{}] Received destroyPipeline request for pipeline {}", id(), pipeline_name);

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
            req.respond(result);
            spdlog::error("[provider:{}] Invalid security token {}", id(), token);
            return;
        }

        {
            std::lock_guard<tl::mutex> lock(m_pipelines_mtx);

            if(m_pipelines.count(pipeline_name) == 0) {
                result.success() = false;
                result.error() = "Pipeline "s + pipeline_name + " not found";
                req.respond(result);
                spdlog::error("[provider:{}] Pipeline {} not found", id(), pipeline_name);
                return;
            }

            result = m_pipelines[pipeline_name]->destroy();
            m_pipelines.erase(pipeline_name);
        }

        req.respond(result);
        spdlog::trace("[provider:{}] Pipeline {} successfully destroyed", id(), pipeline_name);
    }

    void checkPipeline(const tl::request& req,
                       const std::string& pipeline_name) {
        spdlog::trace("[provider:{}] Received checkPipeline request for pipeline {}", id(), pipeline_name);
        RequestResult<bool> result;
        FIND_PIPELINE(pipeline);
        result.success() = true;
        req.respond(result);
        spdlog::trace("[provider:{}] Code successfully executed on pipeline {}", id(), pipeline_name);
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
        FIND_PIPELINE(pipeline);
        result = pipeline->stage(
            sender_addr, dataset_name, iteration, block_id, dimensions, offsets, type, data);
        req.respond(result);
        spdlog::trace("[provider:{}] Data successfully staged on pipeline {}", id(), pipeline_name);
    }

    void execute(const tl::request& req,
                 const std::string& pipeline_name,
                 uint64_t iteration) {
        spdlog::trace("[provider:{}] Received execute request for pipeline {}", id(), pipeline_name);
        RequestResult<int32_t> result;
        FIND_PIPELINE(pipeline);
        result = pipeline->execute(iteration);
        req.respond(result);
        spdlog::trace("[provider:{}] Pipeline {} successfuly executed", id(), pipeline_name);
    }

    void cleanup(const tl::request& req,
                 const std::string& pipeline_name,
                 uint64_t iteration) {
        spdlog::trace("[provider:{}] Received cleanup request for pipeline {}", id(), pipeline_name);
        RequestResult<int32_t> result;
        FIND_PIPELINE(pipeline);
        result = pipeline->cleanup(iteration);
        req.respond(result);
        spdlog::trace("[provider:{}] Pipeline {} successfuly executed", id(), pipeline_name);
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
        auto ph = tl::provider_handle(get_engine(), hg_addr, get_provider_id(), false);
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
            throw Exception("mona_addr_lookup failed with error code "s + std::to_string(ret));
        return addr;
    }

    void _resolveMonaAddresses() {
        // TODO we could speed up this function if the getMonaAddress RPC
        // were to piggy-back the addresses it already knows
        spdlog::trace("[provider:{}] Resolving MoNA addressed of SSG group", id());
        auto self_id = ssg_get_self_id(get_engine().get_margo_instance());
        auto self_rank = ssg_get_group_member_rank(m_gid, self_id);
        auto group_size = ssg_get_group_size(m_gid);
        m_mona_addresses.resize(group_size);
        for(int i = 0; i < group_size; i++) {
            int j = (self_rank + i) % group_size;
            if(i == 0) {
                na_addr_t self_mona_addr;
                mona_addr_self(m_mona, &self_mona_addr);
                m_mona_addresses[j] = self_mona_addr;
            } else {
                ssg_member_id_t member_id = ssg_get_group_member_id_from_rank(m_gid, j);
                m_mona_addresses[j] = _requestMonaAddressFromSSGMember(member_id);
            }
        }
        spdlog::trace("[provider:{}] Done resolving MoNA addressed of SSG group", id());
        spdlog::trace("[provider:{}] {} addresses found in SSG group", id(), group_size);
    }
};

}

#endif
