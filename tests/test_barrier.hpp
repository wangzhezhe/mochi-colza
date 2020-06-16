#ifndef __COLZA_TEST_SEND_RECV_H
#define __COLZA_TEST_SEND_RECV_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/communicator.hpp"
#include "colza/controller.hpp"

class BarrierTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(BarrierTest);
  CPPUNIT_TEST(testBarrierBcast);
  // TODO this test work only for the small scale currently
  // it fails occasionally when the -n parameter is larger than 10 currently
  // the error message is sth like this `SWIM dping req recv error -- group 15428817528112488969 not found`
  CPPUNIT_TEST(testBarrierDissemination);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp();
  void tearDown();
  void testBarrierBcast();
  void testBarrierDissemination();
};

#endif
