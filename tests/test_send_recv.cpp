#include "test_send_recv.hpp"
#include "CppUnitAdditionalMacros.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( SendRecvTest );

namespace tl = thallium;

extern tl::engine* engine;

void SendRecvTest::setUp() {
    m_controller = colza::controller::create(engine, MPI_COMM_WORLD);
    m_comm       = m_controller->get_root_communicator();
}

void SendRecvTest::tearDown() {
    m_controller = nullptr;
    m_comm = nullptr;
}

void SendRecvTest::testSendRecv() {

}

