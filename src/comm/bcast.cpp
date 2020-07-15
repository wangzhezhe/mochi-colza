#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

namespace colza {

int bcast_binomial_tree(communicator* comm, void* data, size_t nbytes,
                        int root);

int bcast_sequential(communicator* comm, void* data, size_t nbytes, int root);

int communicator::bcast(void* data, size_t nbytes, int root,
                        COLZA_Bcast types) {
  int status;
  switch (types) {
    case COLZA_Bcast::sequential:
      status = bcast_sequential(this, data, nbytes, root);
      break;

    case COLZA_Bcast::binomial:
      status = bcast_binomial_tree(this, data, nbytes, root);
      break;

    default:
      status = bcast_binomial_tree(this, data, nbytes, root);
      break;
  }

  return status;
}

int communicator::ibcast(void* data, size_t nbytes, int root, request& req,
                         COLZA_Bcast types) {
  request tmp(std::make_shared<tl::eventual<void>>());
  auto eventual = tmp.m_eventual;
  m_controller->m_pool.make_thread(
      [data, nbytes, root, eventual, types, this]() {
        bcast(data, nbytes, root, types);
        eventual->set_value();
      },
      tl::anonymous());
  req = std::move(tmp);
  return 0;
}

// refer to the tree based algorithm figure listed in
// https://mpitutorial.com/tutorials/mpi-broadcast-and-collective-communication/
// refer to the implementation based on
// https://github.com/pmodels/mpich/blob/master/src/mpi/coll/bcast/bcast_intra_binomial.c
int bcast_binomial_tree(communicator* comm, void* data, size_t nbytes,
                        int root) {
  int rank, comm_size, src, dst;
  int relative_rank, mask;
  int status;

  comm_size = comm->size();
  rank = comm->rank();

  // If there is only one process, return
  if (comm_size == 1) return 0;

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

int bcast_sequential(communicator* comm, void* data, size_t nbytes, int root) {
  int comm_size = comm->size();
  int rank = comm->rank();
  int status;
  if (rank == root) {
    for (int dest = 0; dest < comm_size; dest++) {
      if (dest != root) {
        status = comm->send(data, nbytes, dest, COLZA_BCAST_TAG);
        if (status != 0) {
          return status;
        }
      }
    }
  } else {
    status = comm->recv(data, nbytes, root, COLZA_BCAST_TAG);
    if (status != 0) {
      return status;
    }
  }

  return 0;
}

}  // namespace colza
