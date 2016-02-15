modulesdir=$(prefix)/lib/modules/

INCLUDES = -I../ \
           -I../../../ \
	       -I../../../include

# picasso external system image module png.
modules_LTLIBRARIES = libpsxm_image_png.la

libpsxm_image_png_la_SOURCES = \
                    png_module.c \
                    ../psx_image_io.c

libpsxm_image_png_la_LIBADD = -lpng -lz -ldl

