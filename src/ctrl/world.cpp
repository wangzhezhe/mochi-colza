#include "colza/controller.hpp"
#include "colza/communicator.hpp"
#include "colza/uuid.hpp"

namespace colza {
    
std::shared_ptr<communicator> controller::build_world_communicator() {
    UUID masterid;

    std::shared_ptr<communicator> world_comm;

    if(m_leader_id == m_this_id) { // master
        // create a snapshot of the current state of the group
        int size = ssg_get_group_size(m_ssg_group_id);
        std::vector<ssg_member_id_t> members(size);
        for(unsigned i=0; i < size; i++) {
            members[i] = ssg_get_group_member_id_from_rank(m_ssg_group_id, i);
        // TODO check return value of this call
        }
        // create or modify the world communicator
        {   std::lock_guard<tl::mutex> lck (m_comm_mutex);
            if(m_communicators.count(masterid)) {
                world_comm = m_communicators[masterid];
                world_comm->m_size = size;
                world_comm->m_rank = 0;
                world_comm->m_members = members;
            } else {
                world_comm = std::shared_ptr<communicator>(
                        new communicator(const_cast<controller*>(this),
                                         size, 0, members));
                m_communicators[masterid] = world_comm;
            }
        }
        // send this snapshot to all the members
        for(unsigned i=1; i < size; i++) {
            world_comm->send(&size, sizeof(size), i, 1234);
            world_comm->send(members.data(), members.size()*sizeof(ssg_member_id_t), i, 1234);
        }
        // wait for confirmation from all the members
        char a;
        for(unsigned i=1; i < size; i++) {
            world_comm->recv(&a, 1, i, 1234);
        }
    } else { // non-leader members
        // create or modify the world communicator
        {   std::lock_guard<tl::mutex> lck (m_comm_mutex);
            if(m_communicators.count(masterid)) {
                world_comm = m_communicators[masterid];
            } else {
                // this is a fake world with only this process and the leader
                // we can still correctly receive from rank 0 from this leader
                std::vector<ssg_member_id_t> members = { m_leader_id, m_this_id };
                world_comm = std::shared_ptr<communicator>(
                        new communicator(const_cast<controller*>(this),
                                         2, 1, members));
                m_communicators[masterid] = world_comm;
            }
        }
        // receive the size and members from leader
        int size;
        world_comm->recv(&size, sizeof(size), 0, 1234);
        std::vector<ssg_member_id_t> members(size);
        world_comm->recv(members.data(), sizeof(ssg_member_id_t)*members.size(), 0, 1234);
        world_comm->m_size = size;
        world_comm->m_members = members;
        // find out the rank of this process
        for(int i = 0; i < size; i++) {
            if(members[i] == m_this_id) {
                world_comm->m_rank = i;
                break;
            }
        }
        // make sure that we know all the members
        for(int i = 0; i < size; i++) {
            while(ssg_get_group_member_rank(m_ssg_group_id, members[i]) == -1) {
                tl::thread::yield();
            }
        }
        // send confirmation to leader
        char a = 0;
        world_comm->send(&a, 1, 0, 1234);
    }

    return world_comm;
}

}
