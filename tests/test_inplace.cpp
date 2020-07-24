

#include "test_inplace.hpp"

#include <stdlib.h>

#include <vector>

#include "CppUnitAdditionalMacros.hpp"
#include "colza/request.hpp"
#include "colza/types.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(InplaceTest);

namespace tl = thallium;

extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void InplaceTest::setUp() {}

void InplaceTest::tearDown() {}

// refer to https://www.rookiehpc.com/mpi/docs/mpi_in_place.php
void InplaceTest::testInplaceGather() {
  MPI_Barrier(MPI_COMM_WORLD);
  int my_rank = m_comm->rank();
  if (my_rank == 0) {
    std::cout << "---test testInplaceGather" << std::endl;
  }
  int size = m_comm->size();
  int root_rank = 0;
  // Define my value
  int my_value = my_rank * 100;
  if (my_rank == root_rank) {
    std::vector<int> buffer(size, 0);
    buffer[my_rank] = my_rank * 100;

    int status = m_comm->gather(COLZA_IN_PLACE, 1 * sizeof(int), buffer.data(),
                                root_rank);
    CPPUNIT_ASSERT(status == 0);
    for (int i = 0; i < size; i++) {
      CPPUNIT_ASSERT(buffer[i] == i * 100);
    }

  } else {
    int status = m_comm->gather(&my_value, 1 * sizeof(int), nullptr, root_rank);
    CPPUNIT_ASSERT(status == 0);
  }

  return;
}

void InplaceTest::testInplaceGatherv() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testInplaceGatherv" << std::endl;
  }
  int size = m_comm->size();
  int root = 0;
  char c = 'A' + (rank % 26);

  if (rank == root) {
    std::vector<char> recvdata(size * 2, 0);

    std::vector<size_t> recvcount(size, 0);
    std::vector<size_t> offset(size, 0);
    for (int i = 0; i < size; i++) {
      recvcount[i] = 1;
      offset[i] = i * 2;
    }
    recvdata[rank] = 'A' + (rank % 26);
    int ret =
        m_comm->gatherv(COLZA_IN_PLACE, (void*)recvdata.data(), 1,
                        recvcount.data(), offset.data(), sizeof(char), root);
    CPPUNIT_ASSERT(ret == 0);

    for (int i = 0; i < size; i++) {
      int index = i * 2;
      CPPUNIT_ASSERT(recvdata[index] == 'A' + (i % 26));
    }
  } else {
    int ret =
        m_comm->gatherv(&c, nullptr, 1, nullptr, nullptr, sizeof(char), root);
  }
}

void InplaceTest::testInplacereduce() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testInplacereduce" << std::endl;
  }
  int size = m_comm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);

  int dest = 0;
  int64_t sendData = rank;
  if (rank == dest) {
    // the recv data should be initilized as empty value
    int64_t recvData = 0;
    m_comm->reduce(COLZA_IN_PLACE, &recvData, 1, sizeof(int64_t),
                   colza::COLZA_OP_SUM_INT64, dest);

    // check results
    int64_t expectedValue = 0;
    for (int64_t i = 0; i < size; i++) {
      expectedValue = expectedValue + i;
    }
    CPPUNIT_ASSERT(expectedValue == recvData);
  } else {
    m_comm->reduce(&sendData, nullptr, 1, sizeof(int64_t),
                   colza::COLZA_OP_SUM_INT64, dest);
  }
}
