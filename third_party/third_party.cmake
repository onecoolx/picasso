# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

if (ANDROID)
include (${CMAKE_CURRENT_LIST_DIR}/freetype.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/expat.cmake)
endif()

if (NOT APPLE)
include (${CMAKE_CURRENT_LIST_DIR}/zlib.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/libpng.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/libjpeg.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/giflib.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/libwebp.cmake)
endif()
