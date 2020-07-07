
#include "colza/communicator.hpp"
#include "colza/request.hpp"

namespace colza {
int communicator::wait(request& req) { return req.wait(); }

int communicator::waitAny(int count, request* reqList) {
  // TODO this is an active loop, we should change it
  // when Argobots provide an ABT_eventual_wait_any
  size_t i;
  int ret;
  bool flag = false;
  int has_pending_requests = 0;
try_again:
  for (i = 0; i < count; i++) {
    has_pending_requests = 1;
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
  return count;
}

}  // namespace colza