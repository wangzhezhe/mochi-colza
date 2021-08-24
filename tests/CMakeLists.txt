include_directories(../include)

add_library(colza-test Main.cpp)
target_link_libraries(colza-test PkgConfig::cppunit colza-server colza-admin colza-client)

add_executable(AdminTest AdminTest.cpp)
target_link_libraries(AdminTest colza-test)

add_executable(ClientTest AdminTest.cpp)
target_link_libraries(ClientTest colza-test)

add_executable(PipelineTest PipelineTest.cpp)
target_link_libraries(PipelineTest colza-test)

add_test(NAME AdminTest COMMAND ./AdminTest AdminTest.xml)
add_test(NAME ClientTest COMMAND ./ClientTest ClientTest.xml)
add_test(NAME PipelineTest COMMAND ./PipelineTest PipelineTest.xml)
