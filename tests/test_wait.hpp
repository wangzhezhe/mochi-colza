#ifndef __COLZA_TEST_WAIT_H
#define __COLZA_TEST_WAIT_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class WaitTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(WaitTest);
  CPPUNIT_TEST(testWait);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testWait();
};

#endif
