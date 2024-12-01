# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXIMAGE_DIR ${PROJECT_ROOT}/ext/image_coders/apple)

set(PXIMAGE_SOURCES
    ${PXIMAGE_DIR}/../psx_image_io.h
    ${PXIMAGE_DIR}/../psx_image_io.c
    ${PXIMAGE_DIR}/images_module.m
)

set(LIBX_CGIMAGE psxm_image_cg)

add_definitions(-DEXPORT)
add_library(${LIBX_CGIMAGE} ${PXIMAGE_SOURCES})
#target_link_libraries(${LIBX_CGIMAGE} PRIVATE jpeg)
install(TARGETS ${LIBX_CGIMAGE} LIBRARY DESTINATION lib/modules ARCHIVE DESTINATION lib/modules)

set_target_properties(${LIBX_CGIMAGE}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules"
)
