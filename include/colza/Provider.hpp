/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_PROVIDER_HPP
#define __COLZA_PROVIDER_HPP

#include <thallium.hpp>
#include <ssg.h>
#include <mona.h>
#include <memory>

namespace colza {

namespace tl = thallium;

class ProviderImpl;

/**
 * @brief A Provider is an object that can receive RPCs
 * and dispatch them to specific pipelines.
 */
class Provider {

    public:

    /**
     * @brief Constructor.
     *
     * @param engine Thallium engine to use to receive RPCs.
     * @param gid SSG group this executor is part of.
     * @param mona Mona instance.
     * @param provider_id Provider id.
     * @param config JSON-formatted configuration.
     * @param pool Argobots pool to use to handle RPCs.
     */
    Provider(const tl::engine& engine,
             ssg_group_id_t gid,
             mona_instance_t mona,
             uint16_t provider_id = 0,
             const std::string& config = "",
             const tl::pool& pool = tl::pool());

    /**
     * @brief Constructor.
     *
     * @param mid Margo instance id to use to receive RPCs.
     * @param gid SSG group this executor is part of.
     * @param mona Mona instance.
     * @param provider_id Provider id.
     * @param config JSON-formatted configuration.
     * @param pool Argobots pool to use to handle RPCs.
     */
    Provider(margo_instance_id mid,
             ssg_group_id_t gid,
             mona_instance_t mona,
             uint16_t provider_id = 0,
             const std::string& config = "",
             const tl::pool& pool = tl::pool());

    /**
     * @brief Copy-constructor.
     */
    Provider(const Provider&) = delete;

    /**
     * @brief Move-constructor.
     */
    Provider(Provider&&);

    /**
     * @brief Copy-assignment operator is deleted.
     */
    Provider& operator=(const Provider&) = delete;

    /**
     * @brief Move-assignment operator is deleted.
     */
    Provider& operator=(Provider&&) = delete;

    /**
     * @brief Destructor.
     */
    ~Provider();

    /**
     * @brief Sets a security string that should be provided
     * by Admin RPCs to accept them.
     *
     * @param token Security token to set.
     */
    void setSecurityToken(const std::string& token);

    /**
     * @brief Return a JSON-formatted configuration of the provider.
     *
     * @return JSON formatted string.
     */
    std::string getConfig() const;

    /**
     * @brief Checks whether the Provider instance is valid.
     */
    operator bool() const;

    private:

    std::shared_ptr<ProviderImpl> self;
};

}

#endif
