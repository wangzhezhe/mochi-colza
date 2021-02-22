/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_DISTRIBUTED_PIPELINE_HANDLE_IMPL_H
#define __COLZA_DISTRIBUTED_PIPELINE_HANDLE_IMPL_H

#include "colza/ClientCommunicator.hpp"
#include "colza/PipelineHandle.hpp"
#include "SSGUtil.hpp"
#include <ssg.h>
#include <spdlog/spdlog.h>
#include <vector>
#include <memory>

namespace colza {

class DistributedPipelineHandleImpl {

    public:

    const ClientCommunicator*   m_comm = nullptr;
    std::shared_ptr<ClientImpl> m_client;
    HashFunction                m_hash = [](const std::string&, uint64_t, uint64_t block_id){
        return block_id;
    };
    std::vector<PipelineHandle> m_pipelines;
    // SSG info are only valid on rank 0
    ssg_group_id_t              m_gid;
    uint64_t                    m_group_hash = 0;

    DistributedPipelineHandleImpl(
        const ClientCommunicator* comm,
        const std::shared_ptr<ClientImpl>& client,
        ssg_group_id_t gid)
    : m_comm(comm)
    , m_client(client)
    , m_gid(gid) {
        if(gid != SSG_GROUP_ID_INVALID)
            m_group_hash = ComputeGroupHash(gid);
    }

    DistributedPipelineHandleImpl(
        const ClientCommunicator* comm,
        const std::shared_ptr<ClientImpl>& client,
        ssg_group_id_t gid,
        std::vector<PipelineHandle>&& pipelines)
    : m_comm(comm)
    , m_client(client)
    , m_pipelines(std::move(pipelines))
    , m_gid(gid) {
        if(gid != SSG_GROUP_ID_INVALID)
            m_group_hash = ComputeGroupHash(gid);
    }

    ~DistributedPipelineHandleImpl() {
        if(m_gid != SSG_GROUP_ID_INVALID) {
            ssg_group_unobserve(m_gid);
            //ssg_group_destroy(m_gid);
        }
    }
};

}

#endif
