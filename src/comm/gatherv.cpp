#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

namespace colza {

// refer to
// https://github.com/pmodels/mpich/blob/master/src/mpi/coll/gatherv/gatherv_allcomm_linear.c
int communicator::gatherv(const void *sendBuffer, void *recvBuffer,
                          size_t sendCounts, size_t *recvCounts,
                          size_t *offsets, size_t elementSize, int root) {
  int status;
  int comm_size = this->size();
  int rank = this->rank();

  // the recv buffer is only siginificant for the root process
  if (rank == root) {
    for (int i = 0; i < comm_size; i++) {
      // recv from other process
      if (i == rank) {
        // do the local copy for the root process
        // offset[i] represents the number of the element relative to the
        // recvBuffer for rank i
        if (sendBuffer != COLZA_IN_PLACE) {
          memcpy((char *)recvBuffer + offsets[i] * elementSize, sendBuffer,
                 recvCounts[i] * elementSize);
        }
        continue;
      }
      // recv from the other processes if the send account is not zero
      if (recvCounts[i] != 0) {
        status = this->recv((char *)recvBuffer + offsets[i] * elementSize,
                            sendCounts * elementSize, i, COLZA_GATHERV_TAG);
        if (status != 0) {
          std::cerr << "faild to gatherv because of rcv failure for rank "
                    << rank << " with status " << status << std::endl;
          return status;
        }
      }
    }
  } else {
    // send to the root process
    status = this->send(sendBuffer, sendCounts * elementSize, root,
                        COLZA_GATHERV_TAG);
    if (status != 0) {
      std::cerr << "faild to gatherv because of send failure for rank " << rank
                << " with status " << status << std::endl;

      return status;
    }
  }
  return 0;
}

int communicator::igatherv(const void *sendBuffer, void *recvBuffer,
                           size_t sendCounts, size_t *recvCounts,
                           size_t *offsets, size_t elementSize, int root,
                           request &req) {
  request tmp(std::make_shared<tl::eventual<void>>());
  auto eventual = tmp.m_eventual;
  m_controller->m_pool.make_thread(
      [sendBuffer, recvBuffer, sendCounts, recvCounts, offsets, elementSize,
       root, eventual, this]() {
        gatherv(sendBuffer, recvBuffer, sendCounts, recvCounts, offsets,
                elementSize, root);
        eventual->set_value();
      },
      tl::anonymous());
  req = std::move(tmp);
  return 0;
}

}  // namespace colza
