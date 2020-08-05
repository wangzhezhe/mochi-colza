#ifndef __COLZA_CONTROLLER_HPP
#define __COLZA_CONTROLLER_HPP

#include <mpi.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <thallium.hpp>
#include <colza/config.hpp>
#include <colza/uuid.hpp>

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

    /**
     * @brief Creates a controller belonging to a group with only itself as a member.
     * If the user does not delete the created controller, it will be automatically
     * deleted upon finalizing the engine.
     *
     * This function should be used if the leader starts up alone.
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
     * @brief Creates a controller and makes it join a group formed of the provided array
     * of addresses. All the processes in this array must be creating a controller in the
     * same way. If the user does not delete the created controller, it will be automatically
     * deleted upon finalizing the engine.
     *
     * This function should be used if the processes have a way of communicating their
     * addresses to each other prior to initializing Colza. The first address in the vector
     * will be considered the leader.
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
     * This function is a helper function using MPI communications to exchange addresses
     * before calling the above create function with a vector of addresses.
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

    /**
     * @brief Creates a controller by joining an existing group.
     * The leader's address should be provided.
     *
     * @param engine Thallium engine.
     * @param leader_address Address of the leader.
     * @param provider_id Provider id.
     * @param pool Pool to use to execute RPCs.
     *
     * @return a pointer to a controller. 
     */
    static controller* join(tl::engine* engine,
                            const std::string& leader_address,
                            uint16_t provider_id = 0,
                            const tl::pool& pool = tl::pool());

    /**
     * @brief Creates a controller by joining an existing group.
     * The leader's endpoint should be provided.
     *
     * @param engine Thallium engine.
     * @param leader Endpoint of the leader/
     * @param provider_id Provider id.
     * @param pool Pool to use to execute RPCs.
     *
     * @return a pointer to a controller. 
     */
    static controller* join(tl::engine* engine,
                            const tl::endpoint& leader,
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
    const std::string& leader() const {
        return m_leader_addr;
    }

    private:

    controller(tl::engine* engine, uint16_t provider_id=0, const tl::pool& pool=tl::pool());

    /**
     * @brief RPC handler executed when a remote process calls send on a communicator.
     *
     * @param req Thallium request.
     * @param comm_id Communicator id.
     * @param bulk Bulk handle for the remote data.
     * @param size Size of the remote data.
     * @param tag Tag.
     */
    void on_p2p_transfer(const tl::request& req, const UUID& comm_id, tl::bulk& bulk, size_t size, int32_t source, int32_t tag);

    /**
     * @brief RPC handler executed when a remote process joins the group.
     *
     * @param req Thallium request.
     */
    void on_join(const tl::request& req);

    std::unordered_map<UUID,
        std::shared_ptr<communicator>,
        UUID_hash_fn>                m_communicators;
    tl::mutex                        m_comm_mutex;
    tl::pool                         m_pool;
    tl::remote_procedure             m_p2p_transfer_rpc;
    tl::remote_procedure             m_join_rpc;
    std::vector<tl::provider_handle> m_members;
    std::vector<tl::provider_handle> m_pending_members;
    tl::mutex                        m_members_mutex;
    std::string                      m_leader_addr;
    std::string                      m_this_addr;

};

}

#endif
