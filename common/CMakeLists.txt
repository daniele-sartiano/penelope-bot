cmake_minimum_required(VERSION 3.10)
project(common)

set(CMAKE_CXX_STANDARD 14)

include(ExternalProject)
include(GNUInstallDirs)

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
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/natsc/src)
add_library(NatsCClientLib UNKNOWN IMPORTED)
set_target_properties(NatsCClientLib PROPERTIES IMPORTED_LOCATION ${install_dir}/lib/libnats.so)
add_dependencies(NatsCClientLib NatsCClient)

#########################################################################
set(common_SRC src/natscommunication.cpp)
set(common_HDR src/natscommunication.h)

add_library(common SHARED ${common_SRC} ${common_HDR})
target_link_libraries(common NatsCClientLib)

set_target_properties(common PROPERTIES PUBLIC_HEADER src/natscommunication.h)
target_include_directories(common PUBLIC src)

install(TARGETS common
        EXPORT common
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/common)


