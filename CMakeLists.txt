cmake_minimum_required(VERSION 3.5)

# Project name and version
project(IPbusSWT VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/inc ${PROJECT_SOURCE_DIR}/IPbus/inc ${PROJECT_SOURCE_DIR}/DimRpcParallel/include/DimRpcParallel)

# Set the output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include GNUInstallDirs to use standard directory variables
include(GNUInstallDirs)

# Add the library
add_library(DimRpcParallel SHARED IMPORTED)
set_property(TARGET DimRpcParallel PROPERTY IMPORTED_LOCATION "DimRpcParallel/lib/libDimRpcParallel.so")

add_library(ipbus_swt STATIC src/SWT.cpp src/SWTelectronics.cpp)

add_library(ipbus STATIC IMPORTED)
set_property(TARGET ipbus PROPERTY IMPORTED_LOCATION "../IPbus/lib/libIPbusInterface.a")

# Add the test executable

add_executable(flp_test_server test/flp_test_server.cpp)
add_executable(flp_test_client test/flp_test_client.cpp)

# Link the test executable with the static library
target_link_libraries(flp_test_server ipbus ipbus_swt DimRpcParallel)

set (CMAKE_CXX_FLAGS  "-lboost_system -ldim -lpthread")