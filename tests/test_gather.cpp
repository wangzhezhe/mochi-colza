#include "test_gather.hpp"

#include <stdlib.h>

#include "CppUnitAdditionalMacros.hpp"
#include "colza/request.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(GatherTest);

namespace tl = thallium;

extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void GatherTest::setUp() {}

void GatherTest::tearDown() {}

void GatherTest::testGatherSmallZero() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testGatherSmallZero" << std::endl;
  }
  int size = m_comm->size();
  int root = 0;
  char c = 'A' + (rank % 26);
  std::vector<char> data(size, 0);

  int ret = m_comm->gather(&c, sizeof(char), (void*)data.data(), root);
  CPPUNIT_ASSERT(ret == 0);
  // check results at the root
  if (rank == root) {
    for (int i = 0; i < size; i++) {
      CPPUNIT_ASSERT(data[i] == 'A' + (i % 26));
    }
  }
}

void GatherTest::testGatherSmallNonZero() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testGatherSmallNonZero" << std::endl;
  }
  int size = m_comm->size();
  CPPUNIT_ASSERT(size >= 2);
  int root = 1;

  char c = 'A' + (rank % 26);
  std::vector<char> data(size, 0);

  int ret = m_comm->gather(&c, sizeof(char), (void*)data.data(), root);
  CPPUNIT_ASSERT(ret == 0);
  // check results at the root
  if (rank == root) {
    for (int i = 0; i < size; i++) {
      CPPUNIT_ASSERT(data[i] == 'A' + (i % 26));
    }
  }
}

void GatherTest::testGatherLarge() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testGatherLarge" << std::endl;
  }
  int size = m_comm->size();
  CPPUNIT_ASSERT(size >= 2);
  int root = 1;
  std::vector<char> senddata(256, 0);
  for (int i = 0; i < 256; i++) {
    senddata[i] = ('A' + ((i + rank) % 26));
  }

  std::vector<char> recvdata;

  if (rank == root) {
    for (int i = 0; i < size; i++) {
      for (int j = 0; j < 256; j++) {
        recvdata.push_back(0);
      }
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
  int ret = m_comm->gather((void*)senddata.data(), sizeof(char) * 256,
                           (void*)recvdata.data(), root);
  CPPUNIT_ASSERT(ret == 0);
  // check results at the root
  int index = 0;
  if (rank == root) {
    for (int i = 0; i < size; i++) {
      for (int j = 0; j < 256; j++) {
        index = i * 256 + j;
        CPPUNIT_ASSERT(recvdata[index] == ('A' + ((j + i) % 26)));
      }
    }
  }
}

void GatherTest::testIGather() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testIGather" << std::endl;
  }
  int size = m_comm->size();
  int root = 0;
  char c = 'A' + (rank % 26);
  std::vector<char> data;

  if (rank == 0) {
    for (int i = 0; i < size; i++) {
      data.push_back(0);
    }
  }

  colza::request req;
  int ret = m_comm->igather(&c, sizeof(char), (void*)data.data(), root, req);
  CPPUNIT_ASSERT(ret == 0);
  // wait for the isend to finish
  int finish = req.wait();
  CPPUNIT_ASSERT(finish == 0);
  // check results at the root
  if (rank == root) {
    for (int i = 0; i < size; i++) {
      CPPUNIT_ASSERT(data[i] == 'A' + (i % 26));
    }
  }
}

void GatherTest::testAllGather() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testAllGather" << std::endl;
  }
  int size = m_comm->size();
  int root = 0;
  char c = 'A' + (rank % 26);
  std::vector<char> recvdata(size, 0);
  int ret = m_comm->allgather(&c, recvdata.data(), sizeof(char));
  CPPUNIT_ASSERT(ret == 0);
  // check results at the root
  for (int i = 0; i < size; i++) {
    CPPUNIT_ASSERT(recvdata[i] == 'A' + (i % 26));
  }
}

void GatherTest::testIAllGather() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testIAllGather" << std::endl;
  }
  int size = m_comm->size();
  int root = 0;
  char c = 'A' + (rank % 26);
  std::vector<char> recvdata(size, 0);
  colza::request req;
  int ret = m_comm->iallgather(&c, recvdata.data(), sizeof(char), req);
  CPPUNIT_ASSERT(ret == 0);
  // wait for the isend to finish
  int finish = req.wait();

  // check results at the root
  for (int i = 0; i < size; i++) {
    CPPUNIT_ASSERT(recvdata[i] == 'A' + (i % 26));
  }
}
