cmake_minimum_required(VERSION 3.0)

# add modules
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

# set version
set(LIBNPENGINE_VERSION_MAJOR "0")
set(LIBNPENGINE_VERSION_MINOR "5")
set(LIBNPENGINE_VERSION_PATCH "3")
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
find_package(GLEW REQUIRED)
find_package(JPEG REQUIRED)
find_package(GStreamer REQUIRED)
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(PNG REQUIRED)
find_package(Threads REQUIRED)
include(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(libpng REQUIRED libpng)
PKG_SEARCH_MODULE(PANGOCAIRO REQUIRED pangocairo)

string(REPLACE "." ";" VERSION_LIST ${FLEX_VERSION})
list(GET VERSION_LIST 0 FLEX_VERSION_MAJOR)
list(GET VERSION_LIST 1 FLEX_VERSION_MINOR)
list(GET VERSION_LIST 2 FLEX_VERSION_PATCH)
add_definitions(-DFLEX_VERSION_MAJOR=${FLEX_VERSION_MAJOR})
add_definitions(-DFLEX_VERSION_MINOR=${FLEX_VERSION_MINOR})
add_definitions(-DFLEX_VERSION_PATCH=${FLEX_VERSION_PATCH})

include_directories(
    ${PANGOCAIRO_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${NPA_INCLUDE_DIR}
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_VIDEO_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)

link_directories(
)

flex_target(lexer ${CMAKE_SOURCE_DIR}/src/lexer.l ${CMAKE_SOURCE_DIR}/src/Lexer.cpp)
bison_target(parser ${CMAKE_SOURCE_DIR}/src/parser.y ${CMAKE_SOURCE_DIR}/src/Parser.cpp)

add_library(npengine SHARED
    src/Window.cpp
    src/NSBInterpreter.cpp
    src/ResourceMgr.cpp
    src/Texture.cpp
    src/Variable.cpp
    src/NSBContext.cpp
    src/GLTexture.cpp
    src/Playable.cpp
    src/Movie.cpp
    src/Choice.cpp
    src/Text.cpp
    src/Parser.cpp
    src/Lexer.cpp
    src/Image.cpp
    src/NSBDebugger.cpp
    src/Scrollbar.cpp
)

target_link_libraries(npengine
	${CMAKE_THREAD_LIBS_INIT}
	${SDL2_LIBRARIES}
	${NPA_LIBRARY}
	${GSTREAMER_LIBRARIES}
	${GSTREAMER_APP_LIBRARIES}
	${GSTREAMER_VIDEO_LIBRARIES}
	${PANGOCAIRO_LIBRARIES} 
	${JPEG_LIBRARY} 
	${GLEW_LIBRARY} 
	${PNG_LIBRARIES}
	-lGL)

# install headers and library
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION include/libnpengine
    FILES_MATCHING PATTERN "*.hpp")
install(TARGETS npengine DESTINATION lib)

# create packages
set(CPACK_GENERATOR "TBZ2")
set(CPACK_PACKAGE_VERSION ${LIBNPENGINE_VERSION})
include(CPack)
