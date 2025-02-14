# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

if (WIN32)
    add_compile_definitions(WIN32)
    if (BUILD_SHARED_LIBS)
        add_compile_definitions(DLL_EXPORT)
    endif()
    add_compile_definitions(ENABLE_FAST_COPY=1)
    add_compile_definitions(ENABLE_SYSTEM_MALLOC=1)
    add_compile_definitions(__SSE2__=1)
    add_compile_definitions(_UNICODE=1)
    add_compile_definitions(UNICODE=1)
    add_compile_definitions(_HAS_EXCEPTIONS=0)
    add_compile_definitions(_USE_MATH_DEFINES)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
elseif (ANDROID)
    add_compile_definitions(ENABLE_FREE_TYPE2=1)
    add_compile_definitions(ENABLE_SYSTEM_MALLOC=1)
    add_compile_definitions(__ANDROID__=1)
elseif (UNIX AND NOT APPLE)
    add_compile_definitions(ENABLE_FREE_TYPE2=1)
    add_compile_definitions(ENABLE_FONT_CONFIG=1)
elseif (APPLE)
    add_compile_definitions(ENABLE_SYSTEM_MALLOC=1)
endif()
