#ifndef __COLZA_TEST_LEAVE_H
#define __COLZA_TEST_LEAVE_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class LeaveTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(LeaveTest);
  CPPUNIT_TEST(testLeave);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testLeave();
};

#endif
