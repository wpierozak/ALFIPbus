cmake_minimum_required(VERSION 3.5)

# Project name and version
project(IPbusProject VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/inc)

# Set the output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set (CMAKE_CXX_FLAGS  "-lboost_system -lpthread")

# Include GNUInstallDirs to use standard directory variables
include(GNUInstallDirs)

# Add the library
add_library(IPbusInterface STATIC src/IPbusInterface.cpp)

# Add the test executable
add_executable(flp_test test/flp_test.cpp)

# Link the test executable with the static library
target_link_libraries(flp_test IPbusInterface)

set (CMAKE_CXX_FLAGS  "-lboost_system -ldim -lpthread")