/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_CLIENT_HPP
#define __COLZA_CLIENT_HPP

#include <colza/PipelineHandle.hpp>
#include <colza/UUID.hpp>
#include <thallium.hpp>
#include <memory>

namespace colza {

class ClientImpl;
class PipelineHandle;

/**
 * @brief The Client object is the main object used to establish
 * a connection with a Colza service.
 */
class Client {

    friend class PipelineHandle;

    public:

    /**
     * @brief Default constructor.
     */
    Client();

    /**
     * @brief Constructor using a margo instance id.
     *
     * @param mid Margo instance id.
     */
    Client(margo_instance_id mid);

    /**
     * @brief Constructor.
     *
     * @param engine Thallium engine.
     */
    Client(const thallium::engine& engine);

    /**
     * @brief Copy constructor.
     */
    Client(const Client&);

    /**
     * @brief Move constructor.
     */
    Client(Client&&);

    /**
     * @brief Copy-assignment operator.
     */
    Client& operator=(const Client&);

    /**
     * @brief Move-assignment operator.
     */
    Client& operator=(Client&&);

    /**
     * @brief Destructor.
     */
    ~Client();

    /**
     * @brief Returns the thallium engine used by the client.
     */
    const thallium::engine& engine() const;

    /**
     * @brief Creates a handle to a remote pipeline and returns.
     * You may set "check" to false if you know for sure that the
     * corresponding pipeline exists, which will avoid one RPC.
     *
     * @param address Address of the provider holding the database.
     * @param provider_id Provider id.
     * @param pipeline_id Pipeline UUID.
     * @param check Checks if the Database exists by issuing an RPC.
     *
     * @return a PipelineHandle instance.
     */
    PipelineHandle makePipelineHandle(const std::string& address,
                                      uint16_t provider_id,
                                      const UUID& pipeline_id,
                                      bool check = true) const;

    /**
     * @brief Checks that the Client instance is valid.
     */
    operator bool() const;

    private:

    Client(const std::shared_ptr<ClientImpl>& impl);

    std::shared_ptr<ClientImpl> self;
};

}

#endif
