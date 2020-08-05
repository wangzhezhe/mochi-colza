#include "colza/controller.hpp"
#include "colza/communicator.hpp"
#include "colza/uuid.hpp"

namespace colza {
    
std::shared_ptr<communicator> controller::build_world_communicator() {
    UUID masterid; // masterid = 0
    std::shared_ptr<communicator> world_comm;
    if(m_leader_addr == m_this_addr) { // master
        // create a copy of the current list of members
        auto members = m_members;
        int size = members.size();
        // create or modify the world communicator
        {   std::lock_guard<tl::mutex> lck (m_comm_mutex);
            if(m_communicators.count(masterid)) { // communicator already exists
                world_comm = m_communicators[masterid];
                world_comm->m_size = size;
                world_comm->m_members = members;
            } else { // communicator does not exist, create it
                world_comm = std::shared_ptr<communicator>(
                        new communicator(const_cast<controller*>(this),
                                         size, 0, members));
                m_communicators[masterid] = world_comm;
            }
        }
        // serialize the addresses
        std::vector<char> address_pack;
        for(const auto& member : members) {
            std::string member_addr = static_cast<std::string>(member);
            size_t start = address_pack.size();
            address_pack.resize(start + member_addr.size()+1, '\0');
            std::memcpy(address_pack.data() + start, member_addr.data(),  member_addr.size());
        }
        // send this snapshot to all the members
        int address_pack_size = address_pack.size();
        for(unsigned i=1; i < size; i++) {
            world_comm->send(&address_pack_size, sizeof(size), i, 1234);
            world_comm->send(address_pack.data(), address_pack.size(), i, 1234);
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
                std::vector<tl::provider_handle> members = {
                    tl::provider_handle(get_engine().lookup(m_leader_addr), get_provider_id()),
                    tl::provider_handle(get_engine().lookup(m_this_addr), get_provider_id())
                };
                world_comm = std::shared_ptr<communicator>(
                        new communicator(const_cast<controller*>(this),
                                         2, 1, members));
                m_communicators[masterid] = world_comm;
            }
        }
        // receive the size and members from leader
        int address_pack_size;
        world_comm->recv(&address_pack_size, sizeof(address_pack_size), 0, 1234);
        std::vector<char> address_pack(address_pack_size);
        world_comm->recv(address_pack.data(), address_pack_size, 0, 1234);
        int start = 0;
        world_comm->m_members.resize(0);
        int r = 0;
        while(start < address_pack_size) {
            const char* addr_ptr = &address_pack[start];
            std::string addr = std::string(addr_ptr);
            auto member = tl::provider_handle(get_engine().lookup(addr), get_provider_id());
            start += strlen(addr_ptr) + 1;
            world_comm->m_members.push_back(std::move(member));
            if(addr == m_this_addr) {
                world_comm->m_rank = r;
            }
            r += 1;
        }
        world_comm->m_size = world_comm->m_members.size();
        // send confirmation to leader
        char a = 0;
        world_comm->send(&a, 1, 0, 1234);
    }
    return world_comm;
}

}
