#ifndef __COLZA_COMM_HPP
#define __COLZA_COMM_HPP

#include <mpi.h>
#include <ssg.h>

#include <colza/tags.hpp>
#include <colza/types.hpp>
#include <colza/uuid.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <thallium.hpp>
#include <unordered_map>
#include <vector>

namespace colza {

namespace tl = thallium;

class controller;
class request;

class communicator {
  friend class controller;

 public:
  communicator(const communicator &) = delete;
  communicator(communicator &&) = delete;
  communicator &operator=(const communicator &) = delete;
  communicator &operator=(communicator &&) = delete;
  ~communicator() = default;

  size_t size() const;
  size_t rank() const;

  int send(const void *data, size_t size, int dest, int tag);
  int isend(const void *data, size_t size, int dest, int tag, request &req);
  int recv(void *data, size_t size, int src, int tag);
  int irecv(void *data, size_t size, int src, int tag, request &req);
  int sendrecv(void *sendbuf, size_t sendSize, int dest, int sendtag,
               void *recvbuf, size_t recvSize, int source, int recvtag);
  int barrier(COLZA_Barrier types = COLZA_Barrier::bcast);
  int ibarrier(request &req, COLZA_Barrier types = COLZA_Barrier::bcast);
  int bcast(void *data, size_t nbytes, int root,
            COLZA_Bcast types = COLZA_Bcast::binomial);
  int ibcast(void *data, size_t nbytes, int root, request &req,
             COLZA_Bcast types = COLZA_Bcast::binomial);
  int gather(const void *sendBuffer, size_t sendSize, void *recvBuffer,
             int root);
  int igather(const void *sendBuffer, size_t sendSize, void *recvBuffer,
              int root, request &req);
  int gatherv(const void *sendBuffer, void *recvBuffer, size_t sendCounts,
              size_t *recvCounts, size_t *offsets, size_t elementSize,
              int root);
  int igatherv(const void *sendBuffer, void *recvBuffer, size_t sendCounts,
               size_t *recvCounts, size_t *offsets, size_t elementSize,
               int root, request &req);
  int scatter(const void *sendBuffer, void *recvBuffer, size_t size, int src);
  int iscatter(const void *sendBuffer, void *recvBuffer, size_t size, int src,
               request &req);
  int scatterv(const void *sendBuffer, void *recvBuffer, size_t *sendLengths,
               size_t *offsets, size_t recvLength, int src);
  int iscatterv(const void *sendBuffer, void *recvBuffer, size_t *sendLengths,
                size_t *offsets, size_t recvLength, int src, request &req);
  int allgather(const void *sendBuffer, void *recvBuffer, size_t dataSize);
  int iallgather(const void *sendBuffer, void *recvBuffer, size_t dataSize,
                 request &req);
  int allgatherv(const void *sendBuffer, void *recvBuffer, size_t sendLength,
                 size_t *recvLengths, size_t *offsets);
  int iallgatherv(const void *sendBuffer, void *recvBuffer, size_t sendLength,
                  size_t *recvLengths, size_t *offsets, request &req);
  int reduce(const void *sendBuffer, void *recvBuffer, size_t count,
             size_t elementSize, COLZA_Operation_Func opFunc, int root);
  int ireduce(const void *sendBuffer, void *recvBuffer, size_t count,
              size_t elementSize, COLZA_Operation_Func opFunc, int root,
              request &req);
  int allreduce(const void *sendBuffer, void *recvBuffer, size_t count,
                size_t elementSize, COLZA_Operation_Func opFunc);
  int iallreduce(const void *sendBuffer, void *recvBuffer, size_t count,
                 size_t elementSize, COLZA_Operation_Func opFunc, request &req);
  int alltoall(void *sendBuffer, size_t sendSize, void *recvBuffer,
               size_t recvSize);
  int ialltoall(void *sendBuffer, size_t sendSize, void *recvBuffer,
                size_t recvSize, request &req);

  int duplicate(std::shared_ptr<communicator>* newcommptr);

  int subset(std::shared_ptr<communicator>* newcommptr, int arrayLen, const int32_t *rankArray);

  int wait(request &req);

  int waitAny(int count, request *reqList);

  int destroy();

 private:
  communicator(controller *owner, size_t size, size_t rank,
               std::vector<ssg_member_id_t> members)
      : m_controller(owner),
        m_size(size),
        m_rank(rank),
        m_members(std::move(members)) {}

  int on_p2p_transfer(const tl::endpoint &ep, tl::bulk &remote_bulk,
                      size_t size, int32_t source, int32_t tag);

  controller *m_controller;
  UUID m_comm_id;
  size_t m_size;
  size_t m_rank;
  std::vector<ssg_member_id_t> m_members;

  struct p2p_request {
    tl::bulk *m_bulk;
    const tl::endpoint *m_endpoint;
    size_t m_size;
    tl::eventual<void> m_eventual;
  };

  std::unordered_map<uint64_t, p2p_request *> m_pending_p2p_requests;
  tl::mutex m_pending_p2p_requests_mtx;
  tl::condition_variable m_pending_p2p_requests_cv;
};

}  // namespace colza

#endif
