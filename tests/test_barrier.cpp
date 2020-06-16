#include "test_barrier.hpp"

#include <stdlib.h>

#include <chrono>
#include <thread>

#include "CppUnitAdditionalMacros.hpp"
#include "colza/request.hpp"
#include "colza/types.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(BarrierTest);

namespace tl = thallium;

// extern tl::engine* engine;
extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void BarrierTest::setUp() {}

void BarrierTest::tearDown() {}

void BarrierTest::testBarrierBcast() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testBarrierBcast" << std::endl;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(rank * 1000));
  std::cout << "get to the sync point for rank " << rank << std::endl;
  int status = m_comm->barrier(barrier_algorithm::bcast);
  std::cout << "after the sync point for rank " << rank << std::endl;
  CPPUNIT_ASSERT(status == 0);
}

void BarrierTest::testBarrierDissemination() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testBarrierDissemination" << std::endl;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(rank * 1000));
  std::cout << "get to the sync point for rank " << rank << std::endl;
  int status = m_comm->barrier(barrier_algorithm::dissemination);
  std::cout << "after the sync point for rank " << rank << std::endl;
  CPPUNIT_ASSERT(status == 0);
  return;
}
