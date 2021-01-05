/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_ADMIN_HPP
#define __COLZA_ADMIN_HPP

#include <colza/Exception.hpp>
#include <nlohmann/json.hpp>
#include <thallium.hpp>
#include <string>
#include <memory>

namespace colza {

namespace tl = thallium;

class AdminImpl;

/**
 * @brief Admin interface to a COLZA service. Enables creating
 * and destroying pipelines, and attaching and detaching them
 * from a provider. If COLZA providers have set up a security
 * token, operations from the Admin interface will need this
 * security token.
 */
class Admin {

    public:

    using json = nlohmann::json;

    /**
     * @brief Default constructor.
     */
    Admin();

    /**
     * @brief Constructor using a margo instance id.
     *
     * @param mid Margo instance id.
     */
    Admin(margo_instance_id mid);

    /**
     * @brief Constructor.
     *
     * @param engine Thallium engine.
     */
    Admin(const tl::engine& engine);

    /**
     * @brief Copy constructor.
     */
    Admin(const Admin&);

    /**
     * @brief Move constructor.
     */
    Admin(Admin&&);

    /**
     * @brief Copy-assignment operator.
     */
    Admin& operator=(const Admin&);

    /**
     * @brief Move-assignment operator.
     */
    Admin& operator=(Admin&&);

    /**
     * @brief Destructor.
     */
    ~Admin();

    /**
     * @brief Check if the Admin instance is valid.
     */
    operator bool() const;

    /**
     * @brief Creates a pipeline on the target provider.
     * The config string must be a JSON object acceptable
     * by the desired backend's creation function.
     *
     * @param address Address of the target provider.
     * @param provider_id Provider id.
     * @param name Name of the pipeline.
     * @param type Type of the pipeline to create.
     * @param config JSON configuration for the pipeline.
     */
    void createPipeline(const std::string& address,
                        uint16_t provider_id,
                        const std::string& name,
                        const std::string& type,
                        const std::string& config,
                        const std::string& token="") const;

    /**
     * @brief Creates a pipeline on the target provider.
     * The config string must be a JSON object acceptable
     * by the desired backend's creation function.
     *
     * @param address Address of the target provider.
     * @param provider_id Provider id.
     * @param name Pipeline name.
     * @param type Type of the pipeline to create.
     * @param config JSON configuration for the pipeline.
     */
    void createPipeline(const std::string& address,
                        uint16_t provider_id,
                        const std::string& name,
                        const std::string& type,
                        const char* config,
                        const std::string& token="") const {
        return createPipeline(address, provider_id, name, type, std::string(config), token);
    }

    /**
     * @brief Creates a pipeline on the target provider.
     * The config object must be a JSON object acceptable
     * by the desired backend's creation function.
     *
     * @param address Address of the target provider.
     * @param provider_id Provider id.
     * @param name Pipeline name.
     * @param type Type of the pipeline to create.
     * @param config JSON configuration for the pipeline.
     */
    void createPipeline(const std::string& address,
                        uint16_t provider_id,
                        const std::string& name,
                        const std::string& type,
                        const json& config,
                        const std::string& token="") const {
        return createPipeline(address, provider_id, name, type, config.dump(), token);
    }

    /**
     * @brief Destroys an open pipeline in the target provider.
     *
     * @param address Address of the target provider.
     * @param provider_id Provider id.
     * @param pipeline_id UUID of the pipeline to destroy.
     */
    void destroyPipeline(const std::string& address,
                         uint16_t provider_id,
                         const std::string& pipeline_name,
                         const std::string& token="") const;

    /**
     * @brief Shuts down the target server. The Thallium engine
     * used by the server must have remote shutdown enabled.
     *
     * @param address Address of the server to shut down.
     */
    void shutdownServer(const std::string& address) const;

    private:

    std::shared_ptr<AdminImpl> self;
};

}

#endif
