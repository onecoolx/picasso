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

file(GLOB_RECURSE UNIT_SOURCES ${PROJECT_ROOT}/unittest/*.cpp)

include_directories(${PROJECT_ROOT})
add_executable(unit_test ${UNIT_SOURCES})
target_link_libraries(unit_test PRIVATE GTest::GTest ${LIB_NAME})

if (WIN32)
    add_custom_command(
        TARGET unit_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:unit_test> $<TARGET_FILE_DIR:unit_test>
        COMMAND_EXPAND_LISTS
    )
endif ()

add_test(NAME unittest COMMAND unit_test)

