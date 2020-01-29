#include "colza/controller.hpp"
#include "colza/communicator.hpp"

namespace colza {
    
std::shared_ptr<communicator> controller::build_world_communicator() const {
    int size = ssg_get_group_size(m_ssg_group_id);
    std::vector<ssg_member_id_t> members(size);
    ssg_get_group_member_ids_from_range(m_ssg_group_id, 0, size-1, members.data());
    int rank = ssg_get_group_self_rank(m_ssg_group_id);
    return std::shared_ptr<communicator>(new communicator(const_cast<controller*>(this), size, rank, std::move(members)));
}

}
