#include "colza/controller.hpp"
#include "colza/communicator.hpp"

namespace colza {
    
tl::provider_handle controller::member_id_to_provider_handle(ssg_member_id_t member_id) {
    hg_addr_t addr = ssg_get_group_member_addr(m_ssg_group_id, member_id);
    // TODO check if addr is HG_ADDR_NULL
    return tl::provider_handle(get_engine(), addr, get_provider_id(), false);
}

}
