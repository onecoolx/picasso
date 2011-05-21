TOP_DIR=..

INCLUDES = -I./include \
		   -I../include \
		   -I../ 

VPATH=./ ./inclue ./lib

lib_LTLIBRARIES = libpicasso.la

AM_CFLAGS = -DEXPORT=1
AM_CXXFLAGS = -DEXPORT=1

libpicasso_la_SOURCES= \
			agg_arc.cpp \
			agg_bezier_arc.cpp \
			agg_bspline.cpp \
			agg_curves.cpp \
			agg_font_freetype.cpp \
			agg_font_win32_tt.cpp \
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
			picasso.cpp \
			picasso_canvas.cpp \
			picasso_matrix.cpp \
			picasso_image.cpp \
			picasso_pattern.cpp \
			picasso_path.cpp \
			picasso_gradient.cpp \
			picasso_font.cpp \
			picasso_gpc.cpp \
			picasso_mask.cpp \
			picasso_engine_win32.cpp \
			picasso_engine_freetype2.cpp \
			picasso_freetype2_load.cpp

libpicasso_la_LDFLAGS= -no-undefined -release $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_MICRO)

