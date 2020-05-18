cmake_minimum_required(VERSION 3.15)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
include(ExternalProject)

project(data_manager)

set(CMAKE_CXX_STANDARD 14)


#########################################################################
# Nats C Client

ExternalProject_Add(
        NatsCClient

        GIT_REPOSITORY "https://github.com/nats-io/nats.c.git"
        GIT_TAG "master"

        UPDATE_COMMAND ""
        PATCH_COMMAND ""

        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/natsc"
        CMAKE_ARGS -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Get_property(NatsCClient install_dir)
include_directories(${install_dir}/include)
add_library(NatsCClientLib UNKNOWN IMPORTED)
set_target_properties(NatsCClientLib PROPERTIES IMPORTED_LOCATION ${install_dir}/lib/libnats.so)
add_dependencies(NatsCClientLib NatsCClient)

#########################################################################
# Scylla Driver

ExternalProject_Add(
        ScyllaDriver

        GIT_REPOSITORY "https://github.com/datastax/cpp-driver.git"
        GIT_TAG "master"

        UPDATE_COMMAND ""
        PATCH_COMMAND ""

        SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/scylla"
        CMAKE_ARGS -DBuildShared=ON -DBuildExamples=OFF -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>

)

ExternalProject_Get_property(ScyllaDriver install_dir)
include_directories(${install_dir}/include)
add_library(ScyllaDriverLib UNKNOWN IMPORTED)
set_target_properties(ScyllaDriverLib PROPERTIES IMPORTED_LOCATION ${install_dir}/lib/libcassandra.so)
add_dependencies(ScyllaDriverLib ScyllaDriver)
#############################################################
# Common lib
include_directories(${CMAKE_SOURCE_DIR}/common/include)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/common)
add_library(libcommon SHARED IMPORTED)
set_property(TARGET libcommon PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/common/libcommon.so")

#########################################################################
add_executable(data_manager main.cpp)

TARGET_LINK_LIBRARIES(data_manager NatsCClientLib)
TARGET_LINK_LIBRARIES(data_manager ScyllaDriverLib)
TARGET_LINK_LIBRARIES(data_manager libcommon)