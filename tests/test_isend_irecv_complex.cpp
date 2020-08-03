#include "test_isend_irecv_complex.hpp"

#include <stdlib.h>

#include <vector>

#include "CppUnitAdditionalMacros.hpp"
#include "colza/request.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(ISRCTest);

namespace tl = thallium;

extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void ISRCTest::setUp() { 
  //tl::thread::sleep(m_controller->get_engine(), 2000);
}

void ISRCTest::tearDown() {}

void ISRCTest::testISRComplex() {
  MPI_Barrier(MPI_COMM_WORLD);
  int myrank = m_comm->rank();
  if (myrank == 0) {
    std::cout << "---test testISRComplex" << std::endl;
  }
  int commsize = m_comm->size();

  CPPUNIT_ASSERT(commsize >= 2);
  unsigned bufferSize = 256;
  std::vector<char> sendBuffer(bufferSize, 0);
  for (unsigned i = 0; i < bufferSize; i++) {
    sendBuffer[i] = 'A' + (i % 26);
  }

  std::vector<char> recvBuffer(bufferSize, 0);

  // init the send list
  std::vector<colza::request> sendreqs(commsize, colza::request());
  int tag = 2200;
  for (int dest = 0; dest < commsize; dest++) {
    if (dest != myrank) {
      // send req from myrank to the dest
      // sendreqs.push_back(colza::request());
      int ret = m_comm->isend((const void*)sendBuffer.data(), bufferSize, dest,
                              tag, sendreqs[dest]);

      CPPUNIT_ASSERT(ret == 0);
    }
  }

  // get the recv list
  std::vector<colza::request> recvreqs(commsize, colza::request());
  for (int src = 0; src < commsize; src++) {
    if (src != myrank) {
      // recv req from src to the myrank
      // recvreqs.push_back(colza::request());
      int ret = m_comm->irecv((void*)recvBuffer.data(), bufferSize, src, tag,
                              recvreqs[src]);
      CPPUNIT_ASSERT(ret == 0);
    }
  }

  // wait
  for (int i = 0; i < commsize-1; i++) {
    // waitAny
    int ret = m_comm->waitAny(commsize, sendreqs.data());
    CPPUNIT_ASSERT(ret >= 0);

    // reset the request to empty one
    sendreqs[ret] = colza::request();
  }

  // Wait One for the recvreqs
  for (int i = 0; i < commsize; i++) {
    // waitAny
    int ret = m_comm->wait(recvreqs[i]);
    //recvreqs[ret] = colza::request();
  }

  // reset the request to empty one

  // TODO check results

  return;
}
