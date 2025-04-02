# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(PNG_DIR ${PROJECT_ROOT}/third_party/libpng-1.6.17)

set(PNG_SOURCES
    ${PNG_DIR}/png.c
    ${PNG_DIR}/pngerror.c
    ${PNG_DIR}/pngget.c
    ${PNG_DIR}/pngmem.c
    ${PNG_DIR}/pngpread.c
    ${PNG_DIR}/pngread.c
    ${PNG_DIR}/pngrio.c
    ${PNG_DIR}/pngrtran.c
    ${PNG_DIR}/pngrutil.c
    ${PNG_DIR}/pngset.c
    ${PNG_DIR}/pngtrans.c
    ${PNG_DIR}/pngwio.c
    ${PNG_DIR}/pngwrite.c
    ${PNG_DIR}/pngwtran.c
    ${PNG_DIR}/pngwutil.c
)

if (ANDROID)
set(PNG_SOURCES
    ${PNG_SOURCES}
    ${PNG_DIR}/arm/arm_init.c
    ${PNG_DIR}/arm/filter_neon_intrinsics.c
    ${PNG_DIR}/arm/filter_neon.S
)
endif()

add_definitions(-DZLIB_DLL)
configure_file(${PNG_DIR}/scripts/pnglibconf.h.prebuilt ${CMAKE_CURRENT_BINARY_DIR}/include/pnglibconf.h)
configure_file(${PNG_DIR}/pngconf.h ${CMAKE_CURRENT_BINARY_DIR}/include/pngconf.h)
configure_file(${PNG_DIR}/png.h ${CMAKE_CURRENT_BINARY_DIR}/include/png.h)

add_library(png ${PNG_SOURCES})
install(TARGETS png LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

include_directories(${PNG_DIR} ${CMAKE_CURRENT_BINARY_DIR}/include)
if(ZLIB AND OPT_SYSTEM_ZLIB)
if(NOT OPT_SYSTEM_ZLIB)
message("System zlib not found, building internal zlib")
endif()
target_link_libraries(png PRIVATE ${ZLIB})
else()
target_link_libraries(png PRIVATE zlib)
endif()

if (ANDROID)
    target_compile_options(png PRIVATE -Wno-unused-but-set-variable)
endif()
