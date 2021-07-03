cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 20)

project(demo)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(demo demo.cpp)
target_link_libraries(demo ${OpenCV_LIBS})