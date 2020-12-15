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
#include <colza/Exception.hpp>
#include <colza/AsyncRequest.hpp>

namespace colza {

namespace tl = thallium;

class Client;
class PipelineHandleImpl;

/**
 * @brief A PipelineHandle object is a handle for a remote pipeline
 * on a server. It enables invoking the pipeline's functionalities.
 */
class PipelineHandle {

    friend class Client;

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
     * @brief Sends an RPC to the pipeline to make it print a hello message.
     */
    void sayHello() const;

    /**
     * @brief Requests the target pipeline to compute the sum of two numbers.
     * If result is null, it will be ignored. If req is not null, this call
     * will be non-blocking and the caller is responsible for waiting on
     * the request.
     *
     * @param[in] x first integer
     * @param[in] y second integer
     * @param[out] result result
     * @param[out] req request for a non-blocking operation
     */
    void computeSum(int32_t x, int32_t y,
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
