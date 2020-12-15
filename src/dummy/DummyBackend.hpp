/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __DUMMY_BACKEND_HPP
#define __DUMMY_BACKEND_HPP

#include <colza/Backend.hpp>

using json = nlohmann::json;

/**
 * Dummy implementation of an colza Backend.
 */
class DummyPipeline : public colza::Backend {

    ssg_group_id_t m_gid;
    json           m_config;

    public:

    /**
     * @brief Constructor.
     */
    DummyPipeline(ssg_group_id_t gid, const json& config)
    : m_gid(gid), m_config(config) {}

    /**
     * @brief Move-constructor.
     */
    DummyPipeline(DummyPipeline&&) = delete;

    /**
     * @brief Copy-constructor.
     */
    DummyPipeline(const DummyPipeline&) = delete;

    /**
     * @brief Move-assignment operator.
     */
    DummyPipeline& operator=(DummyPipeline&&) = delete;

    /**
     * @brief Copy-assignment operator.
     */
    DummyPipeline& operator=(const DummyPipeline&) = delete;

    /**
     * @brief Destructor.
     */
    virtual ~DummyPipeline() = default;

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
     * create a DummyPipeline.
     *
     * @param gid SSG group id
     * @param config JSON configuration for the pipeline
     *
     * @return a unique_ptr to a pipeline
     */
    static std::unique_ptr<colza::Backend> create(ssg_group_id_t gid, const json& config);
};

#endif
