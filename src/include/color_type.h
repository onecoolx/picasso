/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2011 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _COLOR_TYPE_H_
#define _COLOR_TYPE_H_

#include "common.h"
#include "math_type.h"
#include "graphic_base.h"

namespace picasso {

typedef enum {
    pix_fmt_rgba,
    pix_fmt_argb,
    pix_fmt_abgr,
    pix_fmt_bgra,
    pix_fmt_rgb,
    pix_fmt_bgr,
    pix_fmt_rgb565,
    pix_fmt_rgb555,
} pix_fmt;

// color rgba 0 ~ 1
struct rgba
{
	scalar r;
	scalar g;
	scalar b;
	scalar a;

	rgba() {}
	rgba(scalar _r, scalar _g, scalar _b, scalar _a = DBL_TO_SCALAR(1.0))
		: r(_r), g(_g), b(_b), a(_a) {}
};

// color rgba8 0 ~ 255
struct rgba8
{
	typedef byte color_type;
	enum color_base {
		shift_value = sizeof(color_type),
		scale_value = 1 << shift_value,
		mask_value = scale_value - 1,
	};

	color_type r;
	color_type g;
	color_type b;
	color_type a;

	rgba8() {}

	rgba8(unsigned int _r, unsigned int _g, unsigned int _b, unsigned int _a = mask_value)
		: r(_r), g(_g), b(_b), a(_a) 
	{
	}

	rgba8(const rgba& c) 
        : r((color_type)uround(c.r * scalar(mask_value))) 
		, g((color_type)uround(c.g * scalar(mask_value))) 
		, b((color_type)uround(c.b * scalar(mask_value))) 
		, a((color_type)uround(c.a * scalar(mask_value))) 
	{
	}

};

}
#endif /*_COLOR_TYPE_H_*/
