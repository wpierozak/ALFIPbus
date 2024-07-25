cmake_minimum_required(VERSION 3.5)

# Project name and version
project(AlfIPbus VERSION 1.0 LANGUAGES CXX)

# Set the output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

find_package(Boost REQUIRED COMPONENTS system log program_options)

add_subdirectory(IpbusSWT)
add_executable(AlfIPbus src/AlfIPbus.cxx src/main.cxx)
# Include directories
target_include_directories(AlfIPbus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)


# Include GNUInstallDirs to use standard directory variables
include(GNUInstallDirs)


target_link_libraries(AlfIPbus ipbus_swt DimRpcParallel dim pthread Boost::system Boost::log Boost::program_options)
