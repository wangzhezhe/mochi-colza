#include <thallium.hpp>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"
#include "colza/tags.hpp"
#include "colza/types.hpp"
namespace tl = thallium;

namespace colza {

int barrier_bcast(communicator* comm);
int barrier_dissemination(communicator* comm);

int communicator::barrier(COLZA_Barrier types) {
  int status;
  switch (types) {
    case COLZA_Barrier::bcast:
      status = barrier_bcast(this);
      break;

    case COLZA_Barrier::dissemination:
      status = barrier_dissemination(this);
      break;

    default:
      // use the bcast version if the types is invalid
      status = barrier_dissemination(this);
      break;
  }

  return status;
}

int communicator::ibarrier(request& req, COLZA_Barrier types) {
  request tmp(std::make_shared<tl::eventual<void>>());
  auto eventual = tmp.m_eventual;
  m_controller->m_pool.make_thread(
      [types, eventual, this]() {
        barrier(types);
        eventual->set_value();
      },
      tl::anonymous());
  req = std::move(tmp);
  return 0;
}

/*
refer to
https://github.com/pmodels/mpich/blob/master/src/mpi/coll/barrier/barrier_inter_bcast.c
*/
int barrier_bcast(communicator* comm) {
  int rank = comm->rank();
  int size = comm->size();
  int status;
  // Trivial barriers return immediately
  if (size == 1) return 0;
  char buffer = 'b';
  // bcast to all the ranks
  // 0-byte bcast will just return without doing anything.
  status = comm->bcast(&buffer, sizeof(char), 0);
  return status;
}

/*
refer to
https://github.com/pmodels/mpich/blob/master/src/mpi/coll/barrier/barrier_intra_dissemination.c
*/
int barrier_dissemination(communicator* comm) {
  int size, rank, src, dst, mask;
  size = comm->size();
  // Trivial barriers return immediately
  if (size == 1) return 0;
  rank = comm->rank();
  mask = 0x1;
  int status;
  while (mask < size) {
    dst = (rank + mask) % size;
    src = (rank - mask + size) % size;
    // the size can not be empty for mercury
    char sendbuffer = 'b';
    char recvbuffer;
    status = comm->sendrecv((void*)&sendbuffer, size_t(sizeof(char)), dst,
                            COLZA_BARRIER_TAG, (void*)&recvbuffer,
                            size_t(sizeof(char)), src, COLZA_BARRIER_TAG);
    if (status != 0) {
      // failed to execute sendrecv
      return status;
    }
    mask <<= 1;
  }
  return 0;
}

}  // namespace colza
