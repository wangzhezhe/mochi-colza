
#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

namespace tl = thallium;

namespace colza {
// create a newcommunicator and bcast the id to all the controller running in
// by different processes
int communicator::duplicate(std::shared_ptr<communicator>* newcommptr) {
  if ((*newcommptr).get() != nullptr) {
    std::cerr << "the newcomm should be a nullptr before duplicate"
              << std::endl;
    return -1;
  }
  // the newID should be created by rank0
  UUID newID;
  if (m_rank == 0) {
    newID = UUID::generate();
  }
  // other controller get the newID by bcast
  int status = this->bcast(newID.m_data, 16 * sizeof(char), 0);
  if (status != 0) {
    std::cerr << "failed to duplicate becased of the bcast error" << std::endl;
    return status;
  }

  auto c = std::shared_ptr<communicator>(new communicator(
      this->m_controller, this->m_size, this->m_rank, this->m_members));
  c->m_comm_id = newID;

  // TODO add lock?
  this->m_controller->m_communicators[newID] = c;

  *newcommptr = c;
  return 0;
}

}  // namespace colza
