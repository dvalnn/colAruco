cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 20)

project(arucoRec)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(arucoRec src/arucoRec.cpp 
                        src/arucoSettings.cpp   include/arucoSettings.hpp
                        src/cameraSettings.cpp  include/arucoSettings.hpp)

target_link_libraries(arucoRec ${OpenCV_LIBS})