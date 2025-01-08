# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXGIF_DIR ${PROJECT_ROOT}/ext/image_coders/gif)

set(PXGIF_SOURCES
    ${PXGIF_DIR}/../psx_image_io.h
    ${PXGIF_DIR}/../psx_image_io.c
    ${PXGIF_DIR}/gif_module.c
)

set(LIBX_GIF psxm_image_gif)

add_definitions(-DEXPORT)
add_library(${LIBX_GIF} ${PXGIF_SOURCES})
target_link_libraries(${LIBX_GIF} PRIVATE gif)
install(TARGETS ${LIBX_GIF} LIBRARY DESTINATION lib/modules ARCHIVE DESTINATION lib/modules)

set_target_properties(${LIBX_GIF}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules"
)
