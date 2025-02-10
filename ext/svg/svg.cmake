# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PXSVG_DIR ${PROJECT_ROOT}/ext/svg)

set(PXSVG_SOURCES
    ${PXSVG_DIR}/psx_xml_token.h
    ${PXSVG_DIR}/psx_xml_token.cpp
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
set_target_properties(${LIBX_SVG} PROPERTIES VERSION ${VERSION_INFO} SOVERSION 1)

include_directories(${PXSVG_DIR} ${PROJECT_ROOT}/ext/common ${PROJECT_ROOT}/include ${PROJECT_ROOT}/include/images ${PROJECT_ROOT}/include/svg)
target_link_libraries(${LIBX_SVG} PRIVATE ${LIB_NAME})

