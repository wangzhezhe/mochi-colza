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
                std::function<std::unique_ptr<Backend>(const PipelineFactoryArgs&)>> PipelineFactory::create_fn;

std::unique_ptr<Backend> PipelineFactory::createPipeline(const std::string& backend_name,
                                                         const PipelineFactoryArgs& args) {
    auto it = create_fn.find(backend_name);
    if(it == create_fn.end()) return nullptr;
    auto& f = it->second;
    return f(args);
}

}
