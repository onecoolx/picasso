modulesdir=$(prefix)/lib/modules/

INCLUDES = -I../ \
           -I../../../ \
           -I../../../build \
           -I../../../include

# picasso external system image module webp.
modules_LTLIBRARIES = libpsxm_image_webp.la

libpsxm_image_webp_la_SOURCES = \
                    webp_module.c \
                    ../psx_image_io.c

libpsxm_image_webp_la_LIBADD = -lwebp -ldl

