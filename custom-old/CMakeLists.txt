cmake_minimum_required(VERSION 3.10)

project(AirmobisCustomBuild VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Base)

qt_add_library(Custom STATIC
    src/CustomPlugin.cc
    src/GeoWork.cc
    src/GeoWork_qmlinit.cc
    src/HerelinkCorePlugin.cc
    src/HerelinkOptions.cc
    src/VideoStreamControl.cc
)

target_include_directories(Custom PUBLIC include)
