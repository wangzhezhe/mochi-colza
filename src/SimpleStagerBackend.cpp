/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "SimpleStagerBackend.hpp"
#include <iostream>

COLZA_REGISTER_BACKEND(simpls_stager, SimpleStagerPipeline);

colza::RequestResult<bool> SimpleStagerPipeline::destroy() {
    colza::RequestResult<bool> result;
    result.value() = true;
    // or result.success() = true
    return result;
}

std::unique_ptr<colza::Backend> SimpleStagerPipeline::create(ssg_group_id_t gid, const json& config) {
    return std::unique_ptr<colza::Backend>(new SimpleStagerPipeline(gid, config));
}
