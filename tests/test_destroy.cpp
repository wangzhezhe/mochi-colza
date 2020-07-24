#include "test_destroy.hpp"

#include <stdlib.h>

#include "CppUnitAdditionalMacros.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(DestroyTest);

namespace tl = thallium;

extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void DestroyTest::setUp() {}

void DestroyTest::tearDown() {}

void DestroyTest::testDestroy() {
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testDestroy" << std::endl;
  }
  // destroy
  int ret = m_comm->destroy();
  CPPUNIT_ASSERT(ret == 0);
  return;
}