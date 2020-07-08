#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

namespace colza {

// we first reduce all data to rank 0
// and the use bcast to send data to all ranks
// TODO add more flexible allreduce implementations
int communicator::allreduce(const void *sendBuffer, void *recvBuffer,
                            size_t count, size_t elementSize,
                            COLZA_Operation_Func opFunc) {
  // reduce
  int root = 0;
  int status;
  status =
      this->reduce(sendBuffer, recvBuffer, count, elementSize, opFunc, root);
  if (status != 0) {
    return status;
  }

  // bcast
  int rank = this->rank();

  status = this->bcast(recvBuffer, count * elementSize, root);
  if (status != 0) {
    return status;
  }

  return 0;
}

int communicator::iallreduce(const void *sendBuffer, void *recvBuffer,
                             size_t count, size_t elementSize,
                             COLZA_Operation_Func opFunc, request &req) {
  request tmp(std::make_shared<tl::eventual<void>>());
  auto eventual = tmp.m_eventual;
  this->m_controller->m_pool.make_thread(
      [sendBuffer, recvBuffer, count, elementSize, opFunc, eventual, this]() {
        allreduce(sendBuffer, recvBuffer, count, elementSize, opFunc);
        eventual->set_value();
      },
      tl::anonymous());
  req = std::move(tmp);
  return 0;
}

}  // namespace colza
