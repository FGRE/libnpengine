cmake_minimum_required(VERSION 2.8)
project (libnpengine CXX)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2")

find_package(SFML 2 REQUIRED COMPONENTS system window graphics)
find_package(Boost REQUIRED COMPONENTS locale)
find_package(GStreamer REQUIRED)
find_package(GLIB REQUIRED)

include_directories(
    ${SFML_INCLUDE_DIR}
    ${GSTREAMER_VIDEO_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/../libnpa/include
)

link_directories(
    ${Boost_LIBRARY_DIRS}
    ${CMAKE_SOURCE_DIR}/../libnpa/
)

add_library(npengine SHARED
    src/drawable.cpp
    src/game.cpp
    src/movie.cpp
    src/nsbbuiltins.cpp
    src/nsbcontext.cpp
    src/nsbdebugger.cpp
    src/nsbinterpreter.cpp
    src/nsbmagichandlers.cpp
    src/playable.cpp
    src/resourcemgr.cpp
    src/text.cpp
)
target_link_libraries(npengine npa)

# install headers and library
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION include/libnpengine
    FILES_MATCHING PATTERN "*.hpp")
install(TARGETS npengine DESTINATION lib)
