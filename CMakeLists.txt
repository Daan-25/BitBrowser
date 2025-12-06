cmake_minimum_required(VERSION 3.19)

project(BitBrowser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    WebEngineWidgets
    WebChannel
    Positioning
)

qt_standard_project_setup()

qt_add_executable(BitBrowser
    WIN32
    MACOSX_BUNDLE
    main.cpp
    mainwindow.h
    mainwindow.cpp
)

target_link_libraries(BitBrowser PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::WebChannel
    Qt6::Positioning
)
