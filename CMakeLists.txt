cmake_minimum_required(VERSION 3.5)

# Project name and version
project(ALFIPbus VERSION 1.0 LANGUAGES CXX)

# Set the output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)


add_subdirectory(IpbusSWT)
add_executable(ALFIPbus src/ALFIPbus.cpp apps/ALF.cpp)
# Include directories
target_include_directories(ALFIPbus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)


# Include GNUInstallDirs to use standard directory variables
include(GNUInstallDirs)


target_link_libraries(ALFIPbus ipbus_swt DimRpcParallel dim pthread)
