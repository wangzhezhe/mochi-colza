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

std::unique_ptr<colza::Backend> DummyPipeline::create(const json& config) {
    return std::unique_ptr<colza::Backend>(new DummyPipeline(config));
}

std::unique_ptr<colza::Backend> DummyPipeline::open(const json& config) {
    return std::unique_ptr<colza::Backend>(new DummyPipeline(config));
}
