# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

file(GLOB_RECURSE SOURCES ${PROJECT_ROOT}/src/*.cpp)

add_library(picasso ${SOURCES})

include_directories(${PROJECT_ROOT}/build
                    ${PROJECT_ROOT}/include
                    ${PROJECT_ROOT}/src/include
                    ${PROJECT_ROOT}/src
                    ${PROJECT_ROOT}/src/gfx
                    ${PROJECT_ROOT}/src/simd)

IF (WIN32)
ELSEIF (UNIX AND NOT APPLE)
    find_package(Freetype REQUIRED)
    find_package(Fontconfig REQUIRED)
    target_include_directories(picasso PRIVATE ${FREETYPE_INCLUDE_DIRS} ${FONTCONFIG_INCLUDE_DIRS})
    target_link_libraries(picasso PUBLIC Freetype::Freetype Fontconfig::Fontconfig)
ELSEIF (APPLE)
ENDIF()
