/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_CLIENT_COMMUNICATOR_HPP
#define __COLZA_CLIENT_COMMUNICATOR_HPP

namespace colza {

/**
 * @brief The ClientCommunicator class is an interface for client
 * to invoke collective communication primitives. Notably, it is
 * a base class for MPIClientCommunicator, wrapping an MPI_Comm to
 * make the rest of Colza MPI-agnostic.
 */
class ClientCommunicator {

    public:

    virtual ~ClientCommunicator() = default;

    virtual int size() const = 0;

    virtual int rank() const = 0;

    virtual void barrier() const = 0;

};

}

#endif
