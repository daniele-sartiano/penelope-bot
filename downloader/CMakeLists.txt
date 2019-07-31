cmake_minimum_required(VERSION 3.10)
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

find_package(hiredis REQUIRED)

add_executable(downloader main.cpp src/QueueManager.cpp src/QueueManager.h src/Downloader.cpp src/Downloader.h)

TARGET_LINK_LIBRARIES(downloader ${CMAKE_THREAD_LIBS_INIT})
TARGET_LINK_LIBRARIES(downloader ${CURL_LIBRARIES})
TARGET_LINK_LIBRARIES(downloader ${HIREDIS_LIBRARIES})
