#include "colza/communicator.hpp"
#include "colza/request.hpp"

namespace colza {

int communicator::allgatherv(const void *sendBuffer, void *recvBuffer, size_t sendLength, size_t *recvLengths, size_t *offsets) {
    request req;
    int ret = iallgatherv(sendBuffer, recvBuffer, sendLength, recvLengths, offsets, req);
    if(ret != 0) return ret;
    return req.wait();
}

int communicator::iallgatherv(const void *sendBuffer, void *recvBuffer, size_t sendLength, size_t *recvLengths, size_t *offsets, request& req) {
    
    return -1;
}

}
