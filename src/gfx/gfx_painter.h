/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_PAINTER_H_
#define _GFX_PAINTER_H_

#include "common.h"
#include "interfaces.h"

#include "picasso.h"
#include "picasso_painter.h"
#include "pixfmt_wrapper.h"
#include "painter_raster.h"
#include "gfx_rendering_buffer.h"
#include "gfx_raster_adapter.h"
#include "gfx_trans_affine.h"
#include "gfx_gradient_adapter.h"

namespace gfx {

inline agg::image_filter_lut * create_image_filter(int f)
{
    switch (f) {
        case FILTER_BILINEAR:
            return new agg::image_filter<agg::image_filter_bilinear>;
        case FILTER_GAUSSIAN:
            return new agg::image_filter<agg::image_filter_gaussian>;
        default:
            //FILTER_NEAREST:
            return 0;
    }
}

template <typename Pixfmt>
class gfx_painter : public abstract_painter
{
    typedef pixfmt_wrapper<Pixfmt, mask_type> pixfmt;
    typedef agg::renderer_pclip<pixfmt> renderer_base_type;
    typedef agg::renderer_scanline_aa_solid<renderer_base_type> renderer_solid_type;
    typedef agg::renderer_scanline_bin_solid<renderer_base_type> renderer_bin_type;
public:
    typedef enum {
        type_solid      = 0,
        type_image      = 1,
        type_pattern    = 2,
        type_gradient   = 3,
        type_canvas     = 4,
    } source_type;

    typedef struct {
        abstract_rendering_buffer* buffer;
        int filter;
        rect_s rect;
        agg::rgba8 key;
        bool transparent;
        bool colorkey;
    } image_holder;

    typedef struct {
        abstract_rendering_buffer* buffer;
        int filter;
        rect_s rect;
        int xtype;
        int ytype;
        abstract_trans_affine* matrix;
        bool transparent;
    } pattern_holder;

    typedef struct {
        abstract_gradient_adapter* gradient;
    } gradient_holder;

    gfx_painter() 
        : m_fill_type(type_solid)
        , m_draw_shadow(false)
        , m_shadow_area(0,0,0,0)
        , m_shadow_buffer(0)
    {
    }

    virtual ~gfx_painter() {}

    virtual void attach(abstract_rendering_buffer*); 
    virtual pix_fmt pixel_format(void) const;

    virtual void set_alpha(scalar a);
    virtual void set_composite(comp_op op);
    virtual void set_stroke_color(const rgba& c);
    virtual void set_fill_color(const rgba& c);
    virtual void set_fill_image(const abstract_rendering_buffer* img, int filter, const rect_s& rc);
    virtual void set_fill_canvas(const abstract_rendering_buffer* img, int filter, const rect_s& rc);
    virtual void set_fill_pattern(const abstract_rendering_buffer* img, int filter, const rect_s& rc, 
                                    int xtype, int ytype, const abstract_trans_affine* mtx);
    virtual void set_fill_gradient(const abstract_gradient_adapter* g);
    virtual void set_font_fill_color(const rgba& c);

    virtual void apply_stroke(abstract_raster_adapter* raster);
    virtual void apply_fill(abstract_raster_adapter* raster);
    virtual void apply_text_fill(abstract_raster_adapter* rs, int t);
    virtual void apply_mono_text_fill(void * storage);
    virtual void apply_clear(const rgba& c);
    virtual void apply_clip_path(const vertex_source& v, int rule, const abstract_trans_affine* mtx);
    virtual void apply_clip_device(const rect_s& rc, scalar xoffset, scalar yoffset);
    virtual void clear_clip(void);

    virtual void apply_masking(abstract_mask_layer*);
    virtual void clear_masking(void);

    virtual void apply_blur(scalar blur);

    virtual bool begin_shadow(const rect_s& rc);
    virtual void apply_shadow(abstract_raster_adapter* rs, const rect_s& r, 
                                                const rgba& c, scalar x, scalar y, scalar b);

    virtual void copy_rect_from(abstract_rendering_buffer* src, const rect& rc, int x, int y);
private:
    pattern_wrapper<pixfmt>* pattern_wrap(int xtype, int ytype, pixfmt& fmt)
    {
        pattern_wrapper<pixfmt>* p = 0;
        if ((xtype == WRAP_TYPE_REPEAT) && (ytype == WRAP_TYPE_REPEAT))
            p = new pattern_wrapper_adaptor<pixfmt, agg::wrap_mode_repeat, agg::wrap_mode_repeat>(fmt);
        else if ((xtype == WRAP_TYPE_REPEAT) && (ytype == WRAP_TYPE_REFLECT))
            p = new pattern_wrapper_adaptor<pixfmt, agg::wrap_mode_repeat, agg::wrap_mode_reflect>(fmt);
        else if ((xtype == WRAP_TYPE_REFLECT) && (ytype == WRAP_TYPE_REPEAT))
            p = new pattern_wrapper_adaptor<pixfmt, agg::wrap_mode_reflect, agg::wrap_mode_repeat>(fmt);
        else if ((xtype == WRAP_TYPE_REFLECT) && (ytype == WRAP_TYPE_REFLECT))
            p = new pattern_wrapper_adaptor<pixfmt, agg::wrap_mode_reflect, agg::wrap_mode_reflect>(fmt);

        return p;
    }
    //fill
    source_type        m_fill_type; 
    agg::rgba          m_fill_color;
    image_holder       m_image_source;
    pattern_holder     m_pattern_source;
    gradient_holder    m_gradient_source;
    //stroke
    agg::rgba          m_stroke_color; //FIXME: need stroke type feature.
    agg::rgba          m_font_fill_color; //FIXME: need stroke type feature.
    pixfmt             m_fmt;
    renderer_base_type m_rb;
    renderer_solid_type m_ren;
    //shadow
    bool               m_draw_shadow;
    rect_s             m_shadow_area;
    agg::int8u*        m_shadow_buffer;
    agg::rendering_buffer   m_shadow_rb;
    agg::pixfmt_rgba32      m_shadow_fmt;
    agg::renderer_pclip<agg::pixfmt_rgba32> m_shadow_base;
};

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::attach(abstract_rendering_buffer* buffer)
{
    if (buffer) {   
        m_fmt.attach(static_cast<gfx_rendering_buffer*>(buffer)->impl());
        m_rb.attach(m_fmt);
        m_ren.attach(m_rb);
    }
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::set_alpha(scalar a)
{
    m_fmt.alpha(a);
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::set_composite(comp_op op)
{
    m_fmt.comp_op((agg::comp_op_e)op);
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::set_fill_color(const rgba& c)
{
    m_fill_type = type_solid;
    m_fill_color = agg::rgba(c);
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::set_font_fill_color(const rgba& c)
{
    m_font_fill_color = agg::rgba(c);
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::set_stroke_color(const rgba& c)
{
    m_stroke_color = agg::rgba(c);
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::set_fill_image(const abstract_rendering_buffer* img, int filter, const rect_s& rc)
{
    m_fill_type = type_image;
    m_image_source.buffer = const_cast<abstract_rendering_buffer*>(img);
    m_image_source.filter = filter;
    m_image_source.rect = rc;
    m_image_source.key = agg::rgba8(img->get_color_channel());
    m_image_source.transparent = img->is_transparent();
    m_image_source.colorkey = img->has_color_channel();
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::set_fill_canvas(const abstract_rendering_buffer* img, int filter, const rect_s& rc)
{
    m_fill_type = type_canvas;
    m_image_source.buffer = const_cast<abstract_rendering_buffer*>(img);
    m_image_source.filter = filter;
    m_image_source.rect = rc;
    m_image_source.key = agg::rgba8(0,0,0,0);
    m_image_source.transparent = true; // canvas default transpaent.
    m_image_source.colorkey = false;
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::set_fill_pattern(const abstract_rendering_buffer* img, int filter, const rect_s& rc,
                                        int xtype, int ytype, const abstract_trans_affine* mtx)
{
    m_fill_type = type_pattern;
    m_pattern_source.buffer = const_cast<abstract_rendering_buffer*>(img);
    m_pattern_source.filter = filter;
    m_pattern_source.rect = rc;
    m_pattern_source.xtype = xtype;
    m_pattern_source.ytype = ytype;
    m_pattern_source.matrix = const_cast<abstract_trans_affine*>(mtx);
    m_pattern_source.transparent = img->is_transparent();
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::set_fill_gradient(const abstract_gradient_adapter* g)
{
    m_fill_type = type_gradient;
    m_gradient_source.gradient = const_cast<abstract_gradient_adapter*>(g);
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::apply_stroke(abstract_raster_adapter* raster)
{
    if (raster) {
        m_ren.color(m_stroke_color);
        agg::scanline_p8 scanline;
        agg::render_scanlines(static_cast<gfx_raster_adapter*>(raster)->stroke_impl(), scanline, m_ren);
    }
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::apply_mono_text_fill(void * storage)
{
    agg::serialized_scanlines_adaptor_bin* storage_bin = (agg::serialized_scanlines_adaptor_bin*)storage;
    renderer_bin_type ren_solid(m_rb);
    ren_solid.color(m_font_fill_color);
    agg::serialized_scanlines_adaptor_bin::embedded_scanline sl;
    agg::render_scanlines(*storage_bin, sl, ren_solid);
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::apply_text_fill(abstract_raster_adapter* raster, int render_type)
{
    if (raster) {
        switch (render_type) {
            case TEXT_SMOOTH:
                {
                    renderer_solid_type ren_solid(m_rb);
                    ren_solid.color(m_font_fill_color);
                    agg::scanline_u8 sl;
                    agg::render_scanlines(static_cast<gfx_raster_adapter*>(raster)->fill_impl(), sl, ren_solid);
                }
                break;
            case TEXT_MONO:
                {
                    renderer_bin_type ren_solid(m_rb);
                    ren_solid.color(m_font_fill_color);
                    agg::scanline_bin sl;
                    agg::render_scanlines(static_cast<gfx_raster_adapter*>(raster)->fill_impl(), sl, ren_solid);
                }
                break;
            case TEXT_STROKE:
                {
                    renderer_solid_type ren_solid(m_rb);
                    ren_solid.color(m_font_fill_color);
                    agg::scanline_p8 sl;
                    agg::render_scanlines(static_cast<gfx_raster_adapter*>(raster)->fill_impl(), sl, ren_solid);
                }
                break;
        }
    }
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::apply_fill(abstract_raster_adapter* raster)
{
    if (raster) {
        switch (m_fill_type) {
        case type_canvas:
            {
                agg::span_allocator<agg::rgba8> sa;
                agg::scanline_u8 scanline;

                pixfmt canvas_fmt(static_cast<gfx_rendering_buffer*>(m_image_source.buffer)->impl());

                rect_s dr = m_image_source.rect;
                agg::trans_affine mtx;
                mtx *= agg::trans_affine_translation((float)agg::iround(dr.x()), (float)agg::iround(dr.y()));
                mtx *= stable_matrix(static_cast<gfx_raster_adapter*>(raster)->transformation());
                mtx.invert();

                agg::span_interpolator_linear<> interpolator(mtx);

                typename painter_raster<Pixfmt>::image_source_type img_src(canvas_fmt);

                if (m_image_source.filter) {
                    agg::image_filter_lut * filter = create_image_filter(m_image_source.filter);

                    typename painter_raster<Pixfmt>::span_canvas_filter_type
                        sg(img_src, interpolator, *(filter));
                    agg::render_scanlines_aa(static_cast<gfx_raster_adapter*>(raster)->fill_impl(),
                            scanline, m_rb, sa, sg);

                    if (filter) delete filter;
                } else {
                    typename painter_raster<Pixfmt>::span_canvas_filter_type_nn
                        sg(img_src, interpolator);
                    agg::render_scanlines_aa(static_cast<gfx_raster_adapter*>(raster)->fill_impl(),
                            scanline, m_rb, sa, sg);
                }
            }
            break;
        case type_image:
            {
                agg::span_allocator<agg::rgba8> sa;
                agg::scanline_u8 scanline;

                pixfmt img_fmt(static_cast<gfx_rendering_buffer*>(m_image_source.buffer)->impl());

                if (m_image_source.colorkey)
                    m_fmt.set_transparent_color(&m_image_source.key);

                rect_s dr = m_image_source.rect;
                bool transparent = m_image_source.transparent;

                scalar xs = (scalar)dr.width() / m_image_source.buffer->width();
                scalar ys = (scalar)dr.height() / m_image_source.buffer->height();

                agg::trans_affine mtx; 
                mtx *= agg::trans_affine_scaling(xs, ys);
                mtx *= agg::trans_affine_translation((float)agg::iround(dr.x()), (float)agg::iround(dr.y()));
                mtx *= stable_matrix(static_cast<gfx_raster_adapter*>(raster)->transformation());
                mtx.invert();

                agg::span_interpolator_linear<> interpolator(mtx);

                typename painter_raster<Pixfmt>::image_source_type img_src(img_fmt);

                if (m_image_source.filter) {
                    agg::image_filter_lut * filter = create_image_filter(m_image_source.filter);

                    if (transparent) {
                        typename painter_raster<Pixfmt>::span_canvas_filter_type
                                                sg(img_src, interpolator, *(filter));
                        agg::render_scanlines_aa(static_cast<gfx_raster_adapter*>(raster)->fill_impl(),
                                                scanline, m_rb, sa, sg);
                    } else {
                        typename painter_raster<Pixfmt>::span_image_filter_type
                                                sg(img_src, interpolator, *(filter));
                        agg::render_scanlines_aa(static_cast<gfx_raster_adapter*>(raster)->fill_impl(),
                                                scanline, m_rb, sa, sg);
                    }

                    if (filter) delete filter;
                } else {
                    if (transparent) {
                        typename painter_raster<Pixfmt>::span_canvas_filter_type_nn
                                                sg(img_src, interpolator);
                        agg::render_scanlines_aa(static_cast<gfx_raster_adapter*>(raster)->fill_impl(),
                                                scanline, m_rb, sa, sg);
                    } else {
                        typename painter_raster<Pixfmt>::span_image_filter_type_nn
                                                sg(img_src, interpolator);
                        agg::render_scanlines_aa(static_cast<gfx_raster_adapter*>(raster)->fill_impl(), 
                                                scanline, m_rb, sa, sg);
                    }
                }

                m_fmt.clear_key();
            }
            break;
        case type_pattern:
            {
                agg::span_allocator<agg::rgba8> sa;
                agg::scanline_u8 scanline;

                pixfmt pattern_fmt(static_cast<gfx_rendering_buffer*>(m_pattern_source.buffer)->impl());

                rect_s dr = m_pattern_source.rect;
                bool transparent = m_pattern_source.transparent;
                agg::trans_affine mtx;
                mtx = static_cast<gfx_trans_affine*>(m_pattern_source.matrix)->impl();
                mtx *= agg::trans_affine_translation((float)agg::iround(dr.x()), (float)agg::iround(dr.y()));
                mtx *= stable_matrix(static_cast<gfx_raster_adapter*>(raster)->transformation());
                mtx.invert();

                agg::span_interpolator_linear<> interpolator(mtx);

                pattern_wrapper<pixfmt>* pattern = 
                            pattern_wrap(m_pattern_source.xtype, m_pattern_source.ytype, pattern_fmt);

                if (m_pattern_source.filter) {
                    agg::image_filter_lut * filter = create_image_filter(m_pattern_source.filter);

                    if (transparent) {
                        typename painter_raster<Pixfmt>::span_canvas_pattern_type 
                                                sg(*pattern, interpolator, *(filter));
                        agg::render_scanlines_aa(static_cast<gfx_raster_adapter*>(raster)->fill_impl(), 
                                                scanline, m_rb, sa, sg);
                    } else {
                        typename painter_raster<Pixfmt>::span_image_pattern_type 
                                                sg(*pattern, interpolator, *(filter));
                        agg::render_scanlines_aa(static_cast<gfx_raster_adapter*>(raster)->fill_impl(), 
                                                scanline, m_rb, sa, sg);
                    }

                    if (filter) delete filter;
                } else {
                    if (transparent) {
                        typename painter_raster<Pixfmt>::span_canvas_pattern_type_nn 
                                                sg(*pattern, interpolator);
                        agg::render_scanlines_aa(static_cast<gfx_raster_adapter*>(raster)->fill_impl(), 
                                                scanline, m_rb, sa, sg);
                    } else {
                        typename painter_raster<Pixfmt>::span_image_pattern_type_nn 
                                                sg(*pattern, interpolator);
                        agg::render_scanlines_aa(static_cast<gfx_raster_adapter*>(raster)->fill_impl(), 
                                                scanline, m_rb, sa, sg);
                    }
                }

                delete pattern;
            }
            break;
        case type_gradient:
            {
                agg::span_allocator<agg::rgba8> sa;
                agg::scanline_u8 scanline;

                gfx_gradient_adapter* gradient = static_cast<gfx_gradient_adapter*>(m_gradient_source.gradient);
                gradient->build();

                agg::trans_affine mtx;
                mtx = gradient->matrix();
                mtx *= stable_matrix(static_cast<gfx_raster_adapter*>(raster)->transformation());
                mtx.invert();

                agg::span_interpolator_linear<> inter(mtx);

                gradient_wrapper* pwr = gradient->wrapper();
                float len = gradient->length();
                float st = gradient->start();

                agg::span_gradient<agg::rgba8, agg::span_interpolator_linear<>, gradient_wrapper, 
                agg::gradient_lut<agg::color_interpolator<agg::rgba8> > > 
                                                    sg(inter, *pwr, gradient->colors(), st, len);

                agg::render_scanlines_aa(static_cast<gfx_raster_adapter*>(raster)->fill_impl(), 
                                         scanline, m_rb, sa, sg);
            }
            break;
        case type_solid: // solid fill default.
        default:
            {   
                m_ren.color(m_fill_color);
                agg::scanline_p8 scanline;
                agg::render_scanlines(static_cast<gfx_raster_adapter*>(raster)->fill_impl(), scanline, m_ren);
            }
        }
    }
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::apply_clip_path(const vertex_source& v, int rule, const abstract_trans_affine* mtx)
{
    abstract_trans_affine* cm = const_cast<abstract_trans_affine*>(mtx); 
    agg::trans_affine & m = static_cast<gfx_trans_affine*>(cm)->impl();
    agg::conv_transform<vertex_source> p(const_cast<vertex_source&>(v), m);

    if (m_draw_shadow) { //in shadow draw context.
        m_shadow_base.add_clipping(p, (agg::filling_rule_e)rule);
    } else {
        m_rb.add_clipping(p, (agg::filling_rule_e)rule);
    }
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::apply_clip_device(const rect_s& rc, scalar xoffset, scalar yoffset)
{
    rect r(agg::iround(rc.x1+xoffset), agg::iround(rc.y1+yoffset), 
                                agg::iround(rc.x2+xoffset), agg::iround(rc.y2+yoffset));

    if (m_draw_shadow) { //in shadow draw context.
        m_shadow_base.clip_box(r.x1, r.y1, r.x2, r.y2);    
    } else {
        m_rb.clip_box(r.x1, r.y1, r.x2, r.y2);        
    }
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::apply_clear(const rgba& c)
{
    m_rb.clear(agg::rgba(c));
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::apply_blur(scalar blur)
{
    if (blur > 0) {
        m_fmt.alpha(1.0f);
        m_fmt.comp_op(comp_op_src_over);
        agg::stack_blur<agg::rgba8, agg::stack_blur_calc_rgba<> > b;
        b.blur(m_fmt, agg::uround(SCALAR_TO_FLT(blur) * 40.0f));
    }
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::clear_clip(void)
{
    if (m_draw_shadow) //in shadow draw context.
        m_shadow_base.reset_clipping(true);
    else
        m_rb.reset_clipping(true);
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::apply_masking(abstract_mask_layer* m)
{
    m_fmt.attach_mask(static_cast<gfx_mask_layer*>(m));
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::clear_masking(void)
{
    m_fmt.clear_mask();
}

template<typename Pixfmt> 
inline bool gfx_painter<Pixfmt>::begin_shadow(const rect_s& rc)
{
    m_draw_shadow = true;
    m_shadow_area = rc;

    unsigned int w = agg::uround(rc.x2 - rc.x1);
    unsigned int h = agg::uround(rc.y2 - rc.y1);
    m_shadow_buffer = (agg::int8u*)mem_calloc(1, h * w * 4);

    if (!m_shadow_buffer) {
        m_draw_shadow = false;
        return false;
    }

    m_shadow_rb.attach(m_shadow_buffer, w, h, w * 4);
    m_shadow_fmt.attach(m_shadow_rb);
    m_shadow_base.attach(m_shadow_fmt);

    return true;
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::apply_shadow(abstract_raster_adapter* rs, 
                                            const rect_s& r, const rgba& c, scalar x, scalar y, scalar blur)
{
    gfx_raster_adapter* ras = static_cast<gfx_raster_adapter*>(rs);

    agg::renderer_scanline_aa_solid<agg::renderer_pclip<agg::pixfmt_rgba32> > ren(m_shadow_base);
    ren.color(agg::rgba(c));

    agg::scanline_p8 scanline;

    if (ras->raster_method() & raster_fill) {
        agg::render_scanlines(ras->fill_impl(), scanline, ren);
    }

    if (ras->raster_method() & raster_stroke) {
        agg::render_scanlines(ras->stroke_impl(), scanline, ren);
    }

    if (blur > FLT_TO_SCALAR(0.0f)) {
        agg::stack_blur<agg::rgba8, agg::stack_blur_calc_rgba<> > b;
        b.set_shading(agg::rgba8(agg::rgba(c)));
        b.blur(m_shadow_fmt, agg::uround(SCALAR_TO_FLT(blur) * 40.0f));
    }

    //Note: shadow need a no clip render base.
    renderer_base_type rb(m_fmt); 
    //blend shadow layer to base.
    rb.blend_from(m_shadow_fmt, 0, agg::iround(SCALAR_TO_FLT(x+r.x1)), agg::iround(SCALAR_TO_FLT(y+r.y1)));

    if (m_shadow_buffer) {
        mem_free(m_shadow_buffer);
        m_shadow_buffer = 0;
    }
    m_draw_shadow = false;
}

template<typename Pixfmt> 
inline void gfx_painter<Pixfmt>::copy_rect_from(abstract_rendering_buffer* src, const rect& rc, int x, int y)
{
    agg::rect_i r(rc.x1, rc.y1, rc.x2, rc.y2);
    agg::rendering_buffer& buffer = static_cast<gfx_rendering_buffer*>(src)->impl();
    m_rb.copy_absolute_from(buffer, &r, x, y);
}

#if ENABLE(FORMAT_RGBA)
template<> 
inline pix_fmt gfx_painter<agg::pixfmt_rgba32>::pixel_format(void) const
{
    return pix_fmt_rgba;
}
#endif

#if ENABLE(FORMAT_ARGB)
template<> 
inline pix_fmt gfx_painter<agg::pixfmt_argb32>::pixel_format(void) const
{
    return pix_fmt_argb;
}
#endif

#if ENABLE(FORMAT_ABGR)
template<> 
inline pix_fmt gfx_painter<agg::pixfmt_abgr32>::pixel_format(void) const
{
    return pix_fmt_abgr;
}
#endif

#if ENABLE(FORMAT_BGRA)
template<> 
inline pix_fmt gfx_painter<agg::pixfmt_bgra32>::pixel_format(void) const
{
    return pix_fmt_bgra;
}
#endif

#if ENABLE(FORMAT_RGB)
template<> 
inline pix_fmt gfx_painter<agg::pixfmt_rgb24>::pixel_format(void) const
{
    return pix_fmt_rgb;
}
#endif

#if ENABLE(FORMAT_BGR)
template<> 
inline pix_fmt gfx_painter<agg::pixfmt_bgr24>::pixel_format(void) const
{
    return pix_fmt_bgr;
}
#endif

#if ENABLE(FORMAT_RGB565)
template<> 
inline pix_fmt gfx_painter<agg::pixfmt_rgb565>::pixel_format(void) const
{
    return pix_fmt_rgb565;
}
#endif

#if ENABLE(FORMAT_RGB555)
template<> 
inline pix_fmt gfx_painter<agg::pixfmt_rgb555>::pixel_format(void) const
{
    return pix_fmt_rgb555;
}
#endif

}
#endif /*_GFX_PAINTER_H_*/

