#include "test_join.hpp"

#include <stdlib.h>

#include <chrono>
#include <thread>

#include "CppUnitAdditionalMacros.hpp"
#include "colza/request.hpp"
#include "colza/types.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(JoinTest);

namespace tl = thallium;

extern tl::engine* engine;
colza::controller* m_controller;
std::shared_ptr<colza::communicator> m_comm;
int m_rank;
int m_size;
std::string m_leader_addr;

void JoinTest::setUp() {
    MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &m_size);
    if(m_rank == 0) {
        m_controller = colza::controller::create(engine, MPI_COMM_SELF);
        m_leader_addr = m_controller->leader();
    }
    int s = m_leader_addr.size();
    MPI_Bcast(&s, sizeof(s), MPI_BYTE, 0, MPI_COMM_WORLD);
    m_leader_addr.resize(s);
    MPI_Bcast(const_cast<char*>(m_leader_addr.data()), s, MPI_BYTE, 0, MPI_COMM_WORLD);
}

void JoinTest::tearDown() {}

void JoinTest::testJoin() {
  MPI_Barrier(MPI_COMM_WORLD);
  char x = 0;
  int ret;
  if(m_rank != 0) {
        tl::thread::sleep(*engine, 1000*m_rank);
        m_controller = colza::controller::join(engine, m_leader_addr);
  }
  while(true) {
    m_comm = m_controller->synchronize();
    x = m_comm->size() == m_size ? 1 : 0;
    if(m_rank == 0) std::cout << "Current number of processes is " << m_comm->size() << std::endl;
    tl::thread::sleep(*engine, 200);
    // note that as new processes arrive, rank 0 might not be the same process
    ret = m_comm->bcast(&x, 1, 0, colza::COLZA_Bcast::sequential);
    tl::thread::sleep(*engine, 200);
    CPPUNIT_ASSERT(ret == 0);
    if(x) break;
  }
}
