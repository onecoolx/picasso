TOP_DIR=..

INCLUDES = -I./include \
		   -I./gfx \
		   -I./gfx/include \
		   -I../include \
		   -I../ 

VPATH=./ ./inclue ./core ./gfx ./gfx/include ./gfx/lib

lib_LTLIBRARIES = libpicasso.la

AM_CFLAGS = -DEXPORT=1
AM_CXXFLAGS = -DEXPORT=1

libpicasso_la_SOURCES= \
			agg_arc.cpp \
			agg_bezier_arc.cpp \
			agg_bspline.cpp \
			agg_curves.cpp \
			agg_image_filters.cpp \
			agg_line_aa_basics.cpp \
			agg_line_profile_aa.cpp \
			agg_rounded_rect.cpp \
			agg_sqrt_tables.cpp \
			agg_trans_affine.cpp \
			agg_vcgen_bspline.cpp \
			agg_vcgen_dash.cpp \
			agg_vcgen_smooth_poly1.cpp \
			agg_vcgen_stroke.cpp \
			\
			fixedopt.cpp \
			device.cpp \
			curve.cpp \
			graphic_path.cpp \
			\
			gfx_device.cpp \
			gfx_trans_affine.cpp \
			gfx_painter.cpp \
			gfx_raster_adapter.cpp \
			gfx_gradient_adapter.cpp \
			gfx_rendering_buffer.cpp \
			gfx_mask_layer.cpp \
			gfx_font_adapter_win32.cpp \
			gfx_font_adapter_freetype2.cpp \
			gfx_font_load_freetype2.cpp \
			\
			picasso_matrix.cpp \
			picasso_painter.cpp \
			picasso_rendering_buffer.cpp \
			picasso_raster_adapter.cpp \
			picasso.cpp \
			picasso_canvas.cpp \
			picasso_image.cpp \
			picasso_pattern.cpp \
			picasso_path.cpp \
			picasso_gradient.cpp \
			picasso_font.cpp \
			picasso_gpc.cpp \
			picasso_mask.cpp 

libpicasso_la_LDFLAGS= -no-undefined -release $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MICRO)

