
#include "colza/communicator.hpp"
#include "colza/request.hpp"

namespace colza {
int communicator::wait(request& req) {
    if(!req) return -1;
    req.m_eventual->wait();
    return 0;
}

int communicator::waitAny(int count, request* reqList) {
  // TODO this is an active loop, we should change it
  // when Argobots provide an ABT_eventual_wait_any
  size_t i;
  int ret;
  bool flag = false;
  bool has_pending_requests;
try_again:
  has_pending_requests = false;
  for (i = 0; i < count; i++) {
    if(!reqList[i].m_eventual)
        continue;
    has_pending_requests = true;
    flag = reqList[i].m_eventual->test();
    if (flag) {
      this->wait(reqList[i]);
      return i;
    }
  }
  tl::thread::yield();
  if (has_pending_requests) goto try_again;
  // there is not pending request
  // TODO add more error status
  return -1;
}

}  // namespace colza
