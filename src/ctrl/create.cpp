#include "colza/controller.hpp"
#include "colza/communicator.hpp"

#include <string>
#include <ssg-mpi.h>

namespace colza {

std::string controller::default_group_name = "colza";

controller::controller(tl::engine* engine, ssg_group_id_t gid, uint16_t provider_id)
: tl::provider<controller>(*engine, provider_id)
, m_ssg_group_id(gid)
, m_root_comm(communicator::create(this)) {}

controller* controller::create(tl::engine* engine, MPI_Comm comm, uint16_t provider_id) {
    std::string group_name(default_group_name);
    group_name += std::to_string(provider_id);
    ssg_group_config_t config = SSG_GROUP_CONFIG_INITIALIZER;
    ssg_group_id_t gid = ssg_group_create_mpi(engine->get_margo_instance(),
            group_name.c_str(), comm, &config, nullptr, nullptr);
    return new controller(engine, gid, provider_id);
}

controller* controller::create(tl::engine* engine, const std::vector<std::string>& addresses, uint16_t provider_id) {
    std::string group_name(default_group_name);
    group_name += std::to_string(provider_id);
    ssg_group_config_t config = SSG_GROUP_CONFIG_INITIALIZER;
    std::vector<const char*> group_addr_strs(addresses.size());
    for(unsigned i=0; i < addresses.size(); i++)
        group_addr_strs[i] = addresses[i].c_str();
    ssg_group_id_t gid = ssg_group_create(engine->get_margo_instance(),
            group_name.c_str(), group_addr_strs.data(),
            group_addr_strs.size(), &config, nullptr, nullptr);
    return new controller(engine, gid, provider_id);
}

controller* controller::create(tl::engine* engine, const std::string& filename, uint16_t provider_id) {
    std::string group_name(default_group_name);
    group_name += std::to_string(provider_id);
    ssg_group_config_t config = SSG_GROUP_CONFIG_INITIALIZER;
    ssg_group_id_t gid = ssg_group_create_config(
            engine->get_margo_instance(),
            group_name.c_str(), filename.c_str(), &config, nullptr, nullptr);
    return new controller(engine, gid, provider_id);
}

}
