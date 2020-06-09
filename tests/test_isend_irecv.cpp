#include "test_isend_irecv.hpp"
#include "CppUnitAdditionalMacros.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(ISendIRecvTest);

namespace tl = thallium;

extern tl::engine* engine;

void ISendIRecvTest::setUp() {
  // init a thread pool for controller
  std::vector<tl::managed<tl::xstream>> ess;
  m_controller =
      colza::controller::create(engine, MPI_COMM_WORLD);
  m_comm = m_controller->build_world_communicator();
}

void ISendIRecvTest::tearDown() {
  m_comm.reset();
  m_controller = nullptr;
}

void ISendIRecvTest::testISendIRecv() {
  int rank = m_comm->rank();
  int size = m_comm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);
  CPPUNIT_ASSERT(size == 2);
  std::vector<char> data(256, 0);
   colza::request req;
  if (rank % 2 == 0) {
    for (unsigned i = 0; i < 256; i++) {
      data[i] = 'A' + (i % 26);
    }
    int ret = m_comm->isend((const void*)data.data(), 256, rank + 1, 1234, req);
    CPPUNIT_ASSERT(ret == 0);
    std::cout << "ok to return from the isend" << std::endl;
    // wait for the isend to finish
    int finish = req.wait();
    CPPUNIT_ASSERT(finish == 0);
  } else {
    int ret = m_comm->irecv((void*)data.data(), 256, rank - 1, 1234, req);
    std::cout << "ok to return from the irecv" << std::endl;

    CPPUNIT_ASSERT(ret == 0);
    // wait for the ircv to finish
    int finish = req.wait();
    CPPUNIT_ASSERT(finish == 0);
    for (unsigned i = 0; i < 256; i++) {
      CPPUNIT_ASSERT(data[i] == 'A' + (i % 26));
    }
  }
}
