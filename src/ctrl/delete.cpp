#include "colza/controller.hpp"
#include "colza/communicator.hpp"

namespace colza {
    
controller::~controller() {
    // TODO deregister RPCs, invalidate communicators
    auto cb = get_engine().pop_finalize_callback(this);
    for(auto& p : m_communicators) {
        auto& comm = p.second;
        if(comm) comm->m_members.clear();
    }
}

}
