# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

IF (WIN32)
    add_compile_definitions(WIN32)
    add_compile_definitions(DLL_EXPORT)
    add_compile_definitions(ENABLE_FAST_COPY=1)
    add_compile_definitions(ENABLE_SYSTEM_MALLOC=1)
    add_compile_definitions(__SSE2__=1)
ELSEIF (UNIX AND NOT APPLE)
    add_compile_definitions(ENABLE_FREE_TYPE2=1)
    add_compile_definitions(ENABLE_FONT_CONFIG=1)
ELSEIF (APPLE)
    add_compile_definitions(ENABLE_SYSTEM_MALLOC=1)
ENDIF()
