# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXPNG_DIR ${PROJECT_ROOT}/ext/image_coders/png)

set(PXPNG_SOURCES
    ${PXPNG_DIR}/../psx_image_io.h
    ${PXPNG_DIR}/../psx_image_io.c
    ${PXPNG_DIR}/png_module.c
)

set(LIBX_PNG psxm_image_png)

add_definitions(-DEXPORT)
add_library(${LIBX_PNG} ${PXPNG_SOURCES})
if(OPT_SYSTEM_PNG AND PNG)
target_link_libraries(${LIBX_PNG} PRIVATE ${PNG})
else()
target_link_libraries(${LIBX_PNG} PRIVATE png)
endif()
install(TARGETS ${LIBX_PNG} LIBRARY DESTINATION lib/modules ARCHIVE DESTINATION lib/modules)

set_target_properties(${LIBX_PNG}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules"
)
