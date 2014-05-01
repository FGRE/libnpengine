cmake_minimum_required(VERSION 2.8)

# add modules
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

# set version
set(LIBNPENGINE_VERSION_MAJOR "0")
set(LIBNPENGINE_VERSION_MINOR "3")
set(LIBNPENGINE_VERSION_PATCH "2")
set(LIBNPENGINE_VERSION
    "${LIBNPENGINE_VERSION_MAJOR}.${LIBNPENGINE_VERSION_MINOR}.${LIBNPENGINE_VERSION_PATCH}"
)

# append git revision if available
include(get-git-revision)
get_git_revision(LIBNPENGINE_VERSION_GIT)
if(NOT "${LIBNPENGINE_VERSION_GIT}" STREQUAL "")
    set(LIBNPENGINE_VERSION "${LIBNPENGINE_VERSION}-${LIBNPENGINE_VERSION_GIT}")
endif()

message(STATUS "Configuring libnpengine version " ${LIBNPENGINE_VERSION})

# project name and language
project (libnpengine CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2")

# include version number in header
configure_file(${PROJECT_SOURCE_DIR}/include/npengineversion.hpp.in
    ${PROJECT_SOURCE_DIR}/include/npengineversion.hpp)

find_package(SFML 2 REQUIRED COMPONENTS system window graphics)
find_package(Boost REQUIRED COMPONENTS locale)
find_package(GStreamer REQUIRED)
find_package(GLIB REQUIRED)
find_package(NPA REQUIRED)

include_directories(
    ${SFML_INCLUDE_DIR}
    ${GSTREAMER_VIDEO_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${NPA_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

link_directories(
    ${Boost_LIBRARY_DIRS}
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

# install headers and library
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION include/libnpengine
    FILES_MATCHING PATTERN "*.hpp")
install(TARGETS npengine DESTINATION lib)

# create packages
set(CPACK_GENERATOR "TBZ2")
set(CPACK_PACKAGE_VERSION ${LIBNPENGINE_VERSION})
include(CPack)
