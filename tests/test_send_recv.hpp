#ifndef __COLZA_TEST_SEND_RECV_H
#define __COLZA_TEST_SEND_RECV_H

#include <cppunit/extensions/HelperMacros.h>

#include "colza/controller.hpp"
#include "colza/communicator.hpp"

class SendRecvTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( SendRecvTest );
    CPPUNIT_TEST( testSendRecv );
    CPPUNIT_TEST_SUITE_END();

    colza::controller*   m_controller = nullptr;
    colza::communicator* m_comm       = nullptr;

    public:

    void setUp();
    void tearDown();
    void testSendRecv();
};

#endif
