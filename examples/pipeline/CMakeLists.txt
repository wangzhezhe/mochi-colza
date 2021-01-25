# set source files
set (dummy-src-files
     DummyBackend.cpp)

# dummy pipeline library
add_library (colza-dummy-pipeline ${dummy-src-files})
target_link_libraries (colza-dummy-pipeline colza-server)

install (TARGETS colza-dummy-pipeline DESTINATION lib)
