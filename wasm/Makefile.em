FREETYPE_INC=-I/usr/include/freetype2
INC=-I./ -I../src -I../src/gfx -I../src/include -I../include ${FREETYPE_INC}

CXXFLAGS=-Wall -O3 -fno-rtti -fno-exceptions
CXXFLAGS +=-DEXPORT 

VPATH=../src ../src/include ../src/core ../src/gfx

objects= \
        device.o \
        curve.o \
        graphic_path.o \
        clipper.o \
        matrix.o \
        \
        gfx_device.o \
        gfx_raster_adapter.o \
        gfx_gradient_adapter.o \
        gfx_image_filters.o \
        gfx_rendering_buffer.o \
        gfx_sqrt_tables.o \
        gfx_blur.o \
        gfx_font_adapter_dummy.o \
        gfx_font_adapter_win32.o \
        gfx_font_adapter_freetype2.o \
        gfx_font_load_freetype2.o \
        \
        picasso_matrix_api.o \
        picasso_painter.o \
        picasso_rendering_buffer.o \
        picasso_raster_adapter.o \
        picasso_canvas.o \
        picasso_image.o \
        picasso_pattern.o \
        picasso_path.o \
        picasso_gradient.o \
        picasso_gradient_api.o \
        picasso_font.o \
        picasso_font_api.o \
        picasso_mask.o \
        picasso_mask_api.o \
        picasso_api.o

all: libpicasso.a

libpicasso.a : ${objects}
	${AR} rc libpicasso.a ${objects}

%.o : %.cpp 
	${CXX} ${CXXFLAGS} -c $<  -o $@ ${INC}

clean:
	rm *.o *.a
