#include "colza/communicator.hpp"
#include "colza/request.hpp"

namespace colza {

int communicator::gather(const void *sendBuffer, void *recvBuffer, size_t size, int dest) {
    request req;
    int ret = igather(sendBuffer, recvBuffer, size, dest, req);
    if(ret != 0) return ret;
    return req.wait();
}

int communicator::igather(const void *sendBuffer, void *recvBuffer, size_t size, int dest, request& req) {
    
    return -1;
}

}
