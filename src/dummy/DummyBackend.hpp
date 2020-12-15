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

    json                   m_config;

    public:

    /**
     * @brief Constructor.
     */
    DummyPipeline(const json& config)
    : m_config(config) {}

    /**
     * @brief Move-constructor.
     */
    DummyPipeline(DummyPipeline&&) = default;

    /**
     * @brief Copy-constructor.
     */
    DummyPipeline(const DummyPipeline&) = default;

    /**
     * @brief Move-assignment operator.
     */
    DummyPipeline& operator=(DummyPipeline&&) = default;

    /**
     * @brief Copy-assignment operator.
     */
    DummyPipeline& operator=(const DummyPipeline&) = default;

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
     * @param config JSON configuration for the pipeline
     *
     * @return a unique_ptr to a pipeline
     */
    static std::unique_ptr<colza::Backend> create(const json& config);

    /**
     * @brief Static factory function used by the PipelineFactory to
     * open a DummyPipeline.
     *
     * @param config JSON configuration for the pipeline
     *
     * @return a unique_ptr to a pipeline
     */
    static std::unique_ptr<colza::Backend> open(const json& config);
};

#endif
