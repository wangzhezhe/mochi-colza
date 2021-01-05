/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "colza/DistributedPipelineHandle.hpp"
#include "colza/RequestResult.hpp"
#include "colza/Exception.hpp"

#include "AsyncRequestImpl.hpp"
#include "ClientImpl.hpp"
#include "DistributedPipelineHandleImpl.hpp"
#include "PipelineHandleImpl.hpp"
#include "TypeSizes.hpp"

#include <thallium/serialization/stl/string.hpp>
#include <thallium/serialization/stl/pair.hpp>

namespace colza {

DistributedPipelineHandle::DistributedPipelineHandle() = default;

DistributedPipelineHandle::DistributedPipelineHandle(const std::shared_ptr<DistributedPipelineHandleImpl>& impl)
: self(impl) {}

DistributedPipelineHandle::DistributedPipelineHandle(const DistributedPipelineHandle&) = default;

DistributedPipelineHandle::DistributedPipelineHandle(DistributedPipelineHandle&&) = default;

DistributedPipelineHandle& DistributedPipelineHandle::operator=(const DistributedPipelineHandle&) = default;

DistributedPipelineHandle& DistributedPipelineHandle::operator=(DistributedPipelineHandle&&) = default;

DistributedPipelineHandle::~DistributedPipelineHandle() = default;

DistributedPipelineHandle::operator bool() const {
    return static_cast<bool>(self);
}

Client DistributedPipelineHandle::client() const {
    return Client(self->m_client);
}

HashFunction DistributedPipelineHandle::getHashFunction() const {
    if(not self) throw Exception("Invalid colza::DistributedPipelineHandle object");
    return self->m_hash;
}

void DistributedPipelineHandle::setHashFunction(const HashFunction& hash) {
    if(not self) throw Exception("Invalid colza::DistributedPipelineHandle object");
    self->m_hash = hash;
}

void DistributedPipelineHandle::stage(const std::string& dataset_name,
           uint64_t iteration,
           uint64_t block_id,
           const std::vector<size_t>& dimensions,
           const std::vector<int64_t>& offsets,
           const Type& type,
           const thallium::bulk& data,
           const std::string& origin_addr,
           int32_t* result,
           AsyncRequest* req) const {
    if(not self) throw Exception("Invalid colza::DistributedPipelineHandle object");
    if(self->m_pipelines.size() == 0)
        throw Exception("No concrete pipeline attached to colza::DistributedPipelineHandle object");
    auto h = self->m_hash(dataset_name, iteration, block_id);
    auto i = h % self->m_pipelines.size();
    auto pipeline = PipelineHandle(self->m_pipelines[i]);
    pipeline.stage(dataset_name,
                   iteration,
                   block_id,
                   dimensions,
                   offsets,
                   type,
                   data,
                   origin_addr,
                   result,
                   req);
}

void DistributedPipelineHandle::stage(const std::string& dataset_name,
           uint64_t iteration,
           uint64_t block_id,
           const std::vector<size_t>& dimensions,
           const std::vector<int64_t>& offsets,
           const Type& type,
           const void* data,
           int32_t* result,
           AsyncRequest* req) const {
    if(not self) throw Exception("Invalid colza::DistributedPipelineHandle object");
    if(self->m_pipelines.size() == 0)
        throw Exception("No concrete pipeline attached to colza::DistributedPipelineHandle object");
    auto h = self->m_hash(dataset_name, iteration, block_id);
    auto i = h % self->m_pipelines.size();
    auto pipeline = PipelineHandle(self->m_pipelines[i]);
    pipeline.stage(dataset_name,
                   iteration,
                   block_id,
                   dimensions,
                   offsets,
                   type,
                   data,
                   result,
                   req);
}

void DistributedPipelineHandle::execute(uint64_t iteration,
             int32_t* result,
             AsyncRequest* req) const {
    if(not self) throw Exception("Invalid colza::DistributedPipelineHandle object");
    self->m_comm->barrier();
    if(self->m_pipelines.size() == 0)
        return;
    auto num_pipelines = self->m_pipelines.size();
    int num_processes = self->m_comm->size();
    int my_rank = self->m_comm->rank();
    if(my_rank != 0) return;

    auto& rpc = self->m_client->m_execute;
    std::vector<uint32_t> results(num_pipelines);
    std::vector<tl::async_response> async_responses;

    for(auto& pipeline : self->m_pipelines) {
        auto async_response = rpc.on(pipeline->m_ph).async(pipeline->m_name, iteration);
        async_responses.push_back(std::move(async_response));
    }

    auto async_request_impl =
        std::make_shared<AsyncRequestImpl>(std::move(async_responses));

    async_request_impl->m_wait_callback =
            [result](AsyncRequestImpl& async_request_impl) {
                    RequestResult<int32_t> response;
                    for(auto& r : async_request_impl.m_async_responses) {
                        RequestResult<int32_t> local_response = r.wait();
                        if(!local_response.success())
                            response = local_response;
                    }
                    async_request_impl.m_async_responses.clear();
                    if(response.success()) {
                        if(result) *result = response.value();
                    } else {
                        throw Exception(response.error());
                    }
            };
    if(req)
        *req = AsyncRequest(std::move(async_request_impl));
    else
        AsyncRequest(std::move(async_request_impl)).wait();
}

void DistributedPipelineHandle::cleanup(uint64_t iteration,
             int32_t* result,
             AsyncRequest* req) const {
    if(not self) throw Exception("Invalid colza::DistributedPipelineHandle object");
    self->m_comm->barrier();
    if(self->m_pipelines.size() == 0)
        return;
    auto num_pipelines = self->m_pipelines.size();
    int num_processes = self->m_comm->size();
    int my_rank = self->m_comm->rank();
    if(my_rank != 0) return;

    auto& rpc = self->m_client->m_cleanup;
    std::vector<uint32_t> results(num_pipelines);
    std::vector<tl::async_response> async_responses;

    for(auto& pipeline : self->m_pipelines) {
        auto async_response = rpc.on(pipeline->m_ph).async(pipeline->m_name, iteration);
        async_responses.push_back(std::move(async_response));
    }

    auto async_request_impl =
        std::make_shared<AsyncRequestImpl>(std::move(async_responses));

    async_request_impl->m_wait_callback =
            [result](AsyncRequestImpl& async_request_impl) {
                    RequestResult<int32_t> response;
                    for(auto& r : async_request_impl.m_async_responses) {
                        RequestResult<int32_t> local_response = r.wait();
                        if(!local_response.success())
                            response = local_response;
                    }
                    async_request_impl.m_async_responses.clear();
                    if(response.success()) {
                        if(result) *result = response.value();
                    } else {
                        throw Exception(response.error());
                    }
            };
    if(req)
        *req = AsyncRequest(std::move(async_request_impl));
    else
        AsyncRequest(std::move(async_request_impl)).wait();
}
}
