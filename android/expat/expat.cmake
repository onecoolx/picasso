# Picasso - a vector graphics library
# 
# Copyright (C) 2024 Zhang Ji Peng
# Contact: onecoolx@gmail.com

set(EXPAT_DIR ${PROJECT_ROOT}/android/expat)

set(EXPAT_SOURCES
	${EXPAT_DIR}/lib/xmltok.c
	${EXPAT_DIR}/lib/xmlrole.c
	${EXPAT_DIR}/lib/xmlparse.c
)

include_directories(${EXPAT_DIR}/ ${EXPAT_DIR}/lib)

add_library(expat STATIC ${EXPAT_SOURCES})
install(TARGETS expat LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
target_compile_definitions(expat PRIVATE -DHAVE_EXPAT_CONFIG_H=1)
target_compile_options(expat PRIVATE  -fexceptions)

