# Picasso - a vector graphics library
# 
# Copyright (C) 2025 Zhang Ji Peng
# Contact: onecoolx@gmail.com

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)

FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)
add_library(GMock::GMock INTERFACE IMPORTED)
target_link_libraries(GMock::GMock INTERFACE gmock_main)


FetchContent_Declare(
  cJSON
  GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
  GIT_TAG        c859b25 # release 1.7.19 version
)
FetchContent_MakeAvailable(cJSON)
add_library(CJson::CJson INTERFACE IMPORTED)
target_compile_definitions(CJson::CJson INTERFACE -DENABLE_CJSON_TEST=OFF)
target_link_libraries(CJson::CJson INTERFACE cjson)


file(GLOB_RECURSE PS_PERF_SOURCES ${PROJECT_ROOT}/perf_tests/ps_*.cpp)
if (OPT_EXTENSIONS)
    file(GLOB_RECURSE EXT_PERF_SOURCES ${PROJECT_ROOT}/perf_tests/ext_*.cpp)
endif()

set(PERF_SOURCES 
    ${PROJECT_ROOT}/perf_tests/test.cpp
    ${PLATFORM_SPECIAL}
    ${PS_PERF_SOURCES}
    ${EXT_PERF_SOURCES}
)

set(PERF_SOURCES ${PERF_SOURCES} ${PLATFORM_SPECIAL})

set(PERF_TESTS perf_tests)

include_directories(${PROJECT_ROOT})
add_executable(${PERF_TESTS} ${PERF_SOURCES})
target_link_libraries(${PERF_TESTS} PRIVATE GTest::GTest GMock::GMock CJson::CJson ${LIB_NAME} ${LIBX_IMAGE} ${LIBX_SVG_STATIC} ${LIBX_COMMON})
target_include_directories(${PERF_TESTS} PRIVATE ${cJSON_SOURCE_DIR})
if (OPT_EXTENSIONS)
    target_compile_definitions(${PERF_TESTS} PRIVATE -DENABLE_EXTENSIONS=1)
endif()

file(COPY ${PROJECT_ROOT}/perf_tests/benchmark DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


if (WIN32)
    add_custom_command(
        TARGET ${PERF_TESTS} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:perf_tests> $<TARGET_FILE_DIR:perf_tests>
        COMMAND_EXPAND_LISTS
    )
else()
    target_compile_definitions(${PERF_TESTS} PRIVATE -DLINUX=1)
    target_compile_options(${PERF_TESTS} PRIVATE -std=gnu++17)
    configure_file(${PROJECT_ROOT}/cfg/ZCOOLXiaoWei-Regular.ttf ${CMAKE_CURRENT_BINARY_DIR}/ZCOOLXiaoWei-Regular.ttf COPYONLY)
endif()

add_test(NAME perftest COMMAND ${PERF_TESTS})

