#include "colza/controller.hpp"
#include "colza/communicator.hpp"
#include "colza/uuid.hpp"

namespace colza {
    
std::shared_ptr<communicator> controller::build_world_communicator() {
    UUID masterid;
    UUID nullid;
    int size = ssg_get_group_size(m_ssg_group_id);
    std::vector<ssg_member_id_t> members(size);
    for(unsigned i=0; i < size; i++) {
        members[i] = ssg_get_group_member_id_from_rank(m_ssg_group_id, i);
        // TODO check return value of this call
    }
    int rank = ssg_get_group_self_rank(m_ssg_group_id);
    auto temp_comm = std::shared_ptr<communicator>(
            new communicator(const_cast<controller*>(this),
                             size, rank, members));
    temp_comm->m_comm_id = nullid;
    {
    std::lock_guard<tl::mutex> lck (m_comm_mutex);
    m_communicators[nullid] = temp_comm;
    }
    if(rank == 0) 
        masterid = UUID::generate();
    temp_comm->bcast(&masterid, sizeof(masterid), 0);
    auto c = std::shared_ptr<communicator>(
            new communicator(const_cast<controller*>(this),
                             size, rank, std::move(members)));
    c->m_comm_id = masterid;
    {
    std::lock_guard<tl::mutex> lck (m_comm_mutex);
    m_communicators[masterid] = c;
    }
    temp_comm->barrier();
    {
    std::lock_guard<tl::mutex> lck (m_comm_mutex);
    m_communicators.erase(nullid);
    }

    return c;
}

}
