cmake_minimum_required(VERSION 3.16.0)

project(picasso LANGUAGES C CXX ASM HOMEPAGE_URL https://onecoolx.github.io/picasso/)

set(BUILD_SHARED_LIBS ON) # build shared library, OFF for static

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build Type" FORCE)
endif()

include (${CMAKE_CURRENT_LIST_DIR}/build/configs.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/build/defines.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/third_party/third_party.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/src/src.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/ext/ext.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/test/test.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/demos/demos.cmake)

