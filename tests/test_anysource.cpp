#include "test_anysource.hpp"

#include <stdlib.h>

#include "CppUnitAdditionalMacros.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(AnysourceTest);

namespace tl = thallium;

extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void AnysourceTest::setUp() {}

void AnysourceTest::tearDown() {}

void AnysourceTest::testAnysource() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  int size = m_comm->size();
  CPPUNIT_ASSERT(size >= 2);
  char c;
  // send
  if (rank == 0) {
    c = 'c';
    for (int dest = 1; dest < size; dest++) {
      int ret = m_comm->send(&c, sizeof(char), dest, 1234);
      CPPUNIT_ASSERT(ret == 0);
    }
  } else {
    // recv
    int ret = m_comm->recv(&c, sizeof(char), -1, 1234);
    CPPUNIT_ASSERT(ret == 0);
    CPPUNIT_ASSERT(c == 'c');
  }

  return;
}
