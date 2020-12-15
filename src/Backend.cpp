/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "colza/Backend.hpp"

namespace tl = thallium;

namespace colza {

using json = nlohmann::json;

std::unordered_map<std::string,
                std::function<std::unique_ptr<Backend>(const json&)>> PipelineFactory::create_fn;

std::unordered_map<std::string,
                std::function<std::unique_ptr<Backend>(const json&)>> PipelineFactory::open_fn;

std::unique_ptr<Backend> PipelineFactory::createPipeline(const std::string& backend_name,
                                                         const json& config) {
    auto it = create_fn.find(backend_name);
    if(it == create_fn.end()) return nullptr;
    auto& f = it->second;
    return f(config);
}

std::unique_ptr<Backend> PipelineFactory::openPipeline(const std::string& backend_name,
                                                       const json& config) {
    auto it = open_fn.find(backend_name);
    if(it == open_fn.end()) return nullptr;
    auto& f = it->second;
    return f(config);
}

}
