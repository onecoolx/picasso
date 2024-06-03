# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

file(GLOB_RECURSE SOURCES ${PROJECT_ROOT}/src/*.cpp)

include_directories(${PROJECT_ROOT}/build
                    ${PROJECT_ROOT}/include
                    ${PROJECT_ROOT}/src/include
                    ${PROJECT_ROOT}/src
                    ${PROJECT_ROOT}/src/gfx
                    ${PROJECT_ROOT}/src/simd)

if (WIN32)
    set(SOURCES
        ${SOURCES}
        ${PROJECT_ROOT}/src/picasso.rc
        ${PROJECT_ROOT}/src/picasso.def
        ${PROJECT_ROOT}/src/resource.h
    )
endif()

add_definitions(-DEXPORT)
add_library(picasso2_sw ${SOURCES})

if (UNIX AND NOT APPLE)
    find_package(Freetype REQUIRED)
    find_package(Fontconfig REQUIRED)
    target_include_directories(picasso2_sw PRIVATE ${FREETYPE_INCLUDE_DIRS} ${FONTCONFIG_INCLUDE_DIRS})
    target_link_libraries(picasso2_sw PUBLIC Freetype::Freetype Fontconfig::Fontconfig)
elseif (APPLE)
endif()


