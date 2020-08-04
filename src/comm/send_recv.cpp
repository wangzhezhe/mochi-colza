#include <algorithm>
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
  if(size==0){
    // do not send the rpc if the size is zero
    return 0;
  }
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
  request tmp(std::make_shared<tl::eventual<void>>());
  auto eventual = tmp.m_eventual;
  m_controller->m_pool.make_thread(
      [data, size, dest, tag, eventual, this]() {
        send(data, size, dest, tag);
        eventual->set_value();
      },
      tl::anonymous());
  req = std::move(tmp);
  return 0;
}

std::list<communicator::key_req>::iterator communicator::req_exist(
    std::list<key_req> &req_list, int src, int tag) {
  std::list<key_req>::iterator key_equal;
  if (src == -1) {
    // any source
    key_equal =
        std::find_if(req_list.begin(), req_list.end(),
                     [src, tag](const key_req &k) { return k.m_tag == tag;});

  } else {
    key_equal = std::find_if(req_list.begin(), req_list.end(),
                             [src, tag](const key_req &k) {
                               return k.m_tag == tag && k.m_source == src;
                             });
  }
  return key_equal;
}

int communicator::recv(void *data, size_t size, int src, int tag) {
  // create local bulk handle
  if(size==0){
    // do not recv the rpc if the size is zero
    return 0;
  }
  std::vector<std::pair<void *, size_t>> segment(1);
  segment[0].first = const_cast<void *>(data);
  segment[0].second = size;
  tl::bulk local_bulk =
      m_controller->get_engine().expose(segment, tl::bulk_mode::write_only);
  p2p_request *req_ptr = nullptr;

  // lock the pending request map to see if there is a matching request
  {
    std::unique_lock<tl::mutex> lock(m_pending_p2p_requests_mtx);
    auto key_equal = req_exist(m_pending_p2p_requests, src, tag);
    if (key_equal == m_pending_p2p_requests.end()) {
      // not find, wait
      m_pending_p2p_requests_cv.wait(lock, [this, src, tag, &key_equal]() {
        key_equal = req_exist(m_pending_p2p_requests, src, tag);
        return (key_equal != m_pending_p2p_requests.end());
      });
    };

    // hold the pointer and delete the entry
    req_ptr = key_equal->m_req;
    m_pending_p2p_requests.erase(key_equal);
  }
  // TODO check that size matches req_ptr->m_size
  local_bulk << req_ptr->m_bulk->on(*req_ptr->m_endpoint);
  req_ptr->m_eventual.set_value();
  return 0;
}

int communicator::irecv(void *data, size_t size, int src, int tag,
                        request &req) {
  request tmp(std::make_shared<tl::eventual<void>>());
  auto eventual = tmp.m_eventual;
  m_controller->m_pool.make_thread(
      [data, size, src, tag, eventual, this]() {
        recv(data, size, src, tag);
        eventual->set_value();
      },
      tl::anonymous());
  req = std::move(tmp);
  return 0;
}

// refer to
// https://github.com/pmodels/mpich/blob/master/src/mpi/coll/helper_fns.c#L282
int communicator::sendrecv(void *sendbuf, size_t sendSize, int dest,
                           int sendtag, void *recvbuf, size_t recvSize,
                           int source, int recvtag) {
  int status;
  colza::request sendreq, recvreq;

  // TODO add the situation that source or dest is null
  status = irecv(recvbuf, recvSize, source, recvtag, recvreq);
  if (status != 0) {
    return status;
  }

  status = isend(sendbuf, sendSize, dest, sendtag, sendreq);
  if (status != 0) {
    return status;
  }

  status = wait(sendreq);
  if (status != 0) {
    return status;
  }
  status = wait(recvreq);
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

  {
    std::unique_lock<tl::mutex> lock(m_pending_p2p_requests_mtx);
    // m_pending_p2p_requests[sig] = &req;
    m_pending_p2p_requests.push_back(key_req(source, tag, &req));
  }

  //for the test case isend_irecv_complex
  //it is important to use notify_all instead of the notify_one
  //otherwise, the irecv coresponds with the isend might not be notified precisely
  //and the program will hang there
  m_pending_p2p_requests_cv.notify_all();
  
  req.m_eventual.wait();
  return 0;
}

}  // namespace colza
