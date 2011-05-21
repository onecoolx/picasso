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

template<typename GradientF, typename WapperF>
class gradient_wrapper_adaptor : public gradient_wrapper
{
public:
	gradient_wrapper_adaptor()
		: m_wrapper(m_gradient) { }

	virtual ~gradient_wrapper_adaptor() { }

	virtual void init(float r, float x, float y)
	{
		m_gradient.init(r, x, y);
	}

	virtual int calculate (int x, int y, int d) const 
	{
		return m_wrapper.calculate(x, y, d);
	}

private:
	GradientF m_gradient;
	WapperF m_wrapper;
};

}

#ifdef __cplusplus
extern "C" {
#endif

ps_gradient* PICAPI ps_gradient_create_linear(ps_gradient_spread sp, const ps_point* s, const ps_point* e)
{
	if (!s || !e) {
		global_status = STATUS_INVALID_ARGUMENT;
		return 0;
	}

	gradient_wrapper* wrapper = 0;
	switch (sp) 
	{
	case GRADIENT_SPREAD_PAD:
		wrapper = new gradient_wrapper_adaptor<gradient_x, gradient_pad_adaptor<gradient_x> >;
		break;
	case GRADIENT_SPREAD_REPEAT:
		wrapper = new gradient_wrapper_adaptor<gradient_x, gradient_repeat_adaptor<gradient_x> >;
		break;
	case GRADIENT_SPREAD_REFLECT:
		wrapper = new gradient_wrapper_adaptor<gradient_x, gradient_reflect_adaptor<gradient_x> >;
		break;
	};

	if (!wrapper) {
		global_status = STATUS_OUT_OF_MEMORY;
		return 0;
	}

	double len = calc_distance_d(s->x, s->y, e->x, e->y);

	trans_affine mtx;
	if (len) {
		if (e->x < s->x)
			mtx *= trans_affine_rotation(pi - (float)asin((e->y - s->y) / len));
		else
			mtx *= trans_affine_rotation((float)asin((e->y - s->y) / len));
	} else
		len = 2.0f; // len can not be zero

	mtx *= trans_affine_translation(float(s->x), float(s->y));

	ps_gradient *p = (ps_gradient*)malloc(sizeof(ps_gradient));
	if (p) {
        p->refcount = 1;
		p->wrapper = wrapper;
		p->start = 0;
		p->length = (float)len;
		p->matrix = mtx;
		p->build = false;
		new ((void*)&(p->colors)) gradient_lut<color_interpolator<rgba8> >;
		p->colors.remove_all(); //clear all colors
		global_status = STATUS_SUCCEED;
		return p;
	} else {
		delete wrapper; //free wrapper on error
		global_status = STATUS_OUT_OF_MEMORY;
		return 0;
	}
}

ps_gradient* PICAPI ps_gradient_create_radial(ps_gradient_spread sp, const ps_point* s, double sr, 
																			const ps_point* e, double er)
{
	if (!s || !e) {
		global_status = STATUS_INVALID_ARGUMENT;
		return 0;
	}

	gradient_wrapper* wrapper = 0;
	if ((s->x == e->x) && (s->y == e->y)) {
		switch (sp) 
		{
			case GRADIENT_SPREAD_PAD:
				wrapper = new gradient_wrapper_adaptor<gradient_radial, gradient_pad_adaptor<gradient_radial> >;
				break;
			case GRADIENT_SPREAD_REPEAT:
				wrapper = new gradient_wrapper_adaptor<gradient_radial, gradient_repeat_adaptor<gradient_radial> >;
				break;
			case GRADIENT_SPREAD_REFLECT:
				wrapper = new gradient_wrapper_adaptor<gradient_radial, gradient_reflect_adaptor<gradient_radial> >;
				break;
		};
	} else {
	switch (sp) 
	{
	case GRADIENT_SPREAD_PAD:
		wrapper = new gradient_wrapper_adaptor<gradient_radial_focus, gradient_pad_adaptor<gradient_radial_focus> >;
		break;
	case GRADIENT_SPREAD_REPEAT:
		wrapper = new gradient_wrapper_adaptor<gradient_radial_focus, gradient_repeat_adaptor<gradient_radial_focus> >;
		break;
	case GRADIENT_SPREAD_REFLECT:
		wrapper = new gradient_wrapper_adaptor<gradient_radial_focus, gradient_reflect_adaptor<gradient_radial_focus> >;
		break;
	};
	}
	if (!wrapper) {
        	global_status = STATUS_OUT_OF_MEMORY;
		return 0;
	}

	float len = (float)fabs(er);
	float fx = (float)(s->x - e->x);
	float fy = (float)(s->y - e->y);

	wrapper->init(len, fx, fy);

	if (!len) 
		len = 2.0f;// len can not be zero

	trans_affine mtx;
	mtx *= trans_affine_translation(float(s->x-fx), float(s->y-fy));

	ps_gradient *p = (ps_gradient*)malloc(sizeof(ps_gradient));
	if (p) {
        p->refcount = 1;
		p->wrapper = wrapper;
		p->start = (float)fabs(sr);
		p->length = len;
		p->matrix = mtx;
		p->build = false;
		new ((void*)&(p->colors)) gradient_lut<color_interpolator<rgba8> >;
		p->colors.remove_all(); //clear all colors
		global_status = STATUS_SUCCEED;
		return p;
	} else {
		delete wrapper; //free wrapper on error
		global_status = STATUS_OUT_OF_MEMORY;
		return 0;
	}
}

ps_gradient* PICAPI ps_gradient_create_conic(ps_gradient_spread s, const ps_point* o, double a)
{
	if (!o) {
		global_status = STATUS_INVALID_ARGUMENT;
		return 0;
	}

	// only support reflect 
	gradient_wrapper* wrapper = new gradient_wrapper_adaptor<gradient_conic, gradient_reflect_adaptor<gradient_conic> >;

	if (!wrapper) {
		global_status = STATUS_OUT_OF_MEMORY;
		return 0;
	}

	trans_affine mtx;
	mtx *= trans_affine_translation(float(o->x), float(o->y));
	mtx *= trans_affine_translation(-float(o->x), -float(o->y));
	mtx *= trans_affine_rotation(float(a));
	mtx *= trans_affine_translation(float(o->x), float(o->y));

	ps_gradient *p = (ps_gradient*)malloc(sizeof(ps_gradient));
	if (p) {
        p->refcount = 1;
		p->wrapper = wrapper;
		p->start = 0;
		p->length = 128;
		p->matrix = mtx;
		p->build = false;
		new ((void*)&(p->colors)) gradient_lut<color_interpolator<rgba8> >;
		p->colors.remove_all(); //clear all colors
		global_status = STATUS_SUCCEED;
		return p;
	} else {
		delete wrapper; //free wrapper on error
		global_status = STATUS_OUT_OF_MEMORY;
		return 0;
	}
}

ps_gradient* PICAPI ps_gradient_ref(ps_gradient* g)
{
	if (!g) {
		global_status = STATUS_INVALID_ARGUMENT;
		return 0;
	}

	g->refcount++;
	global_status = STATUS_SUCCEED;
	return g;
}

void PICAPI ps_gradient_unref(ps_gradient* g)
{
	if (!g) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	g->refcount--;
	if (g->refcount <= 0) {
		(&g->colors)->gradient_lut<color_interpolator<rgba8> >::~gradient_lut();
		delete g->wrapper;
		free(g);
	}
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_gradient_clear_color_stops(ps_gradient* g)
{
	if (!g) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	g->colors.remove_all();
	g->build = false;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_gradient_add_color_stop(ps_gradient* g, double off, const ps_color* c)
{
	if (!g || !c) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	if (off < 0) 
		off = 0.0;
	else if (off > 1)
		off = 1.0;

	g->colors.add_color(float(off), rgba8(rgba((float)c->r, (float)c->g, (float)c->b, (float)c->a)));
	g->build = false;
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_gradient_transform(ps_gradient* g, const ps_matrix* m)
{
	if (!g || !m) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	g->matrix *= m->matrix;
	global_status = STATUS_SUCCEED;
}

#ifdef __cplusplus
}
#endif
