cmake_minimum_required(VERSION 3.5)

project(adcmminer-client VERSION 0.1 LANGUAGES CXX)

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

find_package(QT NAMES Qt6 COMPONENTS Widgets Core Charts Concurrent Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core Charts Concurrent Network REQUIRED)

set(SOURCES
    main.cpp
    mainwindow.cpp
    barchartwidget.cpp
    calibration.cpp
    channelmap.cpp
    controller.cpp
    datadelegate.cpp
    decoder.cpp
    filewatcher.cpp
    filewatcherworker.cpp
    flowlayout.cpp
    histchartwidget.cpp
    mousepresseater.cpp
    histogrammanager.cpp
    peakfinder.cpp
    energypeak.cpp
    chartview.cpp
    piecewiselinearfunction.cpp
    piechartwidget.cpp
    processingworker.cpp
    settings.cpp
    myspinnerwidget.cpp
    client.cpp
)

set(HEADERS
    mainwindow.h
    adcm_df.h
    barchartwidget.h
    calibration.h
    channelmap.h
    constants.h
    controller.h
    datadelegate.h
    decoder.h
    filewatcher.h
    filewatcherworker.h
    flowlayout.h
    histchartwidget.h
    mousepresseater.h
    piechartwidget.h
    processingworker.h
    utils.h
    histogrammanager.h
    peakfinder.h
    energypeak.h
    chartview.h
    piecewiselinearfunction.h
    settings.h
    myspinnerwidget.h
    client.h
    protocol.h
)

set(UI_FILES
    mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(adcmminer-client
        MANUAL_FINALIZATION
        ${SOURCES}
        ${HEADERS}
        ${UI_FILES}
        ${RESOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET adcmminer-client APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(adcmminer-client SHARED
            ${SOURCES}
            ${HEADERS}
            ${UI_FILES}
            ${RESOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(adcmminer-client
            ${SOURCES}
            ${HEADERS}
            ${UI_FILES}
            ${RESOURCES}
        )
    endif()
endif()

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

target_include_directories(adcmminer-client PRIVATE
    ${ROOT_INCLUDE_DIR}
)

target_link_libraries(adcmminer-client PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Network
    ${ROOT_LIBRARIES}
)

set_target_properties(adcmminer-client PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(adcmminer-client)
endif()
