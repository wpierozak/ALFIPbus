cmake_minimum_required(VERSION 3.5)

# Project name and version
project(IPbusSWT VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories


# Set the output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Include GNUInstallDirs to use standard directory variables
include(GNUInstallDirs)

# Find Boost libraries
find_package(Boost REQUIRED COMPONENTS system log)

# Add subdirectories
add_subdirectory(IPbus)
add_subdirectory(DimRpcParallel)

# Add the ipbus_swt library
add_library(ipbus_swt STATIC src/Swt.cxx src/SwtLink.cxx src/utils.cxx)

# Add the test executables
add_executable(flp_test_server test/flp_test_server.cpp)
add_executable(flp_test_client test/flp_test_client.cpp)

target_include_directories(ipbus_swt  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_CURRENT_SOURCE_DIR}/IPbus/inc ${CMAKE_CURRENT_SOURCE_DIR}/DimRpcParallel/include/DimRpcParallel)
target_include_directories(flp_test_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_CURRENT_SOURCE_DIR}/IPbus/inc ${CMAKE_CURRENT_SOURCE_DIR}/DimRpcParallel/include/DimRpcParallel)
target_include_directories(flp_test_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_CURRENT_SOURCE_DIR}/IPbus/inc ${CMAKE_CURRENT_SOURCE_DIR}/DimRpcParallel/include/DimRpcParallel)

# Link the test executables with the libraries
target_link_libraries(flp_test_server IPbusInterface ipbus_swt DimRpcParallel Boost::system pthread dim)
target_link_libraries(flp_test_client IPbusInterface ipbus_swt DimRpcParallel Boost::system pthread dim)
target_link_libraries(ipbus_swt IPbusInterface DimRpcParallel pthread Boost::system Boost::log dim)
