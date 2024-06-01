# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXPNG_DIR ${PROJECT_ROOT}/ext/image_loader/png)

set(PXPNG_SOURCES
    ${PXPNG_DIR}/../psx_image_io.h
    ${PXPNG_DIR}/../psx_image_io.c
    ${PXPNG_DIR}/png_module.c
)

add_definitions(-DEXPORT)
add_library(psxm_image_png ${PXPNG_SOURCES})
target_link_libraries(psxm_image_png PRIVATE png)

set_target_properties(psxm_image_png
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules"
)
