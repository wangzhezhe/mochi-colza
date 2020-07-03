#include "colza/controller.hpp"
#include "colza/communicator.hpp"

namespace colza {
    
void controller::on_p2p_transfer(const tl::request& req,
                         const UUID& comm_id,
                         tl::bulk& bulk,
                         size_t size,
                         int32_t source,
                         int32_t tag) {
    auto it = m_communicators.find(comm_id);
    if(it == m_communicators.end())
        req.respond(static_cast<int>(-1));
    else {
        int ret = it->second->on_p2p_transfer(req.get_endpoint(), bulk, size, source, tag);
        req.respond(ret);
    }
}

}
