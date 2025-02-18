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

file(GLOB_RECURSE UNIT_SOURCES ${PROJECT_ROOT}/unit_tests/*.cpp)

set(UNIT_TESTS unit_tests)

include_directories(${PROJECT_ROOT})
add_executable(${UNIT_TESTS} ${UNIT_SOURCES})
target_link_libraries(${UNIT_TESTS} PRIVATE GTest::GTest ${LIB_NAME} ${LIBX_IMAGE} ${LIBX_SVG})

if (WIN32)
    add_custom_command(
        TARGET ${UNIT_TESTS} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:unit_tests> $<TARGET_FILE_DIR:unit_tests>
        COMMAND_EXPAND_LISTS
    )
else()
    target_compile_options(${UNIT_TESTS} PRIVATE -std=c++17)
endif()

add_test(NAME unittest COMMAND ${UNIT_TESTS})

