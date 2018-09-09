cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(BUILD_SHARED_LIBS ON)

#INCLUDE(cotire)

project(oldpart)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Werror=uninitialized -Werror=return-type")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES($ENV{BOOST_INCLUDE_PATH})
LINK_DIRECTORIES($ENV{BOOST_LIB_PATH})

add_subdirectory(common)
add_subdirectory(opc)
add_subdirectory(ops)
add_subdirectory(test)

FILE(GLOB_RECURSE ${TARGET}_HEADER_LISTS "*.h" "*.hpp")
INSTALL_FILES(include ${TARGET}_HEADER_LISTS)

