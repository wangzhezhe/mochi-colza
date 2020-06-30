#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

namespace colza {
// refer to
// linear gather
// https://github.com/pmodels/mpich/blob/master/src/mpi/coll/gather/gather_inter_linear.c
int communicator::gather(const void *sendBuffer, size_t sendSize,
                         void *recvBuffer, int root) {
  // sendSize and recvSize is the size for every elements, and they should equal
  // with each other
  int status;
  int comm_size = this->size();
  int rank = this->rank();

  if (rank == root) {
    for (int i = 0; i < comm_size; i++) {
      // recv from other process
      if (i == rank) {
        // do the local copy for the root process
        memcpy((char *)recvBuffer + i * sendSize, sendBuffer, sendSize);
        continue;
      }
      status = this->recv((char *)recvBuffer + i * sendSize, sendSize, i,
                          COLZA_GATHER_TAG);
      if (status != 0) {
        std::cerr << "faild to gather because of rcv failure for rank " << rank
                  << " with status " << status << std::endl;
        return status;
      }
    }
  } else {
    // send to the root process
    status = this->send(sendBuffer, sendSize, root, COLZA_GATHER_TAG);
    if (status != 0) {
      std::cerr << "faild to gather because of send failure for rank " << rank
                << " with status " << status << std::endl;

      return status;
    }
  }
  return 0;
}

int communicator::igather(const void *sendBuffer, size_t sendSize,
                          void *recvBuffer, int root, request &req) {
  m_controller->m_pool.make_thread(
      [sendBuffer, sendSize, recvBuffer, root, &req, this]() {
        gather(sendBuffer, sendSize, recvBuffer, root);
        req.m_eventual.set_value();
      },
      tl::anonymous());
  return 0;
}

}  // namespace colza
