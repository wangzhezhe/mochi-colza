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
  rank = this->rank();
  mask = 0x1;
  int status;
  while (mask < size) {
    dst = (rank + mask) % size;
    src = (rank - mask + size) % size;
    // the size can not be empty for mercury
    char sendbuffer = 'b';
    char recvbuffer;
    status = this->sendrecv((void*)&sendbuffer, size_t(sizeof(char)), dst,
                            COLZA_DESTROY_TAG, (void*)&recvbuffer,
                            size_t(sizeof(char)), src, COLZA_DESTROY_TAG);
    if (status != 0) {
      // failed to execute sendrecv
      return status;
    }
    mask <<= 1;
  }

  // delete the current communicator from the controller
  {
    std::lock_guard<tl::mutex> lck(this->m_controller->m_comm_mutex);
    this->m_controller->m_communicators.erase(this->m_comm_id);
  }

  return 0;
}

}  // namespace colza
