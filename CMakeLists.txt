cmake_minimum_required(VERSION 3.16.0)

project(picasso LANGUAGES C CXX ASM HOMEPAGE_URL https://onecoolx.github.io/picasso/)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(PROJECT_OUT ${CMAKE_CURRENT_BINARY_DIR})

set(VERSION_MAJOR 2)
set(VERSION_MINOR 8)
set(VERSION_MICRO 0)
set(VERSION_INFO ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

option(BUILD_SHARED_LIBS "Build shared libraries (default)" ON)

option(OPT_FORMAT_ABGR "Pixel format ABGR support." ON)
option(OPT_FORMAT_ARGB "Pixel format ARGB support." ON)
option(OPT_FORMAT_BGRA "Pixel format BGRA support." ON)
option(OPT_FORMAT_RGBA "Pixel format RGBA support." ON)
option(OPT_FORMAT_RGB "Pixel format RGB support." ON)
option(OPT_FORMAT_BGR "Pixel format BGR support." ON)
option(OPT_FORMAT_RGB565 "Pixel format RGB565 support." ON)
option(OPT_FORMAT_RGB555 "Pixel format RGB555 support." ON)
option(OPT_FORMAT_A8 "Pixel format A8 support." ON)

option(OPT_FAST_COPY "Build Fast Memory Copy used support." OFF)
option(OPT_FREE_TYPE2 "Build FreeType2 is support." OFF)
option(OPT_FONT_CONFIG "Build FontConfig is support." OFF)
option(OPT_LOW_MEMORY "Build Low Memory used support." OFF)
option(OPT_SYSTEM_MALLOC "Build System Memory Allocator (new/delete/malloc/free/realloc/calloc) used support." OFF)

option(OPT_EXTENSIONS "Build extension libraries." ON)

option(OPT_TESTS "Build test apps" ON)
option(OPT_DEMOS "Build example demos" ON)
option(OPT_UNITTEST "Build unit tests" OFF)
option(OPT_SANITIZE "Build with Sanitizer" OFF)
option(OPT_COVERAGE "Build with Coverage" OFF)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

include (${CMAKE_CURRENT_LIST_DIR}/build/configs.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/build/defines.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/src/src.cmake)

if (OPT_EXTENSIONS)
include (${CMAKE_CURRENT_LIST_DIR}/third_party/third_party.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/ext/ext.cmake)
endif()

include (${CMAKE_CURRENT_LIST_DIR}/include/include.cmake)

if (OPT_TESTS)
include (${CMAKE_CURRENT_LIST_DIR}/test/test.cmake)
endif()

if (OPT_DEMOS)
include (${CMAKE_CURRENT_LIST_DIR}/demos/demos.cmake)
endif()

if (OPT_UNITTEST)
enable_testing()
include (${CMAKE_CURRENT_LIST_DIR}/unit_tests/unit_tests.cmake)
endif(OPT_UNITTEST)

include(CheckIncludeFile)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(stddef.h HAVE_STDDEF_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(memory.h HAVE_MEMORY_H)

if (OPT_FORMAT_ABGR)
    set(ENABLE_FORMAT_ABGR 1)
endif(OPT_FORMAT_ABGR)

if (OPT_FORMAT_ARGB)
    set(ENABLE_FORMAT_ARGB 1)
endif(OPT_FORMAT_ARGB)

if (OPT_FORMAT_BGRA)
    set(ENABLE_FORMAT_BGRA 1)
endif(OPT_FORMAT_BGRA)

if (OPT_FORMAT_RGBA)
    set(ENABLE_FORMAT_RGBA 1)
endif(OPT_FORMAT_RGBA)

if (OPT_FORMAT_RGB)
    set(ENABLE_FORMAT_RGB 1)
endif(OPT_FORMAT_RGB)

if (OPT_FORMAT_BGR)
    set(ENABLE_FORMAT_BGR 1)
endif(OPT_FORMAT_BGR)

if (OPT_FORMAT_RGB565)
    set(ENABLE_FORMAT_RGB565 1)
endif(OPT_FORMAT_RGB565)

if (OPT_FORMAT_RGB555)
    set(ENABLE_FORMAT_RGB555 1)
endif(OPT_FORMAT_RGB555)

if (OPT_FORMAT_A8)
    set(ENABLE_FORMAT_A8 1)
endif(OPT_FORMAT_A8)

if (OPT_FAST_COPY)
    set(ENABLE_FAST_COPY 1)
endif(OPT_FAST_COPY)

if (OPT_FREE_TYPE2)
    set(ENABLE_FREE_TYPE2 1)
endif(OPT_FREE_TYPE2)

if (OPT_FONT_CONFIG)
    set(ENABLE_FONT_CONFIG 1)
endif(OPT_FONT_CONFIG)

if (OPT_LOW_MEMORY)
    set(ENABLE_LOW_MEMORY 1)
endif(OPT_LOW_MEMORY)

if (OPT_SYSTEM_MALLOC)
    set(ENABLE_SYSTEM_MALLOC 1)
endif(OPT_SYSTEM_MALLOC)

configure_file(
    "${PROJECT_ROOT}/build/pconfig.h.in"
    "${PROJECT_OUT}/include/pconfig.h"
)

