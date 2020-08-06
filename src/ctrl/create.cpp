#include <string>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/types.hpp"

namespace colza {

controller::controller(tl::engine* engine, uint16_t provider_id,
                       const tl::pool& pool)
    : tl::provider<controller>(*engine, provider_id),
      m_pool(pool),
      m_p2p_transfer_rpc(define("colza_p2p_transfer", &controller::on_p2p_transfer, m_pool)),
      m_join_rpc(define("colza_join", &controller::on_join, m_pool))
{
  // pushing pre-finalize callback that destroys the SSG group
  get_engine().push_prefinalize_callback(this, [ctrl = this]() { });
  // pushing finalize callback that destroys the controller
  get_engine().push_finalize_callback(this, [ctrl = this]() { delete ctrl; });

  if (m_pool.is_null()) {
    // use the pool from the engine if there is not thread pool
    m_pool = engine->get_handler_pool();
  }
  init_ops_map();
}

controller* controller::create(tl::engine* engine, uint16_t provider_id,
                               const tl::pool& pool) {
  auto ctrl = new controller(engine, provider_id, pool);
  std::string self_addr_str = static_cast<std::string>(engine->self());
  ctrl->m_leader_addr = self_addr_str;
  ctrl->m_this_addr   = self_addr_str;
  ctrl->m_members     = { tl::provider_handle(engine->self(), provider_id) };
  // create the world communicator
  UUID masterid; // masterid = 0
  std::shared_ptr<communicator> world_comm = 
      std::shared_ptr<communicator>(
              new communicator(ctrl,1, 0, ctrl->m_members));
  ctrl->m_communicators[masterid] = world_comm;
  return ctrl;
}

controller* controller::create(tl::engine* engine,
                               const std::vector<std::string>& addresses,
                               uint16_t provider_id, const tl::pool& pool) {
  auto ctrl = new controller(engine, provider_id, pool);
  ctrl->m_leader_addr = addresses[0];
  ctrl->m_this_addr   = engine->self();
  if(std::find(addresses.begin(), addresses.end(), ctrl->m_this_addr) == addresses.end()) {
    throw std::runtime_error("Calling controller::create in a process without passing self address");
  }
  ctrl->m_members.reserve(addresses.size());
  int my_rank = 0;
  for (unsigned i = 0; i < addresses.size(); i++) {
    ctrl->m_members.emplace_back(engine->lookup(addresses[i]), provider_id);
    if(addresses[i] == ctrl->m_this_addr)
        my_rank = i;
  }
  // create the world communicator
  UUID masterid; // masterid = 0
  std::shared_ptr<communicator> world_comm = 
      std::shared_ptr<communicator>(
              new communicator(ctrl, addresses.size(), my_rank, ctrl->m_members));
  ctrl->m_communicators[masterid] = world_comm;
  return ctrl;
}

#ifdef COLZA_HAS_MPI
controller* controller::create(tl::engine* engine, MPI_Comm comm,
                               uint16_t provider_id, const tl::pool& pool) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    std::vector<std::string> addresses(size);
    std::string self_addr = engine->self();
    int addr_max_size = self_addr.size();
    int result;
    MPI_Allreduce(&addr_max_size, &result, 1, MPI_INT, MPI_MAX, comm);
    addr_max_size = result;
    addr_max_size += 1;
    self_addr.resize(addr_max_size, '\0');
    std::vector<char> all_addresses(size*addr_max_size);
    MPI_Allgather(self_addr.data(), addr_max_size, MPI_BYTE,
                  all_addresses.data(), addr_max_size, MPI_BYTE, comm);
    for(unsigned i=0; i < size; i++) {
        addresses[i] = all_addresses.data() + addr_max_size*i;
    }
    return create(engine, addresses, provider_id, pool);
}
#endif

controller* controller::join(tl::engine* engine, const std::string& leader_addr,
                             uint16_t provider_id, const tl::pool& pool) {
  // lookup a provider handle for the leader
  auto leader_endpoint = engine->lookup(leader_addr);
  return join(engine, leader_endpoint, provider_id, pool);
}

controller* controller::join(tl::engine* engine, const tl::endpoint& leader_endpoint,
                             uint16_t provider_id, const tl::pool& pool) {
    auto ctrl = new controller(engine, provider_id, pool);
    auto ph = tl::provider_handle(leader_endpoint, provider_id);
    bool result = ctrl->m_join_rpc.on(ph)();
    if(!result) {
        delete ctrl;
        throw std::runtime_error("Could not join leader controller");
    } else {
        ctrl->m_leader_addr = static_cast<std::string>(leader_endpoint);
        ctrl->m_this_addr = static_cast<std::string>(engine->self());
    }
    return ctrl;
}

}  // namespace colza
