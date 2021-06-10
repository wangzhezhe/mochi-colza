/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "colza/DistributedPipelineHandle.hpp"

#include <thallium/serialization/stl/pair.hpp>
#include <thallium/serialization/stl/string.hpp>

#include "AsyncRequestImpl.hpp"
#include "ClientImpl.hpp"
#include "DistributedPipelineHandleImpl.hpp"
#include "PipelineHandleImpl.hpp"
#include "TypeSizes.hpp"
#include "colza/Exception.hpp"
#include "colza/RequestResult.hpp"

namespace colza {

DistributedPipelineHandle::DistributedPipelineHandle() = default;

DistributedPipelineHandle::DistributedPipelineHandle(
    const std::shared_ptr<DistributedPipelineHandleImpl>& impl)
    : self(impl) {}

DistributedPipelineHandle::DistributedPipelineHandle(
    const DistributedPipelineHandle&) = default;

DistributedPipelineHandle::DistributedPipelineHandle(
    DistributedPipelineHandle&&) = default;

DistributedPipelineHandle& DistributedPipelineHandle::operator=(
    const DistributedPipelineHandle&) = default;

DistributedPipelineHandle& DistributedPipelineHandle::operator=(
    DistributedPipelineHandle&&) = default;

DistributedPipelineHandle::~DistributedPipelineHandle() = default;

DistributedPipelineHandle::operator bool() const {
  return static_cast<bool>(self);
}

Client DistributedPipelineHandle::client() const {
  return Client(self->m_client);
}

HashFunction DistributedPipelineHandle::getHashFunction() const {
  if (not self)
    throw Exception(ErrorCode::INVALID_INSTANCE,
                    "Invalid colza::DistributedPipelineHandle object");
  return self->m_hash;
}

void DistributedPipelineHandle::setHashFunction(const HashFunction& hash) {
  if (not self)
    throw Exception(ErrorCode::INVALID_INSTANCE,
                    "Invalid colza::DistributedPipelineHandle object");
  self->m_hash = hash;
}

void DistributedPipelineHandle::start(uint64_t iteration) {
  if (not self)
    throw Exception(ErrorCode::INVALID_INSTANCE,
                    "Invalid colza::DistributedPipelineHandle object");
  spdlog::trace("iteration {} start debug 1", iteration);
  self->m_comm->barrier();
  int my_rank = self->m_comm->rank();

  auto& start = self->m_client->m_start;
  auto& abort = self->m_client->m_abort;

  bool ok = false;
  bool retry = true;
  bool first_attempt = true;
  spdlog::trace("iteration {} start debug 2", iteration);

  spdlog::trace("iteration {} Updating view of SSG group", iteration);
  // this is used to handle the process leave
  // there is ssg issue when makeDistributedPipelineHandle is called multiple times
  /* we do not set this if there is no move operation
  auto new_dist_pipeline_temp =
      Client(self->m_client)
          .makeDistributedPipelineHandle(self->m_comm, self->m_ssg_group_file,
                                         self->m_provider_id, self->m_name,
                                         false);
  self = std::move(new_dist_pipeline_temp.self);
  */
  if (my_rank == 0) {
    while (retry) {
      // debug, try to updating the ssg view anyway since maybe the process can
      // leave before the start call
      if (!first_attempt) {
        spdlog::trace("iteration {} first_attempt updating view of SSG group",
                      iteration);
        auto new_dist_pipeline =
            Client(self->m_client)
                .makeDistributedPipelineHandle(
                    self->m_comm, self->m_ssg_group_file, self->m_provider_id,
                    self->m_name, false);
        self = std::move(new_dist_pipeline.self);
      }

      retry = false;

      const auto group_hash = self->m_group_hash;
      auto num_pipelines = self->m_pipelines.size();

      std::vector<PipelineHandle*> started;
      started.reserve(num_pipelines);
      ok = true;
      spdlog::trace(
          "iteration {} Sending a start command to {} pipelines, with "
          "group_hash = {}",
          iteration, num_pipelines, self->m_group_hash);

      // do not use async here
      // std::vector<tl::async_response> async_responses;

      for (auto& pipeline : self->m_pipelines) {
        // auto async_response = start.on(pipeline.self->m_ph).async(
        //        group_hash, pipeline.self->m_name, iteration);
        // async_responses.push_back(std::move(async_response));
        RequestResult<int32_t> result = start.on(pipeline.self->m_ph)(
            group_hash, pipeline.self->m_name, iteration);

        if (!result.success()) {
          ok = false;
          if ((retry == false) &&
              (result.value() == (int)ErrorCode::INVALID_GROUP_HASH)) {
            spdlog::warn(
                "Invalid group hash detected, group view needs to be updated");
            retry = true;
          }
        } else {
          started.push_back(&pipeline);
        }
      }

      /*
      for(unsigned i = 0; i < async_responses.size(); i++) {
          auto& a = async_responses[i];
          auto& p = self->m_pipelines[i];
          spdlog::trace("iteration {} total response size {} current index
      {}",iteration, async_responses.size(), i); RequestResult<int32_t> result =
      a.wait(); spdlog::trace("iteration {} index {} wait finish", iteration,
      i); if(!result.success()) { ok = false; if((retry == false) &&
      (result.value() == (int)ErrorCode::INVALID_GROUP_HASH)) {
                  spdlog::warn("Invalid group hash detected, group view needs to
      be updated"); retry = true;
              }
          } else {
              started.push_back(&p);
          }
      }

      async_responses.clear();

      spdlog::info("iteration {} async_responses finish clear",iteration);
      */

      self->m_comm->bcast(&ok, sizeof(ok), 0);
      if (!ok) {  // one RPC failed to be sent, send "abort to the
        spdlog::trace("start not ok section");
        for (auto pipeline : started) {
          try {
            abort.on(pipeline->self->m_ph)(pipeline->self->m_name, iteration);
            // async_responses.push_back(std::move(async_response));
          } catch (...) {
            spdlog::error(
                "Could not abort iteration on pipeline {} at address {}",
                pipeline->self->m_name,
                static_cast<std::string>(pipeline->self->m_ph));
          }
        }
        // for(auto& a : async_responses) {
        //    a.wait();
        //}
        // async_responses.clear();
        if (!first_attempt) {
          // if it's the second attempt already, slow down querying the file
          tl::thread::sleep(self->m_client->m_engine, 100);
        }
      }

      first_attempt = false;
    }

  } else {  // non-0 ranks
    // TODO add condition here, if we removed sth, then update it

    spdlog::trace("iteration {} start debug 3", iteration);
    self->m_comm->bcast(&ok, sizeof(ok), 0);
    spdlog::trace("iteration {} non-0 debug 4 bcast 1", iteration);

    // update client when there is inconsistency, is it possible that only this
    // part is called?
    while (not ok) {
      spdlog::trace("iteration {} non-0 debug 5", iteration);
      auto new_dist_pipeline =
          Client(self->m_client)
              .makeDistributedPipelineHandle(
                  self->m_comm, self->m_ssg_group_file, self->m_provider_id,
                  self->m_name, false);
      spdlog::trace("iteration {} non-0 debug 6", iteration);
      self = std::move(new_dist_pipeline.self);
      spdlog::trace("iteration {} non-0 debug 7", iteration);
      self->m_comm->bcast(&ok, sizeof(ok), 0);
      spdlog::trace("iteration {} non-0 debug 8 bcast 2", iteration);
    }
  }
}

void DistributedPipelineHandle::stage(
    const std::string& dataset_name, uint64_t iteration, uint64_t block_id,
    const std::vector<size_t>& dimensions, const std::vector<int64_t>& offsets,
    const Type& type, const thallium::bulk& data,
    const std::string& origin_addr, int32_t* result, AsyncRequest* req) const {
  if (not self)
    throw Exception(ErrorCode::INVALID_INSTANCE,
                    "Invalid colza::DistributedPipelineHandle object");
  if (self->m_pipelines.size() == 0)
    throw Exception(ErrorCode::EMPTY_DIST_PIPELINE,
                    "No concrete pipeline attached to "
                    "colza::DistributedPipelineHandle object");
  auto h = self->m_hash(dataset_name, iteration, block_id);
  auto i = h % self->m_pipelines.size();
  auto pipeline = PipelineHandle(self->m_pipelines[i]);
  pipeline.stage(dataset_name, iteration, block_id, dimensions, offsets, type,
                 data, origin_addr, result, req);
}

void DistributedPipelineHandle::stage(const std::string& dataset_name,
                                      uint64_t iteration, uint64_t block_id,
                                      const std::vector<size_t>& dimensions,
                                      const std::vector<int64_t>& offsets,
                                      const Type& type, const void* data,
                                      int32_t* result,
                                      AsyncRequest* req) const {
  if (not self)
    throw Exception(ErrorCode::INVALID_INSTANCE,
                    "Invalid colza::DistributedPipelineHandle object");
  if (self->m_pipelines.size() == 0)
    throw Exception(ErrorCode::EMPTY_DIST_PIPELINE,
                    "No concrete pipeline attached to "
                    "colza::DistributedPipelineHandle object");
  auto h = self->m_hash(dataset_name, iteration, block_id);
  auto i = h % self->m_pipelines.size();
  auto pipeline = PipelineHandle(self->m_pipelines[i]);
  pipeline.stage(dataset_name, iteration, block_id, dimensions, offsets, type,
                 data, result, req);
}

void DistributedPipelineHandle::execute(uint64_t iteration, int32_t* result,
                                        bool autoCleanup,
                                        AsyncRequest* req) const {
  if (not self)
    throw Exception(ErrorCode::INVALID_INSTANCE,
                    "Invalid colza::DistributedPipelineHandle object");
  self->m_comm->barrier();
  if (self->m_pipelines.size() == 0) return;
  auto num_pipelines = self->m_pipelines.size();
  int num_processes = self->m_comm->size();
  (void)num_processes;  // not useful for now
  int my_rank = self->m_comm->rank();
  if (my_rank != 0) return;

  auto& rpc = self->m_client->m_execute;
  std::vector<uint32_t> results(num_pipelines);
  std::vector<tl::async_response> async_responses;

  for (auto& pipeline : self->m_pipelines) {
    auto async_response =
        rpc.on(pipeline.self->m_ph)
            .async(pipeline.self->m_name, iteration, autoCleanup);
    async_responses.push_back(std::move(async_response));
  }

  auto async_request_impl =
      std::make_shared<AsyncRequestImpl>(std::move(async_responses));

  async_request_impl->m_wait_callback =
      [result](AsyncRequestImpl& async_request_impl) {
        RequestResult<int32_t> response;
        for (auto& r : async_request_impl.m_async_responses) {
          RequestResult<int32_t> local_response = r.wait();
          if (!local_response.success()) response = local_response;
        }
        async_request_impl.m_async_responses.clear();
        if (response.success()) {
          if (result) *result = response.value();
        } else {
          throw Exception((ErrorCode)response.value(), response.error());
        }
      };
  if (req)
    *req = AsyncRequest(std::move(async_request_impl));
  else
    AsyncRequest(std::move(async_request_impl)).wait();
}

void DistributedPipelineHandle::cleanup(uint64_t iteration, int32_t* result,
                                        AsyncRequest* req) const {
  if (not self)
    throw Exception(ErrorCode::INVALID_INSTANCE,
                    "Invalid colza::DistributedPipelineHandle object");
  self->m_comm->barrier();
  if (self->m_pipelines.size() == 0) return;
  auto num_pipelines = self->m_pipelines.size();
  int num_processes = self->m_comm->size();
  (void)num_processes;  // not useful for now
  int my_rank = self->m_comm->rank();
  if (my_rank != 0) return;

  auto& rpc = self->m_client->m_cleanup;
  std::vector<uint32_t> results(num_pipelines);
  std::vector<tl::async_response> async_responses;

  for (auto& pipeline : self->m_pipelines) {
    auto async_response =
        rpc.on(pipeline.self->m_ph).async(pipeline.self->m_name, iteration);
    async_responses.push_back(std::move(async_response));
  }

  auto async_request_impl =
      std::make_shared<AsyncRequestImpl>(std::move(async_responses));

  async_request_impl->m_wait_callback =
      [result](AsyncRequestImpl& async_request_impl) {
        RequestResult<int32_t> response;
        for (auto& r : async_request_impl.m_async_responses) {
          RequestResult<int32_t> local_response = r.wait();
          if (!local_response.success()) response = local_response;
        }
        async_request_impl.m_async_responses.clear();
        if (response.success()) {
          if (result) *result = response.value();
        } else {
          throw Exception((ErrorCode)response.value(), response.error());
        }
      };
  if (req)
    *req = AsyncRequest(std::move(async_request_impl));
  else
    AsyncRequest(std::move(async_request_impl)).wait();
}

void DistributedPipelineHandle::updateComm(ClientCommunicator* updatedComm) {
  this->self->m_comm = updatedComm;
};

}  // namespace colza
