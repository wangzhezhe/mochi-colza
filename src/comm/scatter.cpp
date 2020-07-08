#include "colza/communicator.hpp"
#include "colza/request.hpp"

namespace colza {

int communicator::scatter(const void *sendBuffer, void *recvBuffer, size_t size, int src) {
    request req;
    int ret = iscatter(sendBuffer, recvBuffer, size, src, req);
    if(ret != 0) return ret;
    return wait(req);
}

int communicator::iscatter(const void *sendBuffer, void *recvBuffer, size_t size, int src, request& req) {
    
    return -1;
}

}
