# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(FREETYPE_DIR ${PROJECT_ROOT}/third_party/freetype-2.3.6)

set(FREETYPE_SOURCES
	${FREETYPE_DIR}/src/base/ftbbox.c
	${FREETYPE_DIR}/src/base/ftbitmap.c
	${FREETYPE_DIR}/src/base/ftglyph.c
	${FREETYPE_DIR}/src/base/ftstroke.c
	${FREETYPE_DIR}/src/base/ftxf86.c
	${FREETYPE_DIR}/src/base/ftbase.c
	${FREETYPE_DIR}/src/base/ftsystem.c
	${FREETYPE_DIR}/src/base/ftinit.c
	${FREETYPE_DIR}/src/base/ftgasp.c
	${FREETYPE_DIR}/src/base/ftadvanc.c
	${FREETYPE_DIR}/src/raster/raster.c
	${FREETYPE_DIR}/src/sfnt/sfnt.c
	${FREETYPE_DIR}/src/smooth/smooth.c
	${FREETYPE_DIR}/src/autofit/autofit.c
	${FREETYPE_DIR}/src/truetype/truetype.c
	${FREETYPE_DIR}/src/cff/cff.c
	${FREETYPE_DIR}/src/psnames/psnames.c
	${FREETYPE_DIR}/src/pshinter/pshinter.c
)

include_directories(${FREETYPE_DIR}/include)

add_library(ft2 STATIC ${FREETYPE_SOURCES})
if (UNIX AND NOT APPLE)
    include(GNUInstallDirs)
    install(TARGETS ft2 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/picasso ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/picasso)
else
    install(TARGETS ft2 LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
endif

target_compile_definitions(ft2 PRIVATE -DDARWIN_NO_CARBON=1 -DFT2_BUILD_LIBRARY=1 -DANDROID_FONT_HACK=1)

