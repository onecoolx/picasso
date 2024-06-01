# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

if (WIN32)
elseif (UNIX AND NOT APPLE)
    find_package(GTK2 REQUIRED)
    set(main_file ${PROJECT_ROOT}/demos/platform_gtk2.c)
    set(main_gui_inc ${GTK2_INCLUDE_DIRS})
    set(main_gui_lib ${GTK2_LIBRARIES} pthread)
elseif (APPLE)
endif()

set(DEMOS_SOURCES ${PROJECT_ROOT}/demos/clock.c
                  ${PROJECT_ROOT}/demos/flowers.c
                  ${PROJECT_ROOT}/demos/tiger.c
                  ${PROJECT_ROOT}/demos/subwaymap.c)

foreach(demo_file ${DEMOS_SOURCES})
    get_filename_component(demo ${demo_file} NAME_WLE)
    add_executable(${demo} ${demo_file} ${main_file})
     
    include_directories(${demo} ${main_gui_inc})
    target_link_libraries(${demo} PRIVATE picasso PUBLIC ${main_gui_lib})

endforeach(demo_file ${DEMOS_SOURCES})
