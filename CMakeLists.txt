cmake_minimum_required(VERSION 2.8)
project (libnpengine)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
find_package(SFML 2 REQUIRED COMPONENTS system window graphics)
find_package(Boost REQUIRED COMPONENTS locale thread)
find_package(GStreamer REQUIRED)
find_package(GLIB REQUIRED)

add_definitions(-std=c++11)

set(CMAKE_BUILD_TYPE Debug)
include_directories(
${SFML_INCLUDE_DIR}
${GSTREAMER_VIDEO_INCLUDE_DIRS}
${GLIB_INCLUDE_DIRS}
${CMAKE_SOURCE_DIR}/include
${CMAKE_SOURCE_DIR}/../libnpa/include
)

file(
GLOB_RECURSE
LIBNPENGINE_SRC
${CMAKE_SOURCE_DIR}/src/*
)

link_directories(
${Boost_LIBRARY_DIRS}
${CMAKE_SOURCE_DIR}/../libnpa/
)

add_library(npengine SHARED ${LIBNPENGINE_SRC})
target_link_libraries(npengine npa)
set_target_properties(npengine PROPERTIES LINKER_LANGUAGE CXX)
