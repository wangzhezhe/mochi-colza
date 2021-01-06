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
                           const std::string& token) const {
    auto endpoint  = self->m_engine.lookup(address);
    auto ph        = tl::provider_handle(endpoint, provider_id);
    RequestResult<bool> result = self->m_create_pipeline.on(ph)(token, pipeline_name, pipeline_type, pipeline_config);
    if(not result.success()) {
        throw Exception(result.error());
    }
}

void Admin::destroyPipeline(const std::string& address,
                            uint16_t provider_id,
                            const std::string& pipeline_name,
                            const std::string& token) const {
    auto endpoint  = self->m_engine.lookup(address);
    auto ph        = tl::provider_handle(endpoint, provider_id);
    RequestResult<bool> result = self->m_destroy_pipeline.on(ph)(token, pipeline_name);
    if(not result.success()) {
        throw Exception(result.error());
    }
}

void Admin::createDistributedPipeline(const std::string& ssg_file,
                        uint16_t provider_id,
                        const std::string& name,
                        const std::string& type,
                        const std::string& config,
                        const std::string& token) const {
    ssg_group_id_t gid;
    int num_addrs = -1;
    int ret = ssg_group_id_load(ssg_file.c_str(), &num_addrs, &gid);
    if(ret != SSG_SUCCESS)
        throw Exception("Could not open SSG file "s + ssg_file);

    ret = ssg_group_observe(self->m_engine.get_margo_instance(), gid);
    if(ret != SSG_SUCCESS)
        throw Exception("Could not observe SSG group from "s + ssg_file);

    std::vector<std::string> addresses;
    int group_size = ssg_get_group_size(gid);
    addresses.reserve(group_size);
    for(int i = 0; i < group_size; i++) {
        char *addr = ssg_group_id_get_addr_str(gid, i);
        addresses.emplace_back(addr);
        free(addr);
    }

    ssg_group_unobserve(gid);

    auto es = tl::xstream::self();
    std::vector<tl::managed<tl::thread>> ults;
    for(const auto& addr : addresses) {
        ults.push_back(es.make_thread([&, this](){
            createPipeline(addr, provider_id, name, type, config, token);
        }));
    }

    for(auto& ult : ults) {
        ult->join();
    }
}

void Admin::destroyDistributedPipeline(const std::string& ssg_file,
                         uint16_t provider_id,
                         const std::string& name,
                         const std::string& token) const {
    ssg_group_id_t gid;
    int num_addrs = -1;
    int ret = ssg_group_id_load(ssg_file.c_str(), &num_addrs, &gid);
    if(ret != SSG_SUCCESS)
        throw Exception("Could not open SSG file "s + ssg_file);

    ret = ssg_group_observe(self->m_engine.get_margo_instance(), gid);
    if(ret != SSG_SUCCESS)
        throw Exception("Could not observe SSG group from "s + ssg_file);

    std::vector<std::string> addresses;
    int group_size = ssg_get_group_size(gid);
    addresses.reserve(group_size);
    for(int i = 0; i < group_size; i++) {
        char *addr = ssg_group_id_get_addr_str(gid, i);
        addresses.emplace_back(addr);
        free(addr);
    }

    ssg_group_unobserve(gid);

    auto es = tl::xstream::self();
    std::vector<tl::managed<tl::thread>> ults;
    for(const auto& addr : addresses) {
        ults.push_back(es.make_thread([&, this](){
            destroyPipeline(addr, provider_id, name, token);
        }));
    }

    for(auto& ult : ults) {
        ult->join();
    }
}

void Admin::shutdownServer(const std::string& address) const {
    auto ep = self->m_engine.lookup(address);
    self->m_engine.shutdown_remote_engine(ep);
}

}
