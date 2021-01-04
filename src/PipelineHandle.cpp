/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "colza/PipelineHandle.hpp"
#include "colza/RequestResult.hpp"
#include "colza/Exception.hpp"

#include "AsyncRequestImpl.hpp"
#include "ClientImpl.hpp"
#include "PipelineHandleImpl.hpp"
#include "TypeSizes.hpp"

#include <thallium/serialization/stl/string.hpp>
#include <thallium/serialization/stl/pair.hpp>

namespace colza {

PipelineHandle::PipelineHandle() = default;

PipelineHandle::PipelineHandle(const std::shared_ptr<PipelineHandleImpl>& impl)
: self(impl) {}

PipelineHandle::PipelineHandle(const PipelineHandle&) = default;

PipelineHandle::PipelineHandle(PipelineHandle&&) = default;

PipelineHandle& PipelineHandle::operator=(const PipelineHandle&) = default;

PipelineHandle& PipelineHandle::operator=(PipelineHandle&&) = default;

PipelineHandle::~PipelineHandle() = default;

PipelineHandle::operator bool() const {
    return static_cast<bool>(self);
}

Client PipelineHandle::client() const {
    return Client(self->m_client);
}

void PipelineHandle::stage(const std::string& dataset_name,
           uint64_t iteration,
           uint64_t block_id,
           const std::vector<size_t>& dimensions,
           const std::vector<int64_t>& offsets,
           const Type& type,
           const thallium::bulk& data,
           const std::string& origin_addr,
           int32_t* result,
           AsyncRequest* req) const {
    if(not self) throw Exception("Invalid colza::PipelineHandle object");
    auto& rpc = self->m_client->m_stage;
    auto& ph  = self->m_ph;
    auto& pipeline_id = self->m_pipeline_id;
    auto sender_addr = origin_addr == "" ?
        static_cast<std::string>(self->m_client->m_engine.self()) :
        origin_addr;
    if(req == nullptr) { // synchronous call
        RequestResult<int32_t> response = rpc.on(ph)(
                pipeline_id,
                sender_addr,
                dataset_name,
                iteration,
                block_id,
                dimensions,
                offsets,
                type,
                data);
        if(response.success()) {
            if(result) *result = response.value();
        } else {
            throw Exception(response.error());
        }
    } else { // asynchronous call
        auto async_response = rpc.on(ph).async(
                pipeline_id,
                sender_addr,
                dataset_name,
                iteration,
                block_id,
                dimensions,
                offsets,
                type,
                data);
        auto async_request_impl =
            std::make_shared<AsyncRequestImpl>(std::move(async_response));
        async_request_impl->m_wait_callback =
            [result](AsyncRequestImpl& async_request_impl) {
                RequestResult<int32_t> response =
                    async_request_impl.m_async_responses[0].wait();
                    async_request_impl.m_async_responses.clear();
                    if(response.success()) {
                        if(result) *result = response.value();
                    } else {
                        throw Exception(response.error());
                    }
            };
        *req = AsyncRequest(std::move(async_request_impl));
    }
}

void PipelineHandle::stage(const std::string& dataset_name,
           uint64_t iteration,
           uint64_t block_id,
           const std::vector<size_t>& dimensions,
           const std::vector<int64_t>& offsets,
           const Type& type,
           const void* data,
           int32_t* result,
           AsyncRequest* req) const {
    if(not self) throw Exception("Invalid colza::PipelineHandle object");
    auto& rpc = self->m_client->m_stage;
    auto& ph  = self->m_ph;
    auto& pipeline_id = self->m_pipeline_id;
    auto sender_addr = static_cast<std::string>(self->m_client->m_engine.self());
    std::vector<std::pair<void*, size_t>> segment(1);
    segment[0].first = const_cast<void*>(data);
    segment[0].second = ComputeDataSize(dimensions, type);
    auto bulk = self->m_client->m_engine.expose(segment, tl::bulk_mode::read_only);
    if(req == nullptr) { // synchronous call
        RequestResult<int32_t> response = rpc.on(ph)(
                pipeline_id,
                sender_addr,
                dataset_name,
                iteration,
                block_id,
                dimensions,
                offsets,
                type,
                bulk);
        if(response.success()) {
            if(result) *result = response.value();
        } else {
            throw Exception(response.error());
        }
    } else { // asynchronous call
        auto async_response = rpc.on(ph).async(
                pipeline_id,
                sender_addr,
                dataset_name,
                iteration,
                block_id,
                dimensions,
                offsets,
                type,
                bulk);
        auto async_request_impl =
            std::make_shared<AsyncRequestImpl>(std::move(async_response));
        async_request_impl->m_wait_callback =
            [result, bulk=std::move(bulk)](AsyncRequestImpl& async_request_impl) {
                RequestResult<int32_t> response =
                    async_request_impl.m_async_responses[0].wait();
                    async_request_impl.m_async_responses.clear();
                    if(response.success()) {
                        if(result) *result = response.value();
                    } else {
                        throw Exception(response.error());
                    }
            };
        *req = AsyncRequest(std::move(async_request_impl));
    }
}

void PipelineHandle::execute(uint64_t iteration,
             int32_t* result,
             AsyncRequest* req) const {
    if(not self) throw Exception("Invalid colza::PipelineHandle object");
    auto& rpc = self->m_client->m_execute;
    auto& ph  = self->m_ph;
    auto& pipeline_id = self->m_pipeline_id;
    if(req == nullptr) { // synchronous call
        RequestResult<int32_t> response = rpc.on(ph)(pipeline_id, iteration);
        if(response.success()) {
            if(result) *result = response.value();
        } else {
            throw Exception(response.error());
        }
    } else { // asynchronous call
        auto async_response = rpc.on(ph).async(pipeline_id, iteration);
        auto async_request_impl =
            std::make_shared<AsyncRequestImpl>(std::move(async_response));
        async_request_impl->m_wait_callback =
            [result](AsyncRequestImpl& async_request_impl) {
                RequestResult<int32_t> response =
                    async_request_impl.m_async_responses[0].wait();
                    async_request_impl.m_async_responses.clear();
                    if(response.success()) {
                        if(result) *result = response.value();
                    } else {
                        throw Exception(response.error());
                    }
            };
        *req = AsyncRequest(std::move(async_request_impl));
    }
}

void PipelineHandle::cleanup(uint64_t iteration,
             int32_t* result,
             AsyncRequest* req) const {
    if(not self) throw Exception("Invalid colza::PipelineHandle object");
    auto& rpc = self->m_client->m_cleanup;
    auto& ph  = self->m_ph;
    auto& pipeline_id = self->m_pipeline_id;
    if(req == nullptr) { // synchronous call
        RequestResult<int32_t> response = rpc.on(ph)(pipeline_id, iteration);
        if(response.success()) {
            if(result) *result = response.value();
        } else {
            throw Exception(response.error());
        }
    } else { // asynchronous call
        auto async_response = rpc.on(ph).async(pipeline_id, iteration);
        auto async_request_impl =
            std::make_shared<AsyncRequestImpl>(std::move(async_response));
        async_request_impl->m_wait_callback =
            [result](AsyncRequestImpl& async_request_impl) {
                RequestResult<int32_t> response =
                    async_request_impl.m_async_responses[0].wait();
                    async_request_impl.m_async_responses.clear();
                    if(response.success()) {
                        if(result) *result = response.value();
                    } else {
                        throw Exception(response.error());
                    }
            };
        *req = AsyncRequest(std::move(async_request_impl));
    }
}

#if 0
void PipelineHandle::sayHello() const {
    if(not self) throw Exception("Invalid colza::PipelineHandle object");
    auto& rpc = self->m_client->m_say_hello;
    auto& ph  = self->m_ph;
    auto& pipeline_id = self->m_pipeline_id;
    rpc.on(ph)(pipeline_id);
}

void PipelineHandle::computeSum(
        int32_t x, int32_t y,
        int32_t* result,
        AsyncRequest* req) const
{
    if(not self) throw Exception("Invalid colza::PipelineHandle object");
    auto& rpc = self->m_client->m_compute_sum;
    auto& ph  = self->m_ph;
    auto& pipeline_id = self->m_pipeline_id;
    if(req == nullptr) { // synchronous call
        RequestResult<int32_t> response = rpc.on(ph)(pipeline_id, x, y);
        if(response.success()) {
            if(result) *result = response.value();
        } else {
            throw Exception(response.error());
        }
    } else { // asynchronous call
        auto async_response = rpc.on(ph).async(pipeline_id, x, y);
        auto async_request_impl =
            std::make_shared<AsyncRequestImpl>(std::move(async_response));
        async_request_impl->m_wait_callback =
            [result](AsyncRequestImpl& async_request_impl) {
                RequestResult<int32_t> response =
                    async_request_impl.m_async_response.wait();
                    if(response.success()) {
                        if(result) *result = response.value();
                    } else {
                        throw Exception(response.error());
                    }
            };
        *req = AsyncRequest(std::move(async_request_impl));
    }
}
#endif
}
