/* Picasso - a vector graphics library
 *
 * Copyright (C) 2024 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_BLENDER_PACKED_H_
#define _GFX_BLENDER_PACKED_H_

#include "common.h"

namespace gfx {

// pixfmt blender packed
template <typename Blender, typename RenBuffer>
class pixfmt_blender_packed
{
public:
    typedef RenBuffer buffer_type;
    typedef typename buffer_type::row_data row_data;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename blender_type::color_type color_type;
    typedef typename blender_type::order_type order_type;
    typedef typename color_type::value_type value_type;

    enum {
        base_shift = color_type::base_shift,
        base_scale = color_type::base_scale,
        base_mask = color_type::base_mask,
        pix_width = sizeof(pixel_type)
    };

    pixfmt_blender_packed()
        : m_buffer(0)
        , m_blend_op(comp_op_src_over)
        , m_alpha_factor(base_mask)
    {
    }

    explicit pixfmt_blender_packed(buffer_type& rb, uint32_t op = comp_op_src_over, uint32_t alpha = base_mask)
        : m_buffer(&rb)
        , m_blend_op(op)
        , m_alpha_factor(alpha)
    {
    }

    void attach(buffer_type& rb) { m_buffer = &rb; }

    uint32_t width(void) const { return m_buffer->internal_width(); }
    uint32_t height(void) const { return m_buffer->internal_height(); }
    int stride(void) const { return m_buffer->internal_stride(); }

    byte* row_ptr(int y) { return m_buffer->row_ptr(y); }
    const byte* row_ptr(int y) const { return m_buffer->row_ptr(y); }
    row_data row(int y) const { return m_buffer->row(y); }

    void alpha(scalar a) { m_alpha_factor = uround(a * base_mask); }
    scalar alpha(void) const { return INT_TO_SCALAR(m_alpha_factor) / FLT_TO_SCALAR(255.0f); }

    byte* pix_zero(void) const
    {
        static pixel_type zero = 0;
        return (byte*)&zero;
    }

    byte* pix_ptr(int x, int y) const
    {
        return m_buffer->row_ptr(y) + x * pix_width;
    }

    void blend_op(uint32_t op) { m_blend_op = op; }
    uint32_t blend_op(void) const { return m_blend_op; }

    color_type pixel(int x, int y) const
    {
        return blender_type::make_color(((pixel_type*)m_buffer->row_ptr(y))[x]);
    }

    void copy_pixel(int x, int y, const color_type& c)
    {
        ((pixel_type*)m_buffer->row_ptr(x, y, 1))[x] = blender_type::make_pix(c.r, c.g, c.b);
    }

    void blend_pixel(int x, int y, const color_type& c, uint8_t cover)
    {
        blender_type::blend_pix(m_blend_op,
                                (pixel_type*)m_buffer->row_ptr(x, y, 1) + x,
                                c.r, c.g, c.b, (value_type)alpha_mul(c.a, m_alpha_factor), cover);
    }

    void copy_hline(int x, int y, uint32_t len, const color_type& c)
    {
        pixel_type* p = (pixel_type*)m_buffer->row_ptr(x, y, len) + x;
        pixel_type v = blender_type::make_pix(c.r, c.g, c.b);
        do {
            *p++ = v;
        } while (--len);
    }

    void copy_vline(int x, int y, uint32_t len, const color_type& c)
    {
        pixel_type v = blender_type::make_pix(c.r, c.g, c.b);
        do {
            pixel_type* p = (pixel_type*)m_buffer->row_ptr(x, y++, 1) + x;
            *p = v;
        } while (--len);
    }

    void blend_hline(int x, int y, uint32_t len, const color_type& c, uint8_t cover)
    {
        pixel_type* p = (pixel_type*)m_buffer->row_ptr(x, y, len) + x;
        _REGISTER_ value_type alpha = (value_type)alpha_mul(c.a, m_alpha_factor);

        if ((m_blend_op == comp_op_src_over) && (alpha == base_mask) && (cover == 255)) {
            // optimization.
            pixel_type v = blender_type::make_pix(c.r, c.g, c.b);
            do {
                *p++ = v;
            } while (--len);
        } else {
            do {
                blender_type::blend_pix(m_blend_op, p, c.r, c.g, c.b, alpha, cover);
                p++;
            } while (--len);
        }
    }

    void blend_vline(int x, int y, uint32_t len, const color_type& c, uint8_t cover)
    {
        _REGISTER_ value_type alpha = (value_type)alpha_mul(c.a, m_alpha_factor);
        do {
            blender_type::blend_pix(m_blend_op,
                                    (pixel_type*)m_buffer->row_ptr(x, y++, 1) + x,
                                    c.r, c.g, c.b, alpha, cover);
        } while (--len);
    }

    void blend_solid_hspan(int x, int y, uint32_t len, const color_type& c, const uint8_t* covers)
    {
        pixel_type* p = (pixel_type*)m_buffer->row_ptr(x, y, len) + x;
        _REGISTER_ value_type alpha = (value_type)alpha_mul(c.a, m_alpha_factor);
        do {
            blender_type::blend_pix(m_blend_op,
                                    p, c.r, c.g, c.b, alpha, *covers++);
            p++;
        } while (--len);
    }

    void blend_solid_vspan(int x, int y, uint32_t len, const color_type& c, const uint8_t* covers)
    {
        _REGISTER_ value_type alpha = (value_type)alpha_mul(c.a, m_alpha_factor);
        do {
            blender_type::blend_pix(m_blend_op,
                                    (pixel_type*)m_buffer->row_ptr(x, y++, 1) + x,
                                    c.r, c.g, c.b, alpha, *covers++);
        } while (--len);
    }

    void copy_color_hspan(int x, int y, uint32_t len, const color_type* colors)
    {
        pixel_type* p = (pixel_type*)m_buffer->row_ptr(x, y, len) + x;
        do {
            *p++ = blender_type::make_pix(colors->r, colors->g, colors->b);
            ++colors;
        } while (--len);
    }

    void copy_color_vspan(int x, int y, uint32_t len, const color_type* colors)
    {
        do {
            pixel_type* p = (pixel_type*)m_buffer->row_ptr(x, y++, 1) + x;
            *p = blender_type::make_pix(colors->r, colors->g, colors->b);
            ++colors;
        } while (--len);
    }

    void blend_color_hspan(int x, int y, uint32_t len,
                           const color_type* colors, const uint8_t* covers, uint8_t cover)
    {
        pixel_type* p = (pixel_type*)m_buffer->row_ptr(x, y, len) + x;
        do {
            blender_type::blend_pix(m_blend_op, p,
                                    colors->r,
                                    colors->g,
                                    colors->b,
                                    (value_type)alpha_mul(colors->a, m_alpha_factor),
                                    covers ? *covers++ : cover);
            p++;
            ++colors;
        } while (--len);
    }

    void blend_color_vspan(int x, int y, uint32_t len,
                           const color_type* colors, const uint8_t* covers, uint8_t cover)
    {
        do {
            blender_type::blend_pix(m_blend_op,
                                    (pixel_type*)m_buffer->row_ptr(x, y++, 1) + x,
                                    colors->r,
                                    colors->g,
                                    colors->b,
                                    (value_type)alpha_mul(colors->a, m_alpha_factor),
                                    covers ? *covers++ : cover);
            ++colors;
        } while (--len);
    }

    static void make_pix(byte* p, const color_type& c)
    {
        *(pixel_type*)p = blender_type::make_pix(c.r, c.g, c.b);
    }

    template <typename RenBuffer2>
    void copy_point_from(const RenBuffer2& from, int xdst, int ydst, int xsrc, int ysrc)
    {
        const byte* p = from.row_ptr(ysrc);
        if (p) {
            mem_deep_copy(m_buffer->row_ptr(xdst, ydst, 1) + xdst * pix_width,
                          p + xsrc * pix_width, pix_width);
        }
    }

    template <typename RenBuffer2>
    void copy_from(const RenBuffer2& from, int xdst, int ydst, int xsrc, int ysrc, uint32_t len)
    {
        const byte* p = from.row_ptr(ysrc);
        if (p) {
            mem_deep_copy(m_buffer->row_ptr(xdst, ydst, len) + xdst * pix_width,
                          p + xsrc * pix_width, len * pix_width);
        }
    }

    template <typename SrcPixelFormatRenderer>
    void blend_from(const SrcPixelFormatRenderer& from, int xdst, int ydst,
                    int xsrc, int ysrc, uint32_t len, uint8_t cover)
    {
        typedef typename SrcPixelFormatRenderer::order_type src_order;
        const value_type* psrc = (const value_type*)from.row_ptr(ysrc);
        if (psrc) {
            psrc += xsrc << 2;
            pixel_type* pdst =
                (pixel_type*)m_buffer->row_ptr(xdst, ydst, len) + xdst;

            do {
                blender_type::blend_pix(m_blend_op, pdst,
                                        psrc[src_order::R],
                                        psrc[src_order::G],
                                        psrc[src_order::B],
                                        (value_type)alpha_mul(psrc[src_order::A], m_alpha_factor), cover);
                psrc += 4;
                ++pdst;
            } while (--len);
        }
    }

    template <typename SrcPixelFormatRenderer>
    void blend_point_from(const SrcPixelFormatRenderer& from, int xdst, int ydst,
                          int xsrc, int ysrc, uint8_t cover)
    {
        typedef typename SrcPixelFormatRenderer::order_type src_order;
        const value_type* psrc = (const value_type*)from.row_ptr(ysrc);
        if (psrc) {
            psrc += xsrc << 2;
            pixel_type* pdst = (pixel_type*)m_buffer->row_ptr(xdst, ydst, 1) + xdst;

            blender_type::blend_pix(m_blend_op, pdst,
                                    psrc[src_order::R],
                                    psrc[src_order::G],
                                    psrc[src_order::B],
                                    (value_type)alpha_mul(psrc[src_order::A], m_alpha_factor), cover);
        }
    }

    template <typename SrcPixelFormatRenderer>
    void blend_from_color(const SrcPixelFormatRenderer& from, const color_type& color,
                          int xdst, int ydst, int xsrc, int ysrc, uint32_t len, uint8_t cover)
    {
        typedef typename SrcPixelFormatRenderer::value_type src_value_type;
        const src_value_type* psrc = (src_value_type*)from.row_ptr(ysrc);
        _REGISTER_ value_type alpha = (value_type)alpha_mul(color.a, m_alpha_factor);
        if (psrc) {
            pixel_type* pdst = (pixel_type*)m_buffer->row_ptr(xdst, ydst, len) + xdst;

            do {
                blender_type::blend_pix(m_blend_op, pdst,
                                        color.r, color.g, color.b, alpha,
                                        (*psrc * cover + base_mask) >> base_shift);
                ++psrc;
                ++pdst;
            } while (--len);
        }
    }

    template <class SrcPixelFormatRenderer>
    void blend_from_lut(const SrcPixelFormatRenderer& from, const color_type* color_lut,
                        int xdst, int ydst, int xsrc, int ysrc, uint32_t len, uint8_t cover)
    {
        typedef typename SrcPixelFormatRenderer::value_type src_value_type;
        const src_value_type* psrc = (src_value_type*)from.row_ptr(ysrc);
        if (psrc) {
            pixel_type* pdst = (pixel_type*)m_buffer->row_ptr(xdst, ydst, len) + xdst;

            do {
                const color_type& color = color_lut[*psrc];
                blender_type::blend_pix(m_blend_op, pdst,
                                        color.r, color.g, color.b, (value_type)alpha_mul(color.a, m_alpha_factor),
                                        cover);
                ++psrc;
                ++pdst;
            } while (--len);
        }
    }

private:
    uint32_t alpha_mul(uint32_t a, uint32_t s)
    {
        return (s == 255) ? a : ((a * s + base_mask) >> base_shift);
    }

    pixfmt_blender_packed(const pixfmt_blender_packed&);
    pixfmt_blender_packed& operator=(const pixfmt_blender_packed&);
private:
    buffer_type* m_buffer;
    uint32_t m_blend_op;
    uint32_t m_alpha_factor;
};

}
#endif /*_GFX_BLENDER_PACKED_H_*/
