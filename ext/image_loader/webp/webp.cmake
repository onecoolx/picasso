# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXWEBP_DIR ${PROJECT_ROOT}/ext/image_loader/webp)

set(PXWEBP_SOURCES
    ${PXWEBP_DIR}/../psx_image_io.h
    ${PXWEBP_DIR}/../psx_image_io.c
    ${PXWEBP_DIR}/webp_module.c
)

add_definitions(-DEXPORT)
add_library(psxm_image_webp ${PXWEBP_SOURCES})
target_link_libraries(psxm_image_webp PRIVATE webp)

set_target_properties(psxm_image_webp
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules"
)
