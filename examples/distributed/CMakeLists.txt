add_executable (colza-dist-server ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp)
target_link_libraries (colza-dist-server colza-server MPI::MPI_C ${DRC_LIBRARIES})

add_executable (colza-dist-admin ${CMAKE_CURRENT_SOURCE_DIR}/admin.cpp)
target_link_libraries (colza-dist-admin colza-admin ${DRC_LIBRARIES})

add_executable (colza-dist-client ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp)
target_link_libraries (colza-dist-client colza-client MPI::MPI_C ${DRC_LIBRARIES})

install (TARGETS colza-dist-server colza-dist-admin colza-dist-client DESTINATION bin)
