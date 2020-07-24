#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

namespace tl = thallium;

namespace colza {

int communicator::destroy() {
  // barrier operation based on dissemination
  int size, rank, src, dst, mask;
  size = this->size();
  // Trivial barriers return immediately
  if (size == 1) return 0;
  char buffer = 'b';
  // bcast to all the ranks
  // 0-byte bcast will just return without doing anything.
  int status = this->bcast(&buffer, sizeof(char), 0);
  if (status != 0) {
    // failed to execute sendrecv
    return status;
  }

  // delete the current communicator from the controller
  {
    std::lock_guard<tl::mutex> lck(this->m_controller->m_comm_mutex);
    this->m_controller->m_communicators.erase(this->m_comm_id);
  }

  return 0;
}

}  // namespace colza
