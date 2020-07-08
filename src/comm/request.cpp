#include <thallium.hpp>
#include "colza/request.hpp"

namespace colza {

namespace tl = thallium;

int request::wait(){
    this->m_eventual->wait();
    return 0;
}

} // namespace colza