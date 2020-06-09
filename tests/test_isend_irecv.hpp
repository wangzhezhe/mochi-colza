#ifndef __COLZA_TEST_SEND_RECV_H
#define __COLZA_TEST_SEND_RECV_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"
#include "colza/request.hpp"

class ISendIRecvTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ISendIRecvTest);
  CPPUNIT_TEST(testISendIRecv);
  CPPUNIT_TEST_SUITE_END();

  colza::controller* m_controller = nullptr;
  std::shared_ptr<colza::communicator> m_comm = nullptr;

 public:
  void setUp();
  void tearDown();
  void testISendIRecv();
};

#endif
