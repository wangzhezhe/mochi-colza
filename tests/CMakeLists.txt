set(RUNCOMMAND "mpirun")
set(PARAMETERS "")

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

add_executable(test_duplicate main.cpp test_duplicate.cpp)
target_link_libraries(test_duplicate ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_subset main.cpp test_subset.cpp)
target_link_libraries(test_subset ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_wait main.cpp test_wait.cpp)
target_link_libraries(test_wait ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_destroy main.cpp test_destroy.cpp)
target_link_libraries(test_destroy ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_test(NAME send_recv COMMAND ${RUNCOMMAND} ${PARAMETERS} -n 2 ./test_send_recv)

add_test(NAME test_bcast COMMAND ${RUNCOMMAND} ${PARAMETERS} -n 20 ./test_bcast)

add_test(NAME test_barrier COMMAND ${RUNCOMMAND} ${PARAMETERS} -n 10 ./test_barrier)

add_test(NAME test_reduce COMMAND ${RUNCOMMAND} ${PARAMETERS} -n 10 ./test_reduce)

add_test(NAME test_gather COMMAND ${RUNCOMMAND} ${PARAMETERS} -n 10 ./test_gather)

add_test(NAME test_gatherv COMMAND ${RUNCOMMAND} ${PARAMETERS} -n 10 ./test_gatherv)

add_test(NAME test_alltoall COMMAND ${RUNCOMMAND} ${PARAMETERS} -n 10 ./test_alltoall)

add_test(NAME test_duplicate COMMAND ${RUNCOMMAND} ${PARAMETERS} -n 2 ./test_duplicate)

add_test(NAME test_subset COMMAND ${RUNCOMMAND} ${PARAMETERS} -n 10 ./test_subset)

add_test(NAME test_wait COMMAND ${RUNCOMMAND} ${PARAMETERS} -n 5 ./test_wait)

add_test(NAME test_destroy COMMAND ${RUNCOMMAND} ${PARAMETERS} -n 5 ./test_destroy)
