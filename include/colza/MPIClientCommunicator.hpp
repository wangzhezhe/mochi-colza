/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_MPI_CLIENT_COMMUNICATOR_HPP
#define __COLZA_MPI_CLIENT_COMMUNICATOR_HPP

#include <colza/ClientCommunicator.hpp>
#include <mpi.h>

namespace colza {

class MPIClientCommunicator : public ClientCommunicator {

    MPI_Comm m_comm;

    public:

    MPIClientCommunicator(MPI_Comm comm)
    : m_comm(comm) {}

    ~MPIClientCommunicator() = default;

    int size() const override {
        int s;
        MPI_Comm_size(m_comm, &s);
        return s;
    }

    int rank() const override {
        int r;
        MPI_Comm_rank(m_comm, &r);
        return r;
    }

    void barrier() const override {
        MPI_Barrier(m_comm);
    }

    void bcast(void* buffer, int bytes, int root) const override {
        MPI_Bcast(buffer, bytes, MPI_BYTE, root, m_comm);
    }

};

}

#endif
