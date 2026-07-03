cmake_minimum_required(VERSION 3.14)

project(adcmminer-server LANGUAGES CXX)

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

find_package(QT NAMES Qt6 COMPONENTS Core Network Concurrent Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network Concurrent Gui REQUIRED)

set(SOURCES
    main.cpp
    server.cpp

    controller.cpp
    decoder.cpp
    filewatcherworker.cpp
    processingworker.cpp
    calibration.cpp
    datadelegate.cpp
    histogrammanager.cpp
    peakfinder.cpp
    energypeak.cpp
    piecewiselinearfunction.cpp
#    timepeaksfinder.cpp
#    channelmap.cpp
)

set(HEADERS
    protocol.h
    server.h
    constants.h
    adcm_df.h
    controller.h
    decoder.h
    filewatcherworker.h
    processingworker.h
    calibration.h
    datadelegate.h
    histogrammanager.h
    peakfinder.h
    energypeak.h
    piecewiselinearfunction.h
    utils.h
#    timepeaksfinder.h
#    channelmap.h
)

add_executable(adcmminer-server
  ${HEADERS}
  ${SOURCES}
)

# Get ROOT include path
execute_process(
    COMMAND root-config --incdir
    OUTPUT_VARIABLE ROOT_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get ROOT libraries
execute_process(
    COMMAND root-config --libs
    OUTPUT_VARIABLE ROOT_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Split the libraries into a list
string(REPLACE " " ";" ROOT_LIBRARIES ${ROOT_LIBS})

target_include_directories(adcmminer-server PRIVATE
    ${ROOT_INCLUDE_DIR}
)

target_link_libraries(adcmminer-server
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Gui
    ${ROOT_LIBRARIES}
)
