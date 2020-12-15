/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_PROVIDER_IMPL_H
#define __COLZA_PROVIDER_IMPL_H

#include "colza/Backend.hpp"
#include "colza/UUID.hpp"

#include <thallium.hpp>
#include <thallium/serialization/stl/string.hpp>
#include <thallium/serialization/stl/vector.hpp>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <tuple>

#define FIND_PIPELINE(__var__) \
        std::shared_ptr<Backend> __var__;\
        do {\
            std::lock_guard<tl::mutex> lock(m_backends_mtx);\
            auto it = m_backends.find(pipeline_id);\
            if(it == m_backends.end()) {\
                result.success() = false;\
                result.error() = "Pipeline with UUID "s + pipeline_id.to_string() + " not found";\
                req.respond(result);\
                spdlog::error("[provider:{}] Pipeline {} not found", id(), pipeline_id.to_string());\
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
    // Admin RPC
    tl::remote_procedure m_create_pipeline;
    tl::remote_procedure m_destroy_pipeline;
    // Client RPC
    tl::remote_procedure m_check_pipeline;
    // Backends
    std::unordered_map<UUID, std::shared_ptr<Backend>> m_backends;
    tl::mutex m_backends_mtx;

    ProviderImpl(const tl::engine& engine, ssg_group_id_t gid, uint16_t provider_id, const tl::pool& pool)
    : tl::provider<ProviderImpl>(engine, provider_id)
    , m_gid(gid)
    , m_pool(pool)
    , m_create_pipeline(define("colza_create_pipeline", &ProviderImpl::createPipeline, pool))
    , m_destroy_pipeline(define("colza_destroy_pipeline", &ProviderImpl::destroyPipeline, pool))
    , m_check_pipeline(define("colza_check_pipeline", &ProviderImpl::checkPipeline, pool))
    {
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
                        const std::string& pipeline_type,
                        const std::string& pipeline_config) {

        spdlog::trace("[provider:{}] Received createPipeline request", id());
        spdlog::trace("[provider:{}]    => type = {}", id(), pipeline_type);
        spdlog::trace("[provider:{}]    => config = {}", id(), pipeline_config);

        auto pipeline_id = UUID::generate();
        RequestResult<UUID> result;

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
                    id(), pipeline_id.to_string());
            req.respond(result);
            return;
        }

        std::unique_ptr<Backend> backend;
        try {
            backend = PipelineFactory::createPipeline(pipeline_type, m_gid, json_config);
        } catch(const std::exception& ex) {
            result.success() = false;
            result.error() = ex.what();
            spdlog::error("[provider:{}] Error when creating pipeline {} of type {}:",
                    id(), pipeline_id.to_string(), pipeline_type);
            spdlog::error("[provider:{}]    => {}", id(), result.error());
            req.respond(result);
            return;
        }

        if(not backend) {
            result.success() = false;
            result.error() = "Unknown pipeline type "s + pipeline_type;
            spdlog::error("[provider:{}] Unknown pipeline type {} for pipeline {}",
                    id(), pipeline_type, pipeline_id.to_string());
            req.respond(result);
            return;
        } else {
            std::lock_guard<tl::mutex> lock(m_backends_mtx);
            m_backends[pipeline_id] = std::move(backend);
            result.value() = pipeline_id;
        }

        req.respond(result);
        spdlog::trace("[provider:{}] Successfully created pipeline {} of type {}",
                id(), pipeline_id.to_string(), pipeline_type);
    }

    void destroyPipeline(const tl::request& req,
                         const std::string& token,
                         const UUID& pipeline_id) {
        RequestResult<bool> result;
        spdlog::trace("[provider:{}] Received destroyPipeline request for pipeline {}", id(), pipeline_id.to_string());

        if(m_token.size() > 0 && m_token != token) {
            result.success() = false;
            result.error() = "Invalid security token";
            req.respond(result);
            spdlog::error("[provider:{}] Invalid security token {}", id(), token);
            return;
        }

        {
            std::lock_guard<tl::mutex> lock(m_backends_mtx);

            if(m_backends.count(pipeline_id) == 0) {
                result.success() = false;
                result.error() = "Pipeline "s + pipeline_id.to_string() + " not found";
                req.respond(result);
                spdlog::error("[provider:{}] Pipeline {} not found", id(), pipeline_id.to_string());
                return;
            }

            result = m_backends[pipeline_id]->destroy();
            m_backends.erase(pipeline_id);
        }

        req.respond(result);
        spdlog::trace("[provider:{}] Pipeline {} successfully destroyed", id(), pipeline_id.to_string());
    }

    void checkPipeline(const tl::request& req,
                       const UUID& pipeline_id) {
        spdlog::trace("[provider:{}] Received checkPipeline request for pipeline {}", id(), pipeline_id.to_string());
        RequestResult<bool> result;
        FIND_PIPELINE(pipeline);
        result.success() = true;
        req.respond(result);
        spdlog::trace("[provider:{}] Code successfully executed on pipeline {}", id(), pipeline_id.to_string());
    }

};

}

#endif
