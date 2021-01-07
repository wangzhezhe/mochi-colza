add_executable (dist-server ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp)
target_link_libraries (dist-server colza-server MPI::MPI_C)

add_executable (dist-admin ${CMAKE_CURRENT_SOURCE_DIR}/admin.cpp)
target_link_libraries (dist-admin colza-admin)

add_executable (dist-client ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp)
target_link_libraries (dist-client colza-client MPI::MPI_C)
