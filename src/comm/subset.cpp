
#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

namespace tl = thallium;

namespace colza {
// create a newcommunicator and bcast the id to all the controller
// TODO
// current collective implementation assume that the processes are in same group
// extra argument checking may necessary for bcast/gather/reduction
// if the group created by subset contains process that across multiple groups
int communicator::subset(communicator** newcommptr, int arrayLen,
                         const int32_t* rankArray) {
  if (arrayLen == 0) {
    // the newcommunicator is empty if array size is 0
    *newcommptr = nullptr;
    return 0;
  }
  int status;

  int32_t currentRank = (int32_t)this->rank();
  // check if current rank is in subset
  bool inSubset = false;
  const int32_t* temp = rankArray;
  // if the process is not located in the group, return nullptr
  *newcommptr = nullptr;
  int newRank;
  // if current rank is not the root 0 and not in rankArray, return
  for (int i = 0; i < arrayLen; i++) {
    if (currentRank == *temp) {
      newRank = i;
      inSubset = true;
      break;
    }
    temp++;
  }
  if (!inSubset) {
    return 0;
  }

  // following codes will be executed for the rank in subsetarray
  // the root rank that create the subset id is the first one
  const int rootRank = *rankArray;
  UUID newID;
  if (currentRank == rootRank) {
    newID = UUID::generate();

    // send newID to the coresponding dest
    const int32_t* tempdst = rankArray;
    for (int i = 0; i < arrayLen; i++) {
      if (rootRank == *tempdst) {
        tempdst++;
        continue;
      }
      status = this->send(newID.m_data, 16 * sizeof(char), *tempdst,
                          COLZA_SUBSET_TAG);
      // check error status
      if (status != 0) {
        std::cerr << "failed to subset becased of the isend error" << std::endl;
        return status;
      }
      tempdst++;
    }

  } else {
    status =
        this->recv(newID.m_data, 16 * sizeof(char), rootRank, COLZA_SUBSET_TAG);
    // check error status
    if (status != 0) {
      std::cerr << "failed to subset becased of the irecv error" << std::endl;
      return status;
    }
  }

  // generate std::vector<ssg_member_id_t> members for subset
  std::vector<ssg_member_id_t> subsetMembers(arrayLen);
  const int32_t* tempdst = rankArray;
  for (int i = 0; i < arrayLen; i++) {
    subsetMembers[i] = this->m_members[*tempdst];
    tempdst++;
  }
  // generate the new communicator for subset
  // the rank in new commucnitaor should be updated
  auto c = std::shared_ptr<communicator>(new communicator(
      this->m_controller, arrayLen, newRank, std::move(subsetMembers)));
  c->m_comm_id = newID;

  // TODO add lock?
  this->m_controller->m_communicators[newID] = c;
  // return the subset communicator
  // replace the original content hold by the newcommptr
  *newcommptr = c.get();
  return 0;
}

}  // namespace colza
