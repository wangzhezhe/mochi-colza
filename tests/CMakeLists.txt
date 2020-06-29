add_executable(test_send_recv main.cpp test_send_recv.cpp)
target_link_libraries(test_send_recv ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_bcast main.cpp test_bcast.cpp)
target_link_libraries(test_bcast ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_barrier main.cpp test_barrier.cpp)
target_link_libraries(test_barrier ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_reduce main.cpp test_reduce.cpp)
target_link_libraries(test_reduce ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_test(NAME send_recv COMMAND mpirun -np 2 ./test_send_recv)

add_test(NAME test_bcast COMMAND mpirun -np 20 ./test_bcast)

add_test(NAME test_barrier COMMAND mpirun -np 10 ./test_barrier)

add_test(NAME test_reduce COMMAND mpirun -np 10 ./test_reduce)
