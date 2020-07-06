#include "test_subset.hpp"

#include <stdlib.h>

#include "CppUnitAdditionalMacros.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(CommSubsetTest);

namespace tl = thallium;

extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void CommSubsetTest::setUp() {}

void CommSubsetTest::tearDown() {}

void CommSubsetTest::testCommSubset() {
  int subsetSize = 3;

  int size = m_comm->size();
  CPPUNIT_ASSERT(size >= subsetSize + 1);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testCommSubset" << std::endl;
  }
  std::vector<int32_t> subrank;
  // the id in subset start from 1
  for (int i = 1; i <= subsetSize; i++) {
    subrank.push_back(i);
  }

  // create subcommunicator
  colza::communicator* newcomm = nullptr;
  int ret = m_comm->subset(&newcomm, subsetSize, (int32_t*)subrank.data());
  CPPUNIT_ASSERT(ret == 0);
  if (rank >= 1 && rank <= subsetSize) {
    CPPUNIT_ASSERT(newcomm != nullptr);
    CPPUNIT_ASSERT(newcomm->size() == subsetSize);
  }

  // test bcast to see if the bcast only works for the subset
  int flag = 0;
  if (newcomm != nullptr) {
    int newrank = newcomm->rank();
    if (newrank == 0) {
      flag = 1;
    }
    // the 0 is the id for the new root
    int status =
        newcomm->bcast(&flag, sizeof(int), 0, colza::COLZA_Bcast::sequential);
    CPPUNIT_ASSERT(status == 0);
    CPPUNIT_ASSERT(flag == 1);

    if (newrank == 0) {
      flag = 2;
    }
    // test another implementation for bcast
    status = newcomm->bcast(&flag, sizeof(int), 0);
    CPPUNIT_ASSERT(status == 0);
    CPPUNIT_ASSERT(flag == 2);
  }
}
