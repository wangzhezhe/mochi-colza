#include "test_reduce.hpp"

#include <stdlib.h>

#include "CppUnitAdditionalMacros.hpp"
#include "colza/request.hpp"
#include "colza/types.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(ReduceTest);

namespace tl = thallium;

// extern tl::engine* engine;
extern colza::controller* m_controller;
extern std::shared_ptr<colza::communicator> m_comm;

void ReduceTest::setUp() {}

void ReduceTest::tearDown() {}

void ReduceTest::testReduceSumInt64() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testReduceSumInt64" << std::endl;
  }
  int size = m_comm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);

  int dest = 0;

  int64_t sendData = rank;
  // the recv data should be initilized as empty value
  int64_t recvData = 0;

  m_comm->reduce(&sendData, &recvData, 1, sizeof(int64_t),
                 colza::COLZA_OP_SUM_INT64, dest);

  if (rank == dest) {
    int64_t expectedValue = 0;
    for (int64_t i = 0; i < size; i++) {
      expectedValue = expectedValue + i;
    }
    CPPUNIT_ASSERT(expectedValue == recvData);
  }
}

void ReduceTest::testReduceNonzeroRootSumInt64() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testReduceRandomrootSumInt64" << std::endl;
  }

  int size = m_comm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);
  int dest = size - 1;

  int64_t sendData = rank;
  // the recv data should be initilized as empty value
  int64_t recvData = 0;

  m_comm->reduce(&sendData, &recvData, 1, sizeof(int64_t),
                 colza::COLZA_OP_SUM_INT64, dest);

  if (rank == dest) {
    int64_t expectedValue = 0;
    for (int64_t i = 0; i < size; i++) {
      expectedValue = expectedValue + i;
    }
  }
}

void ReduceTest::testReduceSumInt64Vector() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testReduceSumInt64Vector" << std::endl;
  }
  int size = m_comm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);

  int dest = 0;
  std::vector<int64_t> sendData(256, rank);
  // the recv data should be initilized as empty value
  std::vector<int64_t> recvData(256, 0);
  m_comm->reduce((void*)sendData.data(), (void*)recvData.data(), 256,
                 sizeof(int64_t), colza::COLZA_OP_SUM_INT64, dest);

  if (rank == dest) {
    std::vector<int64_t> expectedValue(256, 0);
    for (int index = 0; index < size; index++) {
      for (int64_t i = 0; i < size; i++) {
        expectedValue[index] = expectedValue[index] + i;
      }
      CPPUNIT_ASSERT(expectedValue[index] == recvData[index]);
    }
  }
}

void ReduceTest::testReduceSumDoubleVector() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testReduceSumDoubleVector" << std::endl;
  }
  int size = m_comm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);

  int dest = 0;
  std::vector<double> sendData(256, 0.1 * rank);
  // the recv data should be initilized as empty value
  std::vector<double> recvData(256, 0.0);
  m_comm->reduce((void*)sendData.data(), (void*)recvData.data(), 256,
                 sizeof(double), colza::COLZA_OP_SUM_DOUBLE, dest);

  if (rank == dest) {
    std::vector<double> expectedValue(256, 0.0);
    for (int index = 0; index < size; index++) {
      for (double i = 0; i < size; i++) {
        expectedValue[index] = expectedValue[index] + 0.1 * i;
      }
      CPPUNIT_ASSERT_DOUBLES_EQUAL(expectedValue[index], recvData[index],
                                   0.000001);
    }
  }
}

void ReduceTest::testIReduceSumInt64Vector() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testIReduceSumInt64Vector" << std::endl;
  }
  int size = m_comm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);

  int dest = 0;
  std::vector<int64_t> sendData(256, rank);
  // the recv data should be initilized as empty value
  std::vector<int64_t> recvData(256, 0);

  colza::request req;
  int ret =
      m_comm->ireduce((void*)sendData.data(), (void*)recvData.data(), 256,
                      sizeof(int64_t), colza::COLZA_OP_SUM_DOUBLE, dest, req);
  CPPUNIT_ASSERT(ret == 0);
  // wait for the isend to finish
  int finish = m_comm->wait(req);
  CPPUNIT_ASSERT(finish == 0);

  if (rank == dest) {
    std::vector<int64_t> expectedValue(256, 0);
    for (int index = 0; index < size; index++) {
      for (int64_t i = 0; i < size; i++) {
        expectedValue[index] = expectedValue[index] + i;
      }
      CPPUNIT_ASSERT(expectedValue[index] == recvData[index]);
    }
  }
}

void ReduceTest::testAllReduceSumInt64Vector() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testAllReduceSumInt64Vector" << std::endl;
  }
  int size = m_comm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);

  std::vector<int64_t> sendData(256, rank);
  // the recv data should be initilized as empty value
  std::vector<int64_t> recvData(256, 0);
  int status =
      m_comm->allreduce((void*)sendData.data(), (void*)recvData.data(), 256,
                        sizeof(int64_t), colza::COLZA_OP_SUM_INT64);

  CPPUNIT_ASSERT(status == 0);
  // check results for every rank
  std::vector<int64_t> expectedValue(256, 0);
  for (int index = 0; index < size; index++) {
    for (int64_t i = 0; i < size; i++) {
      expectedValue[index] = expectedValue[index] + i;
    }
    CPPUNIT_ASSERT(expectedValue[index] == recvData[index]);
  }
}

void ReduceTest::testIAllReduceSumInt64Vector() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testAllReduceSumInt64Vector" << std::endl;
  }
  int size = m_comm->size();
  CPPUNIT_ASSERT(rank >= 0 && rank < size);

  std::vector<int64_t> sendData(256, rank);
  // the recv data should be initilized as empty value
  std::vector<int64_t> recvData(256, 0);

  colza::request req;
  int ret =
      m_comm->iallreduce((void*)sendData.data(), (void*)recvData.data(), 256,
                         sizeof(int64_t), colza::COLZA_OP_SUM_INT64, req);

  CPPUNIT_ASSERT(ret == 0);
  // wait for the isend to finish
  int finish = m_comm->wait(req);
  CPPUNIT_ASSERT(finish == 0);
  // check results for every rank
  std::vector<int64_t> expectedValue(256, 0);
  for (int index = 0; index < size; index++) {
    for (int64_t i = 0; i < size; i++) {
      expectedValue[index] = expectedValue[index] + i;
    }
    CPPUNIT_ASSERT(expectedValue[index] == recvData[index]);
  }
}

void ReduceTest::testReduceMAXDouble() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testReduceMAXDouble" << std::endl;
  }
  int size = m_comm->size();
  std::vector<double> sendData(6, 0.1 + rank);
  // the recv data should be initilized as empty value
  std::vector<double> recvData(6, 0.0);

  colza::COLZA_Operation_Func funcptr =
      colza::get_ops_func(colza::COLZA_Reduction_Op::MAX, COLZA_DOUBLE);

  CPPUNIT_ASSERT(funcptr != nullptr);

  int status = m_comm->allreduce((void*)sendData.data(), (void*)recvData.data(),
                                 6, sizeof(double), funcptr);

  CPPUNIT_ASSERT(status == 0);

  // check results for every rank
  for (int i = 0; i < recvData.size(); i++) {
    // the max value of the rank is size - 1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(recvData[i], 0.1 + 1.0 * (size - 1), 0.000001);
  }
}

void ReduceTest::testReduceMINDouble() {
  MPI_Barrier(MPI_COMM_WORLD);
  int rank = m_comm->rank();
  if (rank == 0) {
    std::cout << "---test testReduceMINDouble" << std::endl;
  }
  int size = m_comm->size();
  std::vector<double> sendData(256, 0.1 + rank);
  // the recv data should be initilized as empty value
  // if the recv data are processed by the operation func???
  std::vector<double> recvData(256, 0.0);

  colza::COLZA_Operation_Func funcptr =
      colza::get_ops_func(colza::COLZA_Reduction_Op::MIN, COLZA_DOUBLE);

  CPPUNIT_ASSERT(funcptr != nullptr);

  int status = m_comm->allreduce((void*)sendData.data(), (void*)recvData.data(),
                                 256, sizeof(double), funcptr);

  CPPUNIT_ASSERT(status == 0);
  // check results for root
  for (int i = 0; i < recvData.size(); i++) {
    CPPUNIT_ASSERT_DOUBLES_EQUAL(recvData[i], 0.1, 0.000001);
  }
}