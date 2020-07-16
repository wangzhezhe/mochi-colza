#ifndef __COLZA_TEST_ANYSOURCE_H
#define __COLZA_TEST_ANYSOURCE_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class AnysourceTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(AnysourceTest);
  CPPUNIT_TEST(testAnysource);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testAnysource();
};

#endif
