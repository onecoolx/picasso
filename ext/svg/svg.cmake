# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXSVG_DIR ${PROJECT_ROOT}/ext/svg)

set(PXSVG_SOURCES
    ${PXSVG_DIR}/psx_xml_token.h
    ${PXSVG_DIR}/psx_xml_token.cpp
    ${PXSVG_DIR}/psx_svg_node.h
    ${PXSVG_DIR}/psx_svg_node.cpp
    ${PXSVG_DIR}/psx_svg_parser.h
    ${PXSVG_DIR}/psx_svg_parser.cpp
    ${PXSVG_DIR}/psx_svg_render.h
    ${PXSVG_DIR}/psx_svg_render.cpp
    ${PXSVG_DIR}/psx_svg.cpp
)

set(LIBX_SVG psx_svg)

if (WIN32)
    set(PXSVG_SOURCES
        ${PXSVG_SOURCES}
        ${PXSVG_DIR}/psx_svg.rc
        ${PXSVG_DIR}/psx_svg.def
        ${PXSVG_DIR}/resource.h
    )
endif()

add_definitions(-DEXPORT)
add_library(${LIBX_SVG} ${PXSVG_SOURCES})
install(TARGETS ${LIBX_SVG} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
set_target_properties(${LIBX_SVG} PROPERTIES VERSION ${VERSION_INFO} SOVERSION ${VERSION_MAJOR})

if (OPT_UNITTEST OR OPT_PERFTEST)
set(LIBX_SVG_STATIC psx_svg_static)
add_library(${LIBX_SVG_STATIC} STATIC ${PXSVG_SOURCES})
install(TARGETS ${LIBX_SVG_STATIC} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
endif()

include_directories(${PXSVG_DIR} ${PROJECT_ROOT}/ext/common ${PROJECT_ROOT}/include ${PROJECT_ROOT}/include/images ${PROJECT_ROOT}/include/svg)
target_link_libraries(${LIBX_SVG} PRIVATE ${LIB_NAME} ${LIBX_IMAGE} ${LIBX_COMMON})



set(LIBX_SVG_IMAGE psxm_image_svg)

set(PXSVG_IMG_SOURCES 
    ${PXSVG_DIR}/../image_coders/psx_image_io.h
    ${PXSVG_DIR}/../image_coders/psx_image_io.c
    ${PXSVG_DIR}/psx_svg_module.c
)
add_library(${LIBX_SVG_IMAGE} ${PXSVG_IMG_SOURCES})
install(TARGETS ${LIBX_SVG_IMAGE} LIBRARY DESTINATION lib/modules ARCHIVE DESTINATION lib/modules)

target_link_libraries(${LIBX_SVG_IMAGE} PUBLIC ${LIBX_SVG} ${IMAGEIO_LIBRARY} ${LIBX_COMMON} ${LIB_NAME})

set_target_properties(${LIBX_SVG_IMAGE}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules"
)
