/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */
#ifndef _PICASSO_HELPER_H_
#define _PICASSO_HELPER_H_

#include "aggheader.h"
#include "picasso_p.h"
#include "picasso_wrapper.h"

namespace picasso {

typedef span_interpolator_linear<> interpolator_type;

template<typename Pixfmt> struct picasso_painter_helper;

//32 bit clolr bilinear
template<> struct picasso_painter_helper<pixfmt_rgba32> 
{
	typedef pixfmt_wrapper<pixfmt_rgba32, mask_type> format;
	typedef image_accessor_clone<format> image_source_type;
	typedef span_image_filter_rgba_nb<image_source_type, interpolator_type> span_image_filter_type;
	typedef span_image_filter_rgba_nn_nb<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef span_image_filter_rgba<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef span_image_filter_rgba_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef span_image_filter_rgba_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef span_image_filter_rgba_nn_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef span_image_filter_rgba<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef span_image_filter_rgba_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

template<> struct picasso_painter_helper<pixfmt_argb32> 
{
	typedef pixfmt_wrapper<pixfmt_argb32, mask_type> format;
	typedef image_accessor_clone<format> image_source_type;
	typedef span_image_filter_rgba_nb<image_source_type, interpolator_type> span_image_filter_type;
	typedef span_image_filter_rgba_nn_nb<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef span_image_filter_rgba<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef span_image_filter_rgba_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef span_image_filter_rgba_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef span_image_filter_rgba_nn_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef span_image_filter_rgba<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef span_image_filter_rgba_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

template<> struct picasso_painter_helper<pixfmt_abgr32> 
{
	typedef pixfmt_wrapper<pixfmt_abgr32, mask_type> format;
	typedef image_accessor_clone<format> image_source_type;
	typedef span_image_filter_rgba_nb<image_source_type, interpolator_type> span_image_filter_type;
	typedef span_image_filter_rgba_nn_nb<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef span_image_filter_rgba<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef span_image_filter_rgba_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef span_image_filter_rgba_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef span_image_filter_rgba_nn_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef span_image_filter_rgba<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef span_image_filter_rgba_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

template<> struct picasso_painter_helper<pixfmt_bgra32> 
{
	typedef pixfmt_wrapper<pixfmt_bgra32, mask_type> format;
	typedef image_accessor_clone<format> image_source_type;
	typedef span_image_filter_rgba_nb<image_source_type, interpolator_type> span_image_filter_type;
	typedef span_image_filter_rgba_nn_nb<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef span_image_filter_rgba<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef span_image_filter_rgba_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef span_image_filter_rgba_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef span_image_filter_rgba_nn_nb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef span_image_filter_rgba<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef span_image_filter_rgba_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

//24 bit clolr bilinear
template<> struct picasso_painter_helper<pixfmt_rgb24> 
{
	typedef pixfmt_wrapper<pixfmt_rgb24, mask_type> format;
	typedef image_accessor_clone<format> image_source_type;
	typedef span_image_filter_rgb<image_source_type, interpolator_type> span_image_filter_type;
	typedef span_image_filter_rgb_nn<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef span_image_filter_rgb<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef span_image_filter_rgb_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef span_image_filter_rgb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef span_image_filter_rgb_nn<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef span_image_filter_rgb<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef span_image_filter_rgb_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

template<> struct picasso_painter_helper<pixfmt_bgr24> 
{
	typedef pixfmt_wrapper<pixfmt_bgr24, mask_type> format;
	typedef image_accessor_clone<format> image_source_type;
	typedef span_image_filter_rgb<image_source_type, interpolator_type> span_image_filter_type;
	typedef span_image_filter_rgb_nn<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef span_image_filter_rgb<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef span_image_filter_rgb_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef span_image_filter_rgb<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef span_image_filter_rgb_nn<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef span_image_filter_rgb<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef span_image_filter_rgb_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

//16 bit color bilinear
template<> struct picasso_painter_helper<pixfmt_rgb565> 
{
	typedef pixfmt_wrapper<pixfmt_rgb565, mask_type> format;
	typedef image_accessor_clone<format> image_source_type;
	typedef span_image_filter_rgb_packed<image_source_type, interpolator_type> span_image_filter_type;
	typedef span_image_filter_rgb_packed_nn<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef span_image_filter_rgb_packed<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef span_image_filter_rgb_packed_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef span_image_filter_rgb_packed<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef span_image_filter_rgb_packed_nn<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef span_image_filter_rgb_packed<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef span_image_filter_rgb_packed_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

template<> struct picasso_painter_helper<pixfmt_rgb555> 
{
	typedef pixfmt_wrapper<pixfmt_rgb555, mask_type> format;
	typedef image_accessor_clone<format> image_source_type;
	typedef span_image_filter_rgb_packed<image_source_type, interpolator_type> span_image_filter_type;
	typedef span_image_filter_rgb_packed_nn<image_source_type, interpolator_type> span_image_filter_type_nn;
	typedef span_image_filter_rgb_packed<image_source_type, interpolator_type> span_canvas_filter_type;
	typedef span_image_filter_rgb_packed_nn<image_source_type, interpolator_type> span_canvas_filter_type_nn;
	//pattern repeat
	typedef span_image_filter_rgb_packed<pattern_wrapper<format>, interpolator_type> span_image_pattern_type;
	typedef span_image_filter_rgb_packed_nn<pattern_wrapper<format>, interpolator_type> span_image_pattern_type_nn;
	typedef span_image_filter_rgb_packed<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type;
	typedef span_image_filter_rgb_packed_nn<pattern_wrapper<format>, interpolator_type> span_canvas_pattern_type_nn;
};

}
#endif/*_PICASSO_HELPER_H_*/
