cmake_minimum_required(VERSION 3.0)
project(ndppd)

set (CMAKE_CXX_STANDARD 14)

set(CMAKE_VERBOSE_MAKEFILE ON)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cc ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(ndppd ${SOURCES})
