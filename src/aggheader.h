/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _AGG_HEADER_H_
#define _AGG_HEADER_H_

#include "agg_basics.h"
#include "agg_math.h"
#include "agg_rendering_buffer.h"
#include "agg_renderer_mclip.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_scanline_p.h"
#include "agg_scanline_bin.h"
#include "agg_scanline_storage_aa.h"
#include "agg_scanline_storage_bin.h"
#include "agg_renderer_scanline.h"
#include "agg_alpha_mask_u8.h"
#include "agg_pixfmt_amask_adaptor.h"
#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgb_packed.h"
#include "agg_path_storage.h"
#include "agg_path_length.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_conv_gpc.h"
#include "agg_conv_curve.h"
#include "agg_ellipse.h"
#include "agg_rounded_rect.h"
#include "agg_curves.h"
#include "agg_arc.h"
#include "agg_blur.h"
#include "agg_trans_affine.h"
#include "agg_conv_transform.h"
#include "agg_gradient_lut.h"
#include "agg_span_allocator.h"
#include "agg_span_interpolator_linear.h"
#include "agg_span_image_filter.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_image_filter_rgb.h"
#include "agg_span_image_filter_rgb_packed.h"
#include "agg_span_pattern_rgba.h"
#include "agg_span_pattern_rgb.h"
#include "agg_span_pattern_rgb_packed.h"
#include "agg_span_gradient.h"
#include "agg_image_filters.h"
#include "agg_image_accessors.h"
#include "agg_font_cache_manager.h"
#if defined(WIN32) && !ENABLE(FREE_TYPE2)
#include "agg_font_win32_tt.h"
#else
#include "agg_font_freetype.h"
#endif
using namespace agg;
#endif /*_AGG_HEADER_H_*/
