#include "colza/communicator.hpp"
#include "colza/request.hpp"

namespace colza {

int communicator::send(const void* data, size_t size, int dest, int tag) {
    request req;
    int ret = isend(data, size, dest, tag, req);
    if(ret != 0) return ret;
    return req.wait();
}

int communicator::isend(const void* data, size_t size, int dest, int tag, request& req) {

    return -1;
}

int communicator::recv(void* data, size_t size, int src, int tag) {
    request req;
    int ret = irecv(data, size, src, tag, req);
    if(ret != 0) return ret;
    return req.wait();
}

int communicator::irecv(void* data, size_t size, int src, int tag, request& req) {
    
    return -1;
}

}
