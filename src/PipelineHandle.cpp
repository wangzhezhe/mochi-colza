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

}
