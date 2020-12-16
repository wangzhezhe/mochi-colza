/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "SimpleStagerBackend.hpp"
#include <iostream>

COLZA_REGISTER_BACKEND(simple_stager, SimpleStagerPipeline);

colza::RequestResult<bool> SimpleStagerPipeline::destroy() {
    colza::RequestResult<bool> result;
    result.value() = true;
    return result;
}

std::unique_ptr<colza::Backend> SimpleStagerPipeline::create(const colza::PipelineFactoryArgs& args) {
    return std::unique_ptr<colza::Backend>(new SimpleStagerPipeline(args));
}
