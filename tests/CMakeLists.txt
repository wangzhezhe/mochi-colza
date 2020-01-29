add_executable(test_send_recv main.cpp test_send_recv.cpp)
target_link_libraries(test_send_recv ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_test(NAME send_recv COMMAND mpirun -np 1 ./test_send_recv)
