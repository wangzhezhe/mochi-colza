#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

namespace colza {
// gather to rank 0 firstly, and then bcast to all ranks
int communicator::allgather(const void *sendBuffer, void *recvBuffer,
                            size_t dataSize) {
  int status;
  int comm_size = this->size();
  int rank = this->rank();
  int root = 0;

  status = this->gather(sendBuffer, dataSize, recvBuffer, root);
  if (status != 0) {
    std::cerr << "failed to gather data for allgather" << std::endl;
    return status;
  }

  status = this->bcast(recvBuffer, comm_size * dataSize, root);
  if (status != 0) {
    std::cerr << "failed to bcast data for allgather" << std::endl;
    return status;
  }

  return 0;
}

int communicator::iallgather(const void *sendBuffer, void *recvBuffer,
                             size_t dataSize, request &req) {
  auto eventual = req.m_eventual;
  m_controller->m_pool.make_thread(
      [sendBuffer, recvBuffer, dataSize, eventual, this]() {
        allgather(sendBuffer, recvBuffer, dataSize);
        eventual->set_value();
      },
      tl::anonymous());
  return 0;
}

}  // namespace colza
