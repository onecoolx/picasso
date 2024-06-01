# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(ZLIB_DIR ${PROJECT_ROOT}/third_party/zlib-1.2.8)

set(ZLIB_SOURCES
    ${ZLIB_DIR}/adler32.c
    ${ZLIB_DIR}/compress.c
    ${ZLIB_DIR}/crc32.c
    ${ZLIB_DIR}/deflate.c
    ${ZLIB_DIR}/infback.c
    ${ZLIB_DIR}/inffast.c
    ${ZLIB_DIR}/inflate.c
    ${ZLIB_DIR}/inftrees.c
    ${ZLIB_DIR}/trees.c
    ${ZLIB_DIR}/uncompr.c
    ${ZLIB_DIR}/zutil.c
)

add_definitions(-DZLIB_DLL)
configure_file(${ZLIB_DIR}/zconf.h ${CMAKE_CURRENT_BINARY_DIR}/include/zconf.h)
configure_file(${ZLIB_DIR}/zlib.h ${CMAKE_CURRENT_BINARY_DIR}/include/zlib.h)

add_library(zlib ${ZLIB_SOURCES})

include_directories(${ZLIB_DIR} ${CMAKE_CURRENT_BINARY_DIR}/include)

