# list of source files
file(GLOB colza-src **/*.cpp *.cpp)

# load package helper for generating cmake CONFIG packages
include (CMakePackageConfigHelpers)

# where to install files for "find_package"
set (colza-pkg "share/cmake/colza")

#
# library version set here (e.g. for shared libs).
#
set (COLZA_VERSION_MAJOR 0)
set (COLZA_VERSION_MINOR 1)
set (COLZA_VERSION_PATCH 0)
set (colza-vers "${COLZA_VERSION_MAJOR}.${COLZA_VERSION_MINOR}")
set (COLZA_VERSION "${colza-vers}.${COLZA_VERSION_PATCH}")

add_library(colza ${colza-src})
target_link_libraries (colza uuid thallium ssg)
target_include_directories (colza PUBLIC $<INSTALL_INTERFACE:include>)

# local include's BEFORE, in case old incompatable .h files in prefix/include
target_include_directories (colza BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)

# for shared libs, establish the lib version
set_target_properties (colza
    PROPERTIES VERSION ${REMI_VERSION}
    SOVERSION ${REMI_VERSION_MAJOR})

#
# installation stuff (packaging and install commands)
#
write_basic_package_version_file(
    "colza-config-version.cmake"
    VERSION ${COLZA_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
configure_file (colza-config.cmake.in colza-config.cmake @ONLY)

# some bits for the pkg-config file
set (DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set (PRIVATE_LIBS "-lcolza")
configure_file ("colza.pc.in" "colza.pc" @ONLY)

#
# "make install" rules
#
install (TARGETS colza EXPORT colza-targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)
install (EXPORT colza-targets
         DESTINATION ${colza-pkg}
         FILE "colza-targets.cmake")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/colza-config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/colza-config-version.cmake"
               "../cmake/xpkg-import.cmake"
         DESTINATION ${colza-pkg} )
install (DIRECTORY ../include/colza
         DESTINATION include
         FILES_MATCHING PATTERN "*.h")
install (DIRECTORY ../include/colza
         DESTINATION include
         FILES_MATCHING PATTERN "*.hpp")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/colza.pc"
         DESTINATION "lib/pkgconfig/")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/../include/colza/config.hpp"
         DESTINATION include/colza)
