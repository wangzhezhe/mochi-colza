#
#  general cmake flags:
#    -DCMAKE_INSTALL_PREFIX=/usr/local     -- the prefix for installing
#    -DCMAKE_BUILD_TYPE=type               -- type can be Debug, Release, ...
#    -DCMAKE_PREFIX_PATH=/dir              -- external packages
#
#     note that CMAKE_PREFIX_PATH can be a list of directories:
#      -DCMAKE_PREFIX_PATH='/dir1;/dir2;/dir3'
#

cmake_minimum_required (VERSION 3.0)
project (colza C CXX)
enable_testing ()

option(ENABLE_TESTS    "Build tests. May require CppUnit_ROOT" OFF)
option(ENABLE_EXAMPLES "Build examples" OFF)
option(ENABLE_MPI      "Build with MPI support" ON)

# add our cmake module directory to the path
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# link shared lib with full rpath
set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# setup cache variables for ccmake
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release
         CACHE STRING "Choose the type of build." FORCE)
    set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS 
                  "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif ()
set (CMAKE_PREFIX_PATH "" CACHE STRING "External dependencies path")
set (BUILD_SHARED_LIBS "OFF" CACHE BOOL "Build a shared library")

if(ENABLE_MPI)
    find_package(MPI REQUIRED)
endif(ENABLE_MPI)

find_package (CppUnit)
if (CPPUNIT_FOUND)
    message(STATUS "CppUnit found, unit tests will be compiled")
    include_directories(${CPPUNIT_INCLUDE_DIR})
    enable_testing()
else (CPPUNIT_FOUND)
    message(STATUS "CppUnit not found, unit tests will not be compiled")
endif (CPPUNIT_FOUND)


# packages we depend on
include (xpkg-import)
find_package (thallium REQUIRED)
xpkg_import_module (ssg REQUIRED ssg)

add_subdirectory (src)
if(CPPUNIT_FOUND AND ENABLE_TESTS AND ENABLE_MPI)
    message(STATUS "Unit tests are enabled, MPI and CppUnit are available, tests will be built")
    add_subdirectory (tests)
endif(CPPUNIT_FOUND AND ENABLE_TESTS AND ENABLE_MPI)
if(${ENABLE_EXAMPLES})
  add_subdirectory (examples)
endif(${ENABLE_EXAMPLES})
