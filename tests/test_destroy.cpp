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
  // destroy
  int ret = m_comm->destroy();
  CPPUNIT_ASSERT(ret == 0);

  // the return value is supposed to be -1
  // if there are no registerd communicator
  int c = 'c';
  ret = m_comm->send(&c, sizeof(char), 0, 12345);
  CPPUNIT_ASSERT(ret == -1);
}