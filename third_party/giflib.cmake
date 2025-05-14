# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(GIF_DIR ${PROJECT_ROOT}/third_party/giflib-5.1.3)

set(GIF_SOURCES
    ${GIF_DIR}/lib/gif_hash.h
    ${GIF_DIR}/lib/gif_lib.h
    ${GIF_DIR}/lib/gif_lib_private.h
    ${GIF_DIR}/lib/dgif_lib.c
    ${GIF_DIR}/lib/egif_lib.c
    ${GIF_DIR}/lib/gif_err.c
    ${GIF_DIR}/lib/gif_font.c
    ${GIF_DIR}/lib/gif_hash.c
    ${GIF_DIR}/lib/gifalloc.c
    ${GIF_DIR}/lib/openbsd-reallocarray.c
    ${GIF_DIR}/lib/quantize.c
)

if (WIN32)
set(GIF_SOURCES
    ${GIF_SOURCES}
    ${GIF_DIR}/gif_lib.def
)
endif()

configure_file(${GIF_DIR}/lib/gif_lib.h ${CMAKE_CURRENT_BINARY_DIR}/include/gif_lib.h)

add_library(gif ${GIF_SOURCES})
install(TARGETS gif LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

include_directories(${GIF_DIR} ${CMAKE_CURRENT_BINARY_DIR}/include)

if (ANDROID)
    target_compile_options(gif PRIVATE -DS_IREAD=S_IRUSR -DS_IWRITE=S_IWUSR -DS_IWRITE=S_IWUSR)
endif()
