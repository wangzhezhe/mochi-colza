#ifndef __COLZA_CONTROLLER_HPP
#define __COLZA_CONTROLLER_HPP

#include <string>
#include <vector>
#include <mpi.h>
#include <thallium.hpp>
#include <ssg.h>

namespace colza {

namespace tl = thallium;

class communicator;

class controller : public tl::provider<controller> {

    public:

    static std::string default_group_name;

    static controller* create(tl::engine* engine, MPI_Comm comm, uint16_t provider_id=0);
    static controller* create(tl::engine* engine, const std::string& filename, uint16_t provider_id=0);
    static controller* create(tl::engine* engine, const std::vector<std::string>& addresses, uint16_t provider_id=0);

    ~controller();

    controller(const controller&) = delete;

    controller(controller&&) = default;

    controller& operator=(const controller&) = delete;

    controller& operator=(controller&&) = default;

    communicator* get_root_communicator() {
        return m_root_comm;
    }

    private:

    controller(tl::engine* engine, ssg_group_id_t gid, uint16_t provider_id=0);

    ssg_group_id_t m_ssg_group_id = SSG_GROUP_ID_INVALID;
    communicator* m_root_comm = nullptr;
};

}

#endif
