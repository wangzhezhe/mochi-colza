#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

namespace colza {

int bcast_binomial_tree(communicator* comm, void* data, int count,
                        size_t elem_size, int root);

int communicator::bcast(void* data, int count, size_t elem_size, int root) {
  int status = bcast_binomial_tree(this, data, count, elem_size, root);
  return status;
}

int communicator::ibcast(void* data, int count, size_t elem_size, int root,
                         request& req) {
  m_controller->m_pool.make_thread(
      [data, count, elem_size, root, &req, this]() {
        bcast(data, count, elem_size, root);
        req.m_eventual.set_value();
      },
      tl::anonymous());
  return 0;
}

// refer to the tree based algorithm figure listed in
// https://mpitutorial.com/tutorials/mpi-broadcast-and-collective-communication/
// refer to the implementation based on
// https://github.com/pmodels/mpich/blob/master/src/mpi/coll/bcast/bcast_intra_binomial.c
int bcast_binomial_tree(communicator* comm, void* data, int count,
                        size_t elem_size, int root) {
  int rank, comm_size, src, dst;
  int relative_rank, mask;
  size_t nbytes;
  int status;

  comm_size = comm->size();
  rank = comm->rank();

  // If there is only one process, return
  if (comm_size == 1) return 0;

  nbytes = elem_size * count;
  if (nbytes == 0) return 0;

  relative_rank = (rank >= root) ? rank - root : rank - root + comm_size;

  // Use short message algorithm, namely, binomial tree
  // operations to recieve the data
  mask = 0x1;
  while (mask < comm_size) {
    if (relative_rank & mask) {
      src = rank - mask;
      if (src < 0) src += comm_size;
      status = comm->recv(data, nbytes, src, COLZA_BCAST_TAG);
      if (status != 0) {
        return status;
      }
      break;
    }
    mask <<= 1;
  }
  mask >>= 1;

  // operations to send the data
  while (mask > 0) {
    if (relative_rank + mask < comm_size) {
      dst = rank + mask;
      if (dst >= comm_size) dst -= comm_size;
      status = comm->send(data, nbytes, dst, COLZA_BCAST_TAG);
      if (status != 0) {
        return status;
      }
    }
    mask >>= 1;
  }
  return 0;
}

}  // namespace colza