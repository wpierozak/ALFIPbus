cmake_minimum_required(VERSION 3.5)

# Project name and version
project(AlfIPbus VERSION 1.0 LANGUAGES CXX)

# Set the output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

find_package(Boost REQUIRED COMPONENTS system log log_setup program_options)

add_subdirectory(IpbusSWT)
add_executable(AlfIPbus src/AlfIPbus.cxx src/main.cxx src/AlfConfig.cxx)
# Include directories
target_include_directories(AlfIPbus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# set(CMAKE_BUILD_TYPE "Release")
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


target_link_libraries(AlfIPbus ipbus_swt DimRpcParallel dim pthread Boost::system Boost::log_setup Boost::log Boost::program_options)
