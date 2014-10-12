/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PIXFMT_WRAPPER_H_
#define _PIXFMT_WRAPPER_H_

#include "common.h"
#include "aggheader.h"

#include "gfx_mask_layer.h"

namespace gfx {

// alpha mask type
typedef agg::alpha_mask_gray8 mask_type;

//pixfmt wrapper
template<typename Pixfmt, typename AlphaMask>
class pixfmt_wrapper
{
public:
    typedef Pixfmt pixfmt_type;
    typedef typename pixfmt_type::color_type color_type;
    typedef typename pixfmt_type::order_type order_type;
    typedef typename pixfmt_type::pixel_type pixel_type;
    typedef typename pixfmt_type::value_type value_type;
    typedef typename pixfmt_type::row_data row_data;
    typedef AlphaMask amask_type;
    typedef typename amask_type::cover_type cover_type;
    enum base_scale_e 
    {
        base_shift = pixfmt_type::base_shift,
        base_scale = pixfmt_type::base_scale,
        base_mask  = pixfmt_type::base_mask,
        pix_width  = pixfmt_type::pix_width,
    };
public:
    pixfmt_wrapper()
        : use_mask(false), m_colorkey(0), m_fmt(), m_filter(), m_mask(m_fmt, m_filter), m_colors(0) 
    {
    }
    explicit pixfmt_wrapper(agg::rendering_buffer& rb)
        : use_mask(false), m_colorkey(0), m_fmt(rb), m_filter(), m_mask(m_fmt, m_filter), m_colors(0) 
    {
    }

    ~pixfmt_wrapper() { clear_mask(); clear_key();}

    void attach(agg::rendering_buffer& rb) { m_fmt.attach(rb); }

    void clear_mask()
    {
        use_mask = false;
        m_colors = 0;
    }

    void clear_key()
    {
        m_colorkey = 0;
    }

    void attach_mask(gfx_mask_layer* mask)
    {
        clear_mask(); // clear old data

        use_mask = true;
        m_filter.attach(mask->buffer());
        if (mask->type() == MASK_COLORS)
            m_colors = &mask->colors();
    }

    void set_transparent_color(agg::rgba8 * color)
    {
        m_colorkey = color;
    }

    bool is_color_mask() const
    {
        return m_colors && m_colors->size();
    }

    bool has_color(const color_type& c) const
    {
        //Note: m_color is not NULL first!
        for (unsigned i=0; i<m_colors->size(); i++)
            if(c == m_colors->at(i))
                return true;
        return false;
    }

public:

    //--------------------------------------------------------------------
    unsigned width()  const { return m_fmt.width();  }
    unsigned height() const { return m_fmt.height(); }
    int      stride() const { return m_fmt.stride(); }

    //--------------------------------------------------------------------
    agg::int8u* row_ptr(int y)       { return m_fmt.row_ptr(y); }
    const agg::int8u* row_ptr(int y) const { return m_fmt.row_ptr(y); }
    row_data     row(int y)     const { return m_fmt.row(y); }

    //--------------------------------------------------------------------
    agg::int8u* pix_ptr(int x, int y)
    {
        return m_fmt.pix_ptr(x, y);
    }

    const agg::int8u* pix_ptr(int x, int y) const
    {
        return m_fmt.pix_ptr(x, y);
    }

    //--------------------------------------------------------------------
    void alpha(float a) { m_fmt.alpha(a); }
    float alpha() const { return m_fmt.alpha(); }

    //--------------------------------------------------------------------
    void comp_op(unsigned op) { m_fmt.comp_op(op); }
    unsigned comp_op() const  { return m_fmt.comp_op(); }

    //--------------------------------------------------------------------
    static void make_pix(agg::int8u* p, const color_type& c)
    {
        pixfmt_type::make_pix(p, c);
    }

    //--------------------------------------------------------------------
    color_type pixel(int x, int y)
    {
        return m_fmt.pixel(x, y);
    }

    //--------------------------------------------------------------------
    void copy_pixel(int x, int y, const color_type& c)
    {
        if (m_colorkey && m_colorkey == c)
            return;

        if (use_mask)
            if (is_color_mask()) { 
                if (has_color(c))
                    m_mask.copy_pixel(x, y, c);
                else
                    m_fmt.copy_pixel(x, y, c);
            } else {
                m_mask.copy_pixel(x, y, c);
            }
        else
            m_fmt.copy_pixel(x, y, c);
    }

    //--------------------------------------------------------------------
    void blend_pixel(int x, int y, const color_type& c, cover_type cover)
    {
        if (m_colorkey && m_colorkey == c)
            return;

        if (use_mask)
            if (is_color_mask()) {
                   if (has_color(c))
                    m_mask.blend_pixel(x, y, c, cover);
                else
                    m_fmt.blend_pixel(x, y, c, cover);
            } else {
                m_mask.blend_pixel(x, y, c, cover);
            }
        else
            m_fmt.blend_pixel(x, y, c, cover);
    }

    //--------------------------------------------------------------------
    void copy_hline(int x, int y, unsigned len, const color_type& c)
    {
        if (m_colorkey && m_colorkey == c)
            return;

        if (use_mask)
            if (is_color_mask()) {
                if (has_color(c))
                    m_mask.copy_hline(x, y, len, c);
                else
                    m_fmt.copy_hline(x, y, len, c);
            } else {
                m_mask.copy_hline(x, y, len, c);
            }
        else
            m_fmt.copy_hline(x, y, len, c);
    }

    //--------------------------------------------------------------------
    void blend_hline(int x, int y, unsigned len, const color_type& c, cover_type cover)
    {
        if (m_colorkey && m_colorkey == c)
            return;

        if (use_mask)
            if (is_color_mask()) {
                if (has_color(c))
                    m_mask.blend_hline(x, y, len, c, cover);
                else
                    m_fmt.blend_hline(x, y, len, c, cover);
            } else {
                m_mask.blend_hline(x, y, len, c, cover);
            }
        else
            m_fmt.blend_hline(x, y, len, c, cover);
    }

    //--------------------------------------------------------------------
    void copy_vline(int x, int y, unsigned len, const color_type& c)
    {
        if (m_colorkey && m_colorkey == c)
            return;

        if (use_mask)
            if (is_color_mask()) {
                if (has_color(c))
                    m_mask.copy_vline(x, y, len, c);
                else
                    m_fmt.copy_vline(x, y, len, c);
            } else {
                m_mask.copy_vline(x, y, len, c);
            }
        else
            m_fmt.copy_vline(x, y, len, c);
    }

    //--------------------------------------------------------------------
    void blend_vline(int x, int y, unsigned len, const color_type& c, cover_type cover)
    {
        if (m_colorkey && m_colorkey == c)
            return;

        if (use_mask)
            if (is_color_mask()) {
                if (has_color(c))
                    m_mask.blend_vline(x, y, len, c, cover);
                else
                    m_fmt.blend_vline(x, y, len, c, cover);
            } else {
                m_mask.blend_vline(x, y, len, c, cover);
            }
        else
            m_fmt.blend_vline(x, y, len, c, cover);
    }

    //--------------------------------------------------------------------
    void blend_solid_hspan(int x, int y, unsigned len, 
                               const color_type& c, const cover_type* covers)
    {
        if (m_colorkey && m_colorkey == c)
            return;

        if (use_mask)
            if (is_color_mask()) {
                if (has_color(c))
                    m_mask.blend_solid_hspan(x, y, len, c, covers);
                else
                    m_fmt.blend_solid_hspan(x, y, len, c, covers);
            } else {
                m_mask.blend_solid_hspan(x, y, len, c, covers);
            }
        else
            m_fmt.blend_solid_hspan(x, y, len, c, covers);
    }

    //--------------------------------------------------------------------
    void blend_solid_vspan(int x, int y, unsigned len, 
                               const color_type& c, const cover_type* covers)
    {
        if (m_colorkey && m_colorkey == c)
            return;

        if (use_mask)
            if (is_color_mask()) {
                if (has_color(c))
                    m_mask.blend_solid_vspan(x, y, len, c, covers);
                else
                    m_fmt.blend_solid_vspan(x, y, len, c, covers);
            } else {
                m_mask.blend_solid_vspan(x, y, len, c, covers);
            }
        else
            m_fmt.blend_solid_vspan(x, y, len, c, covers);
    }

    //--------------------------------------------------------------------
    void copy_color_hspan(int x, int y, unsigned len, const color_type* colors)
    {
        if (m_colorkey) {
            if (use_mask)
                if (is_color_mask()) {
                    for (unsigned i=0; i<len; i++)
                        if (m_colorkey != colors[i]) {
                            if (has_color(colors[i]))
                                m_mask.copy_pixel(x+i, y, colors[i]);
                            else
                                m_fmt.copy_pixel(x+i, y, colors[i]);
                        }
                } else {
                    for (unsigned i=0; i<len; i++) {
                        if (m_colorkey != colors[i]) 
                                m_mask.copy_pixel(x+i, y, colors[i]);
                    }
                } else {
                    for (unsigned i=0; i<len; i++) {
                        if (m_colorkey != colors[i]) 
                                m_fmt.copy_pixel(x+i, y, colors[i]);
                    }
                }
        } else {
            if (use_mask)
                if (is_color_mask()) {
                    for (unsigned i=0; i<len; i++)
                        if (has_color(colors[i]))
                            m_mask.copy_pixel(x+i, y, colors[i]);
                        else
                            m_fmt.copy_pixel(x+i, y, colors[i]);
                } else {
                    m_mask.copy_color_hspan(x, y, len, colors);
                }
                else
                    m_fmt.copy_color_hspan(x, y, len, colors);
        }
    }

    //--------------------------------------------------------------------
    void copy_color_vspan(int x, int y, unsigned len, const color_type* colors)
    {
        if (m_colorkey) {
            if (use_mask)
                if (is_color_mask()) {
                    for (unsigned i=0; i<len; i++)
                        if (m_colorkey != colors[i]) {
                            if (has_color(colors[i]))
                                m_mask.copy_pixel(x, y+i, colors[i]);
                            else
                                m_fmt.copy_pixel(x, y+i, colors[i]);
                        }
                } else {
                    for (unsigned i=0; i<len; i++) {
                        if (m_colorkey != colors[i])
                                m_mask.copy_pixel(x, y+i, colors[i]);
                    }
                } else {
                    for (unsigned i=0; i<len; i++) {
                        if (m_colorkey != colors[i])
                            m_fmt.copy_pixel(x, y+i, colors[i]);
                    }
                }
        } else {
            if (use_mask)
                if (is_color_mask()) {
                    for (unsigned i=0; i<len; i++)
                        if (has_color(colors[i]))
                            m_mask.copy_pixel(x, y+i, colors[i]);
                        else
                            m_fmt.copy_pixel(x, y+i, colors[i]);
                } else {
                    m_mask.copy_color_vspan(x, y, len, colors);
                }
                else
                    m_fmt.copy_color_vspan(x, y, len, colors);
        }
    }

    //--------------------------------------------------------------------
    void blend_color_hspan(int x, int y, unsigned len, const color_type* colors,
            const cover_type* covers, cover_type cover)
    {
        if (m_colorkey) {
            if (use_mask)
                if (is_color_mask()) {
                    for (unsigned i=0; i<len; i++)
                        if (m_colorkey != colors[i]) {
                            if (has_color(colors[i]))
                                m_mask.blend_pixel(x+i, y, colors[i], covers[i]);
                            else
                                m_fmt.blend_pixel(x+i, y, colors[i], covers[i]);
                        }
                } else {
                    for (unsigned i=0; i<len; i++) {
                        if (m_colorkey != colors[i]) 
                                m_mask.blend_pixel(x+i, y, colors[i], covers[i]);
                    }
                } else {
                    for (unsigned i=0; i<len; i++) {
                        if (m_colorkey != colors[i]) 
                                m_fmt.blend_pixel(x+i, y, colors[i], covers[i]);
                    }
                }
        } else {
            if (use_mask)
                if (is_color_mask()) {
                    for (unsigned i=0; i<len; i++)
                        if (has_color(colors[i]))
                            m_mask.blend_pixel(x+i, y, colors[i], covers[i]);
                        else
                            m_fmt.blend_pixel(x+i, y, colors[i], covers[i]);
                } else {
                    m_mask.blend_color_hspan(x, y, len, colors, covers, cover);
                }
                else
                    m_fmt.blend_color_hspan(x, y, len, colors, covers, cover);
        }
    }

    //--------------------------------------------------------------------
    void blend_color_vspan(int x, int y, unsigned len, const color_type* colors,
            const cover_type* covers, cover_type cover)
    {
        if (m_colorkey) {
            if (use_mask)
                if (is_color_mask()) {
                    for (unsigned i=0; i<len; i++)
                        if (m_colorkey != colors[i]) {
                            if (has_color(colors[i]))
                                m_mask.blend_pixel(x, y+i, colors[i], covers[i]);
                            else
                                m_fmt.blend_pixel(x, y+i, colors[i], covers[i]);
                        }
                } else {
                    for (unsigned i=0; i<len; i++) {
                        if (m_colorkey != colors[i]) 
                                m_mask.blend_pixel(x, y+i, colors[i], covers[i]);
                    }
                } else {
                    for (unsigned i=0; i<len; i++) {
                        if (m_colorkey != colors[i]) 
                            m_fmt.blend_pixel(x, y+i, colors[i], covers[i]);
                    }
                }
        } else {
            if (use_mask)
                if (is_color_mask()) {
                    for (unsigned i=0; i<len; i++)
                        if (has_color(colors[i]))
                            m_mask.blend_pixel(x, y+i, colors[i], covers[i]);
                        else
                            m_fmt.blend_pixel(x, y+i, colors[i], covers[i]);
                } else {
                    m_mask.blend_color_vspan(x, y, len, colors, covers, cover);
                } else
                    m_fmt.blend_color_vspan(x, y, len, colors, covers, cover);
        }
    }

    //--------------------------------------------------------------------
    template<class RenBuf2> void copy_from(const RenBuf2& from, 
            int xdst, int ydst,
            int xsrc, int ysrc,
            unsigned len)
    {
        m_fmt.copy_from(from, xdst, ydst, xsrc, ysrc, len);
    }

    //--------------------------------------------------------------------
    template<class RenBuf2>
        void copy_point_from(const RenBuf2& from, 
                int xdst, int ydst,
                int xsrc, int ysrc)
    {
        m_fmt.copy_point_from(from, xdst, ydst, xsrc, ysrc);
    }

    //--------------------------------------------------------------------
    template<class SrcPixelFormatRenderer> 
        void blend_from(const SrcPixelFormatRenderer& from, 
                int xdst, int ydst,
                int xsrc, int ysrc,
                unsigned len,
                cover_type cover)
    {
        m_fmt.blend_from(from, xdst, ydst, xsrc, ysrc, len, cover);
    }

    //--------------------------------------------------------------------
    template<class SrcPixelFormatRenderer> 
        void blend_point_from(const SrcPixelFormatRenderer& from, 
                int xdst, int ydst,
                int xsrc, int ysrc,
                cover_type cover)
    {
        m_fmt.blend_point_from(from, xdst, ydst, xsrc, ysrc, cover);
    }

private:
    bool use_mask;
    agg::rgba8 *m_colorkey;
    pixfmt_type m_fmt;
    amask_type m_filter;
    agg::pixfmt_amask_adaptor<pixfmt_type, amask_type> m_mask;
    agg::pod_bvector<agg::rgba8> *m_colors;
};

inline bool operator == (const agg::rgba8& a, const agg::rgba8& b)
{
    return (a.r==b.r) && (a.g==b.g) && (a.b==b.b) && (a.a==b.a); 
}

// for color key compare
inline bool operator == (const agg::rgba8* a, const agg::rgba8& b)
{
    //Note : for color key only, not compare alpha value.
    return (a->r==b.r) && (a->g==b.g) && (a->b==b.b); 
}

inline bool operator != (const agg::rgba8* a, const agg::rgba8& b)
{
    //Note : for color key only, not compare alpha value.
    return (a->r!=b.r) || (a->g!=b.g) || (a->b!=b.b); 
}


//pattern wrapper
template<typename Pixfmt>
class pattern_wrapper
{
public:
    typedef Pixfmt   pixfmt_type;
    typedef typename pixfmt_type::color_type color_type;
    typedef typename pixfmt_type::order_type order_type;
    typedef typename pixfmt_type::value_type value_type;
    typedef typename pixfmt_type::pixel_type pixel_type;
    enum pix_width_e { pix_width = pixfmt_type::pix_width };

    pattern_wrapper() {}
    virtual ~pattern_wrapper() {}
    virtual const agg::int8u* span(int x, int y, unsigned) = 0;
    virtual const agg::int8u* next_x() = 0;
    virtual const agg::int8u* next_y() = 0;
};

template<typename Pixfmt, typename Wrap_X, typename Wrap_Y>
class pattern_wrapper_adaptor : public pattern_wrapper<Pixfmt>
{
public:
    explicit pattern_wrapper_adaptor(const Pixfmt& fmt)
        : m_wrap(fmt)
    {
    }
    virtual const agg::int8u* span(int x, int y, unsigned i)
    {
        return m_wrap.span(x, y, i);
    }
    virtual const agg::int8u* next_x()
    {
        return m_wrap.next_x();
    }
    virtual const agg::int8u* next_y()
    {
        return m_wrap.next_y();
    }
private:
    agg::image_accessor_wrap<Pixfmt, Wrap_X, Wrap_Y> m_wrap;
};


// gradient wrapper
class gradient_wrapper 
{
public:
    gradient_wrapper() {}
    virtual ~gradient_wrapper() {}
    virtual void init(float r, float x, float y) = 0;
    virtual int calculate(int x, int y, int) const = 0; 
};


template<typename GradientF, typename WapperF>
class gradient_wrapper_adaptor : public gradient_wrapper
{
public:
    gradient_wrapper_adaptor()
        : m_wrapper(m_gradient) {}

    virtual ~gradient_wrapper_adaptor() {}

    virtual void init(float r, float x, float y)
    {
        m_gradient.init(r, x, y);
    }

    virtual int calculate(int x, int y, int d) const 
    {
        return m_wrapper.calculate(x, y, d);
    }

private:
    GradientF m_gradient;
    WapperF m_wrapper;
};

}
#endif /*_PIXFMT_WRAPPER_H_*/
