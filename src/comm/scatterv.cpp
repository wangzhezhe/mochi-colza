#include "colza/communicator.hpp"
#include "colza/request.hpp"

namespace colza {

int communicator::scatterv(const void *sendBuffer, void *recvBuffer, size_t* sendLengths, size_t* offsets, size_t recvLength, int src) {
    request req;
    int ret = iscatterv(sendBuffer, recvBuffer, sendLengths, offsets, recvLength, src, req);
    if(ret != 0) return ret;
    return req.wait();
}

int communicator::iscatterv(const void *sendBuffer, void *recvBuffer, size_t* sendLengths, size_t* offsets, size_t recvLength, int src, request& req) {
    
    return -1;
}

}
