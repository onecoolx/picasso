# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXWEBP_DIR ${PROJECT_ROOT}/ext/image_coders/webp)

set(PXWEBP_SOURCES
    ${PXWEBP_DIR}/../psx_image_io.h
    ${PXWEBP_DIR}/../psx_image_io.c
    ${PXWEBP_DIR}/webp_module.c
)

set(LIBX_WEBP psxm_image_webp)

add_definitions(-DEXPORT)
add_library(${LIBX_WEBP} ${PXWEBP_SOURCES})
if(OPT_SYSTEM_WEBP AND WEBP})
target_link_libraries(${LIBX_WEBP} PRIVATE ${WEBP} ${LIBX_COMMON})
else()
target_link_libraries(${LIBX_WEBP} PRIVATE webp ${LIBX_COMMON})
endif()
install(TARGETS ${LIBX_WEBP} LIBRARY DESTINATION lib/modules ARCHIVE DESTINATION lib/modules)

set_target_properties(${LIBX_WEBP}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules"
)
