/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __SIMPLE_STAGER_BACKEND_HPP
#define __SIMPLE_STAGER_BACKEND_HPP

#include <colza/Backend.hpp>

using json = nlohmann::json;

/**
 * SimpleStager implementation of an colza Backend.
 */
class SimpleStagerPipeline : public colza::Backend {

    ssg_group_id_t m_gid;
    json           m_config;

    public:

    /**
     * @brief Constructor.
     */
    SimpleStagerPipeline(ssg_group_id_t gid, const json& config)
    : m_gid(gid), m_config(config) {}

    /**
     * @brief Move-constructor.
     */
    SimpleStagerPipeline(SimpleStagerPipeline&&) = delete;

    /**
     * @brief Copy-constructor.
     */
    SimpleStagerPipeline(const SimpleStagerPipeline&) = delete;

    /**
     * @brief Move-assignment operator.
     */
    SimpleStagerPipeline& operator=(SimpleStagerPipeline&&) = delete;

    /**
     * @brief Copy-assignment operator.
     */
    SimpleStagerPipeline& operator=(const SimpleStagerPipeline&) = delete;

    /**
     * @brief Destructor.
     */
    virtual ~SimpleStagerPipeline() = default;

    colza::RequestResult<int32_t> stage(
            const std::string& dataset_name,
            uint64_t iteration,
            uint64_t block_id,
            const std::vector<size_t>& dimensions,
            const colza::Type& type,
            const thallium::bulk& data) override {
        // TODO
        (void)dataset_name;
        (void)iteration;
        (void)block_id;
        (void)dimensions;
        (void)type;
        (void)data;
        return colza::RequestResult<int32_t>();
    }

    colza::RequestResult<int32_t> execute(
            uint64_t iteration) override {
        // TODO
        (void)iteration;
        return colza::RequestResult<int32_t>();
    }

    colza::RequestResult<int32_t> cleanup(
            uint64_t iteration) override {
        // TODO
        (void)iteration;
        return colza::RequestResult<int32_t>();
    }

    /**
     * @brief Destroys the underlying pipeline.
     *
     * @return a RequestResult<bool> instance indicating
     * whether the database was successfully destroyed.
     */
    colza::RequestResult<bool> destroy() override;

    /**
     * @brief Static factory function used by the PipelineFactory to
     * create a SimpleStagerPipeline.
     *
     * @param gid SSG group id
     * @param config JSON configuration for the pipeline
     *
     * @return a unique_ptr to a pipeline
     */
    static std::unique_ptr<colza::Backend> create(ssg_group_id_t gid, const json& config);
};

#endif
