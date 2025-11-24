cmake_minimum_required(VERSION 3.16)

project(SwissClock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/ClockWidget.cpp
    src/ClockWidget.h
    src/resources.qrc
)

find_package(X11 REQUIRED)

add_executable(SwissClock ${PROJECT_SOURCES})

target_link_libraries(SwissClock PRIVATE Qt6::Widgets ${X11_LIBRARIES} ${X11_Xshape_LIB} ${X11_Xfixes_LIB})
