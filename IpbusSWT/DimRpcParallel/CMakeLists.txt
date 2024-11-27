cmake_minimum_required(VERSION 3.5)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
  cmake_policy(VERSION 3.12)
endif()

project(DimRpcParallel 
	VERSION 0.1
	DESCRIPTION "Parallel DIM RPC"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

set(CMAKE_INSTALL_LIBDIR lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(DIM REQUIRED)

include(GNUInstallDirs)

# Build targets with install rpath on Mac to dramatically speed up installation
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
  endif()
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()
unset(isSystemDir)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(INCLUDE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/include")

add_library(DimRpcParallel SHARED 
	src/dimrpcparallel.cpp
	src/dimcommandparallel.cpp
	src/dimrpcqueue.cpp
)

target_include_directories(DimRpcParallel
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(DimRpcParallel
	PUBLIC
		dim::dim
)

install(TARGETS DimRpcParallel
	EXPORT DimRpcParallelTargets
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/DimRpcParallel DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(EXPORT DimRpcParallelTargets
	FILE DimRpcParallelTargets.cmake
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DimRpcParallel")

include(CMakePackageConfigHelpers)

write_basic_package_version_file(DimRpcParallelConfigVersion.cmake
                                 VERSION ${DimRpcParallel_VERSION}
                                 COMPATIBILITY SameMajorVersion)
configure_package_config_file(cmake/DimRpcParallelConfig.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/DimRpcParallelConfig.cmake"
                              INSTALL_DESTINATION
                              "${CMAKE_INSTALL_LIBDIR}/cmake/DimRpcParallel")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DimRpcParallelConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/DimRpcParallelConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DimRpcParallel)

