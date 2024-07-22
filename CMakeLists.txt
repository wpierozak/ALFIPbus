cmake_minimum_required(VERSION 3.5)

# Project name and version
project(IPbusSWT VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/inc ${PROJECT_SOURCE_DIR}/IPbus/inc ${PROJECT_SOURCE_DIR}/DimRpcParallel/include/DimRpcParallel)

# Set the output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)

# Include GNUInstallDirs to use standard directory variables
include(GNUInstallDirs)

# Find Boost libraries
find_package(Boost REQUIRED COMPONENTS system)

# Add subdirectories
add_subdirectory(IPbus)
add_subdirectory(DimRpcParallel)

# Add the ipbus_swt library
add_library(ipbus_swt STATIC src/SWT.cpp src/SWTelectronics.cpp src/Utils.cpp)

# Add the test executables
add_executable(flp_test_server test/flp_test_server.cpp)
add_executable(flp_test_client test/flp_test_client.cpp)

# Link the test executables with the libraries
target_link_libraries(flp_test_server IPbusInterface ipbus_swt DimRpcParallel Boost::system pthread dim)
target_link_libraries(flp_test_client IPbusInterface ipbus_swt DimRpcParallel Boost::system pthread dim)
target_link_libraries(ipbus_swt IPbusInterface DimRpcParallel Boost::system dim)
