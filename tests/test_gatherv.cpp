#include "test_gatherv.hpp"

#include <stdlib.h>

#include "CppUnitAdditionalMacros.hpp"
#include "colza/request.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(GathervTest);

namespace tl = thallium;

extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void GathervTest::setUp() {}

void GathervTest::tearDown() {}

void GathervTest::testGatherv() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testGatherv" << std::endl;
  }
  int size = m_comm->size();
  int root = 0;
  char c = 'A' + (rank % 26);
  std::vector<char> recvdata(size * 2, 0);
  std::vector<size_t> recvcount(size, 0);
  std::vector<size_t> offset(size, 0);
  if (rank == root) {
    for (int i = 0; i < size; i++) {
      recvcount[i] = 1;
      offset[i] = i * 2;
    }
  }

  int ret = m_comm->gatherv(&c, (void*)recvdata.data(), 1, recvcount.data(),
                            offset.data(), sizeof(char), root);
  CPPUNIT_ASSERT(ret == 0);
  // check results at the root
  if (rank == root) {
    for (int i = 0; i < size; i++) {
      int index = i * 2;
      CPPUNIT_ASSERT(recvdata[index] == 'A' + (i % 26));
    }
  }
}

void GathervTest::testIGatherv() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testIGatherv" << std::endl;
  }
  int size = m_comm->size();
  int root = 0;
  char c = 'A' + (rank % 26);
  std::vector<char> recvdata(size * 2, 0);
  std::vector<size_t> recvcount(size, 0);
  std::vector<size_t> offset(size, 0);
  if (rank == root) {
    for (int i = 0; i < size; i++) {
      recvcount[i] = 1;
      offset[i] = i * 2;
    }
  }
  colza::request req;
  int ret = m_comm->igatherv(&c, (void*)recvdata.data(), 1, recvcount.data(),
                             offset.data(), sizeof(char), root, req);
  CPPUNIT_ASSERT(ret == 0);
  // wait for the isend to finish
  int finish = req.wait();
  CPPUNIT_ASSERT(finish == 0);
  // check results at the root
  if (rank == root) {
    for (int i = 0; i < size; i++) {
      int index = i * 2;
      CPPUNIT_ASSERT(recvdata[index] == 'A' + (i % 26));
    }
  }
}