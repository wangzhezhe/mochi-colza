#ifndef __COLZA_TEST_GATHER_H
#define __COLZA_TEST_GATHER_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class GatherTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(GatherTest);
  CPPUNIT_TEST(testGatherSmallZero);
  CPPUNIT_TEST(testGatherSmallNonZero);
  CPPUNIT_TEST(testGatherLarge);
  CPPUNIT_TEST(testIGather);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testGatherSmallZero();
  void testGatherSmallNonZero();
  void testGatherLarge();
  void testIGather();
};

#endif