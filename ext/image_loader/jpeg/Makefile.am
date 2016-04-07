modulesdir=$(prefix)/lib/modules/

INCLUDES = -I../ \
           -I../../../ \
	       -I../../../include

# picasso external system image module jpeg.
modules_LTLIBRARIES = libpsxm_image_jpeg.la

libpsxm_image_jpeg_la_SOURCES = \
					jpeg_module.c \
                    ../psx_image_io.c

libpsxm_image_jpeg_la_LIBADD = -ljpeg -ldl

