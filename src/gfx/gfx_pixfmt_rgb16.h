/* Picasso - a vector graphics library
 *
 * Copyright (C) 2014 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_PIXFMT_RGB16_H_
#define _GFX_PIXFMT_RGB16_H_

#include "common.h"
#include "gfx_composite_packed.h"
#include "gfx_blender_packed.h"
#include "gfx_rendering_buffer.h"

namespace gfx {

// blender_rgb555
class blender_rgb555
{
public:
    typedef rgba8 color_type;
    typedef order_rgb555 order_type;
    typedef color_type::value_type value_type;
    typedef color_type::calc_type calc_type;
    typedef uint16_t pixel_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    static _FORCE_INLINE_ void blend_pix(uint32_t op, pixel_type* p,
                                         uint32_t cr, uint32_t cg,
                                         uint32_t cb, uint32_t ca, uint32_t cover)
    {
        blend_op_table_packed<color_type, order_type, blender_rgb555>::g_packed_blend_op_func[op]
        (p, (cr * ca + base_mask) >> base_shift,
         (cg * ca + base_mask) >> base_shift,
         (cb * ca + base_mask) >> base_shift,
         ca, cover);
    }

    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t cr, uint32_t cg,
                                         uint32_t cb, uint32_t ca, uint32_t)
    {
        _REGISTER_ pixel_type rgb = *p;
        calc_type r = (rgb >> 7) & 0xF8;
        calc_type g = (rgb >> 2) & 0xF8;
        calc_type b = (rgb << 3) & 0xF8;
        *p = (pixel_type)(((((cr - r) * ca + (r << 8)) >> 1) & 0x7C00) |
                          ((((cg - g) * ca + (g << 8)) >> 6) & 0x03E0) |
                          (((cb - b) * ca + (b << 8)) >> 11) | 0x8000);
    }

    static pixel_type make_pix(uint32_t r, uint32_t g, uint32_t b)
    {
        return (pixel_type)(((r & 0xF8) << 7) | ((g & 0xF8) << 2) | (b >> 3) | 0x8000);
    }

    static color_type make_color(pixel_type p)
    {
        return color_type((p >> 7) & 0xF8, (p >> 2) & 0xF8, (p << 3) & 0xF8);
    }
};

// blender_rgb565
class blender_rgb565
{
public:
    typedef rgba8 color_type;
    typedef order_rgb565 order_type;
    typedef color_type::value_type value_type;
    typedef color_type::calc_type calc_type;
    typedef uint16_t pixel_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    static _FORCE_INLINE_ void blend_pix(uint32_t op, pixel_type* p,
                                         uint32_t cr, uint32_t cg,
                                         uint32_t cb, uint32_t ca, uint32_t cover)
    {
        blend_op_table_packed<color_type, order_type, blender_rgb565>::g_packed_blend_op_func[op]
        (p, (cr * ca + base_mask) >> base_shift,
         (cg * ca + base_mask) >> base_shift,
         (cb * ca + base_mask) >> base_shift,
         ca, cover);
    }

    static pixel_type make_pix(uint32_t r, uint32_t g, uint32_t b)
    {
        return (pixel_type)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
    }

    static color_type make_color(pixel_type p)
    {
        return color_type((p >> 8) & 0xF8, (p >> 3) & 0xFC, (p << 3) & 0xF8);
    }
};

typedef pixfmt_blender_packed<blender_rgb555, gfx_rendering_buffer> pixfmt_rgb555; // pixfmt_rgb555
typedef pixfmt_blender_packed<blender_rgb565, gfx_rendering_buffer> pixfmt_rgb565; // pixfmt_rgb565

}
#endif /*_GFX_PIXFMT_RGB16_H_*/
