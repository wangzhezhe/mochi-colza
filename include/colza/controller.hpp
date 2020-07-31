#ifndef __COLZA_CONTROLLER_HPP
#define __COLZA_CONTROLLER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <thallium.hpp>
#include <ssg.h>
#include <colza/config.hpp>
#include <colza/uuid.hpp>
#ifdef COLZA_HAS_MPI
#include <ssg-mpi.h>
#endif
#ifdef COLZA_HAS_PMIX
#include <ssg-pmix.h>
#endif

namespace colza {

namespace tl = thallium;

class communicator;

/**
 * @brief The colza::controller class is a Thallium provider that
 * is responsible for defining the required RPCs for communications,
 * and to queue received requests from colza::communicator instances.
 */
class controller : public tl::provider<controller> {

    friend class communicator;

    public:

    static std::string default_ssg_group_name;        //!< Default SSG group name
    static ssg_group_config default_ssg_group_config; //!< Default SSG configuration

    /**
     * @brief Creates a controller belonging to a group with only itself as a member.
     * If the user does not delete the created controller, it will be automatically
     * deleted upon finalizing the engine.
     *
     * @param engine Thallium engine.
     * @param provider_id Provider id.
     * @param pool Pool to use to execute RPCs.
     *
     * @return a pointer to a controller.
     */
    static controller* create(tl::engine* engine,
                              uint16_t provider_id=0,
                              const tl::pool& pool = tl::pool());

    /**
     * @brief Creates a controller and makes it join a group defined in the provided file.
     * If the user does not delete the created controller, it will be automatically
     * deleted upon finalizing the engine.
     *
     * @param engine Thallium engine.
     * @param filename SSG group file.
     * @param provider_id Provider id.
     * @param pool Pool to use to execute RPCs.
     *
     * @return a pointer to a controller.
     */
    static controller* create(tl::engine* engine,
                              const std::string& filename,
                              uint16_t provider_id=0,
                              const tl::pool& pool = tl::pool());

    /**
     * @brief Creates a controller and makes it join a group formed of the provided array
     * of addresses. All the processes in this array must be creating a controller in the
     * same way. If the user does not delete the created controller, it will be automatically
     * deleted upon finalizing the engine.
     *
     * @param engine Thallium engine.
     * @param addresses array of addresses of all the members, including the caller.
     * @param provider_id Provider id.
     * @param pool Pool to use to execute RPCs.
     *
     * @return a pointer to a controller.
     */
    static controller* create(tl::engine* engine,
                              const std::vector<std::string>& addresses,
                              uint16_t provider_id=0,
                              const tl::pool& pool = tl::pool());

#ifdef COLZA_HAS_MPI
    /**
     * @brief Creates a controller and makes it join a group formed of all the processes
     * in the provided MPI communicator. All the other processes must call this function
     * as well. If the user does not delete the created controller, it will be automatically
     * deleted upon finalizing the engine.
     *
     * This function requires SSG to have been compiled with MPI support.
     *
     * @param engine Thallium engine.
     * @param comm MPI communicator to bootstrap from.
     * @param provider_id Provider id.
     * @param pool Pool to use to execute RPCs.
     *
     * @return a pointer to a controller.
     */
    static controller* create(tl::engine* engine,
                              MPI_Comm comm,
                              uint16_t provider_id=0,
                              const tl::pool& pool = tl::pool());
#endif

#ifdef COLZA_HAS_PMIX
    /**
     * @brief Creates a controller by bootstrapping from PMIx. All other processes
     * involved in the group should call this function. If the user does not delete
     * the created controller, it will be automatically deleted upon finalizing
     * the engine.
     *
     * This function requires SSG to have been compiled with PMIx support.
     *
     * @param engine Thallium engine.
     * @param proc PMIx proc object.
     * @param provider_id Provider id.
     * @param pool Pool to use to execute RPCs.
     *
     * @return a pointer to a controller.
     */
    static controller* create(tl::engine* engine,
                              pmix_proc_t proc,
                              uint16_t provider_id=0,
                              const tl::pool& pool = tl::pool());
#endif
   
    /**
     * @brief Creates a controller by joining an existing group.
     * The descriptor should have been generated with the descriptor() function.
     *
     * @param engine Thallium engine.
     * @param descriptor Group descriptor.
     * @param provider_id Provider id.
     * @param pool Pool to use to execute RPCs.
     *
     * @return a pointer to a controller. 
     */
    static controller* join(tl::engine* engine,
                            const std::string& descriptor,
                            uint16_t provider_id = 0,
                            const tl::pool& pool = tl::pool());

    /**
     * @brief Destructor.
     */
    ~controller();

    /**
     * @brief Copy constructor is deleted.
     */
    controller(const controller&) = delete;

    /**
     * @brief Move constructor is deleted.
     */
    controller(controller&&) = delete;

    /**
     * @brief Copy-assignment is deleted.
     */
    controller& operator=(const controller&) = delete;

    /**
     * @brief Move-assignment is deleted.
     */
    controller& operator=(controller&&) = default;

    /**
     * @brief Gets a communicator gathering all the members of the SSG group
     * managed by this controller. This communicator can be seen as the
     * equivalent of MPI_COMM_WORLD.
     *
     * @return a pointer to a communicator.
     */
    std::shared_ptr<communicator> build_world_communicator();

    /**
     * @brief Creates a descriptor that can be used by other processes
     * to call controller::join. Note that descriptors change as the group members
     * join and leave, so make sure to use controller::join on a fairly recent
     * version of the group descriptor.
     *
     * @return descriptor.
     */
    std::string descriptor() const;

    private:

    controller(tl::engine* engine, uint16_t provider_id=0, const tl::pool& pool=tl::pool());

    void init(ssg_group_id_t gid);
    
    /**
     * SSG membership update callbacks.
     */
    void on_member_joined(ssg_member_id_t member_id);
    void on_member_died(ssg_member_id_t member_id);
    void on_member_left(ssg_member_id_t member_id);
    static void group_membership_update(void* uargs, ssg_member_id_t member_id, ssg_member_update_type_t update_type);

    /**
     * @brief RPC handlers executed when a remote process calls send on a communicator.
     *
     * @param req Thallium request.
     * @param comm_id Communicator id.
     * @param bulk Bulk handle for the remote data.
     * @param size Size of the remote data.
     * @param tag Tag.
     */
    void on_p2p_transfer(const tl::request& req, const UUID& comm_id, tl::bulk& bulk, size_t size, int32_t source, int32_t tag);

    tl::provider_handle member_id_to_provider_handle(ssg_member_id_t member_id);

    ssg_group_id_t       m_ssg_group_id = SSG_GROUP_ID_INVALID;
    tl::pool             m_pool;
    tl::mutex            m_comm_mutex;
    std::unordered_map<UUID, std::shared_ptr<communicator>, UUID_hash_fn> m_communicators;
    tl::remote_procedure m_p2p_transfer_rpc;
};

}

#endif
