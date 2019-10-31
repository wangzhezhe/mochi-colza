#include "colza/communicator.hpp"
#include "colza/request.hpp"

namespace colza {

int communicator::barrier() {
    request req;
    int ret = ibarrier(req);
    if(ret != 0) return ret;
    return req.wait();
}

int communicator::ibarrier(request& req) {
    
    return -1;
}

}
