set(RUNCOMMAND "mpirun -np")

add_executable(test_send_recv main.cpp test_send_recv.cpp)
target_link_libraries(test_send_recv ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_bcast main.cpp test_bcast.cpp)
target_link_libraries(test_bcast ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_barrier main.cpp test_barrier.cpp)
target_link_libraries(test_barrier ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_reduce main.cpp test_reduce.cpp)
target_link_libraries(test_reduce ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_gather main.cpp test_gather.cpp)
target_link_libraries(test_gather ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_gatherv main.cpp test_gatherv.cpp)
target_link_libraries(test_gatherv ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_alltoall main.cpp test_alltoall.cpp)
target_link_libraries(test_alltoall ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_test(NAME send_recv COMMAND ${RUNCOMMAND} 2 ./test_send_recv)

add_test(NAME test_bcast COMMAND ${RUNCOMMAND} 20 ./test_bcast)

add_test(NAME test_barrier COMMAND ${RUNCOMMAND} 10 ./test_barrier)

add_test(NAME test_reduce COMMAND ${RUNCOMMAND} 10 ./test_reduce)

add_test(NAME test_gather COMMAND ${RUNCOMMAND} 10 ./test_gather)

add_test(NAME test_gatherv COMMAND ${RUNCOMMAND} 10 ./test_gatherv)

add_test(NAME test_alltoall COMMAND ${RUNCOMMAND} 10 ./test_alltoall)
