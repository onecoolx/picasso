# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

if (WIN32)
    set(plat_file ${PROJECT_ROOT}/demos/win32/platform_win32.c)
    set(app_type WIN32)
elseif (ANDROID)
    set(plat_file ${PROJECT_ROOT}/demos/android/platform_android.cpp
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )
    set(plat_gui_inc ${PROJECT_ROOT}/demos/ ${ANDROID_NDK}/sources/android/native_app_glue)
    set(plat_gui_lib log android EGL GLESv1_CM)
elseif (UNIX AND NOT APPLE)
    find_package(GTK2 REQUIRED)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
    set(plat_file ${PROJECT_ROOT}/demos/gtk2/platform_gtk2.c)
    set(plat_gui_inc ${GTK2_INCLUDE_DIRS})
    set(plat_gui_lib ${GTK2_LIBRARIES} pthread)
    add_definitions(-DLINUX)
elseif (APPLE)
    find_library(APPKIT_LIBRARY AppKit)
    find_library(CARBON_LIBRARY Carbon)
    set(plat_file ${PROJECT_ROOT}/demos/mac/platform_apple.m)
    set(plat_gui_lib ${APPKIT_LIBRARY} ${CARBON_LIBRARY})
    add_definitions(-DUNIX)
endif()

set(DEMOS_SOURCES ${PROJECT_ROOT}/demos/clock.c
                  ${PROJECT_ROOT}/demos/flowers.c
                  ${PROJECT_ROOT}/demos/tiger.c
                  ${PROJECT_ROOT}/demos/subwaymap.c)

foreach(demo_file ${DEMOS_SOURCES})
    get_filename_component(demo ${demo_file} NAME_WLE)
    if (ANDROID)
        add_library(${demo} SHARED ${app_type} ${demo_file} ${plat_file})
        target_compile_options(${demo} PRIVATE -Wno-deprecated-declarations -Wno-implicit-const-int-float-conversion)
    else()
        add_executable(${demo} ${app_type} ${demo_file} ${plat_file})
    endif()
     
    if (APPLE)
        set(CMAKE_XCODE_ATTRIBUTE_INFOPLIST_FILE "${PROJECT_ROOT}/demos/mac/Info.plist")
        set_source_files_properties(${plat_file} PROPERTIES COMPILE_FLAGS "-fobjc-arc")
    endif()
    include_directories(${demo} ${plat_gui_inc})
    target_link_libraries(${demo} PRIVATE picasso2_sw PUBLIC ${plat_gui_lib})

endforeach(demo_file ${DEMOS_SOURCES})
