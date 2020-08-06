#ifndef __COLZA_TEST_JOIN_H
#define __COLZA_TEST_JOIN_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class JoinTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(JoinTest);
  CPPUNIT_TEST(testJoin);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testJoin();
};

#endif
