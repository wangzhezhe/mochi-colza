#include "colza/controller.hpp"
#include "colza/communicator.hpp"

namespace colza {
    
std::shared_ptr<communicator> controller::build_world_communicator() {
    auto it = m_communicators.find(0);
    if(it != m_communicators.end()) {
        return it->second;
    }
    int size = ssg_get_group_size(m_ssg_group_id);
    std::vector<ssg_member_id_t> members(size);
    for(unsigned i=0; i < size; i++) {
        members[i] = ssg_get_group_member_id_from_rank(m_ssg_group_id, i);
        // TODO check return value of this call
    }
    int rank = ssg_get_group_self_rank(m_ssg_group_id);
    auto c = std::shared_ptr<communicator>(new communicator(const_cast<controller*>(this), size, rank, std::move(members)));
    m_communicators[0] = c;
    return c;
}

}
