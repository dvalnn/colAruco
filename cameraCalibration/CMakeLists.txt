cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 20)

project(cameraCalibration)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(cameraCalibration src/cameraCalibration.cpp)
target_link_libraries(cameraCalibration ${OpenCV_LIBS})