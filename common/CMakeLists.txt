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
# Download RapidJSON
ExternalProject_Add(
        rapidjson
        PREFIX "vendor/rapidjson"
        GIT_REPOSITORY "https://github.com/Tencent/rapidjson.git"
        GIT_TAG f54b0e47a08782a6131cc3d60f94d038fa6e0a51
        TIMEOUT 10
        CMAKE_ARGS
        -DRAPIDJSON_BUILD_TESTS=OFF
        -DRAPIDJSON_BUILD_DOC=OFF
        -DRAPIDJSON_BUILD_EXAMPLES=OFF
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""
)

ExternalProject_Get_Property(rapidjson source_dir)
set(RAPIDJSON_INCLUDE_DIR ${source_dir}/include)
include_directories(${RAPIDJSON_INCLUDE_DIR})

#########################################################################
# Google Test
# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.gtest.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()
#########################################################################

set(common_SRC src/natscommunication.cpp src/model.cpp)
set(common_HDR src/natscommunication.h src/model.h)

add_library(common SHARED ${common_SRC} ${common_HDR})
add_dependencies(common rapidjson)

target_link_libraries(common NatsCClientLib)

set_target_properties(common PROPERTIES PUBLIC_HEADER src/natscommunication.h)
target_include_directories(common PUBLIC src)

install(TARGETS common
        EXPORT common
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/common)


add_executable(tests ${common_SRC} ${common_HDR} tests/tests.cpp)
add_dependencies(tests rapidjson)
target_include_directories(tests PUBLIC src)
target_link_libraries(tests gtest)
target_link_libraries(tests NatsCClientLib)
add_test(NAME example_test COMMAND tests)