#ifndef __COLZA_REQUEST_HPP
#define __COLZA_REQUEST_HPP

namespace colza {

class request {

    public:

    request() = default;
    ~request() = default;

    int wait();

};

}

#endif
