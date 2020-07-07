#include "test_wait.hpp"

#include <stdlib.h>

#include "CppUnitAdditionalMacros.hpp"
#include "colza/request.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(WaitTest);

namespace tl = thallium;

extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void WaitTest::setUp() {}

void WaitTest::tearDown() {}

void WaitTest::testWait() {
  MPI_Barrier(MPI_COMM_WORLD);
  int size = m_comm->size();
  CPPUNIT_ASSERT(size == 5);
  int rank = m_comm->rank();

  int buffer[100];
  colza::request reqList[size - 1];
  int index = -1;
  int tag = 12345;
  int status = 0;
  if (rank == 0) {
    // init the buffer for rank 0
    for (int i = 0; i < 100; i++) {
      buffer[i] = i + 100;
    }
    // send to the dest
    for (int i = 0; i < size - 1; i++) {
      status = m_comm->isend(buffer, 100 * sizeof(int), i + 1, tag, reqList[i]);
      CPPUNIT_ASSERT(status == 0);
    }

    unsigned remaining = size - 1;
    while (remaining != 0) {
      index = m_comm->waitAny(remaining, reqList);
      if (remaining != 1) {
        std::swap(reqList[remaining - 1], reqList[index]);
      }
      remaining = remaining - 1;
    }

  } else {
    status = m_comm->recv(buffer, 100 * sizeof(int), 0, tag);
    CPPUNIT_ASSERT(status == 0);

    for (int i = 0; i < 100; i++) {
      CPPUNIT_ASSERT(buffer[i] = i + 100);
    }
  }
}
