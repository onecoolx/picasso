/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PICASSO_PAINTER_H_
#define _PICASSO_PAINTER_H_

#include "aggheader.h"
#include "picasso.h"
#include "picasso_p.h"
#include "picasso_wrapper.h"
#include "picasso_helper.h"

namespace picasso {

inline image_filter_lut * create_image_filter(ps_filter f)
{
	switch (f) {
		case FILTER_BILINEAR:
			return new image_filter<image_filter_bilinear>;
		case FILTER_GAUSSIAN:
			return new image_filter<image_filter_gaussian>;
		default:
			//FILTER_NEAREST:
			return 0;
	}
}

//graphic painter
template<typename Pixfmt>
class graphic_painter : public painter {
    typedef pixfmt_wrapper<Pixfmt, mask_type> pixfmt;
    typedef renderer_pclip<pixfmt> renderer_base_type;
    typedef renderer_scanline_aa_solid<renderer_base_type> renderer_solid_type;
    typedef renderer_scanline_bin_solid<renderer_base_type> renderer_bin_type;
public:
    graphic_painter() {}
    virtual ~graphic_painter() {}
	virtual void attach(rendering_buffer&);

    virtual void render_stroke(ps_context*, path_storage&); 
    virtual void render_fill(ps_context*, path_storage&); 
	virtual void render_shadow(ps_context*, path_storage&, bool, bool);
	virtual void render_clip(ps_context*, bool);
	virtual void render_clear(ps_context*);
	virtual void render_blur(ps_context*);
	virtual void render_gamma(ps_context*);

	virtual void render_mask(ps_canvas*, bool);
	virtual void render_copy(ps_canvas*, rect_i*, ps_canvas*, int, int);
	virtual void render_font_cache(ps_context*, font_cache_manager<font_engine>&, glyph_data_type);
	virtual void render_font_raster(ps_context*);
private:
	trans_affine get_stable_matrix(trans_affine&, path_storage&);
	void create_stroke_data(ps_context*, path_storage&, float, float);
	void create_fill_data(ps_context*, path_storage&, float, float);
	pattern_wrapper<pixfmt>* get_pattern_wrap(ps_context* ctx, pixfmt& fmt)
	{
		pattern_wrapper<pixfmt>* p = 0;
		ps_pattern* pt = static_cast<ps_pattern*>(ctx->state->brush.data);
		if ((pt->xtype == WRAP_TYPE_REPEAT) && (pt->ytype == WRAP_TYPE_REPEAT))
			p = new pattern_wrapper_adaptor<pixfmt, wrap_mode_repeat, wrap_mode_repeat>(fmt);
		else if ((pt->xtype == WRAP_TYPE_REPEAT) && (pt->ytype == WRAP_TYPE_REFLECT))
			p = new pattern_wrapper_adaptor<pixfmt, wrap_mode_repeat, wrap_mode_reflect>(fmt);
		else if ((pt->xtype == WRAP_TYPE_REFLECT) && (pt->ytype == WRAP_TYPE_REPEAT))
			p = new pattern_wrapper_adaptor<pixfmt, wrap_mode_reflect, wrap_mode_repeat>(fmt);
		else if ((pt->xtype == WRAP_TYPE_REFLECT) && (pt->ytype == WRAP_TYPE_REFLECT))
			p = new pattern_wrapper_adaptor<pixfmt, wrap_mode_reflect, wrap_mode_reflect>(fmt);

		return p;
	}

	template<typename RenBase>
	void create_clip_data(ps_context* ctx, RenBase& rb, float x=0, float y=0) /*dx, dy only for shadow*/
	{
		if (ctx->state->clip.needclip) {
			rb.reset_clipping(true);
			if (!ctx->state->clip.cliprect) {
				trans_affine mtx = ctx->state->world_matrix;
				mtx *= trans_affine_translation((float)iround(x), (float)iround(y));
				conv_transform<path_storage> p(ctx->state->clip.path, mtx);
				rb.add_clipping(p, ctx->state->clip.rule);
			} else {
				//FIXME: for fast clip rect function
				float dx = float(ctx->state->clip.fs_rect.x);
				float dy = float(ctx->state->clip.fs_rect.y);
				float dx1 = float(ctx->state->clip.fs_rect.x + ctx->state->clip.fs_rect.w);
				float dy1 = float(ctx->state->clip.fs_rect.y + ctx->state->clip.fs_rect.h);

				trans_affine mtx;
				mtx *= trans_affine_translation((float)iround(x), (float)iround(y));
				mtx.transform(&dx, &dy);
				mtx.transform(&dx1, &dy1);

				int x = iround(dx);
				int y = iround(dy);
				int x1 = iround(dx1);
				int y1 = iround(dy1);
				rb.clip_box(x, y, x1, y1);		
			}
		}
	}

private:
	pixfmt			   m_fmt;
	renderer_base_type m_rb;
};

template<typename Pixfmt> 
inline void graphic_painter<Pixfmt>::attach(rendering_buffer& buffer)
{
	m_fmt.attach(buffer);
	m_rb.attach(m_fmt);
}

template<typename Pixfmt> 
inline trans_affine graphic_painter<Pixfmt>::get_stable_matrix(trans_affine& m, path_storage& p)
{
	trans_affine mtx;
	if (p.is_rect() && is_boxer(m.rotation())) {
		int tx = (int)floor(m.tx);
		int ty = (int)floor(m.ty);
		mtx = trans_affine(m.sx, m.shy, m.shx, m.sy, (float)tx, (float)ty);
	} else  {
		mtx = m;
	}
	return mtx;
}

template<typename Pixfmt> 
void inline graphic_painter<Pixfmt>::create_fill_data(ps_context* ctx, path_storage& path, float xoffset, float yoffset)
{
	trans_affine tmtx = get_stable_matrix(ctx->state->world_matrix, path);
	tmtx *= trans_affine_translation((float)iround(xoffset), (float)iround(yoffset));
	conv_transform<path_storage> w(path, tmtx);

	ctx->raster.add_path(w);
}

template<typename Pixfmt> 
void inline graphic_painter<Pixfmt>::create_stroke_data(ps_context* ctx, path_storage& path, float xoffset, float yoffset)
{
    if (ctx->state->pen.style == pen_style_dash) {
        conv_dash<path_storage> d(path);

        for (unsigned i = 0; i < ctx->state->pen.ndashes; i+=2)
            d.add_dash((float)ctx->state->pen.dashes[i], (float)ctx->state->pen.dashes[i+1]);

        d.dash_start((float)ctx->state->pen.dstart);

        conv_stroke<conv_dash<path_storage> > p(d);

        p.width(ctx->state->pen.width);
        p.line_cap(ctx->state->pen.cap);
        p.line_join(ctx->state->pen.join);
        p.inner_join(ctx->state->pen.inner);
        p.miter_limit(ctx->state->pen.miter_limit);

		trans_affine tmtx = get_stable_matrix(ctx->state->world_matrix, path);
		tmtx *= trans_affine_translation(0.5f, 0.5f); //adjust edge
		tmtx *= trans_affine_translation((float)iround(xoffset), (float)iround(yoffset));
        conv_transform<conv_stroke<conv_dash<path_storage> > > w(p, tmtx);

        ctx->raster.add_path(w);
    } else {
		conv_curve<path_storage> c(path);

        conv_stroke<conv_curve<path_storage> > p(c);

        p.width(ctx->state->pen.width);
        p.line_cap(ctx->state->pen.cap);
        p.line_join(ctx->state->pen.join);
        p.inner_join(ctx->state->pen.inner);
        p.miter_limit(ctx->state->pen.miter_limit);

		trans_affine tmtx = get_stable_matrix(ctx->state->world_matrix, path);
		tmtx *= trans_affine_translation(0.5f, 0.5f); //adjust edge
		tmtx *= trans_affine_translation((float)iround(xoffset), (float)iround(yoffset));
        conv_transform<conv_stroke<conv_curve<path_storage> > > w(p, tmtx);

        ctx->raster.add_path(w);
    }
}

template<typename Pixfmt> 
inline void graphic_painter<Pixfmt>::render_copy(ps_canvas* s, rect_i* r, ps_canvas* d, int x, int y)
{
	graphic_painter<Pixfmt>* dst = static_cast<graphic_painter<Pixfmt>*>(d->p);
	dst->m_rb.copy_absolute_from(s->buffer, r, x, y);
}

template<typename Pixfmt> 
inline void graphic_painter<Pixfmt>::render_mask(ps_canvas* canvas, bool mask)
{
	if (mask) {
		m_fmt.attach_mask(canvas->mask);
	} else {
		m_fmt.clear_mask();
	}
}

template<typename Pixfmt> 
inline void graphic_painter<Pixfmt>::render_clip(ps_context* ctx, bool clip)
{
	if (clip) {
		create_clip_data(ctx, m_rb);
	} else {
		m_rb.reset_clipping(true);
	}
}

template<typename Pixfmt> 
void inline graphic_painter<Pixfmt>::render_shadow(ps_context* ctx, path_storage& path, bool fill, bool stroke)
{
	if (ctx->state->shadow.use_shadow) {
		pixfmt fmt(ctx->canvas->buffer);
		renderer_base_type rb(fmt); //Note: shadow need a no clip render base.

		fmt.alpha(ctx->state->alpha);
		fmt.comp_op(ctx->state->composite);

		rect_d bbox;
		trans_affine tmtx = get_stable_matrix(ctx->state->world_matrix, path);
		conv_transform<path_storage> tp(path, tmtx);
		bounding_rect_single(tp, 0, &bbox.x1, &bbox.y1, &bbox.x2, &bbox.y2);

		bbox.x1 -= (ctx->state->shadow.blur*40+5);
		bbox.y1 -= (ctx->state->shadow.blur*40+5);
		bbox.x2 += (ctx->state->shadow.blur*40+5);
		bbox.y2 += (ctx->state->shadow.blur*40+5);

		bool alloc_buffer = true;
		unsigned w = uround(bbox.x2-bbox.x1);
		unsigned h = uround(bbox.y2-bbox.y1);
		int8u * buf = 0;
		if (!(buf = (int8u*)BuffersAlloc(1, h*w*4))) {
			alloc_buffer = false;
	   		buf	= (int8u*)calloc(1, h*w*4);
		}

		if (buf) {
			rendering_buffer buffer(buf, w, h, w*4);
			pixfmt_rgba32 shadow_fmt(buffer);
			renderer_pclip<pixfmt_rgba32> base(shadow_fmt);

			renderer_scanline_aa_solid<renderer_pclip<pixfmt_rgba32> > ren(base);
			ren.color(ctx->state->shadow.color);

			create_clip_data(ctx, base, -bbox.x1, -bbox.y1);
			scanline_p8 scanline;

			if (fill) {
				create_fill_data(ctx, path, -bbox.x1, -bbox.y1);
				render_scanlines(ctx->raster, scanline, ren);
			}

			if (stroke) {
				create_stroke_data(ctx, path, -bbox.x1, -bbox.y1);
				render_scanlines(ctx->raster, scanline, ren);
			}
			ctx->raster.reset();

			if (ctx->state->shadow.blur > 0.0) {
				stack_blur<rgba8, stack_blur_calc_rgba<> > b;
				b.set_shading(rgba8(ctx->state->shadow.color));
				b.blur(shadow_fmt, uround(ctx->state->shadow.blur*40));
			}

			//copy shadow to base from layer
			rb.blend_from(shadow_fmt, 0, iround(ctx->state->shadow.x_offset+bbox.x1), 
										   iround(ctx->state->shadow.y_offset+bbox.y1));
			if (alloc_buffer)
				BufferFree(buf);
			else
				free(buf);
		}

	}
}

template<typename Pixfmt> 
void inline graphic_painter<Pixfmt>::render_gamma(ps_context* ctx)
{
	if (ctx->state->antialias) {
		ctx->raster.gamma(gamma_power(ctx->state->gamma));
	} else {
		ctx->raster.gamma(gamma_threshold(0.5));
	}
}

template<typename Pixfmt> 
void inline graphic_painter<Pixfmt>::render_blur(ps_context* ctx)
{
	if (ctx->state->blur > 0) {
		m_fmt.alpha(1.0);
		m_fmt.comp_op(COMPOSITE_SRC_OVER);

		stack_blur<rgba8, stack_blur_calc_rgba<> > b;
		b.blur(m_fmt, uround(ctx->state->blur*40));
	}
}

template<typename Pixfmt> 
void inline graphic_painter<Pixfmt>::render_clear(ps_context* ctx)
{
	m_rb.clear(ctx->state->brush.color);
}

template<typename Pixfmt> 
void inline graphic_painter<Pixfmt>::render_font_cache(ps_context* ctx, 
											font_cache_manager<font_engine>& f, glyph_data_type t)
{
	if (t == glyph_data_mono) { //for embeded bitmap font.
		m_fmt.alpha(ctx->state->alpha);
		m_fmt.comp_op(ctx->state->composite);

		renderer_bin_type ren_solid(m_rb);
		ren_solid.color(ctx->state->font_fcolor);
		float tx = 0, ty = 0;
		ctx->state->world_matrix.translation(&tx, &ty);
		float x = f.mono_adaptor().x()+tx;
		float y = f.mono_adaptor().y()+ty;
		f.mono_adaptor().setX((int)x);
		f.mono_adaptor().setY((int)y);
		render_scanlines(f.mono_adaptor(), f.mono_scanline(), ren_solid);
	} else {
		typedef font_cache_manager<font_engine> font_cache_type;
		typedef conv_curve<font_cache_type::path_adaptor_type> conv_curve_type;

		conv_curve_type curve(f.path_adaptor());

#if defined(WINCE) 
		float tx = 0, ty = 0;
		ctx->state->world_matrix.translation(&tx, &ty);
		float x = f.path_adaptor().x()+tx;
		float y = f.path_adaptor().y()+ty;
		f.path_adaptor().setX((int)x);
		f.path_adaptor().setY((int)y);
		ctx->raster.add_path(curve);
#else
		conv_transform<conv_curve_type> w(curve, ctx->state->world_matrix);
		ctx->raster.add_path(w);
#endif
	}
}

template<typename Pixfmt> 
void inline graphic_painter<Pixfmt>::render_font_raster(ps_context* ctx)
{
	if (!ctx->raster.initial()) {
		m_fmt.alpha(ctx->state->alpha);
		m_fmt.comp_op(ctx->state->composite);
		switch (ctx->state->font->type) 
		{
			case FONT_TYPE_SMOOTH:
				{
					renderer_solid_type ren_solid(m_rb);
					ren_solid.color(ctx->state->font_fcolor);
					scanline_u8 sl;
					render_scanlines(ctx->raster, sl, ren_solid);
				}
				break;
			case FONT_TYPE_MONO:
				{
					renderer_bin_type ren_solid(m_rb);
					ren_solid.color(ctx->state->font_fcolor);
					scanline_bin sl;
					render_scanlines(ctx->raster, sl, ren_solid);
				}
				break;
			case FONT_TYPE_STROKE:
				{
					renderer_solid_type ren_solid(m_rb);
					ren_solid.color(ctx->state->font_fcolor);
					scanline_p8 sl;
					render_scanlines(ctx->raster, sl, ren_solid);
				}
				break;
		}
		ctx->raster.reset();
	}
}

template<typename Pixfmt> 
void inline graphic_painter<Pixfmt>::render_stroke(ps_context* ctx, path_storage& path) 
{
	create_stroke_data(ctx, path, 0, 0);

	m_fmt.alpha(ctx->state->alpha);
	m_fmt.comp_op(ctx->state->composite);
    renderer_solid_type ren(m_rb);

    scanline_p8 scanline;

    ren.color(ctx->state->pen.color);
    render_scanlines(ctx->raster, scanline, ren);
}

template<typename Pixfmt> 
void inline graphic_painter<Pixfmt>::render_fill(ps_context* ctx, path_storage& path) 
{
    ctx->raster.filling_rule(ctx->state->brush.rule);
	if (ctx->state->brush.style) {
		span_allocator<rgba8> sa;
    	scanline_u8 scanline;

		m_fmt.alpha(ctx->state->alpha);
		m_fmt.comp_op(ctx->state->composite);

		if (ctx->state->brush.style == brush_style_gradient) {

			if (!(static_cast<ps_gradient*>(ctx->state->brush.data)->build)) {
				static_cast<ps_gradient*>(ctx->state->brush.data)->colors.build_lut();
				static_cast<ps_gradient*>(ctx->state->brush.data)->build = true;
			}

			create_fill_data(ctx, path, 0, 0);

			trans_affine mtx;
			mtx = static_cast<ps_gradient*>(ctx->state->brush.data)->matrix;
			mtx *= get_stable_matrix(ctx->state->world_matrix, path);
			mtx.invert();

			span_interpolator_linear<> inter(mtx);

			gradient_wrapper* pwr = static_cast<ps_gradient*>(ctx->state->brush.data)->wrapper;
			int32u len = uround(static_cast<ps_gradient*>(ctx->state->brush.data)->length);
			int32u st = uround(static_cast<ps_gradient*>(ctx->state->brush.data)->start);

			span_gradient<rgba8, span_interpolator_linear<>, gradient_wrapper, 
				gradient_lut<color_interpolator<rgba8> > > 
					sg(inter, *pwr, static_cast<ps_gradient*>(ctx->state->brush.data)->colors, (float)st, (float)len);

			render_scanlines_aa(ctx->raster, scanline, m_rb, sa, sg); //gradient raster

		} else {

			ps_rect dr = _path_bounding_rect(path);
			image_filter_lut * filter = create_image_filter(ctx->state->filter);

			if (ctx->state->brush.style == brush_style_image) { //image fill
				ps_image * pimg = static_cast<ps_image*>(ctx->state->brush.data);
				pixfmt img_fmt(pimg->buffer);

				if (pimg->colorkey)
					m_fmt.set_transparent_color(&pimg->key);

				create_fill_data(ctx, path, 0, 0);

				float xs = (float)dr.w / pimg->buffer.width();
				float ys = (float)dr.h / pimg->buffer.height();

				trans_affine mtx; 
				mtx *= trans_affine_scaling(xs, ys);
				mtx *= trans_affine_translation((float)iround((float)dr.x), (float)iround((float)dr.y));
				mtx *= get_stable_matrix(ctx->state->world_matrix, path);
				mtx.invert();

				span_interpolator_linear<> interpolator(mtx);

				typename picasso_painter_helper<Pixfmt>::image_source_type img_src(img_fmt);

				if (filter) {
					if (pimg->transparent) {
						typename picasso_painter_helper<Pixfmt>::span_canvas_filter_type
							sg(img_src, interpolator, *(filter));
						render_scanlines_aa(ctx->raster, scanline, m_rb, sa, sg); //image raster
					} else {
						typename picasso_painter_helper<Pixfmt>::span_image_filter_type
							sg(img_src, interpolator, *(filter));
						render_scanlines_aa(ctx->raster, scanline, m_rb, sa, sg); //image raster
					}
				} else {
					if (pimg->transparent) {
						typename picasso_painter_helper<Pixfmt>::span_canvas_filter_type_nn
							sg(img_src, interpolator);
						render_scanlines_aa(ctx->raster, scanline, m_rb, sa, sg); //image raster
					} else {
						typename picasso_painter_helper<Pixfmt>::span_image_filter_type_nn
							sg(img_src, interpolator);
						render_scanlines_aa(ctx->raster, scanline, m_rb, sa, sg); //image raster
					}
				}

				m_fmt.clear_key();
			} else if (ctx->state->brush.style == brush_style_pattern) { //pattern fill
				pixfmt pattern_fmt(static_cast<ps_pattern*>(ctx->state->brush.data)->img->buffer);
				ps_image* pimg = static_cast<ps_pattern*>(ctx->state->brush.data)->img;

				create_fill_data(ctx, path, 0, 0);

				trans_affine mtx;
				mtx = static_cast<ps_pattern*>(ctx->state->brush.data)->matrix;
				mtx *= trans_affine_translation((float)iround((float)dr.x), (float)iround((float)dr.y));
				mtx *= get_stable_matrix(ctx->state->world_matrix, path);
				mtx.invert();

				span_interpolator_linear<> interpolator(mtx);

				pattern_wrapper<pixfmt>* pattern = get_pattern_wrap(ctx, pattern_fmt);
				if (filter) {
					if (pimg->transparent) {
						typename picasso_painter_helper<Pixfmt>::span_canvas_pattern_type 
							sg(*pattern, interpolator, *(filter));
						render_scanlines_aa(ctx->raster, scanline, m_rb, sa, sg); //pattern raster
					} else {
						typename picasso_painter_helper<Pixfmt>::span_image_pattern_type 
							sg(*pattern, interpolator, *(filter));
						render_scanlines_aa(ctx->raster, scanline, m_rb, sa, sg); //pattern raster
					}
				} else {
					if (pimg->transparent) {
						typename picasso_painter_helper<Pixfmt>::span_canvas_pattern_type_nn 
							sg(*pattern, interpolator);
						render_scanlines_aa(ctx->raster, scanline, m_rb, sa, sg); //pattern raster
					} else {
						typename picasso_painter_helper<Pixfmt>::span_image_pattern_type_nn 
							sg(*pattern, interpolator);
						render_scanlines_aa(ctx->raster, scanline, m_rb, sa, sg); //pattern raster
					}
				}
				delete pattern;
			} else if (ctx->state->brush.style == brush_style_canvas) { //canvas fill
				pixfmt canvas_fmt(static_cast<ps_canvas*>(ctx->state->brush.data)->buffer);

				create_fill_data(ctx, path, 0, 0);

				trans_affine mtx;
				mtx *= trans_affine_translation((float)iround((float)dr.x), (float)iround((float)dr.y));
				mtx *= get_stable_matrix(ctx->state->world_matrix, path);
				mtx.invert();

				span_interpolator_linear<> interpolator(mtx);

				typename picasso_painter_helper<Pixfmt>::image_source_type canvas_src(canvas_fmt);

				if (filter) {
					typename picasso_painter_helper<Pixfmt>::span_canvas_filter_type
						sg(canvas_src, interpolator, *(filter));
					render_scanlines_aa(ctx->raster, scanline, m_rb, sa, sg); //image raster
				} else {
					typename picasso_painter_helper<Pixfmt>::span_canvas_filter_type_nn
						sg(canvas_src, interpolator);
					render_scanlines_aa(ctx->raster, scanline, m_rb, sa, sg); //image raster
				}
			}

			//free image filter if needed.
			if (filter)
				delete filter;
		}
	} else { //brush_style_solid
		create_fill_data(ctx, path, 0, 0);

		m_fmt.alpha(ctx->state->alpha);
		m_fmt.comp_op(ctx->state->composite);
		renderer_solid_type ren(m_rb);

    	scanline_p8 scanline;
		ren.color(ctx->state->brush.color); //solid raster
		render_scanlines(ctx->raster, scanline, ren);
	}
}

}
#endif/*_PICASSO_PAINTER_H_*/
