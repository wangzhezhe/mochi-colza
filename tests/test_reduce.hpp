#ifndef __COLZA_TEST_SEND_RECV_H
#define __COLZA_TEST_SEND_RECV_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class ReduceTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ReduceTest);
  CPPUNIT_TEST(testReduceSumInt64);
  CPPUNIT_TEST(testReduceNonzeroRootSumInt64);
  CPPUNIT_TEST(testReduceSumInt64Vector);
  CPPUNIT_TEST(testReduceSumDoubleVector);
  CPPUNIT_TEST(testIReduceSumInt64Vector);
  CPPUNIT_TEST(testAllReduceSumInt64Vector);
  CPPUNIT_TEST(testIAllReduceSumInt64Vector);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testReduceSumInt64();
  void testReduceNonzeroRootSumInt64();
  void testReduceSumInt64Vector();
  void testReduceSumDoubleVector();
  void testIReduceSumInt64Vector();
  void testAllReduceSumInt64Vector();
  void testIAllReduceSumInt64Vector();
};

#endif
