#include "colza/communicator.hpp"
#include "colza/request.hpp"

namespace colza {

int communicator::allgather(const void *sendBuffer, void *recvBuffer, size_t size) {
    request req;
    int ret = iallgather(sendBuffer, recvBuffer, size, req);
    if(ret != 0) return ret;
    return req.wait();
}

int communicator::iallgather(const void *sendBuffer, void *recvBuffer, size_t size, request& req) {

    return -1;
}

}
