/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_DISTRIBUTED_PIPELINE_HANDLE_HPP
#define __COLZA_DISTRIBUTED_PIPELINE_HANDLE_HPP

#include <mpi.h>
#include <thallium.hpp>
#include <colza/Types.hpp>
#include <colza/AsyncRequest.hpp>
#include <colza/PipelineHandle.hpp>

namespace colza {

namespace tl = thallium;

class Client;
class DistributedPipelineHandleImpl;

/**
 * @brief The HashFunction type is an std::function that takes the dataset name,
 * the iteration number, and the block id, and returns a hash that will guide
 * selection of the target server.
 */
typedef std::function<uint64_t(const std::string&, uint64_t, uint64_t)> HashFunction;

/**
 * @brief A DistributedPipelineHandle object is a handle for a set of
 * remote pipelines on multiple servers. It enables invoking the pipeline's
 * functionalities across processes.
 */
class DistributedPipelineHandle {

    friend class Client;

    public:

    /**
     * @brief Constructor. The resulting PipelineHandle handle will be invalid.
     */
    DistributedPipelineHandle();

    /**
     * @brief Copy-constructor.
     */
    DistributedPipelineHandle(const DistributedPipelineHandle&);

    /**
     * @brief Move-constructor.
     */
    DistributedPipelineHandle(DistributedPipelineHandle&&);

    /**
     * @brief Copy-assignment operator.
     */
    DistributedPipelineHandle& operator=(const DistributedPipelineHandle&);

    /**
     * @brief Move-assignment operator.
     */
    DistributedPipelineHandle& operator=(DistributedPipelineHandle&&);

    /**
     * @brief Destructor.
     */
    ~DistributedPipelineHandle();

    /**
     * @brief Returns the client this database has been opened with.
     */
    Client client() const;


    /**
     * @brief Checks if the PipelineHandle instance is valid.
     */
    operator bool() const;

    /**
     * @brief Get the HashFunction that the DistributedPipelineHandle
     * uses to select the server to send data to.
     *
     * @return The HashFunction.
     */
    HashFunction getHashFunction() const;

    /**
     * @brief Set the HashFunction that the DistributedPipelineHandle
     * will use to select the server to send data to.
     *
     * @param hash HashFunction
     */
    void setHashFunction(const HashFunction& hash);

    /**
     * @brief Start the pipeline on a given iteration.
     * This function is not marked const since it can lead to the
     * distributed pipeline reconfiguring itself.
     *
     * @param iteration Iteration to start.
     */
    void start(uint64_t iteration);

    /**
     * @brief Stage some data into the pipeline using a bulk handle.
     *
     * @param[in] dataset_name Dataset name
     * @param[in] iteration Iteration
     * @param[in] block_id Block id
     * @param[in] dimensions Dimensions
     * @param[in] offsets Offsets
     * @param[in] type Type
     * @param[in] data Data as bulk handle
     * @param[in] origin_addr Address of the bulk handle ("" if this process)
     * @param[out] result Result
     * @param[out] req Asynchronous request
     */
    void stage(const std::string& dataset_name,
               uint64_t iteration,
               uint64_t block_id,
               const std::vector<size_t>& dimensions,
               const std::vector<int64_t>& offsets,
               const Type& type,
               const thallium::bulk& data,
               const std::string& origin_addr = "",
               int32_t* result = nullptr,
               AsyncRequest* req = nullptr) const;

    /**
     * @brief Stage some local data into the pipeline.
     *
     * @param[in] dataset_name Dataset name
     * @param[in] iteration Iteration
     * @param[in] block_id Block id
     * @param[in] dimensions Dimensions
     * @param[in] offsets Offsets
     * @param[in] type Type
     * @param[in] data Local data
     * @param[out] result Result
     * @param[out] req Asynchronous request
     */
    void stage(const std::string& dataset_name,
               uint64_t iteration,
               uint64_t block_id,
               const std::vector<size_t>& dimensions,
               const std::vector<int64_t>& offsets,
               const Type& type,
               const void* data,
               int32_t* result = nullptr,
               AsyncRequest* req = nullptr) const;


    /**
     * @brief Execute the pipeline on a given iteration.
     *
     * @param iteration Iteration of data on which to execute.
     * @param result Result.
     * @param autoCleanup Whether to auto-cleanup after execution.
     * @param req Asynchronous request.
     */
    void execute(uint64_t iteration,
                 int32_t* result = nullptr,
                 bool autoCleanup = false,
                 AsyncRequest* req = nullptr) const;

    /**
     * @brief Cleanup the pipeline on a given iteration.
     *
     * @param iteration Iteration to cleanup.
     * @param result Result.
     * @param req Asynchronous request.
     */
    void cleanup(uint64_t iteration,
                 int32_t* result = nullptr,
                 AsyncRequest* req = nullptr) const;

    private:

    /**
     * @brief Constructor is private. Use a Client object
     * to create a DistributedPipelineHandle instance.
     *
     * @param impl Pointer to implementation.
     */
    DistributedPipelineHandle(const std::shared_ptr<DistributedPipelineHandleImpl>& impl);

    std::shared_ptr<DistributedPipelineHandleImpl> self;
};

}

#endif
