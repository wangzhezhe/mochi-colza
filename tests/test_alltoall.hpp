#ifndef __COLZA_TEST_ALLTOALL_H
#define __COLZA_TEST_ALLTOALL_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class AlltoallTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(AlltoallTest);
  CPPUNIT_TEST(testAlltoall);
  CPPUNIT_TEST(testIAlltoall);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testAlltoall();
  void testIAlltoall();
};

#endif
