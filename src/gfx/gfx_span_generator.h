/* Picasso - a vector graphics library
 *
 * Copyright (C) 2014 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_SPAN_GENERATOR_H_
#define _GFX_SPAN_GENERATOR_H_

#include "common.h"
#include "data_vector.h"
#include "matrix.h"

#include "gfx_line_generator.h"

namespace gfx {

// span allocater
template <typename ColorType>
class gfx_span_allocator
{
public:
    typedef ColorType color_type;

    enum {
        base_span_shift = 8,
        base_span_mask = 1 << base_span_shift,
        base_span_block = base_span_mask - 1,
    };

    color_type* allocate(uint32_t span_len)
    {
        if (span_len > m_span.size()) {
            m_span.resize(((span_len + base_span_block) >> 8) << 8);
        }
        return &m_span[0];
    }

    color_type* span(void) { return &m_span[0]; }
    uint32_t max_span_len(void) const { return m_span.size(); }

private:
    pod_array<color_type> m_span;
};

// span interpolator linear
class gfx_span_interpolator_linear
{
public:
    enum {
        subpixel_shift = 8,
        subpixel_scale = 1 << subpixel_shift,
    };

    gfx_span_interpolator_linear(const trans_affine& trans)
        : m_trans(&trans)
    {
    }

    void begin(scalar x, scalar y, uint32_t len)
    {
        scalar tx;
        scalar ty;

        tx = x;
        ty = y;
        m_trans->transform(&tx, &ty);
        int32_t x1 = iround(tx * subpixel_scale);
        int32_t y1 = iround(ty * subpixel_scale);

        tx = x + len;
        ty = y;
        m_trans->transform(&tx, &ty);
        int32_t x2 = iround(tx * subpixel_scale);
        int32_t y2 = iround(ty * subpixel_scale);

        m_li_x = gfx_dda2_line_interpolator(x1, x2, len);
        m_li_y = gfx_dda2_line_interpolator(y1, y2, len);
    }

    void operator++()
    {
        ++m_li_x;
        ++m_li_y;
    }

    void coordinates(int32_t* x, int32_t* y) const
    {
        *x = m_li_x.y();
        *y = m_li_y.y();
    }

private:
    const trans_affine* m_trans;
    gfx_dda2_line_interpolator m_li_x;
    gfx_dda2_line_interpolator m_li_y;
};

}
#endif /*_GFX_SPAN_GENERATOR_H_*/
