# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

if (WIN32)
    set(plat_file ${PROJECT_ROOT}/demos/platform_win32.c)
    set(app_type WIN32)
elseif (UNIX AND NOT APPLE)
    find_package(GTK2 REQUIRED)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
    set(plat_file ${PROJECT_ROOT}/demos/platform_gtk2.c)
    set(plat_gui_inc ${GTK2_INCLUDE_DIRS})
    set(plat_gui_lib ${GTK2_LIBRARIES} pthread)
    add_definitions(-DLINUX)
elseif (APPLE)
    find_library(APPKIT_LIBRARY AppKit)
    find_library(CARBON_LIBRARY Carbon)
    set(plat_file ${PROJECT_ROOT}/demos/platform_apple.m)
    set(plat_gui_lib ${APPKIT_LIBRARY} ${CARBON_LIBRARY})
    add_definitions(-DUNIX)
endif()

set(DEMOS_SOURCES ${PROJECT_ROOT}/demos/clock.c
                  ${PROJECT_ROOT}/demos/flowers.c
                  ${PROJECT_ROOT}/demos/tiger.c
                  ${PROJECT_ROOT}/demos/subwaymap.c)

foreach(demo_file ${DEMOS_SOURCES})
    get_filename_component(demo ${demo_file} NAME_WLE)
    add_executable(${demo} ${app_type} ${demo_file} ${plat_file})
     
    if (APPLE)
        set(CMAKE_XCODE_ATTRIBUTE_INFOPLIST_FILE "${PROJECT_ROOT}/demos/Info.plist")
    endif()
    include_directories(${demo} ${plat_gui_inc})
    target_link_libraries(${demo} PRIVATE picasso2_sw PUBLIC ${plat_gui_lib})

endforeach(demo_file ${DEMOS_SOURCES})
