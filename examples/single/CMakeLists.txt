add_executable (single-server ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp)
target_link_libraries (single-server colza-server)

add_executable (single-admin ${CMAKE_CURRENT_SOURCE_DIR}/admin.cpp)
target_link_libraries (single-admin colza-admin)

add_executable (single-client ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp)
target_link_libraries (single-client colza-client)
