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
     * @param library Dynamic library to load
     * @param token Security token
     */
    void createPipeline(const std::string& address,
                        uint16_t provider_id,
                        const std::string& name,
                        const std::string& type,
                        const std::string& config,
                        const std::string& library="",
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
     * @param library Dynamic library to load
     * @param token Security token
     */
    void createPipeline(const std::string& address,
                        uint16_t provider_id,
                        const std::string& name,
                        const std::string& type,
                        const char* config,
                        const std::string& library="",
                        const std::string& token="") const {
        return createPipeline(address, provider_id, name, type, std::string(config), library, token);
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
     * @param library Dynamic library to load
     * @param token Security token
     */
    void createPipeline(const std::string& address,
                        uint16_t provider_id,
                        const std::string& name,
                        const std::string& type,
                        const json& config,
                        const std::string& library="",
                        const std::string& token="") const {
        return createPipeline(address, provider_id, name, type, config.dump(), library, token);
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
     * @brief Creates a pipeline on the target providers
     * listed in an SSG group file.
     * The config string must be a JSON object acceptable
     * by the desired backend's creation function.
     *
     * @param ssg_file SSG file listing providers.
     * @param provider_id Provider id.
     * @param name Name of the pipeline.
     * @param type Type of the pipeline to create.
     * @param config JSON configuration for the pipeline.
     * @param library Dynamic library to load
     * @param token Security token
     */
    void createDistributedPipeline(const std::string& ssg_file,
                        uint16_t provider_id,
                        const std::string& name,
                        const std::string& type,
                        const std::string& config,
                        const std::string& library="",
                        const std::string& token="") const;

    /**
     * @brief Creates a pipeline on the target providers
     * listed in an SSG group file.
     * The config string must be a JSON object acceptable
     * by the desired backend's creation function.
     *
     * @param ssg_file SSG file listing providers.
     * @param provider_id Provider id.
     * @param name Pipeline name.
     * @param type Type of the pipeline to create.
     * @param config JSON configuration for the pipeline.
     * @param library Dynamic library to load
     * @param token Security token
     */
    void createDistributedPipeline(const std::string& ssg_file,
                        uint16_t provider_id,
                        const std::string& name,
                        const std::string& type,
                        const char* config,
                        const std::string& library="",
                        const std::string& token="") const {
        return createDistributedPipeline(ssg_file, provider_id, name, type, std::string(config), library, token);
    }

    /**
     * @brief Creates a pipeline on the providers present in the
     * provided SSG group.
     * The config object must be a JSON object acceptable
     * by the desired backend's creation function.
     *
     * @param ssg_file SSG file containing addresses of providers.
     * @param provider_id Provider id.
     * @param name Pipeline name.
     * @param type Type of the pipeline to create.
     * @param config JSON configuration for the pipeline.
     * @param library Dynamic library to load
     * @param token Security token
     */
    void createDistributedPipeline(const std::string& ssg_file,
                        uint16_t provider_id,
                        const std::string& name,
                        const std::string& type,
                        const json& config,
                        const std::string& library="",
                        const std::string& token="") const {
        return createDistributedPipeline(ssg_file, provider_id, name, type, config.dump(), library, token);
    }

    /**
     * @brief Destroys an open pipeline in the providers listed in the SSG group.
     *
     * @param ssg_file SSG file containing addresses of providers.
     * @param provider_id Provider id.
     * @param pipeline_name Name of the pipeline to destroy.
     */
    void destroyDistributedPipeline(const std::string& ssg_file,
                         uint16_t provider_id,
                         const std::string& pipeline_name,
                         const std::string& token="") const;

    /**
     * @brief Shutdown all the members of the SSG group.
     *
     * @param ssg_file SSG file containing addresses of providers.
     */
    void shutdownGroup(const std::string& ssg_file) const;


    /**
     * @brief Shuts down the target server. The Thallium engine
     * used by the server must have remote shutdown enabled.
     *
     * @param address Address of the server to shut down.
     */
    void shutdownServer(const std::string& address) const;

    /**
     * @brief Request the server to leave the group and shutdown
     * by calling the "leave" RPC of a provider.
     *
     * @param address Address of the server.
     * @param provider_id Provider id of the provider.
     */
    void makeServerLeave(const std::string& address, uint16_t provider_id) const;

    /**
     * @brief Request a set of servers, represented by their rank
     * in an SSG group file, to leave the group and shutdown.
     *
     * @param ssg_file SSG group file name.
     * @param ranks Ranks to shutdown.
     * @param provider_id Provider id.
     */
    void makeServersLeave(
        const std::string& ssg_file,
        const std::vector<int>& ranks,
        uint16_t provider_id) const;

    private:

    std::shared_ptr<AdminImpl> self;
};

}

#endif
