# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(JPEG_DIR ${PROJECT_ROOT}/third_party/libjpeg-turbo-1.4.1)

set(JPEG_SOURCES
    ${JPEG_DIR}/jcapimin.c
    ${JPEG_DIR}/jcapistd.c
    ${JPEG_DIR}/jccoefct.c
    ${JPEG_DIR}/jccolor.c
    ${JPEG_DIR}/jcdctmgr.c
    ${JPEG_DIR}/jchuff.c
    ${JPEG_DIR}/jcinit.c
    ${JPEG_DIR}/jcmainct.c
    ${JPEG_DIR}/jcmarker.c
    ${JPEG_DIR}/jcmaster.c
    ${JPEG_DIR}/jcomapi.c
    ${JPEG_DIR}/jcparam.c
    ${JPEG_DIR}/jcphuff.c
    ${JPEG_DIR}/jcprepct.c
    ${JPEG_DIR}/jcsample.c
    ${JPEG_DIR}/jctrans.c
    ${JPEG_DIR}/jdapimin.c
    ${JPEG_DIR}/jdapistd.c
    ${JPEG_DIR}/jdatadst.c
    ${JPEG_DIR}/jdatasrc.c
    ${JPEG_DIR}/jdcoefct.c
    ${JPEG_DIR}/jdcolor.c
    ${JPEG_DIR}/jddctmgr.c
    ${JPEG_DIR}/jdhuff.c
    ${JPEG_DIR}/jdinput.c
    ${JPEG_DIR}/jdmainct.c
    ${JPEG_DIR}/jdmarker.c
    ${JPEG_DIR}/jdmaster.c
    ${JPEG_DIR}/jdmerge.c
    ${JPEG_DIR}/jdphuff.c
    ${JPEG_DIR}/jdpostct.c
    ${JPEG_DIR}/jdsample.c
    ${JPEG_DIR}/jdtrans.c
    ${JPEG_DIR}/jerror.c
    ${JPEG_DIR}/jfdctflt.c
    ${JPEG_DIR}/jfdctfst.c
    ${JPEG_DIR}/jfdctint.c
    ${JPEG_DIR}/jidctflt.c
    ${JPEG_DIR}/jidctfst.c
    ${JPEG_DIR}/jidctint.c
    ${JPEG_DIR}/jidctred.c
    ${JPEG_DIR}/jquant1.c
    ${JPEG_DIR}/jquant2.c
    ${JPEG_DIR}/jutils.c
    ${JPEG_DIR}/jmemmgr.c
    ${JPEG_DIR}/jmemnobs.c
    ${JPEG_DIR}/jaricom.c
    ${JPEG_DIR}/jcarith.c
    ${JPEG_DIR}/jdarith.c
    ${JPEG_DIR}/turbojpeg.c
    ${JPEG_DIR}/transupp.c
    ${JPEG_DIR}/jdatadst-tj.c
    ${JPEG_DIR}/jdatasrc-tj.c
    ${JPEG_DIR}/jsimd_none.c
)

if (WIN32)
    add_definitions(-DHAVE_BOOLEAN)
    add_definitions(-DXMD_H)
    set(JPEG_SOURCES
        ${JPEG_SOURCES}
        ${JPEG_DIR}/win/jpeg8.def
    )
endif()

configure_file(${JPEG_DIR}/build/jconfig.h ${CMAKE_CURRENT_BINARY_DIR}/include/jconfig.h)
configure_file(${JPEG_DIR}/jmorecfg.h ${CMAKE_CURRENT_BINARY_DIR}/include/jmorecfg.h)
configure_file(${JPEG_DIR}/jpeglib.h ${CMAKE_CURRENT_BINARY_DIR}/include/jpeglib.h)

add_library(jpeg ${JPEG_SOURCES})
install(TARGETS jpeg LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

include_directories(${JPEG_DIR} ${JPEG_DIR}/build ${JPEG_DIR}/simd ${CMAKE_CURRENT_BINARY_DIR}/include)

