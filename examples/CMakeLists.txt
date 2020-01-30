add_executable(example_send_recv example_send_recv.cpp)
target_link_libraries(example_send_recv ${MPI_CXX_LIBRARIES} colza)
