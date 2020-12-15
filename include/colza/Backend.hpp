/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __COLZA_BACKEND_HPP
#define __COLZA_BACKEND_HPP

#include <colza/RequestResult.hpp>
#include <colza/Types.hpp>

#include <ssg.h>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include <thallium.hpp>

/**
 * @brief Helper class to register backend types into the backend factory.
 */
template<typename BackendType>
class __ColzaBackendRegistration;

namespace colza {

/**
 * @brief Interface for pipeline backends. To build a new backend,
 * implement a class MyBackend that inherits from Backend, and put
 * COLZA_REGISTER_BACKEND(mybackend, MyBackend); in a cpp file
 * that includes your backend class' header file.
 *
 * Your backend class should also have a static function to
 * create a pipeline:
 *
 * std::unique_ptr<Backend> create(ssg_group_id_t gid, const json& config)
 */
class Backend {

    public:

    /**
     * @brief Constructor.
     */
    Backend() = default;

    /**
     * @brief Move-constructor.
     */
    Backend(Backend&&) = default;

    /**
     * @brief Copy-constructor.
     */
    Backend(const Backend&) = default;

    /**
     * @brief Move-assignment operator.
     */
    Backend& operator=(Backend&&) = default;

    /**
     * @brief Copy-assignment operator.
     */
    Backend& operator=(const Backend&) = default;

    /**
     * @brief Destructor.
     */
    virtual ~Backend() = default;

    /**
     * @brief Stage some data for a future execution of the pipeline.
     *
     * @param dataset Dataset name
     * @param iteration Iteration
     * @param block_id Block id
     * @param dimensions Dimensions
     * @param data Data
     *
     * @return COLZA_SUCCESS or other error codes.
     */
    virtual RequestResult<int32_t> stage(
            const std::string& dataset_name,
            uint64_t iteration,
            uint64_t block_id,
            const std::vector<size_t>& dimensions,
            const Type& type,
            const thallium::bulk& data) = 0;

    /**
     * @brief Execute the pipeline on a specific iteration of data.
     *
     * @param iteration Iteration
     *
     * @return COLZA_SUCCESS or other error codes.
     */
    virtual RequestResult<int32_t> execute(
            uint64_t iteration) = 0;

    /**
     * @brief Cleanup the data associated to the provided iteration number.
     *
     * @param iteration Iteration number.
     *
     * @return COLZA_SUCCESS or other error codes.
     */
    virtual RequestResult<int32_t> cleanup(
            uint64_t iteration) = 0;

    /**
     * @brief Destroys the underlying pipeline.
     *
     * @return a RequestResult<bool> instance indicating
     * whether the database was successfully destroyed.
     */
    virtual RequestResult<bool> destroy() = 0;

};

/**
 * @brief The PipelineFactory contains functions to create
 * or open pipelines.
 */
class PipelineFactory {

    template<typename BackendType>
    friend class ::__ColzaBackendRegistration;

    using json = nlohmann::json;

    public:

    PipelineFactory() = delete;

    /**
     * @brief Creates a pipeline and returns a unique_ptr to the created instance.
     *
     * @param backend_name Name of the backend to use.
     * @param gid Group gathering other providers running the same pipeline.
     * @param config Configuration object to pass to the backend's create function.
     *
     * @return a unique_ptr to the created Pipeline.
     */
    static std::unique_ptr<Backend> createPipeline(const std::string& backend_name,
                                                   ssg_group_id_t gid,
                                                   const json& config);

    private:

    static std::unordered_map<std::string,
                std::function<std::unique_ptr<Backend>(ssg_group_id_t, const json&)>> create_fn;
};

} // namespace colza


#define COLZA_REGISTER_BACKEND(__backend_name, __backend_type) \
    static __ColzaBackendRegistration<__backend_type> __colza ## __backend_name ## _backend( #__backend_name )

template<typename BackendType>
class __ColzaBackendRegistration {

    using json = nlohmann::json;

    public:

    __ColzaBackendRegistration(const std::string& backend_name)
    {
        colza::PipelineFactory::create_fn[backend_name] = [](ssg_group_id_t gid, const json& config) {
            return BackendType::create(gid, config);
        };
    }
};

#endif
