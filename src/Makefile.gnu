#DLL=-DDLL_EXPORT
FREETYPE_INC=-I/usr/include/freetype2
#FREETYPE_INC=-I/extra/freetype2/include -I/extra/freetype2/include/freetype2
#CXX=g++ -Wall -O3 -fno-rtti -fno-exceptions -fno-strict-aliasing
CXX=g++ -Wall -O3 -g -pg -fno-rtti -fno-exceptions -fno-strict-aliasing
#CXX=g++ -Wall -O3 -g -fprofile-arcs -ftest-coverage -fno-rtti -fno-exceptions -fno-strict-aliasing
INC=-I. -I./gfx -I./gfx/include -I./include -I./../ -I./../include -I./../build ${FREETYPE_INC}
CXXFLAGS=-DEXPORT ${DLL} 

VPATH=./ ./include ./core ./gfx ./gfx/include ./gfx/lib

objects= \
		agg_arc.o \
		agg_bezier_arc.o \
		agg_bspline.o \
		agg_curves.o \
		agg_image_filters.o \
		agg_line_aa_basics.o \
		agg_line_profile_aa.o \
		agg_rounded_rect.o \
		agg_sqrt_tables.o \
		agg_trans_affine.o \
		agg_vcgen_bspline.o \
		agg_vcgen_dash.o \
		agg_vcgen_smooth_poly1.o \
		agg_vcgen_stroke.o \
		\
		fixedopt.o \
		device.o \
		curve.o \
		graphic_path.o \
		\
		gfx_device.o \
		gfx_trans_affine.o \
		gfx_painter.o \
		gfx_raster_adapter.o \
		gfx_gradient_adapter.o \
		gfx_rendering_buffer.o \
		gfx_mask_layer.o \
		gfx_font_adapter_win32.o \
		gfx_font_adapter_freetype2.o \
		gfx_font_load_freetype2.o \
		\
		picasso_matrix.o \
		picasso_painter.o \
		picasso_rendering_buffer.o \
		picasso_raster_adapter.o \
		picasso.o \
		picasso_canvas.o \
		picasso_image.o \
		picasso_pattern.o \
		picasso_path.o \
		picasso_gradient.o \
		picasso_font.o \
		picasso_gpc.o \
		picasso_mask.o 

all: libpicasso.a

libpicasso.a : ${objects}
	ar rcu libpicasso.a ${objects}

%.o : %.cpp 
	${CXX} ${CXXFLAGS} -c $<  -o $@ ${INC}

clean:
	rm *.o *.a
