#ifndef __COLZA_TEST_COMMDUPLICATE_H
#define __COLZA_TEST_COMMDUPLICATE_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class CommDuplicateTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CommDuplicateTest);
  CPPUNIT_TEST(testCommDuplicate);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testCommDuplicate();
};

#endif
