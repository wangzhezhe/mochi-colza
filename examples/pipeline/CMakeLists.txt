# set source files
set (dummy-src-files
     DummyBackend.cpp)

# dummy pipeline library
add_library (dummy-pipeline ${dummy-src-files})
target_link_libraries (dummy-pipeline colza-server)
