cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

include(ExternalProject)

project(downloader)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(Threads REQUIRED)
if(THREADS_FOUND)
    message(STATUS "Threads libraries found at: ${THREADS_LIBRARIES}")
    message(STATUS "Threads includes found at: ${THREADS_INCLUDE_DIRS}")
else()
    message(SEND_ERROR "Could not find Threads on your system")
endif(THREADS_FOUND)

find_package(CURL REQUIRED)
if(CURL_FOUND)
    message(STATUS "Curl libraries found at: ${CURL_LIBRARIES}")
    message(STATUS "Curl includes found at: ${CURL_INCLUDE_DIRS}")
else()
    message(SEND_ERROR "Could not find cURL on your system")
endif(CURL_FOUND)

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
# Common lib
include_directories(${CMAKE_SOURCE_DIR}/common/include)
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/common)
add_library(libcommon SHARED IMPORTED)
set_property(TARGET libcommon PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/common/libcommon.so")

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

# Prepare RapidJSON (RapidJSON is a header-only library)
ExternalProject_Get_Property(rapidjson source_dir)
set(RAPIDJSON_INCLUDE_DIR ${source_dir}/include)
include_directories(${RAPIDJSON_INCLUDE_DIR})
#########################################################################
find_package(hiredis REQUIRED)
find_package(rdkafka REQUIRED)

add_executable(downloader main.cpp src/QueueManager.cpp src/QueueManager.h src/Downloader.cpp src/Downloader.h src/KafkaReader.cpp src/KafkaReader.h)
add_dependencies(downloader rapidjson)

#TARGET_LINK_LIBRARIES(downloader ${CMAKE_THREAD_LIBS_INIT})
TARGET_LINK_LIBRARIES(downloader ${CURL_LIBRARIES})
TARGET_LINK_LIBRARIES(downloader ${HIREDIS_LIBRARIES})
TARGET_LINK_LIBRARIES(downloader ${RDKAFKA_LIBRARIES})
TARGET_LINK_LIBRARIES(downloader NatsCClientLib)
TARGET_LINK_LIBRARIES(downloader libcommon)
