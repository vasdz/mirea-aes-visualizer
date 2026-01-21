cmake_minimum_required(VERSION 3.20)
project(mirea_aes_visualizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(mirea_aes_visualizer main.cpp mainwindow.cpp mainwindow.hpp)
target_link_libraries(mirea_aes_visualizer PRIVATE Qt6::Widgets)
