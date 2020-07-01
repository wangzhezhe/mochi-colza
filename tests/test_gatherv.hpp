#ifndef __COLZA_TEST_GATHERV_H
#define __COLZA_TEST_GATHERV_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class GathervTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(GathervTest);
  CPPUNIT_TEST(testGatherv);
  CPPUNIT_TEST(testIGatherv);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testGatherv();
  void testIGatherv();
};

#endif