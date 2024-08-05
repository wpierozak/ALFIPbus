cmake_minimum_required(VERSION 3.5)

# Project name and version
project(IPbusProject VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Set the output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Include GNUInstallDirs to use standard directory variables
include(GNUInstallDirs)

# Find Boost libraries
find_package(Boost REQUIRED COMPONENTS system log)

# Add the library
add_library(ipbus STATIC src/IPbusMaster.cxx src/IPbusRequest.cxx src/IPbusResponse.cxx src/IPbusSlave.cxx)

# Link Boost to the library
target_include_directories(ipbus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(ipbus Boost::system Boost::log)

# Add an option for TEST
option(TEST "Compile the flp_test target" OFF)

if(TEST)
    # Add the test executable
    add_executable(flp_test test/flp_test.cpp)
    add_executable(slave_test test/slave_test.cpp)

    # Link the test executable with the static library and Boost.System library
    target_include_directories(flp_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
    target_link_libraries(flp_test ipbus Boost::system pthread dim)
    target_include_directories(slave_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
    target_link_libraries(slave_test ipbus Boost::system pthread dim)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)