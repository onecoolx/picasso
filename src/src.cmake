# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

file(GLOB_RECURSE SOURCES ${PROJECT_ROOT}/src/*.cpp)

include_directories(${PROJECT_ROOT}/include
                    ${PROJECT_ROOT}/src/include
                    ${PROJECT_ROOT}/src
                    ${PROJECT_ROOT}/src/font
                    ${PROJECT_ROOT}/src/gfx
                    ${PROJECT_ROOT}/src/simd
                    ${PROJECT_OUT}/include)

set(LIB_NAME picasso2_sw)

if (WIN32)
    set(SOURCES
        ${SOURCES}
        ${PROJECT_ROOT}/src/picasso.rc
        ${PROJECT_ROOT}/src/picasso.def
        ${PROJECT_ROOT}/src/resource.h
    )
elseif (APPLE)
    set(SOURCES
        ${SOURCES}
        ${PROJECT_ROOT}/src/font/font_adapter_apple.mm
    )
endif()

add_definitions(-DEXPORT)
if (BUILD_SHARED_LIBS)
  set(BUILD_LIBS_TYPE SHARED)
else()
  set(BUILD_LIBS_TYPE STATIC)
endif()

add_library(${LIB_NAME} ${BUILD_LIBS_TYPE} ${SOURCES})
install(TARGETS ${LIB_NAME} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
set_target_properties(${LIB_NAME} PROPERTIES VERSION ${VERSION_INFO} SOVERSION 1)

if (ANDROID)
    target_include_directories(${LIB_NAME} PRIVATE ${PROJECT_ROOT}/android/freetype/include ${PROJECT_ROOT}/android/expat/lib)
    target_link_libraries(${LIB_NAME} PUBLIC ft2 expat)
elseif (UNIX AND NOT APPLE)
    find_package(Freetype REQUIRED)
    find_package(Fontconfig REQUIRED)
    target_include_directories(${LIB_NAME} PRIVATE ${FREETYPE_INCLUDE_DIRS} ${FONTCONFIG_INCLUDE_DIRS})
    target_link_libraries(${LIB_NAME} PUBLIC Freetype::Freetype Fontconfig::Fontconfig)
elseif (APPLE)
    find_library(CORETEXT_LIBRARY CoreText)
    find_library(COREGRAPHICS_LIBRARY CoreGraphics)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(${LIB_NAME} PUBLIC ${CORETEXT_LIBRARY} ${COREGRAPHICS_LIBRARY} ${COREFOUNDATION_LIBRARY})
endif()


