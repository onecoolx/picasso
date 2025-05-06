# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXIMG_DIR ${PROJECT_ROOT}/ext/image_coders)

set(PXIMG_SOURCES
    ${PXIMG_DIR}/psx_image_io.h
    ${PXIMG_DIR}/psx_image_io.c
    ${PXIMG_DIR}/psx_image_loader.h
    ${PXIMG_DIR}/psx_image_loader.c
    ${PXIMG_DIR}/psx_image_modules.h
    ${PXIMG_DIR}/psx_image_modules.c
)

set(LIBX_IMAGE psx_image)

if (WIN32)
    set(PXIMG_SOURCES
        ${PXIMG_SOURCES}
        ${PXIMG_DIR}/psx_image.rc
        ${PXIMG_DIR}/psx_image.def
        ${PXIMG_DIR}/resource.h
    )
endif()

add_definitions(-DEXPORT)
add_library(${LIBX_IMAGE} ${PXIMG_SOURCES})
install(TARGETS ${LIBX_IMAGE} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
set_target_properties(${LIBX_IMAGE} PROPERTIES VERSION ${VERSION_INFO} SOVERSION ${VERSION_MAJOR})

include_directories(${PXIMG_DIR} ${PROJECT_ROOT}/ext/common ${PROJECT_ROOT}/include ${PROJECT_ROOT}/include/images)
target_link_libraries(${LIBX_IMAGE} PRIVATE ${LIB_NAME})

if (UNIX AND NOT APPLE)
target_link_libraries(${LIBX_IMAGE} PUBLIC dl)
endif()

if (NOT APPLE)
include (${PXIMG_DIR}/png/png.cmake)
include (${PXIMG_DIR}/jpeg/jpeg.cmake)
include (${PXIMG_DIR}/gif/gif.cmake)
include (${PXIMG_DIR}/webp/webp.cmake)
else()
include (${PXIMG_DIR}/apple/apple.cmake)
endif()
