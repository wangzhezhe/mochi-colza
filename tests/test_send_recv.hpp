#ifndef __COLZA_TEST_SEND_RECV_H
#define __COLZA_TEST_SEND_RECV_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class SendRecvTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(SendRecvTest);
  CPPUNIT_TEST(testSendRecv);
  CPPUNIT_TEST(testISendIRecv);
  CPPUNIT_TEST(testSendRecvInOneCall);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testSendRecv();
  void testISendIRecv();
  void testSendRecvInOneCall();
};

#endif
