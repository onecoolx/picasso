# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXJPEG_DIR ${PROJECT_ROOT}/ext/image_coders/jpeg)

set(PXJPEG_SOURCES
    ${PXJPEG_DIR}/../psx_image_io.h
    ${PXJPEG_DIR}/../psx_image_io.c
    ${PXJPEG_DIR}/jpeg_module.c
)

set(LIBX_JPEG psxm_image_jpeg)

add_definitions(-DEXPORT)
add_library(${LIBX_JPEG} ${PXJPEG_SOURCES})
if(OPT_SYSTEM_JPEG AND JPEG)
target_link_libraries(${LIBX_JPEG} PRIVATE ${JPEG})
else()
target_link_libraries(${LIBX_JPEG} PRIVATE jpeg)
endif()
install(TARGETS ${LIBX_JPEG} LIBRARY DESTINATION lib/modules ARCHIVE DESTINATION lib/modules)

set_target_properties(${LIBX_JPEG}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules"
)
