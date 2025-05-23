/* Picasso - a vector graphics library
 *
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "device.h"

#include "picasso.h"
#include "picasso_global.h"
#include "picasso_mask.h"
#include "picasso_objects.h"

#ifdef __cplusplus
extern "C" {
#endif

ps_mask* PICAPI ps_mask_create(int32_t w, int32_t h)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return NULL;
    }

    if (w <= 0 || h <= 0) {
        global_status = STATUS_INVALID_ARGUMENT;
        return NULL;
    }

    ps_mask* p = (ps_mask*)mem_malloc(sizeof(ps_mask));
    if (p) {
        p->refcount = 1;
        byte* buf = NULL;
        if ((buf = (byte*)BufferAlloc(h * w))) {
            p->flage = buffer_alloc_surface;
            new ((void*) & (p->mask))picasso::mask_layer(buf, w, h, w); //gray color format for alpha
            global_status = STATUS_SUCCEED;
            return p;
        } else if ((buf = (byte*)mem_calloc(h * w, 1))) {
            p->flage = buffer_alloc_malloc;
            new ((void*) & (p->mask))picasso::mask_layer(buf, w, h, w); //gray color format for alpha
            global_status = STATUS_SUCCEED;
            return p;
        } else {
            (&p->mask)->picasso::mask_layer::~mask_layer();
            mem_free(p);
            global_status = STATUS_OUT_OF_MEMORY;
            return NULL;
        }
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return NULL;
    }
}

ps_mask* PICAPI ps_mask_create_with_data(ps_byte* data, int32_t w, int32_t h)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return NULL;
    }

    if (!data || w <= 0 || h <= 0) {
        global_status = STATUS_INVALID_ARGUMENT;
        return NULL;
    }

    ps_mask* p = (ps_mask*)mem_malloc(sizeof(ps_mask));
    if (p) {
        p->refcount = 1;
        p->flage = buffer_alloc_none;
        new ((void*) & (p->mask))picasso::mask_layer(data, w, h, w); //gray color format for alpha
        global_status = STATUS_SUCCEED;
        return p;
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return NULL;
    }
}

ps_mask* PICAPI ps_mask_ref(ps_mask* mask)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return NULL;
    }

    if (!mask) {
        global_status = STATUS_INVALID_ARGUMENT;
        return NULL;
    }

    mask->refcount++;
    global_status = STATUS_SUCCEED;
    return mask;
}

void PICAPI ps_mask_unref(ps_mask* mask)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!mask) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    mask->refcount--;
    if (mask->refcount <= 0) {
        if (mask->flage == buffer_alloc_surface) {
            BufferFree(mask->mask.buffer());
        } else if (mask->flage == buffer_alloc_malloc) {
            mem_free(mask->mask.buffer());
        }
        (&mask->mask)->picasso::mask_layer::~mask_layer();
        mem_free(mask);
    }
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_mask_add_color_filter(ps_mask* mask, const ps_color* c)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!mask || !c) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    mask->mask.set_mask_type(MASK_COLORS);
    mask->mask.add_filter_color(picasso::rgba(FLT_TO_SCALAR(c->r),
                                              FLT_TO_SCALAR(c->g), FLT_TO_SCALAR(c->b), FLT_TO_SCALAR(c->a)));
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_mask_clear_color_filters(ps_mask* mask)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!mask) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    mask->mask.clear_filter_colors();
    mask->mask.set_mask_type(MASK_ALPHA);
    global_status = STATUS_SUCCEED;
}

#ifdef __cplusplus
}
#endif
