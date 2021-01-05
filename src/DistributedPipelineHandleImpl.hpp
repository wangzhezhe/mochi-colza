/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_DISTRIBUTED_PIPELINE_HANDLE_IMPL_H
#define __COLZA_DISTRIBUTED_PIPELINE_HANDLE_IMPL_H

#include "colza/ClientCommunicator.hpp"
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
    std::vector<std::shared_ptr<PipelineHandleImpl>> m_pipelines;

    DistributedPipelineHandleImpl(
        const ClientCommunicator* comm,
        const std::shared_ptr<ClientImpl>& client)
    : m_comm(comm)
    , m_client(client) {}

    DistributedPipelineHandleImpl(
        const ClientCommunicator* comm,
        const std::shared_ptr<ClientImpl>& client,
        std::vector<std::shared_ptr<PipelineHandleImpl>>&& pipelines)
    : m_comm(comm)
    , m_client(client)
    , m_pipelines(std::move(pipelines)) {}
};

}

#endif
