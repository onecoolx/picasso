# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com


file(GLOB_RECURSE TEST_SOURCES ${PROJECT_ROOT}/test/*_func.c)
file(GLOB_RECURSE IMAGE_SOURCES ${PROJECT_ROOT}/test/image_*.c)

if (WIN32)
elseif (UNIX AND NOT APPLE)
    find_package(GTK2 REQUIRED)
    set(thread_file ${PROJECT_ROOT}/test/thr_posix.c)
    set(main_file ${PROJECT_ROOT}/test/testGtk2.c)
    set(host_gui_inc ${GTK2_INCLUDE_DIRS})
    set(host_gui_lib ${GTK2_LIBRARIES} pthread)
    configure_file(${PROJECT_ROOT}/test/pat.png ${CMAKE_CURRENT_BINARY_DIR}/pat.png COPYONLY)
    configure_file(${PROJECT_ROOT}/test/selt2.png ${CMAKE_CURRENT_BINARY_DIR}/selt2.png COPYONLY)
elseif (APPLE)
endif()

foreach(test_file ${TEST_SOURCES})
    get_filename_component(test_name ${test_file} NAME_WLE)
    string(REPLACE "_" ";" test_app ${test_name})
    list(GET test_app 0 test)

    if (${test} STREQUAL "thread")
        add_executable(${test} ${test_file} ${main_file} ${thread_file})
    else()
        add_executable(${test} ${test_file} ${main_file})
    endif()

    include_directories(${test} ${host_gui_inc})
    target_link_libraries(${test} PRIVATE picasso PUBLIC ${host_gui_lib})

endforeach(test_file ${TEST_SOURCES})

foreach(image_file ${IMAGE_SOURCES})
    get_filename_component(image_name ${image_file} NAME_WLE)

    if (${image_name} STREQUAL "image_info")
        add_executable(${image_name} ${image_file})
    else()
        add_executable(${image_name} ${image_file} ${main_file})
    endif()

    include_directories(${image_name} ${host_gui_inc})
    target_link_libraries(${image_name} PRIVATE picasso psx_image PUBLIC ${host_gui_lib})

endforeach(image_file ${IMAGE_SOURCES})
