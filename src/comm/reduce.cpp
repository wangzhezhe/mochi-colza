#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

namespace colza {

// refer to
// https://github.com/pmodels/mpich/blob/master/src/mpi/coll/reduce/reduce_intra_binomial.c
int communicator::reduce(const void *sendBuffer, void *recvBuffer, size_t count,
                         size_t elementSize, COLZA_Operation_Func opFunc,
                         int root) {
  // use the binomial method
  int status;
  int comm_size, rank, lroot, relrank;
  int mask, source;
  // the temp is to store the intermediate results recieved from the source
  void *tempSrc = nullptr;
  int mallocRcvbuffer = false;

  if (count == 0) return 0;

  comm_size = this->size();
  rank = this->rank();

  // only support commutative operation for the current implementation

  // If I'm not the root, then my recvbuf may not be valid, therefore
  // I have to allocate a temporary one
  tempSrc = (void *)malloc(elementSize * count);

  if (rank != root && recvBuffer != nullptr) {
    mallocRcvbuffer = true;
    recvBuffer = (void *)malloc(elementSize * count);
  }
  //recv buffer should be reinnitilized by sendBuffer for all ranks
  //this aims to avoid the init value of the recvbuffer to influence the results
  memcpy(recvBuffer, sendBuffer, elementSize * count);


  mask = 0x1;
  lroot = root;
  // adjusted rank, the relrank for the root is 0
  relrank = (rank - lroot + comm_size) % comm_size;

  while (mask < comm_size) {
    /* Receive */
    if ((mask & relrank) == 0) {
      source = (relrank | mask);
      if (source < comm_size) {
        source = (source + lroot) % comm_size;

        status =
            this->recv(tempSrc, elementSize * count, source, COLZA_REDUCE_TAG);

        if (status != 0) {
          return status;
        }
        // for the first iteration, the recv buffer have already stored the
        // value from the send buffer
        status = opFunc(tempSrc, recvBuffer, elementSize, count, NULL);
        if (status != 0) {
          return status;
        }
      }
    } else {
      /* I've received all that I'm going to.  Send my result to
       * my parent */
      source = ((relrank & (~mask)) + lroot) % comm_size;

      status =
          this->send(recvBuffer, elementSize * count, source, COLZA_REDUCE_TAG);

      if (status != 0) {
        return status;
      }
      break;
    }
    mask <<= 1;
  }

  // free tmp buffer
  if (tempSrc != nullptr) {
    free(tempSrc);
  }

  if (mallocRcvbuffer) {
    free(recvBuffer);
  }

  return 0;
}

int communicator::ireduce(const void *sendBuffer, void *recvBuffer,
                          size_t count, size_t elementSize,
                          COLZA_Operation_Func opFunc, int root, request &req) {
  request tmp(std::make_shared<tl::eventual<void>>());
  auto eventual = tmp.m_eventual;
  this->m_controller->m_pool.make_thread(
      [sendBuffer, recvBuffer, count, elementSize, opFunc, root, eventual,
       this]() {
        reduce(sendBuffer, recvBuffer, count, elementSize, opFunc, root);
        eventual->set_value();
      },
      tl::anonymous());
  req = std::move(tmp);
  return 0;
}

}  // namespace colza
