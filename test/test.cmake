# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com


file(GLOB_RECURSE TEST_SOURCES ${PROJECT_ROOT}/test/*_func.c)
file(GLOB_RECURSE IMAGE_SOURCES ${PROJECT_ROOT}/test/image_*.c)

if (WIN32)
    set(thread_file ${PROJECT_ROOT}/test/win32/thr_win32.c)
    set(main_file ${PROJECT_ROOT}/test/win32/testWin.c)
    set(app_type WIN32)
    add_custom_command(TARGET picasso2_sw POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_ROOT}/test/pat.bmp" "$(ProjectDir)/$(Configuration)" 
    ) 
    add_custom_command(TARGET picasso2_sw POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_ROOT}/test/selt2.bmp" "$(ProjectDir)/$(Configuration)" 
    ) 
    set(host_gui_inc ${PROJECT_ROOT}/test/)
elseif (ANDROID)
    set(thread_file ${PROJECT_ROOT}/test/posix/thr_posix.c)
    set(main_file ${PROJECT_ROOT}/test/android/testAndroid.cpp 
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )
    set(host_gui_inc ${PROJECT_ROOT}/test/ ${ANDROID_NDK}/sources/android/native_app_glue)
    set(host_gui_lib log android EGL GLESv1_CM)
    set(main_file ${main_file} ${resources})
elseif (UNIX AND NOT APPLE)
    find_package(GTK2 REQUIRED)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
    set(thread_file ${PROJECT_ROOT}/test/posix/thr_posix.c)
    set(main_file ${PROJECT_ROOT}/test/gtk2/testGtk2.c)
    set(host_gui_inc ${GTK2_INCLUDE_DIRS} ${PROJECT_ROOT}/test/)
    set(host_gui_lib ${GTK2_LIBRARIES} pthread)
    configure_file(${PROJECT_ROOT}/test/pat.png ${CMAKE_CURRENT_BINARY_DIR}/pat.png COPYONLY)
    configure_file(${PROJECT_ROOT}/test/selt2.png ${CMAKE_CURRENT_BINARY_DIR}/selt2.png COPYONLY)
    add_definitions(-DLINUX)
elseif (APPLE)
    find_library(APPKIT_LIBRARY AppKit)
    find_library(CARBON_LIBRARY Carbon)
    set(thread_file ${PROJECT_ROOT}/test/posix/thr_posix.c)
    set(main_file ${PROJECT_ROOT}/test/mac/testMac.m)
    set(host_gui_inc ${PROJECT_ROOT}/test/)
    set(host_gui_lib ${APPKIT_LIBRARY} ${CARBON_LIBRARY})
    add_definitions(-DUNIX)
    set(app_type MACOSX_BUNDLE)
    set(resources ${PROJECT_ROOT}/test/pat.png ${PROJECT_ROOT}/test/selt2.png)
    set(main_file ${main_file} ${resources})
endif()

foreach(test_file ${TEST_SOURCES})
    get_filename_component(test_name ${test_file} NAME_WLE)
    string(REPLACE "_" ";" test_app ${test_name})
    list(GET test_app 0 test)

    if (NOT WIN32)
        if (${test} STREQUAL "thread")
            add_library(${test} SHARED ${app_type} ${test_file} ${main_file} ${thread_file})
        else()
            add_library(${test} SHARED ${app_type} ${test_file} ${main_file})
        endif()
        target_compile_options(${test} PRIVATE -Wno-deprecated-declarations -Wno-implicit-const-int-float-conversion)
    else()
        if (${test} STREQUAL "thread")
            add_executable(${test} ${app_type} ${test_file} ${main_file} ${thread_file})
        else()
            add_executable(${test} ${app_type} ${test_file} ${main_file})
        endif()
    endif()

    if (APPLE)
        set(CMAKE_XCODE_ATTRIBUTE_INFOPLIST_FILE "${PROJECT_ROOT}/test/mac/Info.plist")
        set_target_properties(${test} PROPERTIES
        MACOSX_BUNDLE TRUE
        RESOURCE "${resources}")
    endif()
    include_directories(${test} ${host_gui_inc})
    target_link_libraries(${test} PRIVATE picasso2_sw PUBLIC ${host_gui_lib})

endforeach(test_file ${TEST_SOURCES})

if (NOT ANDROID)
foreach(image_file ${IMAGE_SOURCES})
    get_filename_component(image_name ${image_file} NAME_WLE)

    if (${image_name} STREQUAL "image_info")
        add_executable(${image_name} ${image_file})
    else()
        add_executable(${image_name} ${app_type} ${image_file} ${main_file})
    endif()

    if (APPLE)
        set(CMAKE_XCODE_ATTRIBUTE_INFOPLIST_FILE "${PROJECT_ROOT}/test/mac/Info.plist")
    endif()
    include_directories(${image_name} ${host_gui_inc})
    target_link_libraries(${image_name} PRIVATE picasso2_sw psx_image PUBLIC ${host_gui_lib})

endforeach(image_file ${IMAGE_SOURCES})
endif()
