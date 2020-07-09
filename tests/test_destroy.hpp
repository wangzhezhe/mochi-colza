#ifndef __COLZA_TEST_DESTROY_H
#define __COLZA_TEST_DESTROY_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class DestroyTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DestroyTest);
  CPPUNIT_TEST(testDestroy);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testDestroy();
};

#endif
