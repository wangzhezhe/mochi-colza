add_executable (example-server ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp)
target_link_libraries (example-server colza-server)

add_executable (example-server2 ${CMAKE_CURRENT_SOURCE_DIR}/server2.cpp)
target_link_libraries (example-server2 colza-server MPI::MPI_C)

add_executable (example-admin ${CMAKE_CURRENT_SOURCE_DIR}/admin.cpp)
target_link_libraries (example-admin colza-admin)

add_executable (example-admin2 ${CMAKE_CURRENT_SOURCE_DIR}/admin2.cpp)
target_link_libraries (example-admin2 colza-admin)

add_executable (example-client ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp)
target_link_libraries (example-client colza-client)

add_executable (example-client2 ${CMAKE_CURRENT_SOURCE_DIR}/client2.cpp)
target_link_libraries (example-client2 colza-client)
