cmake_minimum_required(VERSION 3.5)

project(adcmminer-ipc VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_THREAD_LIBS_INIT)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(Threads_FOUND TRUE)
endif()

find_package(QT NAMES Qt6 COMPONENTS Widgets Core Charts Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core Charts Concurrent REQUIRED)

add_subdirectory(adcmminer-client)
add_subdirectory(adcmminer-server)
