#include "test_leave.hpp"

#include <stdlib.h>

#include <chrono>
#include <thread>
#include <algorithm>
#include <random>

#include "CppUnitAdditionalMacros.hpp"
#include "colza/request.hpp"
#include "colza/types.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(LeaveTest);

namespace tl = thallium;

extern tl::engine* engine;
colza::controller* m_controller;
std::shared_ptr<colza::communicator> m_comm;
int m_rank;
int m_size;
std::vector<int> m_leave_order;

void LeaveTest::setUp() {
    MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &m_size);
    m_controller = colza::controller::create(engine, MPI_COMM_WORLD);
    for(unsigned i=1; i < m_size; i++) {
        m_leave_order.push_back(i);
    }
    // important to pass the same seed in all processes
    std::shuffle(m_leave_order.begin(),
                 m_leave_order.end(),
                 std::default_random_engine(0));
    m_leave_order.push_back(0);
}

void LeaveTest::tearDown() {
    m_comm.reset();
}

void LeaveTest::testLeave() {
  MPI_Barrier(MPI_COMM_WORLD);

  for(unsigned i=0; i < m_leave_order.size(); i++) {
    tl::thread::sleep(*engine, 200);
    m_comm = m_controller->synchronize();
    m_comm->barrier();
    if(m_rank == m_leave_order[i]) {
        if(m_rank != 0) {
            m_controller->leave();
            std::cout << "Rank " << m_rank << " left" << std::endl;
        }
        break;
    }
  }
  std::cout << "Test done for rank " << m_rank << std::endl;
}
