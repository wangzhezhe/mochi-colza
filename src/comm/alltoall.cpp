#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

namespace colza {
// refer to
// https://github.com/pmodels/mpich/blob/master/src/mpi/coll/alltoall/alltoall_inter_pairwise_exchange.c
int communicator::alltoall(void* sendBuffer, size_t sendSize, void* recvBuffer,
                           size_t recvSize) {
  int status;
  int comm_size = this->size();
  int rank = this->rank();
  int src, dst;
  char* recvaddr = nullptr;
  char* sendaddr = nullptr;
  for (int i = 0; i < comm_size; i++) {
    src = (rank - i + comm_size) % comm_size;
    dst = (rank + i) % comm_size;
    recvaddr = (char*)sendBuffer + recvSize * src;
    sendaddr = (char*)recvBuffer + sendSize * dst;
    status = this->sendrecv(sendaddr, sendSize, dst, COLZA_ALLTOALL_TAG,
                            recvaddr, recvSize, src, COLZA_ALLTOALL_TAG);

    if (status != 0) {
      std::cerr << "failed to sendrecv in alltoall" << std::endl;
    }
  }
  return 0;
}

int communicator::ialltoall(void* sendBuffer, size_t sendSize, void* recvBuffer,
                            size_t recvSize, request& req) {
  auto eventual = req.m_eventual;
  this->m_controller->m_pool.make_thread(
      [sendBuffer, sendSize, recvBuffer, recvSize, eventual, this]() {
        alltoall(sendBuffer, sendSize, recvBuffer, recvSize);
        eventual->set_value();
      },
      tl::anonymous());
  return 0;
}

}  // namespace colza
