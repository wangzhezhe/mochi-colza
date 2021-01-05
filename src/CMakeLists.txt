# set source files
set (server-src-files
     Provider.cpp
     Backend.cpp)

set (client-src-files
     Client.cpp
     PipelineHandle.cpp
     DistributedPipelineHandle.cpp
     AsyncRequest.cpp)

set (admin-src-files
     Admin.cpp)

set (backends-src-files
     SimpleStagerBackend.cpp)

set (module-src-files
     BedrockModule.cpp)

# load package helper for generating cmake CONFIG packages
include (CMakePackageConfigHelpers)

# where to install files for "find_package"
set (colza-pkg "share/cmake/colza")

# set version fo shared library
set (colza-vers "${COLZA_VERSION_MAJOR}.${COLZA_VERSION_MINOR}")

# server library
add_library (colza-server ${server-src-files} ${backends-src-files})
target_link_libraries (colza-server
    thallium
    PkgConfig::SSG
    nlohmann_json::nlohmann_json)
target_include_directories (colza-server PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (colza-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (colza-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (colza-server
    PROPERTIES VERSION ${COLZA_VERSION}
    SOVERSION ${COLZA_VERSION_MAJOR})

# client library
add_library (colza-client ${client-src-files})
target_link_libraries (colza-client PkgConfig::SSG thallium MPI::MPI_C)
target_include_directories (colza-client PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (colza-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (colza-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (colza-client
    PROPERTIES VERSION ${COLZA_VERSION}
    SOVERSION ${COLZA_VERSION_MAJOR})

# admin library
add_library (colza-admin ${admin-src-files})
target_link_libraries (colza-admin thallium nlohmann_json::nlohmann_json)
target_include_directories (colza-admin PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (colza-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (colza-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (colza-admin
    PROPERTIES VERSION ${COLZA_VERSION}
    SOVERSION ${COLZA_VERSION_MAJOR})

if (${ENABLE_BEDROCK})
# bedrock module libraries
add_library (colza-bedrock-module ${module-src-files})
target_link_libraries (colza-bedrock-module colza-server colza-admin colza-client)
target_include_directories (colza-bedrock-module PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (colza-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (colza-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (colza-bedrock-module
    PROPERTIES VERSION ${COLZA_VERSION}
    SOVERSION ${COLZA_VERSION_MAJOR})
endif ()

# installation stuff (packaging and install commands)
write_basic_package_version_file(
    "colza-config-version.cmake"
    VERSION ${COLZA_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
set(INCLUDE_INSTALL_DIR include/)
set(LIB_INSTALL_DIR lib/)
configure_package_config_file(colza-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/colza-config.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/colza/cmake
  PATH_VARS INCLUDE_INSTALL_DIR)

# some bits for the pkg-config file
set (DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set (SERVER_PRIVATE_LIBS "-lcolza-server")
set (CLIENT_PRIVATE_LIBS "-lcolza-client")
set (ADMIN_PRIVATE_LIBS  "-lcolza-admin")
configure_file ("colza-server.pc.in" "colza-server.pc" @ONLY)
configure_file ("colza-client.pc.in" "colza-client.pc" @ONLY)
configure_file ("colza-admin.pc.in"  "colza-admin.pc"  @ONLY)

# configure config.h
configure_file ("config.h.in" "config.h" @ONLY)

# "make install" rules
install (TARGETS colza-admin colza-server colza-client
         EXPORT colza-targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)
if (${ENABLE_BEDROCK})
    install (TARGETS colza-bedrock-module
             ARCHIVE DESTINATION lib
             LIBRARY DESTINATION lib)
endif ()
install (EXPORT colza-targets
         DESTINATION ${colza-pkg}
         FILE "colza-targets.cmake")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/colza-config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/colza-config-version.cmake"
         DESTINATION ${colza-pkg} )
install (DIRECTORY ../include/colza
         DESTINATION include
         FILES_MATCHING PATTERN "*.h")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/colza-server.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/colza-client.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/colza-admin.pc"
         DESTINATION "lib/pkgconfig/")
