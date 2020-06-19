#include <thallium.hpp>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

namespace colza {

namespace tl = thallium;

int communicator::send(const void *data, size_t size, int dest, int tag) {
  // request endpoint from member id
  tl::provider_handle dest_pr =
      m_controller->member_id_to_provider_handle(m_members[dest]);
  // create bulk handle to expose the data
  std::vector<std::pair<void *, size_t>> segment(1);
  segment[0].first = const_cast<void *>(data);
  segment[0].second = size;
  tl::bulk local_bulk =
      m_controller->get_engine().expose(segment, tl::bulk_mode::read_only);
  // send the RPC
  int ret = m_controller->m_p2p_transfer_rpc.on(dest_pr)(
      m_comm_id, local_bulk, size, (int32_t)m_rank, (int32_t)tag);
  return ret;
}

int communicator::isend(const void *data, size_t size, int dest, int tag,
                        request &req) {
  m_controller->m_pool.make_thread(
      [data, size, dest, tag, &req, this]() {
        send(data, size, dest, tag);
        req.m_eventual.set_value();
      },
      tl::anonymous());
  return 0;
}

int communicator::recv(void *data, size_t size, int src, int tag) {
  // create local bulk handle
  std::vector<std::pair<void *, size_t>> segment(1);
  segment[0].first = const_cast<void *>(data);
  segment[0].second = size;
  tl::bulk local_bulk =
      m_controller->get_engine().expose(segment, tl::bulk_mode::write_only);
  p2p_request *req_ptr = nullptr;
  // lock the pending request map to see if there is a matching request
  {
    uint64_t sig = ((uint64_t)src << 32) | tag;
    std::unique_lock<tl::mutex> lock(m_pending_p2p_requests_mtx);
    if(m_pending_p2p_requests.count(sig) == 0) {
        m_pending_p2p_requests_cv.wait(
            lock, [this, sig]() { return m_pending_p2p_requests.count(sig) != 0; });
    }
    auto it = m_pending_p2p_requests.find(sig);
    req_ptr = it->second;
    m_pending_p2p_requests.erase(sig);
  }
  // TODO check that size matches req_ptr->m_size
  local_bulk << req_ptr->m_bulk->on(*req_ptr->m_endpoint);
  req_ptr->m_eventual.set_value();
  return 0;
}

int communicator::irecv(void *data, size_t size, int src, int tag,
                        request &req) {
  m_controller->m_pool.make_thread(
      [data, size, src, tag, &req, this]() {
        recv(data, size, src, tag);
        req.m_eventual.set_value();
      },
      tl::anonymous());
  return 0;
}

// refer to
// https://github.com/pmodels/mpich/blob/master/src/mpi/coll/helper_fns.c#L282
int communicator::sendrecv(void *sendbuf, int sendcount, size_t sendsize,
                           int dest, int sendtag, void *recvbuf, int recvcount,
                           size_t recvsize, int source, int recvtag) {
  int status;
  colza::request sendreq, recvreq;

  // TODO add the situation that source or dest is null
  status = irecv(recvbuf, recvsize * recvcount, source, recvtag, recvreq);
  if (status != 0) {
    return status;
  }

  status = isend(sendbuf, sendsize * sendcount, dest, sendtag, sendreq);
  if (status != 0) {
    return status;
  }

  status = sendreq.wait();
  if (status != 0) {
    return status;
  }
  status = recvreq.wait();
  if (status != 0) {
    return status;
  }

  return status;
}

int communicator::on_p2p_transfer(const tl::endpoint &ep, tl::bulk &remote_bulk,
                                  size_t size, int32_t source, int32_t tag) {
  p2p_request req;
  req.m_bulk = &remote_bulk;
  req.m_endpoint = &ep;
  req.m_size = size;
  uint64_t sig = ((uint64_t)source << 32) | tag;
  {
    std::unique_lock<tl::mutex> lock(m_pending_p2p_requests_mtx);
    m_pending_p2p_requests[sig] = &req;
  }
  m_pending_p2p_requests_cv.notify_one();
  req.m_eventual.wait();
  return 0;
}

}  // namespace colza
