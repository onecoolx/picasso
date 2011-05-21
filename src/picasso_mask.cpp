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

#ifdef __cplusplus
extern "C" {
#endif

ps_mask* PICAPI ps_mask_create_with_data(ps_byte* data, int w, int h)
{
	if (!data || w <= 0 || h <= 0) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
	}
	
	ps_mask *p = (ps_mask*)malloc(sizeof(ps_mask));
	if (p) {
        p->refcount = 1;
		p->color_mask = false;
		new ((void*)&(p->colors)) pod_bvector<rgba8>;
		new ((void*)&(p->mask_buffer)) rendering_buffer;
		p->mask_buffer.attach(data, w, h, w); //gray color format
    	global_status = STATUS_SUCCEED;
		return p;
	} else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
	}
}

ps_mask* PICAPI ps_mask_ref(ps_mask* mask)
{
	if (!mask) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
	}

	mask->refcount++;
    global_status = STATUS_SUCCEED;
    return mask;
}

void PICAPI ps_mask_unref(ps_mask* mask)
{
	if (!mask) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
	}

	mask->refcount--;
	if (mask->refcount <= 0) {
		mask->colors.free_all();
		(&mask->colors)->pod_bvector<rgba8>::~pod_bvector();
        (&mask->mask_buffer)->rendering_buffer::~rendering_buffer();
        free(mask);
	}
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_mask_add_color_filter(ps_mask* mask, const ps_color* c)
{
	if (!mask || !c) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}

	mask->color_mask = true;
	mask->colors.add(rgba8(rgba((float)c->r, (float)c->g, (float)c->b, (float)c->a))); 
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_mask_clear_color_filters(ps_mask* mask)
{
	if (!mask) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	mask->colors.free_all();
	mask->color_mask = false;
	global_status = STATUS_SUCCEED;
}

#ifdef __cplusplus
}
#endif
