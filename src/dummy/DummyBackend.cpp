/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "DummyBackend.hpp"
#include <iostream>

COLZA_REGISTER_BACKEND(dummy, DummyPipeline);

colza::RequestResult<bool> DummyPipeline::destroy() {
    colza::RequestResult<bool> result;
    result.value() = true;
    // or result.success() = true
    return result;
}

std::unique_ptr<colza::Backend> DummyPipeline::create(ssg_group_id_t gid, const json& config) {
    return std::unique_ptr<colza::Backend>(new DummyPipeline(gid, config));
}
