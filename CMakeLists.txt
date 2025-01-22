cmake_minimum_required(VERSION 3.5)
enable_testing()

# Project name and version
project(AlfIPbus VERSION 1.0 LANGUAGES CXX)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Set the output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

find_package(Boost REQUIRED COMPONENTS system log log_setup program_options)

#add_subdirectory(IpbusSWT)
file(GLOB_RECURSE SOURCES "src/*.cxx")
add_executable(AlfIPbus src/AlfIPbus.cxx src/main.cxx src/AlfConfig.cxx ${SOURCES})
# Include directories
target_include_directories(AlfIPbus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(AlfIPbus DimRpcParallel dim pthread Boost::system Boost::log_setup Boost::log Boost::program_options)
target_include_directories(AlfIPbus PUBLIC ${CMAKE_SOURCE_DIR}/IpbusSWT/DimRpcParallel/include/DimRpcParallel)

set(CMAKE_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Check if we are in debug mode and define DEBUG macro
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(AlfIPbus PRIVATE DEBUG)
    message("DEBUG mode compilation")
endif()

# Include GNUInstallDirs to use standard directory variables
include(GNUInstallDirs)


add_subdirectory(IpbusSWT/DimRpcParallel)


add_test(NAME in-container-test COMMAND ./test/in-container-test/in-container-test.sh)
set_tests_properties(in-container-test PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_tests_properties(in-container-test PROPERTIES
    ENVIRONMENT "PATH=/usr/local/bin:${PATH};LD_LIBRARY_PATH=/usr/local/lib")

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

