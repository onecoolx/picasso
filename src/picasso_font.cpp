/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */
#include <new>
#include <stdlib.h>

#include "pconfig.h"
#include "picasso.h"
#include "picasso_utils.h"
#include "picasso_p.h"

using namespace picasso;

namespace picasso {

#define GLYPH_STABLE_ADVANCE_WIDTH(g) \
	(((g->bounds.x2-g->bounds.x1-1) <= (g->advance_x/2) \
	 && (g->advance_x>8) && ((g->bounds.x2-g->bounds.x1)>2)) \
	 ? (((g->bounds.x2 > g->advance_x/2) && (g->bounds.x2 < g->advance_x)) ? (g->bounds.x2+1) : \
	 (g->advance_x/2+1)) : (g->advance_x))

ps_font* _default_font(void)
{
	ps_font* f = (ps_font*)malloc(sizeof(ps_font));
	if (f) {
		f->refcount = 1;
		memset(f->name, 0, MAX_FONT_NAME_LENGTH);
		strcpy(f->name, "Arial");
		f->charset = CHARSET_ANSI;
		f->type = FONT_TYPE_STROKE;
		f->resolution = 0;
		f->height = 12;
		f->width = 0;
		f->weight = 400;
		f->italic = false;
		f->hint = true;
		f->flip_y = true;
		f->kerning = true;
		global_status = STATUS_SUCCEED;
		return f;
	} else  {
        	global_status = STATUS_OUT_OF_MEMORY;
		return 0;
	}
}

static inline void set_font_attr(ps_context* ctx, ps_font* f)
{
	//font attrib set
	ctx->fonts->resolution(f->resolution);
	ctx->fonts->hinting(f->hint);
	ctx->fonts->height(f->height);
	ctx->fonts->width(f->width);
	ctx->fonts->weight(f->weight);
	ctx->fonts->italic(f->italic);
	ctx->fonts->flip_y(f->flip_y);
	ctx->fonts->char_set(f->charset);
}

static bool inline is_same_font(ps_context* ctx)
{
	return (ctx->fonts->resolution() == ctx->state->font->resolution) &&
		   (ctx->fonts->hinting() == ctx->state->font->hint) &&
		   (ctx->fonts->height() == ctx->state->font->height) &&
		   (ctx->fonts->width() == ctx->state->font->width) &&
		   (ctx->fonts->weight() == ctx->state->font->weight) &&
		   (ctx->fonts->italic() == ctx->state->font->italic) &&
		   (ctx->fonts->flip_y() == ctx->state->font->flip_y) &&
		   (ctx->fonts->char_set() == ctx->state->font->charset) &&
		   (ctx->fonts->antialias() == (ctx->font_antialias ? true : false)) &&
		   (strcmp(ctx->fonts->name(), ctx->state->font->name) == 0);
}

static bool inline create_device_font(ps_context* ctx, trans_affine& mtx)
{
	ctx->fonts->transform(mtx);
	if (is_same_font(ctx))
		return true;

	glyph_rendering gren = glyph_ren_outline;
	set_font_attr(ctx, ctx->state->font);
	ctx->fonts->set_antialias(ctx->font_antialias ? true : false);

	if (ctx->fonts->create_font(ctx->state->font->name, gren))
		return true;
	else
		return false;
}

static inline void _add_glyph_to_path(ps_context* ctx, font_cache_manager<font_engine>& f, path_storage& path)	
{
	typedef font_cache_manager<font_engine> font_cache_type;
	typedef conv_curve<font_cache_type::path_adaptor_type> conv_curve_type;
	conv_curve_type curve(f.path_adaptor());

	float x = 0;
	float y = 0;

	while (true) {
		unsigned cmd = curve.vertex(&x, &y);
		if (cmd == path_cmd_stop) {
			path.add_vertex(x, y, path_cmd_end_poly);
			break;
		} else 
			path.add_vertex(x, y, cmd);
	}
}

}

#ifdef __cplusplus
extern "C" {
#endif

ps_font* PICAPI ps_font_create_copy(const ps_font* font)
{
	if (!font) {
		global_status = STATUS_INVALID_ARGUMENT;
		return 0;
	}

	ps_font* f = (ps_font*)malloc(sizeof(ps_font));
	if (f) {
		f->refcount = 1;
		memset(f->name, 0, MAX_FONT_NAME_LENGTH);
		strncpy(f->name, font->name, MAX_FONT_NAME_LENGTH-1);
		f->charset = font->charset;
		f->type = font->type;
		f->resolution = font->resolution;
		f->height = font->height;
		f->width = font->width;
		f->weight = font->weight;
		f->italic = font->italic;
		f->hint = font->hint;
		f->flip_y = font->flip_y;
		f->kerning = font->kerning;
		global_status = STATUS_SUCCEED;
		return f;
	} else {
        global_status = STATUS_OUT_OF_MEMORY;
		return 0;
	}
}

ps_font* PICAPI ps_font_create(const char* name, ps_charset c, ps_font_type t, double s, int w, ps_bool i)
{
	if (!name) {
		global_status = STATUS_INVALID_ARGUMENT;
		return 0;
	}

	ps_font* f = (ps_font*)malloc(sizeof(ps_font));
	if (f) {
		f->refcount = 1;
		memset(f->name, 0, MAX_FONT_NAME_LENGTH);
		strncpy(f->name, name, MAX_FONT_NAME_LENGTH-1);
		f->charset = c;
		f->type = t;
		f->resolution = 0;
		f->height = (float)s;
		f->width = 0;
		f->weight = w;
		f->italic = i ? true : false;
		f->hint = true;
		f->flip_y = true;
		f->kerning = true;
		global_status = STATUS_SUCCEED;
		return f;
	} else {
        global_status = STATUS_OUT_OF_MEMORY;
		return 0;
	}
}

ps_font* PICAPI ps_font_ref(ps_font* f)
{
	if (!f) {
		global_status = STATUS_INVALID_ARGUMENT;
		return 0;
	}

	f->refcount++;
	global_status = STATUS_SUCCEED;
	return f;
}

void PICAPI ps_font_unref(ps_font* f)
{
	if (!f) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	f->refcount--;
	if (f->refcount <= 0) {
		free(f);
	}
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_font_set_size(ps_font* f, double s)
{
	if (!f || s < 0.0) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	
	f->height = (float)s;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_font_set_weight(ps_font* f, int w)
{
	if (!f || w < 100 || w > 900) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	f->weight = w;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_font_set_italic(ps_font* f, ps_bool it)
{
	if (!f) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	f->italic = it ? true : false;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_font_set_hint(ps_font* f, ps_bool h)
{
	if (!f) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	f->hint = h ? true : false;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_font_set_flip(ps_font* f, ps_bool l)
{
	if (!f) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	
	f->flip_y = l ? false : true;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_font_set_charset(ps_font* f, ps_charset c)
{
	if (!f) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	f->charset = c;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_font_set_type(ps_font* f, ps_font_type t)
{
	if (!f) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	f->type = t;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_text_out_length(ps_context* ctx, double x, double y, const char* text, unsigned int len)
{
	if (!ctx || !text || !len) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	float gx = 0;
	float gy = 0;

	float tx = (float)x;
	float ty = (float)y;

	trans_affine mtx = ctx->text_matrix;

	mtx.translation(&gx, &gy);
	mtx.transform(&tx, &ty);
	mtx *= trans_affine_translation(-gx, -gy);

	if (create_device_font(ctx, mtx)) {

		ty += ctx->fonts->ascent();
		const char* p = text;

		while (*p && len) {

			const glyph_cache* glyph = ctx->cache->glyph(*p);
			if (glyph) {
				if (ctx->state->font->kerning)
					ctx->cache->add_kerning(&tx, &ty);
				ctx->cache->init_embedded_adaptors(glyph, tx, ty);

				ctx->canvas->p->render_font_cache(ctx, *(ctx->cache), glyph->data_type);

				tx += GLYPH_STABLE_ADVANCE_WIDTH(glyph);
				ty += glyph->advance_y;
			}
			len--;
			p++;
		}
		ctx->canvas->p->render_font_raster(ctx);
	}
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_wide_text_out_length(ps_context* ctx, double x, double y, const ps_uchar16* text, unsigned int len)
{
	if (!ctx || !text || !len) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	float gx = 0;
	float gy = 0;

	float tx = (float)x;
	float ty = (float)y;

	trans_affine mtx = ctx->text_matrix;

	mtx.translation(&gx, &gy);
	mtx.transform(&tx, &ty);
	mtx *= trans_affine_translation(-gx, -gy);

	if (create_device_font(ctx, mtx)) {

		ty += ctx->fonts->ascent();
		const ps_uchar16* p = text;

		while (*p && len) {

			const glyph_cache* glyph = ctx->cache->glyph(*p);
			if (glyph) {
				if (ctx->state->font->kerning)
					ctx->cache->add_kerning(&tx, &ty);
				ctx->cache->init_embedded_adaptors(glyph, tx, ty);

				ctx->canvas->p->render_font_cache(ctx, *(ctx->cache), glyph->data_type);

				tx += GLYPH_STABLE_ADVANCE_WIDTH(glyph);
				ty += glyph->advance_y;
			}
			len--;
			p++;
		}
		ctx->canvas->p->render_font_raster(ctx);
	}
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_draw_text(ps_context* ctx, const ps_rect* area, const void* text, unsigned int len,
																ps_draw_text_type type, ps_text_align align)
{
	if (!ctx || !area || !text || !len) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	trans_affine mtx = ctx->text_matrix;

	float gx = 0;
	float gy = 0;

	float x = (float)area->x;
	float y = (float)area->y;

	mtx.translation(&gx, &gy);
	mtx *= trans_affine_translation(-gx, -gy);

	path_storage text_path;

	if (create_device_font(ctx, mtx)) {

		// align layout
		float w = 0, h = 0;
		const glyph_cache* glyph_test = 0;

		if (ctx->state->font->charset == CHARSET_ANSI) {
			const char* p = (const char*)text;
			glyph_test = ctx->cache->glyph(*p);
		} else {
			const ps_uchar16* p = (const ps_uchar16*)text;
			glyph_test = ctx->cache->glyph(*p);
		}

		if (glyph_test) {
			w = GLYPH_STABLE_ADVANCE_WIDTH(glyph_test);
			h = glyph_test->advance_y;
		}

		w *= len; //note: estimate

		if (align & TEXT_ALIGN_LEFT)
			x = (float)area->x;
		else if (align & TEXT_ALIGN_RIGHT)
			x = (float)(area->x + (area->w - w));
		else
			x = (float)(area->x + (area->w - w)/2);

		if (align & TEXT_ALIGN_TOP) {
			y = (float)area->y;
			y += ctx->fonts->ascent();
		} else if (align & TEXT_ALIGN_BOTTOM) {
			y = (float)(area->y + (area->h - h));
			y -= ctx->fonts->descent();
		} else {
			y = (float)(area->y + (area->h - h)/2);
			y += (ctx->fonts->ascent() - ctx->fonts->descent())/2;
		}

		// draw the text

		if (ctx->state->font->charset == CHARSET_ANSI) {
			const char* p = (const char*)text;

			while (*p && len) {

				const glyph_cache* glyph = ctx->cache->glyph(*p);
				if (glyph) {
					if (ctx->state->font->kerning)
						ctx->cache->add_kerning(&x,&y);
					ctx->cache->init_embedded_adaptors(glyph, x, y);

					_add_glyph_to_path(ctx, *(ctx->cache), text_path);

					x += GLYPH_STABLE_ADVANCE_WIDTH(glyph);
					y += glyph->advance_y;
				}
				len--;
				p++;
			}
		} else {
			const ps_uchar16* p = (const ps_uchar16*)text;

			while (*p && len) {

				const glyph_cache* glyph = ctx->cache->glyph(*p);
				if (glyph) {
					if (ctx->state->font->kerning)
						ctx->cache->add_kerning(&x,&y);

					ctx->cache->init_embedded_adaptors(glyph, x, y);

					_add_glyph_to_path(ctx, *(ctx->cache), text_path);

					x += GLYPH_STABLE_ADVANCE_WIDTH(glyph);
					y += glyph->advance_y;
				}
				len--;
				p++;
			}
		}
	}
	//store the old color
	rgba bc = ctx->state->brush.color;
	rgba pc = ctx->state->pen.color;

	ctx->state->brush.color = ctx->state->font_fcolor;
	ctx->state->pen.color = ctx->state->font_scolor;

	switch (type) {
		case DRAW_TEXT_FILL:
			ctx->canvas->p->render_shadow(ctx, text_path, true, false);
    		ctx->canvas->p->render_fill(ctx, text_path);
    		ctx->canvas->p->render_blur(ctx);
			break;
		case DRAW_TEXT_STROKE:
			ctx->canvas->p->render_shadow(ctx, text_path, false, true);
    		ctx->canvas->p->render_stroke(ctx, text_path);
    		ctx->canvas->p->render_blur(ctx);
			break;
		case DRAW_TEXT_BOTH:
			ctx->canvas->p->render_shadow(ctx, text_path, true, true);
    		ctx->canvas->p->render_fill(ctx, text_path);
    		ctx->canvas->p->render_stroke(ctx, text_path);
    		ctx->canvas->p->render_blur(ctx);
			break;
	}

	ctx->state->brush.color = bc;
	ctx->state->pen.color = pc;
	ctx->raster.reset();
	global_status = STATUS_SUCCEED;
}

ps_size PICAPI ps_get_text_extent(ps_context* ctx, const void* text, unsigned int len)
{
	ps_size size = {0 , 0};

	if (!ctx || !text || !len) {
		global_status = STATUS_INVALID_ARGUMENT;
		return size;
	}

	float width = 0;
	float height = 0;

	trans_affine mtx;

	if (create_device_font(ctx, mtx)) {

		if (ctx->state->font->charset == CHARSET_ANSI) {
			const char* p = (const char*)text;

			while (*p && len) {

				const glyph_cache* glyph = ctx->cache->glyph(*p);
				if (glyph) {
					if (ctx->state->font->kerning)
						ctx->cache->add_kerning(&width, &height);
					width += GLYPH_STABLE_ADVANCE_WIDTH(glyph);
				}
				len--;
				p++;
			}
		} else {
			const ps_uchar16* p = (const ps_uchar16*)text;

			while (*p && len) {

				const glyph_cache* glyph = ctx->cache->glyph(*p);
				if (glyph) {
					if (ctx->state->font->kerning)
						ctx->cache->add_kerning(&width, &height);
					width += GLYPH_STABLE_ADVANCE_WIDTH(glyph);
				}
				len--;
				p++;
			}
		}
	}

	size.h = ctx->state->font->height;
	size.w = width;
	global_status = STATUS_SUCCEED;
	return size;
}

ps_bool PICAPI ps_get_glyph(ps_context* ctx, int ch, ps_glyph* g)
{
	if (!ctx || !g) {
		global_status = STATUS_INVALID_ARGUMENT;
		return False;
	}

	trans_affine mtx;
	if (create_device_font(ctx, mtx)) {

		if (ctx->state->font->charset == CHARSET_ANSI) {
			char c = (char)ch;
			g->glyph = (void*)ctx->cache->glyph(c);
		} else {
			ps_uchar16 c = (ps_uchar16)ch;
			g->glyph = (void*)ctx->cache->glyph(c);
		}
		global_status = STATUS_SUCCEED;
		return True;
	} else {
		g->glyph = NULL;
		global_status = STATUS_SUCCEED;
		return False;
	}
}

void PICAPI ps_show_glyphs(ps_context* ctx, double x, double y,  ps_glyph* g, unsigned int len)
{
	if (!ctx || !g || !len) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	trans_affine mtx = ctx->text_matrix;

	float gx = 0;
	float gy = 0;

	float tx = (float)x;
	float ty = (float)y;

	mtx.translation(&gx, &gy);
	mtx.transform(&tx, &ty);
	mtx *= trans_affine_translation(-gx, -gy);
	
	if (create_device_font(ctx, mtx)) {

		ty += ctx->fonts->ascent();

		for (unsigned int i = 0; i < len; i++) {
			const glyph_cache* glyph = (const glyph_cache*)g[i].glyph;
			if (glyph) {
				if (ctx->state->font->kerning)
					ctx->cache->add_kerning(&tx, &ty);
				ctx->cache->init_embedded_adaptors(glyph, tx, ty);

				ctx->canvas->p->render_font_cache(ctx, *(ctx->cache), glyph->data_type);

				tx += GLYPH_STABLE_ADVANCE_WIDTH(glyph);
				ty += glyph->advance_y;
			}
		}
		ctx->canvas->p->render_font_raster(ctx);
	}
	global_status = STATUS_SUCCEED;
}

ps_bool PICAPI ps_get_path_from_glyph(ps_context* ctx, const ps_glyph* g, ps_path* p)
{
	if (!ctx || !g || !p) {
		global_status = STATUS_INVALID_ARGUMENT;
		return False;
	}

	p->path.remove_all(); // clear the path

	trans_affine mtx = ctx->text_matrix;

	float gx = 0;
	float gy = 0;
	float x = 0;
	float y = 0;

	mtx.translation(&gx, &gy);
	mtx.transform(&x, &y);
	mtx *= trans_affine_translation(-gx, -gy);
	
	if (create_device_font(ctx, mtx)) {

		y += ctx->fonts->ascent();

		const glyph_cache* glyph = (const glyph_cache*)g->glyph;
		if (glyph) {
			if (ctx->state->font->kerning)
				ctx->cache->add_kerning(&x, &y);

			ctx->cache->init_embedded_adaptors(glyph, x, y);

			_add_glyph_to_path(ctx, *(ctx->cache), p->path);
		}
	}


	p->path.close_polygon();
	global_status = STATUS_SUCCEED;
	return True;
}

ps_size PICAPI ps_glyph_get_extent(const ps_glyph* g)
{
	ps_size size = {0 , 0};

	if (!g) {
		global_status = STATUS_INVALID_ARGUMENT;
		return size;
	}

	if (g->glyph) {
		const glyph_cache* glyph = (const glyph_cache*)g->glyph;
		size.w = GLYPH_STABLE_ADVANCE_WIDTH(glyph);
		size.h = glyph->height;
	}
	global_status = STATUS_SUCCEED;
	return size;
}

ps_bool PICAPI ps_get_font_info(ps_context* ctx, ps_font_info* info)
{
	if (!ctx || !info) {
		global_status = STATUS_INVALID_ARGUMENT;
		return False;
	}

	trans_affine mtx;
	if (create_device_font(ctx, mtx)) {
		info->size = ctx->fonts->height();
		info->ascent = ctx->fonts->ascent();
		info->descent = ctx->fonts->descent();
		info->leading = ctx->fonts->leading();
		info->unitsEM = ctx->fonts->units_per_em();
	}

	global_status = STATUS_SUCCEED;
	return True;
}

ps_font* PICAPI ps_set_font(ps_context* ctx, const ps_font* f)
{
	if (!ctx || !f) {
		global_status = STATUS_INVALID_ARGUMENT;
		return 0;
	}

	ps_font* old = ctx->state->font;
	ps_font_unref(ctx->state->font);
	ctx->state->font = ps_font_ref(const_cast<ps_font*>(f));
	global_status = STATUS_SUCCEED;
	return old;
}

void PICAPI ps_set_font_antialias(ps_context* ctx, ps_bool a)
{
	if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	ctx->font_antialias = a;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_text_color(ps_context* ctx, const ps_color* c)
{
	if (!ctx || !c) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	ctx->state->font_fcolor = rgba((float)c->r, (float)c->g, (float)c->b, (float)c->a);
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_text_stroke_color(ps_context* ctx, const ps_color* c)
{
	if (!ctx || !c) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	ctx->state->font_scolor = rgba((float)c->r, (float)c->g, (float)c->b, (float)c->a);
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_text_transform(ps_context* ctx, const ps_matrix* m)
{
	if (!ctx || !m) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	ctx->text_matrix *= m->matrix;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_text_matrix(ps_context* ctx, const ps_matrix* m)
{
	if (!ctx || !m) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	ctx->text_matrix = m->matrix;
	global_status = STATUS_SUCCEED;
}

#ifdef __cplusplus
}
#endif
