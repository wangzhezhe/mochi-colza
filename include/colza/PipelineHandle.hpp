/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_PIPELINE_HANDLE_HPP
#define __COLZA_PIPELINE_HANDLE_HPP

#include <thallium.hpp>
#include <memory>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <colza/Client.hpp>
#include <colza/Types.hpp>
#include <colza/Exception.hpp>
#include <colza/AsyncRequest.hpp>

namespace colza {

namespace tl = thallium;

class Client;
class PipelineHandleImpl;
class DistributedPipelineHandle;

/**
 * @brief A PipelineHandle object is a handle for a remote pipeline
 * on a server. It enables invoking the pipeline's functionalities.
 */
class PipelineHandle {

    friend class Client;
    friend class DistributedPipelineHandle;

    public:

    /**
     * @brief Constructor. The resulting PipelineHandle handle will be invalid.
     */
    PipelineHandle();

    /**
     * @brief Copy-constructor.
     */
    PipelineHandle(const PipelineHandle&);

    /**
     * @brief Move-constructor.
     */
    PipelineHandle(PipelineHandle&&);

    /**
     * @brief Copy-assignment operator.
     */
    PipelineHandle& operator=(const PipelineHandle&);

    /**
     * @brief Move-assignment operator.
     */
    PipelineHandle& operator=(PipelineHandle&&);

    /**
     * @brief Destructor.
     */
    ~PipelineHandle();

    /**
     * @brief Returns the client this database has been opened with.
     */
    Client client() const;


    /**
     * @brief Checks if the PipelineHandle instance is valid.
     */
    operator bool() const;

    /**
     * @brief Tell the pipeline that an iterarion is starting.
     *
     * @param iteration Iteration number
     * @param result Result sent by the pipeline
     * @param req Asynchronous request
     */
    void start(uint64_t iteration,
               int32_t* result = nullptr,
               AsyncRequest* req = nullptr) const;

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
     * @param req Asynchronous request.
     */
    void execute(uint64_t iteration,
                 int32_t* result = nullptr,
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
     * to create a PipelineHandle instance.
     *
     * @param impl Pointer to implementation.
     */
    PipelineHandle(const std::shared_ptr<PipelineHandleImpl>& impl);

    std::shared_ptr<PipelineHandleImpl> self;
};

}

#endif
