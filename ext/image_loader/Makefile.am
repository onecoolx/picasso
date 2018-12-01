SUBDIRS =

if PSX_IMAGE_GIF_SUPPORT
SUBDIRS += gif
endif

if PSX_IMAGE_PNG_SUPPORT
SUBDIRS += png
endif

if PSX_IMAGE_JPG_SUPPORT
SUBDIRS += jpeg
endif

if PSX_IMAGE_WEBP_SUPPORT
SUBDIRS += webp
endif

INCLUDES = -I./ \
           -I../../ \
           -I../../build \
           -I../../include

lib_LTLIBRARIES = libpsx_image.la

libpsx_image_la_SOURCES = \
            psx_list.h \
            psx_image_io.h \
            psx_image_io.c \
            psx_image_loader.h \
            psx_image_loader.c \
            psx_image_modules.h \
            psx_image_modules.c

libpsx_image_la_LDFLAGS = -no-undefined -release $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MICRO)

libpsx_image_la_LIBADD = ../../src/libpicasso.la -ldl

