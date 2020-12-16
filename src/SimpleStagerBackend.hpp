/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __SIMPLE_STAGER_BACKEND_HPP
#define __SIMPLE_STAGER_BACKEND_HPP

#include <thallium.hpp>
#include <colza/Backend.hpp>

using json = nlohmann::json;
namespace tl = thallium;

struct DataBlock {

    std::vector<char>   data;
    std::vector<size_t> dimensions;
    colza::Type         type;

    DataBlock()                            = default;
    DataBlock(DataBlock&&)                 = default;
    DataBlock(const DataBlock&)            = default;
    DataBlock& operator=(DataBlock&&)      = default;
    DataBlock& operator=(const DataBlock&) = default;
    ~DataBlock()                           = default;

};

/**
 * SimpleStager implementation of an colza Backend.
 */
class SimpleStagerPipeline : public colza::Backend {

    protected:

    tl::engine     m_engine;
    ssg_group_id_t m_gid;
    json           m_config;
    std::map<uint64_t,         // iteration
        std::map<std::string,  // dataset name
            std::map<uint64_t, // block id
                     DataBlock
                    >
                >
            > m_datasets;
    tl::mutex m_datasets_mtx;

    public:

    /**
     * @brief Constructor.
     */
    SimpleStagerPipeline(const colza::PipelineFactoryArgs& args)
    : m_engine(args.engine)
    , m_gid(args.gid)
    , m_config(args.config) {}

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

    /**
     * @brief Stage some data.
     */
    colza::RequestResult<int32_t> stage(
            // TODO add sender's address
            const std::string& dataset_name,
            uint64_t iteration,
            uint64_t block_id,
            const std::vector<size_t>& dimensions,
            const colza::Type& type,
            const thallium::bulk& data) override {
        colza::RequestResult<int32_t> result;
        {
            std::lock_guard<tl::mutex> g(m_datasets_mtx);
            if(m_datasets.count(iteration) != 0
            && m_datasets[iteration].count(dataset_name) != 0
            && m_datasets[iteration][dataset_name].count(block_id) != 0) {
                result.error() = "Block already exists for provided iteration, name, and id";
                result.success() = false;
                return result;
            }
        }
        DataBlock block;
        block.dimensions = dimensions;
        block.type       = type;
        block.data.resize(data.size());
        // TODO issue bulk transfer
        {
            std::lock_guard<tl::mutex> g(m_datasets_mtx);
            m_datasets[iteration][dataset_name][block_id] = std::move(block);
        }
        return result;
    }

    /**
     * @brief The execute method in this backend is not doing anything.
     */
    colza::RequestResult<int32_t> execute(
            uint64_t iteration) override {
        (void)iteration;
        auto result = colza::RequestResult<int32_t>();
        result.value() = 0;
        return result;;
    }

    /**
     * @brief Erase all the data blocks associated with a given iteration.
     */
    colza::RequestResult<int32_t> cleanup(
            uint64_t iteration) override {
        std::lock_guard<tl::mutex> g(m_datasets_mtx);
        m_datasets.erase(iteration);
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
     * @param args arguments used for creating the pipeline.
     *
     * @return a unique_ptr to a pipeline
     */
    static std::unique_ptr<colza::Backend> create(const colza::PipelineFactoryArgs& args);
};

#endif
