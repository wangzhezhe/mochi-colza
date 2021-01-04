/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_DISTRIBUTED_PIPELINE_HANDLE_IMPL_H
#define __COLZA_DISTRIBUTED_PIPELINE_HANDLE_IMPL_H

#include <mpi.h>
#include <vector>
#include <memory>

namespace colza {

class DistributedPipelineHandleImpl {

    public:

    MPI_Comm                    m_comm = MPI_COMM_NULL;
    std::shared_ptr<ClientImpl> m_client;
    HashFunction                m_hash = [](const std::string&, uint64_t, uint64_t block_id){
        return block_id;
    };
    std::vector<std::shared_ptr<PipelineHandleImpl>> m_pipelines;

    DistributedPipelineHandleImpl() = default;

    DistributedPipelineHandleImpl(
        MPI_Comm comm,
        const std::shared_ptr<ClientImpl>& client,
        std::vector<std::shared_ptr<PipelineHandleImpl>>&& pipelines)
    : m_comm(comm)
    , m_client(client)
    , m_pipelines(std::move(pipelines)) {}
};

}

#endif
