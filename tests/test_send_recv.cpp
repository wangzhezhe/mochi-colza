#include "test_send_recv.hpp"
#include "CppUnitAdditionalMacros.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( SendRecvTest );

namespace tl = thallium;

extern tl::engine* engine;

void SendRecvTest::setUp() {
    m_controller = colza::controller::create(engine, MPI_COMM_WORLD);
    m_comm       = m_controller->build_world_communicator();
}

void SendRecvTest::tearDown() {
    m_comm.reset();
    m_controller = nullptr;
}

void SendRecvTest::testSendRecv() {
    int rank = m_comm->rank();
    int size = m_comm->size();
    CPPUNIT_ASSERT(rank >= 0 && rank < size);
    CPPUNIT_ASSERT(size == 2);
    std::vector<char> data(256, 0);
    if(rank % 2 == 0) {
        for(unsigned i = 0; i < 256; i++) {
            data[i] = 'A'+(i%26);
        }
        int ret = m_comm->send((const void*)data.data(), 256, rank+1, 1234);
        CPPUNIT_ASSERT(ret == 0);
    } else {
        int ret = m_comm->recv((void*)data.data(), 256, rank-1, 1234);
        CPPUNIT_ASSERT(ret == 0);
        for(unsigned i = 0; i < 256; i++) {
            CPPUNIT_ASSERT(data[i] == 'A'+(i%26));
        }
    }
}

