/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "colza/Exception.hpp"
#include "colza/Client.hpp"
#include "colza/PipelineHandle.hpp"
#include "colza/DistributedPipelineHandle.hpp"
#include "colza/RequestResult.hpp"

#include "ClientImpl.hpp"
#include "PipelineHandleImpl.hpp"
#include "DistributedPipelineHandleImpl.hpp"

#include <ssg.h>
#include <thallium/serialization/stl/string.hpp>

namespace tl = thallium;

namespace colza {

using namespace std::string_literals;

Client::Client() = default;

Client::Client(const tl::engine& engine)
: self(std::make_shared<ClientImpl>(engine)) {}

Client::Client(margo_instance_id mid)
: self(std::make_shared<ClientImpl>(mid)) {}

Client::Client(const std::shared_ptr<ClientImpl>& impl)
: self(impl) {}

Client::Client(Client&& other) = default;

Client& Client::operator=(Client&& other) = default;

Client::Client(const Client& other) = default;

Client& Client::operator=(const Client& other) = default;


Client::~Client() = default;

const tl::engine& Client::engine() const {
    return self->m_engine;
}

Client::operator bool() const {
    return static_cast<bool>(self);
}

PipelineHandle Client::makePipelineHandle(
        const std::string& address,
        uint16_t provider_id,
        const std::string& pipeline_name,
        bool check) const {
    auto endpoint  = self->m_engine.lookup(address);
    auto ph        = tl::provider_handle(endpoint, provider_id);
    RequestResult<int32_t> result;
    result.success() = true;
    if(check) {
        result = self->m_check_pipeline.on(ph)(pipeline_name);
    }
    if(result.success()) {
        auto pipeline_impl = std::make_shared<PipelineHandleImpl>(self, std::move(ph), pipeline_name);
        return PipelineHandle(pipeline_impl);
    } else {
        throw Exception((ErrorCode)result.value(), result.error());
        return PipelineHandle(nullptr);
    }
}

DistributedPipelineHandle Client::makeDistributedPipelineHandle(
        const ClientCommunicator* comm,
        const std::string& ssg_group_file,
        uint16_t provider_id,
        const std::string& pipeline_name,
        bool check) const {

    std::vector<PipelineHandle> pipelines;

    ssg_group_id_t gid = SSG_GROUP_ID_INVALID;

    if(comm->rank() == 0) {
        // load SSG group file
        int num_addrs = SSG_ALL_MEMBERS;
        int ret = ssg_group_id_load(ssg_group_file.c_str(), &num_addrs, &gid);
        if(ret != SSG_SUCCESS)
            throw Exception(ErrorCode::SSG_ERROR,
                "Could not open SSG group file "s + ssg_group_file);
        auto mid = self->m_engine.get_margo_instance();
        ret = ssg_group_observe(mid, gid);
        if(ret != SSG_SUCCESS)
            throw Exception(ErrorCode::SSG_ERROR,
                "Could not observe the SSG group from file "s + ssg_group_file);
        // get string addresses
        int group_size = ssg_get_group_size(gid);
        std::vector<char> packed_addresses(group_size*256, 0);
        for(int i = 0 ; i < group_size ; i++) {
            auto addr = ssg_group_id_get_addr_str(gid, i);
            strcpy(packed_addresses.data() + i*256, addr);
            try {
                auto pipeline = makePipelineHandle(addr, provider_id, pipeline_name, check);
                pipelines.push_back(std::move(pipeline));
            } catch(...) {
                group_size = -1;
                comm->bcast(&group_size, sizeof(group_size), 0);
                free(addr);
                throw;
            }
            free(addr);
        }
        // communicate group size to everybody
        comm->bcast(&group_size, sizeof(group_size), 0);
        // communicate addresses to everybody
        comm->bcast(packed_addresses.data(),
                    packed_addresses.size(), 0);
    } else {
        // get group size from rank 0
        int group_size;
        comm->bcast(&group_size, sizeof(group_size), 0);
        if(group_size == -1) {
            throw Exception(ErrorCode::SSG_ERROR,
                "Master client could not resolve pipeline");
        }
        // get addresses from rank 0
        std::vector<char> packed_addresses(group_size*256);
        comm->bcast(packed_addresses.data(),
                    packed_addresses.size(), 0);
        // create pipelines
        for(int i = 0; i < group_size; i++) {
            char* addr = packed_addresses.data() + i*256;
            auto pipeline = makePipelineHandle(addr, provider_id, pipeline_name, false);
            pipelines.push_back(std::move(pipeline));
        }
    }

    auto impl = std::make_shared<DistributedPipelineHandleImpl>(
            comm, self, gid, std::move(pipelines));

    return DistributedPipelineHandle(std::move(impl));
}

}
