# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

if (ANDROID)
include (${CMAKE_CURRENT_LIST_DIR}/freetype.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/expat.cmake)
elseif (OPT_INTERNAL_FREETYPE)
include (${CMAKE_CURRENT_LIST_DIR}/freetype.cmake)
endif()

if (NOT APPLE)
if((NOT ZLIB) OR (NOT OPT_SYSTEM_ZLIB))
if(NOT ZLIB)
message("System zlib not found, building internal zlib")
endif()
include (${CMAKE_CURRENT_LIST_DIR}/zlib.cmake)
endif()

if((NOT PNG) OR (NOT OPT_SYSTEM_PNG))
if(NOT PNG)
message("System libpng not found, building internal libpng")
endif()
include (${CMAKE_CURRENT_LIST_DIR}/libpng.cmake)
endif()

if((NOT JPEG) OR (NOT OPT_SYSTEM_JPEG))
if(NOT JPEG)
message("System libjpeg not found, building internal libjpeg")
endif()
include (${CMAKE_CURRENT_LIST_DIR}/libjpeg.cmake)
endif()

if((NOT GIF) OR (NOT OPT_SYSTEM_GIF))
if(NOT GIF)
message("System giflib not found, building internal giflib")
endif()
include (${CMAKE_CURRENT_LIST_DIR}/giflib.cmake)
endif()

if((NOT WEBP) OR (NOT OPT_SYSTEM_WEBP))
if(NOT WEBP)
message("System libwebp not found, building internal libwebp")
endif()
include (${CMAKE_CURRENT_LIST_DIR}/libwebp.cmake)
endif()
endif()
