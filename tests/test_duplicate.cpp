#include "test_duplicate.hpp"

#include <stdlib.h>

#include "CppUnitAdditionalMacros.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(CommDuplicateTest);

namespace tl = thallium;

extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void CommDuplicateTest::setUp() {}

void CommDuplicateTest::tearDown() {}

void CommDuplicateTest::testCommDuplicate() {
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testDuplicate" << std::endl;
  }
  colza::communicator* newcomm = nullptr;
  int ret = m_comm->duplicate(&newcomm);
  CPPUNIT_ASSERT(ret == 0);
  CPPUNIT_ASSERT(newcomm != nullptr);

  // sendrecv for the new communicator
  rank = newcomm->rank();
  int size = newcomm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);
  CPPUNIT_ASSERT(size == 2);
  std::vector<char> data(256, 0);
  if (rank % 2 == 0) {
    for (unsigned i = 0; i < 256; i++) {
      data[i] = 'A' + (i % 26);
    }
    int ret = newcomm->send((const void*)data.data(), 256, rank + 1, 1234);
    CPPUNIT_ASSERT(ret == 0);
  } else {
    int ret = newcomm->recv((void*)data.data(), 256, rank - 1, 1234);
    CPPUNIT_ASSERT(ret == 0);
    for (unsigned i = 0; i < 256; i++) {
      CPPUNIT_ASSERT(data[i] == 'A' + (i % 26));
    }
  }
}
