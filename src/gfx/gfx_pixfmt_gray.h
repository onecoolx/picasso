/* Picasso - a vector graphics library
 *
 * Copyright (C) 2023 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_PIXFMT_GRAY_H_
#define _GFX_PIXFMT_GRAY_H_

#include "common.h"
#include "gfx_composite_packed.h"
#include "gfx_blender_packed.h"
#include "gfx_rendering_buffer.h"

namespace gfx {

typedef uint8_t gray8;

template <typename ColorType>
class blend_op_adaptor_gray
{
public:
    typedef ColorType color_type;
    typedef gray8 pixel_type;
    typedef order_gray order_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    static _FORCE_INLINE_ void blend_pix(uint32_t op, pixel_type* p,
                                         uint32_t cr, uint32_t cg, uint32_t cb,
                                         uint32_t ca, uint32_t cover)
    {
        blend_op_table_packed<color_type, order_type, blend_op_adaptor_gray<rgba8>>::g_packed_blend_op_func[op]
        (p, (cr * ca + base_mask) >> base_shift,
         (cg * ca + base_mask) >> base_shift,
         (cb * ca + base_mask) >> base_shift,
         ca, cover);
    }

    static pixel_type make_pix(uint32_t r, uint32_t g, uint32_t b)
    {
        return pixel_type((55u * r + 184u * g + 18u * b) >> 8);
    }

    static color_type make_color(pixel_type p)
    {
        return color_type(p, p, p);
    }
};

typedef blend_op_adaptor_gray<rgba8> blender_gray8;
typedef pixfmt_blender_packed<blender_gray8, gfx_rendering_buffer> pixfmt_gray8; // pixfmt_grey

}
#endif /*_GFX_PIXFMT_GRAY_H_*/
