# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXGIF_DIR ${PROJECT_ROOT}/ext/image_loader/gif)

set(PXGIF_SOURCES
    ${PXGIF_DIR}/../psx_image_io.h
    ${PXGIF_DIR}/../psx_image_io.c
    ${PXGIF_DIR}/gif_module.c
)

add_definitions(-DEXPORT)
add_library(psxm_image_gif ${PXGIF_SOURCES})
target_link_libraries(psxm_image_gif PRIVATE gif)

set_target_properties(psxm_image_gif
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules"
)
