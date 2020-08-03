#ifndef __COLZA_TEST_IS_IR_CX_H
#define __COLZA_TEST_IS_IR_CX_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class ISRCTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ISRCTest);
  CPPUNIT_TEST(testISRComplex);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testISRComplex();
};

#endif
