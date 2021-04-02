/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "colza/Admin.hpp"
#include "colza/Exception.hpp"
#include "colza/RequestResult.hpp"

#include "AdminImpl.hpp"

#include <ssg.h>
#include <thallium/serialization/stl/string.hpp>

namespace tl = thallium;

namespace colza {

using namespace std::string_literals;

Admin::Admin() = default;

Admin::Admin(const tl::engine& engine)
: self(std::make_shared<AdminImpl>(engine)) {}

Admin::Admin(margo_instance_id mid)
: self(std::make_shared<AdminImpl>(mid)) {}

Admin::Admin(Admin&& other) = default;

Admin& Admin::operator=(Admin&& other) = default;

Admin::Admin(const Admin& other) = default;

Admin& Admin::operator=(const Admin& other) = default;

Admin::~Admin() = default;

Admin::operator bool() const {
    return static_cast<bool>(self);
}

void Admin::createPipeline(const std::string& address,
                           uint16_t provider_id,
                           const std::string& pipeline_name,
                           const std::string& pipeline_type,
                           const std::string& pipeline_config,
                           const std::string& library,
                           const std::string& token) const {
    auto endpoint  = self->m_engine.lookup(address);
    auto ph        = tl::provider_handle(endpoint, provider_id);
    RequestResult<int32_t> result = self->m_create_pipeline.on(ph)(
            token, pipeline_name, pipeline_type, pipeline_config, library);
    if(not result.success()) {
        throw Exception((ErrorCode)result.value(), result.error());
    }
}

void Admin::destroyPipeline(const std::string& address,
                            uint16_t provider_id,
                            const std::string& pipeline_name,
                            const std::string& token) const {
    auto endpoint  = self->m_engine.lookup(address);
    auto ph        = tl::provider_handle(endpoint, provider_id);
    RequestResult<int32_t> result = self->m_destroy_pipeline.on(ph)(token, pipeline_name);
    if(not result.success()) {
        throw Exception((ErrorCode)result.value(), result.error());
    }
}

void Admin::createDistributedPipeline(const std::string& ssg_file,
                        uint16_t provider_id,
                        const std::string& name,
                        const std::string& type,
                        const std::string& config,
                        const std::string& library,
                        const std::string& token) const {
    ssg_group_id_t gid;
    int num_addrs = -1;
    int ret = ssg_group_id_load(ssg_file.c_str(), &num_addrs, &gid);
    if(ret != SSG_SUCCESS)
        throw Exception(ErrorCode::SSG_ERROR, "Could not open SSG file "s + ssg_file);

    ret = ssg_group_observe(self->m_engine.get_margo_instance(), gid);
    if(ret != SSG_SUCCESS)
        throw Exception(ErrorCode::SSG_ERROR, "Could not observe SSG group from "s + ssg_file);

    int group_size = ssg_get_group_size(gid);
    std::vector<tl::managed<tl::thread>> ults;
    for(int i = 0; i < group_size; i++) {
        ults.push_back(tl::xstream::self().make_thread(
                [gid, this, i, provider_id, &name, &type, &config, &library, &token]() {
            ssg_member_id_t member_id = ssg_get_group_member_id_from_rank(gid, i);
            hg_addr_t a = ssg_get_group_member_addr(gid, member_id);
            auto e = tl::endpoint(self->m_engine, a, false);
            createPipeline(static_cast<std::string>(e), provider_id, name, type, config, library, token);
        }));
    }
    for(auto& ult : ults) {
        ult->join();
    }
    ssg_group_unobserve(gid);
}

void Admin::destroyDistributedPipeline(const std::string& ssg_file,
                         uint16_t provider_id,
                         const std::string& name,
                         const std::string& token) const {
    ssg_group_id_t gid;
    int num_addrs = -1;
    int ret = ssg_group_id_load(ssg_file.c_str(), &num_addrs, &gid);
    if(ret != SSG_SUCCESS)
        throw Exception(ErrorCode::SSG_ERROR,
            "Could not open SSG file "s + ssg_file);

    ret = ssg_group_observe(self->m_engine.get_margo_instance(), gid);
    if(ret != SSG_SUCCESS)
        throw Exception(ErrorCode::SSG_ERROR,
            "Could not observe SSG group from "s + ssg_file);

    int group_size = ssg_get_group_size(gid);
    std::vector<tl::managed<tl::thread>> ults;
    for(int i = 0; i < group_size; i++) {
        ults.push_back(tl::xstream::self().make_thread([gid, this, i, provider_id, &name, &token]() {
            ssg_member_id_t member_id = ssg_get_group_member_id_from_rank(gid, i);
            hg_addr_t a = ssg_get_group_member_addr(gid, member_id);
            auto e = tl::endpoint(self->m_engine, a, false);
            destroyPipeline(static_cast<std::string>(e), provider_id, name, token);
        }));
    }
    for(auto& ult : ults) {
        ult->join();
    }

    ssg_group_unobserve(gid);
}

void Admin::shutdownServer(const std::string& address) const {
    auto ep = self->m_engine.lookup(address);
    self->m_engine.shutdown_remote_engine(ep);
}

void Admin::shutdownGroup(const std::string& ssg_file) const {
    ssg_group_id_t gid;
    int num_addrs = -1;
    int ret = ssg_group_id_load(ssg_file.c_str(), &num_addrs, &gid);
    if(ret != SSG_SUCCESS)
        throw Exception(ErrorCode::SSG_ERROR,
            "Could not open SSG file "s + ssg_file);

    ret = ssg_group_observe(self->m_engine.get_margo_instance(), gid);
    if(ret != SSG_SUCCESS)
        throw Exception(ErrorCode::SSG_ERROR,
            "Could not observe SSG group from "s + ssg_file);

    int group_size = ssg_get_group_size(gid);
    std::vector<tl::managed<tl::thread>> ults;
    for(int i = 0; i < group_size; i++) {
        ults.push_back(tl::xstream::self().make_thread([gid, this, i]() {
            ssg_member_id_t member_id = ssg_get_group_member_id_from_rank(gid, i);
            hg_addr_t a = ssg_get_group_member_addr(gid, member_id);
            auto e = tl::endpoint(self->m_engine, a, false);
            self->m_engine.shutdown_remote_engine(e);
        }));
    }
    for(auto& ult : ults) {
        ult->join();
    }

    ssg_group_unobserve(gid);
}

void Admin::makeServerLeave(const std::string& address, uint16_t provider_id) const {
    auto ep = self->m_engine.lookup(address);
    auto ph = tl::provider_handle(ep, provider_id);
    self->m_leave.on(ph)();
}

void Admin::makeServersLeave(
        const std::string& ssg_file,
        const std::vector<int>& ranks,
        uint16_t provider_id) const {
    ssg_group_id_t gid;
    int num_addrs = -1;
    int ret = ssg_group_id_load(ssg_file.c_str(), &num_addrs, &gid);
    if(ret != SSG_SUCCESS)
        throw Exception(ErrorCode::SSG_ERROR,
            "Could not open SSG file "s + ssg_file);

    ret = ssg_group_observe(self->m_engine.get_margo_instance(), gid);
    if(ret != SSG_SUCCESS)
        throw Exception(ErrorCode::SSG_ERROR,
            "Could not observe SSG group from "s + ssg_file);

    int group_size = ssg_get_group_size(gid);
    for(auto rank : ranks) {
        if(rank < 0 || rank >= group_size)
            continue;
        ssg_member_id_t member_id = ssg_get_group_member_id_from_rank(gid, rank);
        hg_addr_t a = ssg_get_group_member_addr(gid, member_id);
        auto ph = tl::provider_handle(self->m_engine, a, provider_id, false);
        self->m_leave.on(ph)();
    }

    ssg_group_unobserve(gid);
}

}
