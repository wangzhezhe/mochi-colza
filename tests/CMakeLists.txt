add_executable(test_send_recv main.cpp test_send_recv.cpp)
target_link_libraries(test_send_recv ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_executable(test_isend_irecv main.cpp test_isend_irecv.cpp)
target_link_libraries(test_isend_irecv ${CPPUNIT_LIBRARIES} ${MPI_CXX_LIBRARIES} colza)

add_test(NAME send_recv COMMAND mpirun -np 2 ./test_send_recv)

add_test(NAME isend_irecv COMMAND mpirun -np 2 ./test_isend_irecv)
