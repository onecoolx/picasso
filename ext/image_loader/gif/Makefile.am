modulesdir=$(prefix)/lib/modules/

INCLUDES = -I../ \
           -I../../../ \
	       -I../../../include

# picasso external system image module gif.
modules_LTLIBRARIES = libpsxm_image_gif.la

libpsxm_image_gif_la_SOURCES = \
					gif_module.c \
                    ../psx_image_io.c

libpsxm_image_gif_la_LIBADD = -lgif -ldl

