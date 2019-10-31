#include "colza/controller.hpp"
#include "colza/communicator.hpp"

namespace colza {
    
controller::~controller() {
    ssg_group_leave(m_ssg_group_id);
    delete m_root_comm;
}

}
