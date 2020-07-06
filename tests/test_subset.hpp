#ifndef __COLZA_TEST_COMMSUBSET_H
#define __COLZA_TEST_COMMSUBSET_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class CommSubsetTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CommSubsetTest);
  CPPUNIT_TEST(testCommSubset);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testCommSubset();
};

#endif
