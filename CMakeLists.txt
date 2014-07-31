cmake_minimum_required(VERSION 2.8)

# add modules
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

# set version
set(LIBNPENGINE_VERSION_MAJOR "0")
set(LIBNPENGINE_VERSION_MINOR "0")
set(LIBNPENGINE_VERSION_PATCH "0")
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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -Wall")

# include version number in header
configure_file(${PROJECT_SOURCE_DIR}/include/npengineversion.hpp.in
    ${PROJECT_SOURCE_DIR}/include/npengineversion.hpp)

find_package(NPA REQUIRED)
include(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(libpng REQUIRED libpng)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${NPA_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

link_directories(
)

add_library(npengine SHARED
    src/Window.cpp
    src/NSBInterpreter.cpp
    src/ResourceMgr.cpp
    src/Texture.cpp
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
