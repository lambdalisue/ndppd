cmake_minimum_required(VERSION 3.0)
project(ndppd)

set (CMAKE_CXX_STANDARD 14)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

INCLUDE(FindPkgConfig)
pkg_check_modules(LIBNL3 libnl-3.0)
pkg_check_modules(LIBNL3_ROUTE libnl-route-3.0)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cc ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(ndppd ${SOURCES})
target_include_directories(ndppd PUBLIC ${LIBNL3_INCLUDE_DIRS} ${LIBNL3_ROUTE_INCLUDE_DIRS})
link_directories(${LIBNL3_LIBRARY_DIRS} ${LIBNL3_ROUTE_LIBRARY_DIRS})
target_link_libraries(ndppd Threads::Threads ${LIBNL3_LIBRARIES} ${LIBNL3_ROUTE_LIBRARIES})