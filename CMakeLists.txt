cmake_minimum_required(VERSION 2.8)
project (libnpengine)

if (CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=gnu++11)
endif()

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
find_package(SFML 2 REQUIRED COMPONENTS system audio window graphics)
find_package(sfeMovie)
find_package(Boost REQUIRED COMPONENTS locale thread)

set(CMAKE_BUILD_TYPE Debug)
include_directories(
${SFML_INCLUDE_DIR}
${SFEMOVIE_INCLUDE_DIR}
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
target_link_libraries(npengine ${SFEMOVIE_LIBRARY} ${Boost_LIBRARIES} npa)
set_target_properties(npengine PROPERTIES LINKER_LANGUAGE CXX)
