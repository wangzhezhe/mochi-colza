add_executable (colza-single-server ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp)
target_link_libraries (colza-single-server colza-server)

add_executable (colza-single-admin ${CMAKE_CURRENT_SOURCE_DIR}/admin.cpp)
target_link_libraries (colza-single-admin colza-admin)

add_executable (colza-single-client ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp)
target_link_libraries (colza-single-client colza-client)

install (TARGETS colza-single-server colza-single-admin colza-single-client DESTINATION bin)
