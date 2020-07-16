

#include "test_alltoall.hpp"

#include <stdlib.h>

#include "CppUnitAdditionalMacros.hpp"
#include "colza/request.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(AlltoallTest);

namespace tl = thallium;

extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void AlltoallTest::setUp() {}

void AlltoallTest::tearDown() {}

// https://www.rookiehpc.com/mpi/docs/mpi_alltoall.php
void AlltoallTest::testAlltoall() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testAlltoall" << std::endl;
  }
  int size = m_comm->size();
  int root = 0;
  std::vector<char> senddata(size, 0);
  for (int i = 0; i < size; i++) {
    senddata[i] = 'A' + (i % 26);
  }

  std::vector<char> recvdata(size, 0);

  m_comm->alltoall(senddata.data(), sizeof(char), recvdata.data(),
                   sizeof(char));

  // check results
  // the results in rank=0 is AAAA.. rank=1 is BBBB...
  for (int i = 0; i < size; i++) {
    CPPUNIT_ASSERT(recvdata[i] = 'A' + (rank % 26));
  }
}

void AlltoallTest::testIAlltoall() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testIAlltoall" << std::endl;
  }
  int size = m_comm->size();
  int root = 0;
  std::vector<char> senddata(size, 0);
  for (int i = 0; i < size; i++) {
    senddata[i] = 'A' + (i % 26);
  }
  std::vector<char> recvdata(size, 0);
  colza::request req;
  int ret = m_comm->ialltoall(senddata.data(), sizeof(char), recvdata.data(),
                              sizeof(char), req);
  CPPUNIT_ASSERT(ret == 0);
  // wait for the isend to finish
  int finish = m_comm->wait(req);

  // check results
  for (int i = 0; i < size; i++) {
    CPPUNIT_ASSERT(recvdata[i] == 'A' + (rank % 26));
  }
}
