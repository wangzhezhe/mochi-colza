#include "colza/communicator.hpp"
#include "colza/request.hpp"

namespace colza {

int communicator::reduce(const void *sendBuffer, void *recvBuffer, size_t size, operation& op, int dest) {
    request req;
    int ret = ireduce(sendBuffer, recvBuffer, size, op, dest, req);
    if(ret != 0) return ret;
    return req.wait();
}

int communicator::ireduce(const void *sendBuffer, void *recvBuffer, size_t size, operation& op, int dest, request& req) {
    
    return -1;
}

}
