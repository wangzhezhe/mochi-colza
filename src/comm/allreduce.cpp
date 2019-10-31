#include "colza/communicator.hpp"
#include "colza/request.hpp"

namespace colza {

int communicator::allreduce(const void *sendBuffer, void *recvBuffer, size_t size, operation& op) {
    request req;
    int ret = iallreduce(sendBuffer, recvBuffer, size, op, req);
    if(ret != 0) return ret;
    return req.wait();
}

int communicator::iallreduce(const void *sendBuffer, void *recvBuffer, size_t size, operation& op, request& req) {
    
    return -1;
}

}
