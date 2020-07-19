#include <string>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/types.hpp"

namespace colza {

std::string controller::default_ssg_group_name = "colza";
ssg_group_config controller::default_ssg_group_config =
    SSG_GROUP_CONFIG_INITIALIZER;

controller::controller(tl::engine* engine, uint16_t provider_id,
                       const tl::pool& pool)
    : tl::provider<controller>(*engine, provider_id),
      m_ssg_group_id(SSG_GROUP_ID_INVALID),
      m_pool(pool),
      m_p2p_transfer_rpc(
          define("colza_p2p_transfer", &controller::on_p2p_transfer, m_pool)) {
  // pushing pre-finalize callback that destroys the SSG group
  get_engine().push_prefinalize_callback(this, [ctrl = this]() {
    ssg_group_destroy(ctrl->m_ssg_group_id);
    ctrl->m_ssg_group_id = SSG_GROUP_ID_INVALID;
  });
  // pushing finalize callback that destroys the controller
  get_engine().push_finalize_callback(this, [ctrl = this]() { delete ctrl; });

  if (m_pool.is_null()) {
    // use the pool from the engine if there is not thread pool
    m_pool = engine->get_handler_pool();
  }
}

void controller::init(ssg_group_id_t gid) { m_ssg_group_id = gid; init_ops_map(); }

controller* controller::create(tl::engine* engine, uint16_t provider_id,
                               const tl::pool& pool) {
  auto ctrl = new controller(engine, provider_id, pool);
  std::string group_name(default_ssg_group_name);
  group_name += std::to_string(provider_id);
  ssg_group_config_t config = default_ssg_group_config;
  std::string self_addr_str = static_cast<std::string>(engine->self());
  std::vector<const char*> group_addr_strs = {self_addr_str.c_str()};
  ssg_group_id_t gid = ssg_group_create(
      engine->get_margo_instance(), group_name.c_str(), group_addr_strs.data(),
      group_addr_strs.size(), &config, controller::group_membership_update,
      static_cast<void*>(ctrl));
  ctrl->init(gid);
  return ctrl;
}

controller* controller::create(tl::engine* engine,
                               const std::vector<std::string>& addresses,
                               uint16_t provider_id, const tl::pool& pool) {
  auto ctrl = new controller(engine, provider_id, pool);
  std::string group_name(default_ssg_group_name);
  group_name += std::to_string(provider_id);
  ssg_group_config_t config = default_ssg_group_config;
  std::vector<const char*> group_addr_strs(addresses.size());
  for (unsigned i = 0; i < addresses.size(); i++)
    group_addr_strs[i] = addresses[i].c_str();
  ssg_group_id_t gid = ssg_group_create(
      engine->get_margo_instance(), group_name.c_str(), group_addr_strs.data(),
      group_addr_strs.size(), &config, controller::group_membership_update,
      static_cast<void*>(ctrl));
  ctrl->init(gid);
  return ctrl;
}

controller* controller::create(tl::engine* engine, const std::string& filename,
                               uint16_t provider_id, const tl::pool& pool) {
  auto ctrl = new controller(engine, provider_id, pool);
  std::string group_name(default_ssg_group_name);
  group_name += std::to_string(provider_id);
  ssg_group_config_t config = default_ssg_group_config;
  ssg_group_id_t gid = ssg_group_create_config(
      engine->get_margo_instance(), group_name.c_str(), filename.c_str(),
      &config, controller::group_membership_update, static_cast<void*>(ctrl));
  ctrl->init(gid);
  return ctrl;
}

#ifdef COLZA_HAS_MPI
controller* controller::create(tl::engine* engine, MPI_Comm comm,
                               uint16_t provider_id, const tl::pool& pool) {
  auto ctrl = new controller(engine, provider_id, pool);
  std::string group_name(default_ssg_group_name);
  group_name += std::to_string(provider_id);
  ssg_group_config_t config = default_ssg_group_config;
  ssg_group_id_t gid = ssg_group_create_mpi(
      engine->get_margo_instance(), group_name.c_str(), comm, &config,
      controller::group_membership_update, static_cast<void*>(ctrl));
  ctrl->init(gid);
  return ctrl;
}
#endif

#ifdef COLZA_HAS_PMIX
controller* controller::create(tl::engine* engine, pmix_proc_t proc,
                               uint16_t provider_id, const tl::pool& pool) {
  auto ctrl = new controller(engine, provider_id, pool);
  std::string group_name(default_ssg_group_name);
  group_name += std::to_string(provider_id);
  ssg_group_config_t config = default_ssg_group_config;
  ssg_group_id_t gid = ssg_group_create_pmix(
      engine->get_margo_instance(), group_name.c_str(), proc, &config,
      controller::group_membership_update, static_cast<void*>(ctrl));
  ctrl->init(gid);
  return new ctrl;
}
#endif

controller* controller::join(tl::engine* engine, const std::string& descriptor,
                             uint16_t provider_id, const tl::pool& pool) {
  auto ctrl = new controller(engine, provider_id, pool);
  ssg_group_id_t gid;
  int num_addrs = 128;
  ssg_group_id_deserialize(
          descriptor.c_str(),
          descriptor.size(),
          &num_addrs, &gid);
  int ret = ssg_group_join_target(
          engine->get_margo_instance(),
          gid, nullptr, controller::group_membership_update,
          static_cast<void*>(ctrl));
  // TODO check return value
  ctrl->init(gid);
  return ctrl;
}

}  // namespace colza
