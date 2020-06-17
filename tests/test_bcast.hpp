#ifndef __COLZA_TEST_SEND_RECV_H
#define __COLZA_TEST_SEND_RECV_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class BcastTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(BcastTest);
  CPPUNIT_TEST(testBcastRootZeroSeq);
  CPPUNIT_TEST(testBcastRootZeroBinomial);
  CPPUNIT_TEST(testBcastRootRandom);
  CPPUNIT_TEST(testIBcast);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testBcastRootZeroSeq();
  void testBcastRootZeroBinomial();
  void testBcastRootRandom();
  void testIBcast();
};

#endif
