#include "test_bcast.hpp"

#include <stdlib.h>

#include "CppUnitAdditionalMacros.hpp"
#include "colza/request.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(CollectiveTest);

namespace tl = thallium;

// extern tl::engine* engine;
extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void CollectiveTest::setUp() {}

void CollectiveTest::tearDown() {}

void CollectiveTest::testBcastRootZero() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testBcastRootZero" << std::endl;
  }
  int size = m_comm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);
  CPPUNIT_ASSERT(size == 20);

  std::vector<char> data(256, 0);
  if (rank == 0) {
    for (unsigned i = 0; i < 256; i++) {
      data[i] = 'A' + (i % 26);
    }
  }
  int ret = m_comm->bcast((void*)data.data(), 256, sizeof(char), 0);
  CPPUNIT_ASSERT(ret == 0);

  // check results
  for (unsigned i = 0; i < 256; i++) {
    CPPUNIT_ASSERT(data[i] == 'A' + (i % 26));
  }
}

void CollectiveTest::testBcastRootRandom() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testBcastRootRandom" << std::endl;
  }
  int size = m_comm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);
  CPPUNIT_ASSERT(size == 20);

  srand(time(NULL));
  int random_rank = rand() % 10;

  std::vector<char> data(256, 0);
  if (rank == 0) {
    std::cout << "---test testBcastRootRandom root is " << random_rank
              << std::endl;
  }
  if (rank == random_rank) {
    for (unsigned i = 0; i < 256; i++) {
      data[i] = 'A' + (i % 26);
    }
  }
  int ret = m_comm->bcast((void*)data.data(), 256, sizeof(char), random_rank);
  CPPUNIT_ASSERT(ret == 0);
  for (unsigned i = 0; i < 256; i++) {
    CPPUNIT_ASSERT(data[i] == 'A' + (i % 26));
  }
}

void CollectiveTest::testIBcast() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testIBcast" << std::endl;
  }
  int size = m_comm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);
  CPPUNIT_ASSERT(size == 20);

  std::vector<char> data(256, 0);
  if (rank == 0) {
    for (unsigned i = 0; i < 256; i++) {
      data[i] = 'A' + (i % 26);
    }
  }

  colza::request req;
  int ret = m_comm->ibcast((void*)data.data(), 256, sizeof(char), 0, req);
  CPPUNIT_ASSERT(ret == 0);
  // wait for the isend to finish
  int finish = req.wait();
  CPPUNIT_ASSERT(finish == 0);

  for (unsigned i = 0; i < 256; i++) {
    CPPUNIT_ASSERT(data[i] == 'A' + (i % 26));
  }
}
