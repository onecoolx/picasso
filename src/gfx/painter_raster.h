/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PAINTER_RASTER_H_
#define _PAINTER_RASTER_H_

#include "aggheader.h"
#include "pixfmt_wrapper.h"

namespace gfx {

typedef agg::span_interpolator_linear<> interpolator_type;

template<typename Pixfmt> struct painter_raster;

//32 bit clolr bilinear
template<> struct painter_raster<agg::pixfmt_rgba32> 
{
	typedef pixfmt_wrapper<agg::pixfmt_rgba32, mask_type> format;
	typedef agg::image_accessor_clone<format> image_source_type;
	typedef agg::span_image_filter_rgba_nb<image_source_type, interpolator_type> span_image_filter_type;
	typedef agg::span_image_filter_rgba_nn_nb<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef agg::span_image_filter_rgba<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef agg::span_image_filter_rgba_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef agg::span_image_filter_rgba_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef agg::span_image_filter_rgba_nn_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef agg::span_image_filter_rgba<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef agg::span_image_filter_rgba_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

template<> struct painter_raster<agg::pixfmt_argb32> 
{
	typedef pixfmt_wrapper<agg::pixfmt_argb32, mask_type> format;
	typedef agg::image_accessor_clone<format> image_source_type;
	typedef agg::span_image_filter_rgba_nb<image_source_type, interpolator_type> span_image_filter_type;
	typedef agg::span_image_filter_rgba_nn_nb<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef agg::span_image_filter_rgba<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef agg::span_image_filter_rgba_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef agg::span_image_filter_rgba_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef agg::span_image_filter_rgba_nn_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef agg::span_image_filter_rgba<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef agg::span_image_filter_rgba_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

template<> struct painter_raster<agg::pixfmt_abgr32> 
{
	typedef pixfmt_wrapper<agg::pixfmt_abgr32, mask_type> format;
	typedef agg::image_accessor_clone<format> image_source_type;
	typedef agg::span_image_filter_rgba_nb<image_source_type, interpolator_type> span_image_filter_type;
	typedef agg::span_image_filter_rgba_nn_nb<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef agg::span_image_filter_rgba<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef agg::span_image_filter_rgba_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef agg::span_image_filter_rgba_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef agg::span_image_filter_rgba_nn_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef agg::span_image_filter_rgba<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef agg::span_image_filter_rgba_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

template<> struct painter_raster<agg::pixfmt_bgra32> 
{
	typedef pixfmt_wrapper<agg::pixfmt_bgra32, mask_type> format;
	typedef agg::image_accessor_clone<format> image_source_type;
	typedef agg::span_image_filter_rgba_nb<image_source_type, interpolator_type> span_image_filter_type;
	typedef agg::span_image_filter_rgba_nn_nb<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef agg::span_image_filter_rgba<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef agg::span_image_filter_rgba_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef agg::span_image_filter_rgba_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef agg::span_image_filter_rgba_nn_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef agg::span_image_filter_rgba<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef agg::span_image_filter_rgba_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

//24 bit clolr bilinear
template<> struct painter_raster<agg::pixfmt_rgb24> 
{
	typedef pixfmt_wrapper<agg::pixfmt_rgb24, mask_type> format;
	typedef agg::image_accessor_clone<format> image_source_type;
	typedef agg::span_image_filter_rgb<image_source_type, interpolator_type> span_image_filter_type;
	typedef agg::span_image_filter_rgb_nn<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef agg::span_image_filter_rgb<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef agg::span_image_filter_rgb_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef agg::span_image_filter_rgb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef agg::span_image_filter_rgb_nn<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef agg::span_image_filter_rgb<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef agg::span_image_filter_rgb_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

template<> struct painter_raster<agg::pixfmt_bgr24> 
{
	typedef pixfmt_wrapper<agg::pixfmt_bgr24, mask_type> format;
	typedef agg::image_accessor_clone<format> image_source_type;
	typedef agg::span_image_filter_rgb<image_source_type, interpolator_type> span_image_filter_type;
	typedef agg::span_image_filter_rgb_nn<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef agg::span_image_filter_rgb<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef agg::span_image_filter_rgb_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef agg::span_image_filter_rgb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef agg::span_image_filter_rgb_nn<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef agg::span_image_filter_rgb<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef agg::span_image_filter_rgb_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

//16 bit color bilinear
template<> struct painter_raster<agg::pixfmt_rgb565> 
{
	typedef pixfmt_wrapper<agg::pixfmt_rgb565, mask_type> format;
	typedef agg::image_accessor_clone<format> image_source_type;
	typedef agg::span_image_filter_rgb_packed<image_source_type, interpolator_type> span_image_filter_type;
	typedef agg::span_image_filter_rgb_packed_nn<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef agg::span_image_filter_rgb_packed<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef agg::span_image_filter_rgb_packed_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef agg::span_image_filter_rgb_packed<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef agg::span_image_filter_rgb_packed_nn<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef agg::span_image_filter_rgb_packed<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef agg::span_image_filter_rgb_packed_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

template<> struct painter_raster<agg::pixfmt_rgb555> 
{
	typedef pixfmt_wrapper<agg::pixfmt_rgb555, mask_type> format;
	typedef agg::image_accessor_clone<format> image_source_type;
	typedef agg::span_image_filter_rgb_packed<image_source_type, interpolator_type> span_image_filter_type;
	typedef agg::span_image_filter_rgb_packed_nn<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef agg::span_image_filter_rgb_packed<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef agg::span_image_filter_rgb_packed_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef agg::span_image_filter_rgb_packed<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef agg::span_image_filter_rgb_packed_nn<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef agg::span_image_filter_rgb_packed<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef agg::span_image_filter_rgb_packed_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

}

#endif /*_PAINTER_RASTER_H_*/
