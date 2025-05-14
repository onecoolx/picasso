# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)

FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)
add_library(GMock::GMock INTERFACE IMPORTED)
target_link_libraries(GMock::GMock INTERFACE gmock_main)

FetchContent_Declare(
  lodepng
  GIT_REPOSITORY https://github.com/onecoolx/lodepng.git
  GIT_TAG        939fbb9 # current used version
)
FetchContent_MakeAvailable(lodepng)
add_library(lodepng INTERFACE IMPORTED)

file(GLOB_RECURSE UNIT_SOURCES ${PROJECT_ROOT}/unit_tests/*.cpp)
set(UNIT_SOURCES ${UNIT_SOURCES} ${lodepng_SOURCE_DIR}/lodepng.cpp)

set(UNIT_TESTS unit_tests)

include_directories(${PROJECT_ROOT})
add_executable(${UNIT_TESTS} ${UNIT_SOURCES})
target_link_libraries(${UNIT_TESTS} PRIVATE GTest::GTest GMock::GMock lodepng ${LIB_NAME} ${LIBX_COMMON} ${LIBX_IMAGE} ${LIBX_SVG_STATIC})
target_include_directories(${UNIT_TESTS} PRIVATE ${lodepng_SOURCE_DIR})

file(COPY ${PROJECT_ROOT}/unit_tests/snapshots DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${PROJECT_ROOT}/unit_tests/test.png ${CMAKE_CURRENT_BINARY_DIR}/test.png COPYONLY)
configure_file(${PROJECT_ROOT}/unit_tests/tiger.svg ${CMAKE_CURRENT_BINARY_DIR}/tiger.svg COPYONLY)

if (WIN32)
    add_custom_command(
        TARGET ${UNIT_TESTS} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:unit_tests> $<TARGET_FILE_DIR:unit_tests>
        COMMAND_EXPAND_LISTS
    )
else()
    target_compile_options(${UNIT_TESTS} PRIVATE -std=c++17)
    configure_file(${PROJECT_ROOT}/cfg/ZCOOLXiaoWei-Regular.ttf ${CMAKE_CURRENT_BINARY_DIR}/ZCOOLXiaoWei-Regular.ttf COPYONLY)
endif()

add_test(NAME unittest COMMAND ${UNIT_TESTS})

