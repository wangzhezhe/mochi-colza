#include "colza/controller.hpp"
#include "colza/communicator.hpp"

namespace colza {
    
controller::~controller() {
    if(SSG_GROUP_ID_INVALID != m_ssg_group_id) {
        ssg_group_destroy(m_ssg_group_id);
        m_ssg_group_id = SSG_GROUP_ID_INVALID;
        auto cb = get_engine().pop_prefinalize_callback(this);
    }
    // TODO deregister RPCs, invalidate communicators
    auto cb = get_engine().pop_finalize_callback(this);
}

}
