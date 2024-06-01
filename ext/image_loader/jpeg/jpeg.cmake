# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXJPEG_DIR ${PROJECT_ROOT}/ext/image_loader/jpeg)

set(PXJPEG_SOURCES
    ${PXJPEG_DIR}/../psx_image_io.h
    ${PXJPEG_DIR}/../psx_image_io.c
    ${PXJPEG_DIR}/jpeg_module.c
)

add_definitions(-DEXPORT)
add_library(psxm_image_jpeg ${PXJPEG_SOURCES})
target_link_libraries(psxm_image_jpeg PRIVATE jpeg)

set_target_properties(psxm_image_jpeg
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules"
)
