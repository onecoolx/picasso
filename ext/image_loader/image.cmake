# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXIMG_DIR ${PROJECT_ROOT}/ext/image_loader)

set(PXIMG_SOURCES
    ${PXIMG_DIR}/psx_list.h
    ${PXIMG_DIR}/psx_image_io.h
    ${PXIMG_DIR}/psx_image_io.c
    ${PXIMG_DIR}/psx_image_loader.h
    ${PXIMG_DIR}/psx_image_loader.c
    ${PXIMG_DIR}/psx_image_modules.h
    ${PXIMG_DIR}/psx_image_modules.c
)

add_definitions(-DEXPORT)
add_library(psx_image ${PXIMG_SOURCES})

include_directories(${PXIMG_DIR} ${PROJECT_ROOT}/include)
target_link_libraries(psx_image PRIVATE picasso PUBLIC dl)

if (NOT APPLE)
include (${PXIMG_DIR}/png/png.cmake)
include (${PXIMG_DIR}/jpeg/jpeg.cmake)
include (${PXIMG_DIR}/gif/gif.cmake)
include (${PXIMG_DIR}/webp/webp.cmake)
else()
include (${PXIMG_DIR}/apple/apple.cmake)
endif()
