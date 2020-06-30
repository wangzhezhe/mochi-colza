#include "colza/communicator.hpp"
#include "colza/request.hpp"

namespace colza {
    
int communicator::gatherv(const void *sendBuffer, void *recvBuffer, size_t sendLength, size_t* recvLengths, size_t* offsets, int dest) {
    request req;
    int ret = igatherv(sendBuffer, recvBuffer, sendLength, recvLengths, offsets, dest, req);
    if(ret != 0) return ret;
    return req.wait();
}

int communicator::igatherv(const void *sendBuffer, void *recvBuffer, size_t sendLength, size_t* recvLengths, size_t* offsets, int dest, request& req) {

    return -1;
}

}
