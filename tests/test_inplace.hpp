#ifndef __COLZA_TEST_INPLACE_H
#define __COLZA_TEST_INPLACE_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class InplaceTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(InplaceTest);
  CPPUNIT_TEST(testInplaceGather);
  CPPUNIT_TEST(testInplaceGatherv);
  CPPUNIT_TEST(testInplacereduce);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testInplaceGather();
  void testInplaceGatherv();
  void testInplacereduce();
};

#endif
