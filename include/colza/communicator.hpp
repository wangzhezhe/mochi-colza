#ifndef __COLZA_COMM_HPP
#define __COLZA_COMM_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <mpi.h>

namespace colza {

class controller;
class request;
class operation;

class communicator {

    friend class controller;

    public:

    communicator(const communicator&) = delete;
    communicator(communicator&&) = default;
    communicator& operator=(const communicator&) = delete;
    communicator& operator=(communicator&&) = default;
    ~communicator() = default;

    size_t size() const;
    size_t rank() const;

    int send(const void* data, size_t size, int dest, int tag);
    int isend(const void* data, size_t size, int dest, int tag, request& req);
    int recv(void* data, size_t size, int src, int tag);
    int irecv(void* data, size_t size, int src, int tag, request& req);
    int barrier();
    int ibarrier(request& req);
    int gather(const void *sendBuffer, void *recvBuffer, size_t size, int dest);
    int igather(const void *sendBuffer, void *recvBuffer, size_t size, int dest, request& req);
    int gatherv(const void *sendBuffer, void *recvBuffer, size_t sendLength, size_t* recvLengths, size_t* offsets, int dest);
    int igatherv(const void *sendBuffer, void *recvBuffer, size_t sendLength, size_t* recvLengths, size_t* offsets, int dest, request& req);
    int scatter(const void *sendBuffer, void *recvBuffer, size_t size, int src);
    int iscatter(const void *sendBuffer, void *recvBuffer, size_t size, int src, request& req);
    int scatterv(const void *sendBuffer, void *recvBuffer, size_t* sendLengths, size_t* offsets, size_t recvLength, int src);
    int iscatterv(const void *sendBuffer, void *recvBuffer, size_t* sendLengths, size_t* offsets, size_t recvLength, int src, request& req);
    int allgather(const void *sendBuffer, void *recvBuffer, size_t size);
    int iallgather(const void *sendBuffer, void *recvBuffer, size_t size, request& req);
    int allgatherv(const void *sendBuffer, void *recvBuffer, size_t sendLength, size_t *recvLengths, size_t *offsets); 
    int iallgatherv(const void *sendBuffer, void *recvBuffer, size_t sendLength, size_t *recvLengths, size_t *offsets, request& req);
    int reduce(const void *sendBuffer, void *recvBuffer, size_t size, operation& op, int dest);
    int ireduce(const void *sendBuffer, void *recvBuffer, size_t size, operation& op, int dest, request& req);
    int allreduce(const void *sendBuffer, void *recvBuffer, size_t size, operation& op);
    int iallreduce(const void *sendBuffer, void *recvBuffer, size_t size, operation& op, request& req);

    private:

    static communicator* create(controller* owner);

    communicator(controller* owner)
    : m_controller(owner) {}

    controller* m_controller;

};

}

#endif
