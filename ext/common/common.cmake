# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXCOMMON_DIR ${PROJECT_ROOT}/ext/common)

set(PXCOMMON_SOURCES
    ${PXCOMMON_DIR}/psx_linear_allocator.h
    ${PXCOMMON_DIR}/psx_linear_allocator.c
    ${PXCOMMON_DIR}/psx_file.h
    ${PXCOMMON_DIR}/psx_file.c
    ${PXCOMMON_DIR}/psx_common.h
    ${PXCOMMON_DIR}/psx_common.c
)

set(LIBX_COMMON psx_common)

add_library(${LIBX_COMMON} STATIC ${PXCOMMON_SOURCES})
install(TARGETS ${LIBX_COMMON} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
include_directories(${PXCOMMON_DIR} ${PROJECT_ROOT}/ext/common ${PROJECT_ROOT}/include ${PROJECT_OUT}/include)

