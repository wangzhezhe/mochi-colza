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
        }while(0)

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
    tl::mutex            m_mona_mtx;
    mona_instance_t      m_mona;
    std::string          m_mona_addr;
    // Admin RPC
    tl::remote_procedure m_create_pipeline;
    tl::remote_procedure m_destroy_pipeline;
    // Client RPC
    tl::remote_procedure m_check_pipeline;
    tl::remote_procedure m_stage;
    tl::remote_procedure m_execute;
    tl::remote_procedure m_cleanup;
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
            m_mona_addr = buf;
        }
        spdlog::trace("[provider:{0}] Registered provider with id {0}", id());
    }

    ~ProviderImpl() {
        spdlog::trace("[provider:{}] Deregistering provider", id());
        m_create_pipeline.deregister();
        m_destroy_pipeline.deregister();
        m_check_pipeline.deregister();
        spdlog::trace("[provider:{}]    => done!", id());
    }

    void createPipeline(const tl::request& req,
                        const std::string& token,
                        const std::string& pipeline_name,
                        const std::string& pipeline_type,
                        const std::string& pipeline_config) {

        spdlog::trace("[provider:{}] Received createPipeline request", id());
        spdlog::trace("[provider:{}]    => type = {}", id(), pipeline_type);
        spdlog::trace("[provider:{}]    => config = {}", id(), pipeline_config);

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
            json_config = json::parse(pipeline_config);
        } catch(json::parse_error& e) {
            result.error() = e.what();
            result.success() = false;
            spdlog::error("[provider:{}] Could not parse pipeline configuration for pipeline {}",
                    id(), pipeline_name);
            req.respond(result);
            return;
        }

        std::unique_ptr<Backend> pipeline;
        try {
            PipelineFactoryArgs args;
            args.engine = get_engine();
            args.config = json_config;
            args.gid = m_gid;
            pipeline = PipelineFactory::createPipeline(pipeline_type, args);
        } catch(const std::exception& ex) {
            result.success() = false;
            result.error() = ex.what();
            spdlog::error("[provider:{}] Error when creating pipeline {} of type {}:",
                    id(), pipeline_name, pipeline_type);
            spdlog::error("[provider:{}]    => {}", id(), result.error());
            req.respond(result);
            return;
        }

        if(not pipeline) {
            result.success() = false;
            result.error() = "Unknown pipeline type "s + pipeline_type;
            spdlog::error("[provider:{}] Unknown pipeline type {} for pipeline {}",
                    id(), pipeline_type, pipeline_name);
            req.respond(result);
            return;
        } else {
            std::lock_guard<tl::mutex> lock(m_pipelines_mtx);
            m_pipelines[pipeline_name] = std::move(pipeline);
            result.value() = true;
        }

        req.respond(result);
        spdlog::trace("[provider:{}] Successfully created pipeline {} of type {}",
                id(), pipeline_name, pipeline_type);
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
};

}

#endif
