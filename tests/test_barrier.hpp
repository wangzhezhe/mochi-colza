#ifndef __COLZA_TEST_BARRIER_H
#define __COLZA_TEST_BARRIER_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class BarrierTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(BarrierTest);
  CPPUNIT_TEST(testBarrierBcast);
  CPPUNIT_TEST(testBarrierDissemination);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testBarrierBcast();
  void testBarrierDissemination();
};

#endif
