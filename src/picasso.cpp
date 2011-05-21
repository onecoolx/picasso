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
#include "picasso_painter.h"

#define PICASSO_VERSION 12000 //version 1.2

using namespace picasso;

#ifdef __cplusplus
extern "C" {
#endif

#if ENABLE(LOW_MEMORY)
static font_engine g_engine;
static font_cache_manager<font_engine> g_cache(g_engine);
#endif

#if ENABLE(FREE_TYPE2)
static bool _init = false;
#endif

ps_status global_status = STATUS_SUCCEED;

int PICAPI ps_version(void)
{
	return PICASSO_VERSION;
}

ps_bool PICAPI ps_initialize(void)
{
#if ENABLE(FREE_TYPE2)
	if (!_init) {
	    _init = _load_fonts();
	}
	return  _init ? True : False;
#else
	return True;
#endif
}

void PICAPI ps_shutdown(void)
{
#if ENABLE(FREE_TYPE2)
	if (_init) {
	    _free_fonts();
	    _init = false;
	}
#endif
}

ps_status PICAPI ps_last_status(void)
{
    return global_status;
}

ps_context* PICAPI ps_context_create(ps_canvas* canvas)
{
    if (!canvas) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }

	context_state * state = new context_state;
    if (!state) {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }

    ps_context *c = (ps_context*)malloc(sizeof(ps_context));
	if (c) {
		c->refcount = 1;
		c->canvas = ps_canvas_ref(canvas);
		c->state = state;
#if ENABLE(LOW_MEMORY)
		c->font_antialias = False;
		c->fonts = &g_engine;
		c->cache = &g_cache;
#else
		c->font_antialias = True;
		c->fonts = new font_engine;
		c->cache = new font_cache_manager<font_engine>(*(c->fonts));
#endif
		new ((void*)&(c->path)) path_storage;
		new ((void*)&(c->raster)) rasterizer_scanline_aa<>;
		new ((void*)&(c->text_matrix)) trans_affine;
		global_status = STATUS_SUCCEED;
		return c;
	} else {
		delete state; //free state on error
        	global_status = STATUS_OUT_OF_MEMORY;
		return 0;
	}
}

ps_canvas* PICAPI ps_context_set_canvas(ps_context* ctx, ps_canvas* canvas)
{
    if (!ctx || !canvas) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }
	ps_canvas* old = ctx->canvas; // context's canvas must more than 2
	ctx->canvas = ps_canvas_ref(canvas);
	ps_canvas_unref(old);// release context's reference
    global_status = STATUS_SUCCEED;
	return old;
}

ps_canvas* PICAPI ps_context_get_canvas(ps_context* ctx)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }
	
	ps_canvas* canvas= ctx->canvas;
    global_status = STATUS_SUCCEED;
    return canvas;
}

ps_context* PICAPI ps_context_ref(ps_context* ctx)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }

    ctx->refcount++;
    global_status = STATUS_SUCCEED;
    return ctx;
}

void PICAPI ps_context_unref(ps_context* ctx)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    ctx->refcount--;
    if (ctx->refcount <= 0) {
		ps_canvas_unref(ctx->canvas);
		while (ctx->state) {
			context_state * p = ctx->state;
			ctx->state = ctx->state->next;
			delete p;
		}
#if !ENABLE(LOW_MEMORY)
		delete ctx->cache;
        delete ctx->fonts;
#endif
        (&ctx->path)->path_storage::~path_storage();
        (&ctx->raster)->rasterizer_scanline_aa<>::~rasterizer_scanline_aa();
		(&ctx->text_matrix)->trans_affine::~trans_affine();
        free(ctx);
    }
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_source_gradient(ps_context* ctx, const ps_gradient* gradient)
{
	if (!ctx || !gradient) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	ctx->state->brush.clear(); //clear source
	ctx->state->brush.set_gradient_brush(const_cast<ps_gradient*>(gradient));
    	global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_source_pattern(ps_context* ctx, const ps_pattern* pattern)
{
	if (!ctx || !pattern) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	ctx->state->brush.clear(); //clear source
	ctx->state->brush.set_pattern_brush(const_cast<ps_pattern*>(pattern));
    	global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_source_image(ps_context* ctx, const ps_image* image)
{
	if (!ctx || !image) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	ctx->state->brush.clear(); //clear source
	ctx->state->brush.set_image_brush(const_cast<ps_image*>(image));
    	global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_source_color(ps_context* ctx, const ps_color* color)
{
    if(!ctx || !color) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    ctx->state->brush.clear(); //clear source
    ctx->state->brush.color.r = (float)color->r;
    ctx->state->brush.color.g = (float)color->g;
    ctx->state->brush.color.b = (float)color->b;
    ctx->state->brush.color.a = (float)color->a;
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_source_canvas(ps_context* ctx, const ps_canvas* canvas)
{
    if(!ctx || !canvas) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
	ctx->state->brush.clear(); //clear source
	ctx->state->brush.set_canvas_brush(const_cast<ps_canvas*>(canvas));
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_stroke_color(ps_context* ctx, const ps_color* color)
{
    if(!ctx || !color) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    ctx->state->pen.color.r = (float)color->r;
    ctx->state->pen.color.g = (float)color->g;
    ctx->state->pen.color.b = (float)color->b;
    ctx->state->pen.color.a = (float)color->a;
    global_status = STATUS_SUCCEED;
}

ps_filter PICAPI ps_set_filter(ps_context* ctx, ps_filter filter)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return FILTER_UNKNOWN;
    }
	ps_filter old = ctx->state->filter;
	ctx->state->filter = filter;
   	global_status = STATUS_SUCCEED;
	return old;
}


void PICAPI ps_stroke(ps_context* ctx)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	ctx->canvas->p->render_shadow(ctx, ctx->path, false, true);
    ctx->canvas->p->render_stroke(ctx, ctx->path);
    ctx->canvas->p->render_blur(ctx);
    ctx->path.free_all();
    ctx->raster.reset();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_fill(ps_context* ctx)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	ctx->canvas->p->render_shadow(ctx, ctx->path, true, false);
    ctx->canvas->p->render_fill(ctx, ctx->path);
    ctx->canvas->p->render_blur(ctx);
    ctx->path.free_all();
    ctx->raster.reset();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_paint(ps_context* ctx)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	ctx->canvas->p->render_shadow(ctx, ctx->path, true, true);
    ctx->canvas->p->render_fill(ctx, ctx->path);
    ctx->canvas->p->render_stroke(ctx, ctx->path);
    ctx->canvas->p->render_blur(ctx);
    ctx->path.free_all();
    ctx->raster.reset();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_clear(ps_context* ctx)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
	
	ctx->canvas->p->render_clear(ctx);
    global_status = STATUS_SUCCEED;
}

double PICAPI ps_set_alpha(ps_context* ctx, double a)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0.0;
    }

	//alpha range 0 ~ 1
	if (a < 0.0)
		a = 0.0;
	else if (a > 1.0)
		a = 1.0;

	float rd = ctx->state->alpha;
	ctx->state->alpha = (float)a;
    global_status = STATUS_SUCCEED;
	return rd;
}

double PICAPI ps_set_blur(ps_context* ctx, double b)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0.0;
    }

	//blur range 0 ~ 1
	if (b < 0.0)
		b = 0.0;
	else if (b > 1.0)
		b = 1.0;

	float rd = ctx->state->blur;
	ctx->state->blur = (float)b;
    global_status = STATUS_SUCCEED;
	return rd;
}

double PICAPI ps_set_gamma(ps_context* ctx, double g)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0.0;
    }

	//gamma range 0.6 ~ 3.0
	if (g < 0.6)
		g = 0.6;
	else if (g > 3.0)
		g = 3.0;

	float rd = ctx->state->gamma;
	if (rd != g) {
		ctx->state->gamma = (float)g;
		ctx->canvas->p->render_gamma(ctx);
	}
    global_status = STATUS_SUCCEED;
	return rd;
}

void PICAPI ps_set_antialias(ps_context* ctx, ps_bool anti)
{
	if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	ps_bool old = ctx->state->antialias ? True : False;
	if (old != anti) {
		ctx->state->antialias = anti ? true : false;
		ctx->canvas->p->render_gamma(ctx);
	}
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_shadow(ps_context* ctx, double x, double y, double b)
{
	if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	//blur range 0 ~ 1
	if (b < 0.0)
		b = 0.0;
	else if (b > 1.0)
		b = 1.0;

	ctx->state->shadow.use_shadow = true;
	ctx->state->shadow.x_offset = (float)x;
	ctx->state->shadow.y_offset = (float)y;
	ctx->state->shadow.blur = (float)b;
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_shadow_color(ps_context* ctx, const ps_color* c)
{
	if (!ctx || !c) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	ctx->state->shadow.color = rgba((float)c->r, (float)c->g, (float)c->b, (float)c->a);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_reset_shadow(ps_context* ctx)
{
	if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	ctx->state->shadow = shadow_state();
    global_status = STATUS_SUCCEED;
}

ps_fill_rule PICAPI ps_set_fill_rule(ps_context* ctx, ps_fill_rule rule)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return FILL_RULE_ERROR;
    }

	ps_fill_rule rl = static_cast<ps_fill_rule>(ctx->state->brush.rule);
    switch (rule) 
    {
        case FILL_RULE_WINDING:
            ctx->state->brush.rule = fill_non_zero;
            break;
        case FILL_RULE_EVEN_ODD:
            ctx->state->brush.rule = fill_even_odd;
            break;
        default:
    	    global_status = STATUS_UNKNOWN_ERROR;
            return FILL_RULE_ERROR;
    }
    global_status = STATUS_SUCCEED;
	return rl;
}

void PICAPI ps_set_line_cap(ps_context* ctx, ps_line_cap line_cap)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    switch (line_cap) 
    {
        case LINE_CAP_BUTT:
            ctx->state->pen.cap = butt_cap;
            break;
        case LINE_CAP_SQUARE:
            ctx->state->pen.cap = square_cap;
            break;
        case LINE_CAP_ROUND:
            ctx->state->pen.cap = round_cap;
            break;
        default:
    	    global_status = STATUS_UNKNOWN_ERROR;
            return;
    }
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_line_inner_join(ps_context* ctx, ps_line_inner_join line_inner_join)
{
    if (!ctx) {
	global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    switch (line_inner_join) 
    {
        case LINE_INNER_MITER:
            ctx->state->pen.inner = inner_miter;
            break;
        case LINE_INNER_ROUND:
            ctx->state->pen.inner = inner_round;
            break;
        case LINE_INNER_BEVEL:
            ctx->state->pen.inner = inner_bevel;
            break;
        case LINE_INNER_JAG:
            ctx->state->pen.inner = inner_jag;
            break;
        default:
    	    global_status = STATUS_UNKNOWN_ERROR;
            return;
    }
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_line_join(ps_context* ctx, ps_line_join line_join)
{
    if (!ctx) {
	global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    switch (line_join) 
    {
        case LINE_JOIN_MITER:
            ctx->state->pen.join = miter_join;
            break;
        case LINE_JOIN_MITER_REVERT:
            ctx->state->pen.join = miter_join_revert;
            break;
        case LINE_JOIN_MITER_ROUND:
            ctx->state->pen.join = miter_join_round;
            break;
        case LINE_JOIN_ROUND:
            ctx->state->pen.join = round_join;
            break;
        case LINE_JOIN_BEVEL:
            ctx->state->pen.join = bevel_join;
            break;
        default:
    	    global_status = STATUS_UNKNOWN_ERROR;
            return;
    }
    global_status = STATUS_SUCCEED;
}

double PICAPI ps_set_line_width(ps_context* ctx, double width)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0.0;
    }

    if (width < 0.0)
        width = 0.0;

	float rw = ctx->state->pen.width;
    ctx->state->pen.width = (float)width;
    global_status = STATUS_SUCCEED;
	return rw;
}

double PICAPI ps_set_miter_limit(ps_context* ctx, double limit)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0.0;
    }

    if (limit < 0.0)
        limit = 0.0;

	float rd = ctx->state->pen.miter_limit;
    ctx->state->pen.miter_limit = (float)limit;
    global_status = STATUS_SUCCEED;
	return rd;
}

void PICAPI ps_set_line_dash(ps_context* ctx, double start, double* dashes, unsigned int num_dashes)
{
    if (!ctx || !dashes || !num_dashes) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    if (start < 0.0)
        start = 0.0;

    ctx->state->pen.clear_dash();
    ctx->state->pen.set_dash(start, dashes, num_dashes);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_reset_line_dash(ps_context* ctx)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    ctx->state->pen.clear_dash();
    global_status = STATUS_SUCCEED;
}

//path function
void PICAPI ps_new_path(ps_context* ctx)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    ctx->path.free_all();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_new_sub_path(ps_context* ctx)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    ctx->path.end_poly();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_rectangle(ps_context* ctx, const ps_rect* pr)
{
    if (!ctx || !pr) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	ctx->path.set_rect(true); //It is because boder edge.
    ctx->path.move_to((float)floor(pr->x), (float)floor(pr->y));
    ctx->path.hline_rel((float)floor(pr->w));
    ctx->path.vline_rel((float)floor(pr->h));
    ctx->path.hline_rel(-(float)floor(pr->w));
    ctx->path.vline_rel(-(float)floor(pr->h));
    ctx->path.end_poly();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_rounded_rect(ps_context* ctx, const ps_rect* r, double ltx, double lty, double rtx, double rty,
																	double lbx, double lby, double rbx, double rby)
{
	if (!ctx || !r) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	rounded_rect rr;
	rr.rect((float)floor(r->x), (float)floor(r->y), 
						(float)(floor(r->x)+r->w), (float)(floor(r->y)+r->h));
	rr.radius(float(ltx), float(lty), float(rtx), float(rty), 
						float(rbx), float(rby), float(lbx), float(lby));
	rr.normalize_radius();
    if (_is_closed_path(ctx->path))
        ctx->path.concat_path(rr, 0);
    else
        ctx->path.join_path(rr, 0);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_ellipse(ps_context* ctx, const ps_rect* r)
{
    if (!ctx || !r) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    ellipse e(float(floor(r->x)+r->w/2), float(floor(r->y)+r->h/2), float(r->w/2), float(r->h/2));
    if (_is_closed_path(ctx->path))
        ctx->path.concat_path(e, 0);
    else
        ctx->path.join_path(e, 0);
    global_status = STATUS_SUCCEED;
}


void PICAPI ps_close_path(ps_context* ctx)
{
    if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    ctx->path.close_polygon();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_move_to(ps_context* ctx, const ps_point* pt)
{
    if (!ctx || !pt) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    ctx->path.move_to((float)floor(pt->x), (float)floor(pt->y));
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_line_to(ps_context* ctx, const ps_point* pt)
{
    if (!ctx || !pt) {
	global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    ctx->path.line_to((float)floor(pt->x), (float)floor(pt->y));
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_bezier_curve_to(ps_context* ctx, const ps_point* fcp, 
                                            const ps_point* scp, const ps_point* ep)
{
    if (!ctx || !fcp || !scp || !ep) {
	global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    curve4 c((float)floor(ctx->path.last_x()), (float)floor(ctx->path.last_y()), 
			(float)floor(fcp->x), (float)floor(fcp->y), (float)floor(scp->x), (float)floor(scp->y), 
			(float)floor(ep->x), (float)floor(ep->y));

    if (_is_closed_path(ctx->path))
        ctx->path.concat_path(c, 0);
    else
        ctx->path.join_path(c, 0);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_quad_curve_to(ps_context* ctx, const ps_point* cp, const ps_point* ep)
{
    if (!ctx || !cp || !ep) {
	global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    curve3 c((float)floor(ctx->path.last_x()), (float)floor(ctx->path.last_y()), 
					(float)floor(cp->x), (float)floor(cp->y), (float)floor(ep->x), (float)floor(ep->y));

    if (_is_closed_path(ctx->path))
        ctx->path.concat_path(c, 0);
    else
        ctx->path.join_path(c, 0);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_arc(ps_context* ctx, const ps_point* cp, double r, 
                                            double sa, double ea, ps_bool clockwise)
{
    if (!ctx || !cp || r <=0 ) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    arc a((float)floor(cp->x), (float)floor(cp->y), float(r), float(r), float(sa), float(ea), (clockwise?true:false));

    if (_is_closed_path(ctx->path))
        ctx->path.concat_path(a, 0);
    else
        ctx->path.join_path(a, 0);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_tangent_arc(ps_context* ctx, const ps_rect* r, double sa, double sw)
{
    if (!ctx || !r) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	float xr = float(r->w / 2);
	float yr = float(r->h / 2);
	float cx = float(r->x + xr);
	float cy = float(r->y + yr);

    bezier_arc ba((float)floor(cx), (float)floor(cy), xr, yr, (float)sa, (float)sw);
    conv_curve<bezier_arc> cr(ba);

    if (_is_closed_path(ctx->path))
        ctx->path.concat_path(cr, 0);
    else
        ctx->path.join_path(cr, 0);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_path(ps_context* ctx, const ps_path* path)
{
    if (!ctx || !path) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    ctx->path = path->path;
    global_status = STATUS_SUCCEED;
}

// transform world
void PICAPI ps_translate(ps_context* ctx, double tx, double ty)
{
	if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	ctx->state->world_matrix *= trans_affine_translation((float)tx, (float)ty);
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_scale(ps_context* ctx, double sx, double sy)
{
	if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	ctx->state->world_matrix *= trans_affine_scaling((float)sx, (float)sy);
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_shear(ps_context* ctx, double sx, double sy)
{
	if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	ctx->state->world_matrix *= trans_affine_skewing((float)sx, (float)sy);
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_rotate(ps_context* ctx, double angle)
{
	if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	ctx->state->world_matrix *= trans_affine_rotation((float)angle);
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_identity(ps_context* ctx)
{
	if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	ctx->state->world_matrix.reset();
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_transform(ps_context* ctx, const ps_matrix* matrix)
{
	if (!ctx || !matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	ctx->state->world_matrix *= matrix->matrix;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_set_matrix(ps_context* ctx, const ps_matrix* matrix)
{
	if (!ctx || !matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	ctx->state->world_matrix = matrix->matrix;
	global_status = STATUS_SUCCEED;
}

ps_bool PICAPI ps_get_matrix(ps_context* ctx, ps_matrix* matrix)
{
	if (!ctx || !matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
		return False;
	}
	matrix->matrix = ctx->state->world_matrix;
	global_status = STATUS_SUCCEED;
	return True;
}

void PICAPI ps_world_to_viewport(ps_context* ctx, ps_point* point)
{
	if (!ctx ||!point) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	float x = (float)point->x;
	float y = (float)point->y;
	ctx->state->world_matrix.transform(&x, &y);
	point->x = x;
	point->y = y;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_viewport_to_world(ps_context* ctx, ps_point* point)
{
	if (!ctx ||!point) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	float x = (float)point->x;
	float y = (float)point->y;
	ctx->state->world_matrix.inverse_transform(&x, &y);
	point->x = x;
	point->y = y;
	global_status = STATUS_SUCCEED;
}

ps_composite PICAPI ps_set_composite_operator(ps_context* ctx, ps_composite composite)
{
	if (!ctx) { 
		global_status = STATUS_INVALID_ARGUMENT;
		return COMPOSITE_ERROR;
	}

	ps_composite op = static_cast<ps_composite>(ctx->state->composite);

	switch (composite)
	{
		case COMPOSITE_CLEAR:
			ctx->state->composite = comp_op_clear;
			break;
		case COMPOSITE_SRC:
			ctx->state->composite = comp_op_src;
			break;
		case COMPOSITE_SRC_OVER:
			ctx->state->composite = comp_op_src_over;
			break;
		case COMPOSITE_SRC_IN:
			ctx->state->composite = comp_op_src_in;
			break;
		case COMPOSITE_SRC_OUT:
			ctx->state->composite = comp_op_src_out;
			break;
		case COMPOSITE_SRC_ATOP:
			ctx->state->composite = comp_op_src_atop;
			break;
		case COMPOSITE_DST:
			ctx->state->composite = comp_op_dst;
			break;
		case COMPOSITE_DST_OVER:
			ctx->state->composite = comp_op_dst_over;
			break;
		case COMPOSITE_DST_IN:
			ctx->state->composite = comp_op_dst_in;
			break;
		case COMPOSITE_DST_OUT:
			ctx->state->composite = comp_op_dst_out;
			break;
		case COMPOSITE_DST_ATOP:
			ctx->state->composite = comp_op_dst_atop;
			break;
		case COMPOSITE_XOR:
			ctx->state->composite = comp_op_xor;
			break;
		case COMPOSITE_DARKEN:
			ctx->state->composite = comp_op_darken;
			break;
		case COMPOSITE_LIGHTEN:
			ctx->state->composite = comp_op_lighten;
			break;
		case COMPOSITE_OVERLAY:
			ctx->state->composite = comp_op_overlay;
			break;
		case COMPOSITE_SCREEN:
			ctx->state->composite = comp_op_screen;
			break;
		case COMPOSITE_MULTIPLY:
			ctx->state->composite = comp_op_multiply;
			break;
		case COMPOSITE_PLUS:
			ctx->state->composite = comp_op_plus;
			break;
		case COMPOSITE_MINUS:
			ctx->state->composite = comp_op_minus;
			break;
		case COMPOSITE_EXCLUSION:
			ctx->state->composite = comp_op_exclusion;
			break;
		case COMPOSITE_DIFFERENCE:
			ctx->state->composite = comp_op_difference;
			break;
		case COMPOSITE_SOFTLIGHT:
			ctx->state->composite = comp_op_soft_light;
			break;
		case COMPOSITE_HARDLIGHT:
			ctx->state->composite = comp_op_hard_light;
			break;
		case COMPOSITE_BURN:
			ctx->state->composite = comp_op_color_burn;
			break;
		case COMPOSITE_DODGE:
			ctx->state->composite = comp_op_color_dodge;
			break;
		case COMPOSITE_CONTRAST:
			ctx->state->composite = comp_op_contrast;
			break;
		case COMPOSITE_INVERT:
			ctx->state->composite = comp_op_invert;
			break;
		case COMPOSITE_INVERT_BLEND:
			ctx->state->composite = comp_op_invert_rgb;
			break;
		default:
			global_status = STATUS_UNKNOWN_ERROR;
			return op;
	}
	global_status = STATUS_SUCCEED;
	return op;
}

void PICAPI ps_clip(ps_context* ctx)
{
	if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	if (!ctx->path.total_vertices()) {
		global_status = STATUS_SUCCEED;
		return;
	}

	ctx->state->clip.needclip = true;
	_clip_path(ctx, ctx->path, (filling_rule_e)ctx->state->brush.rule);
	ctx->canvas->p->render_clip(ctx, true);
	ctx->path.free_all();
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_clip_path(ps_context* ctx, const ps_path* p, ps_fill_rule r)
{
	if (!ctx || !p) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}	

	ctx->state->clip.needclip = true;
	_clip_path(ctx, p->path, (filling_rule_e)r);
	ctx->canvas->p->render_clip(ctx, true);
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_clip_device_rect(ps_context* ctx, const ps_rect* r)
{
	if (!ctx || !r) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	rect_d tr((float)r->x, (float)r->y, (float)(r->x+r->w), (float)(r->y+r->h));
	trans_affine mtx = ctx->state->world_matrix;
	float rotate = mtx.rotation();
	mtx.rotate(-rotate); //not support rotation
	mtx.transform(&(tr.x1), &(tr.y1));
	mtx.transform(&(tr.x2), &(tr.y2));

	if (ctx->state->clip.cliprect) {
		rect_d cr(float(ctx->state->clip.fs_rect.x),
				  float(ctx->state->clip.fs_rect.y),
				  float(ctx->state->clip.fs_rect.x+ctx->state->clip.fs_rect.w),
				  float(ctx->state->clip.fs_rect.y+ctx->state->clip.fs_rect.h));

		if (cr.clip(tr)) {
			ctx->state->clip.fs_rect.x = cr.x1;
			ctx->state->clip.fs_rect.y = cr.y1;
			ctx->state->clip.fs_rect.w = cr.x2-cr.x1;
			ctx->state->clip.fs_rect.h = cr.y2-cr.y1;
		}
	} else {
		ctx->state->clip.fs_rect.x = tr.x1;
		ctx->state->clip.fs_rect.y = tr.y1;
		ctx->state->clip.fs_rect.w = tr.x2-tr.x1;
		ctx->state->clip.fs_rect.h = tr.y2-tr.y1;
	}
	ctx->state->clip.needclip = true;
	ctx->state->clip.cliprect = true;
	ctx->canvas->p->render_clip(ctx, true);
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_clip_rect(ps_context* ctx, const ps_rect* r)
{
	if (!ctx || !r) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	path_storage path;
	path.move_to(float(r->x), float(r->y));
	path.hline_rel(float(r->w));
	path.vline_rel(float(r->h));
	path.hline_rel(-float(r->w));
	path.end_poly();

	ctx->state->clip.needclip = true;
	_clip_path(ctx, path, fill_non_zero);
	ctx->canvas->p->render_clip(ctx, true);
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_clip_rects(ps_context* ctx, const ps_rect* rs, unsigned int num_rs)
{
	if (!ctx || !rs || !num_rs) {
		global_status = STATUS_INVALID_ARGUMENT;
	 	return;
	}

	path_storage path;
	for (unsigned int i = 0; i < num_rs; i++) {
		path.move_to(float(rs[i].x), float(rs[i].y));
		path.hline_rel(float(rs[i].w));
		path.vline_rel(float(rs[i].h));
		path.hline_rel(-float(rs[i].w));
		path.end_poly();
	}
	ctx->state->clip.needclip = true;
	_clip_path(ctx, path, fill_non_zero);
	ctx->canvas->p->render_clip(ctx, true);
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_reset_clip(ps_context* ctx)
{
	if (!ctx) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	ctx->canvas->p->render_clip(ctx, false);
	ctx->state->clip.rule = fill_non_zero;
	ctx->state->clip.path.free_all();
	ctx->state->clip.cliprect = false;
	ctx->state->clip.needclip = false;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_save(ps_context* ctx)
{
	if (!ctx || !ctx->state) {
		global_status = STATUS_INVALID_ARGUMENT;
	 	return;
	}

	context_state * new_state = new context_state(*ctx->state); //copy constructor
	if (!new_state) {
		global_status = STATUS_OUT_OF_MEMORY;
        	return;
	}

	new_state->next = ctx->state;
	ctx->state = new_state;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_restore(ps_context* ctx)
{
	if (!ctx || !ctx->state->next) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	context_state * old_state = ctx->state;

	ctx->state = ctx->state->next;

	if (old_state->clip.is_not_same(ctx->state->clip)) {
		ctx->canvas->p->render_clip(ctx, false);
		ctx->canvas->p->render_clip(ctx, true);
	}

	if ((old_state->gamma != ctx->state->gamma) 
		|| (old_state->antialias != ctx->state->antialias)) {
		ctx->canvas->p->render_gamma(ctx);
	}

	delete old_state;
	global_status = STATUS_SUCCEED;
}

#ifdef __cplusplus
}
#endif

namespace picasso {

void _clip_path(ps_context* ctx, const path_storage& p, filling_rule_e r)
{
	if (!ctx->state->clip.path.total_vertices()) {
		ctx->state->clip.path = p;
	} else if (p.total_vertices()) {
		path_storage rp;
		_path_operation(gpc_and, ctx->state->clip.path, p, rp);
		ctx->state->clip.path = rp;
	}
	ctx->state->clip.rule = r;
}

}


