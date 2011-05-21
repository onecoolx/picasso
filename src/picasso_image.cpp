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
ps_image* PICAPI ps_image_create(ps_color_format fmt, int w, int h)
{
    if (w <= 0 || h <= 0) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }

    ps_image *img = (ps_image*)malloc(sizeof(ps_image));
    if (img) {
        img->refcount = 1;
		img->fmt = fmt;
		img->host = 0;
		img->transparent = false;
		img->colorkey = false;
		new ((void*)&(img->key)) rgba8;
		new ((void*)&(img->buffer)) rendering_buffer; 
		int pitch = _byte_pre_color(fmt) * w;
		int8u* buf = 0;
		if ((buf = (int8u*)BufferAlloc(h * pitch))) {
			img->flage = buffer_alloc_surface;
			img->buffer.attach(buf, w, h, pitch);
    		global_status = STATUS_SUCCEED;
			return img;
		} else if ((buf = (int8u*)malloc(h * pitch))) {
			img->flage = buffer_alloc_malloc;
			img->buffer.attach(buf, w, h, pitch);
    		global_status = STATUS_SUCCEED;
			return img;
		} else {
			free(img);
        	global_status = STATUS_OUT_OF_MEMORY;
        	return 0;
		}
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

ps_image* PICAPI ps_image_create_from_data(ps_byte* data, ps_color_format fmt, int w, int h, int pitch)
{
    if (!data || w <= 0 || h <= 0 || pitch <= 0) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }

    ps_image *img = (ps_image*)malloc(sizeof(ps_image));
    if (img) {
        img->refcount = 1;
		img->fmt = fmt;
		img->host = 0;
		img->transparent = false;
		img->colorkey = false;
		new ((void*)&(img->key)) rgba8;
		new ((void*)&(img->buffer)) rendering_buffer; 
		int pitch = _byte_pre_color(fmt) * w;
		int8u* buf = 0;
		if ((buf = (int8u*)BufferAlloc(h * pitch))) {
			BufferCopy(buf, data, pitch*h);
			img->flage = buffer_alloc_surface;
			img->buffer.attach(buf, w, h, pitch);
    		global_status = STATUS_SUCCEED;
			return img;
		} else if ((buf = (int8u*)malloc(h * pitch))) {
			memcpy(buf, data, pitch*h);
			img->flage = buffer_alloc_malloc;
			img->buffer.attach(buf, w, h, pitch);
    		global_status = STATUS_SUCCEED;
			return img;
		} else {
			free(img);
        	global_status = STATUS_OUT_OF_MEMORY;
        	return 0;
		}
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

ps_image* PICAPI ps_image_create_with_data(ps_byte* data, ps_color_format fmt, int w, int h, int pitch)
{
    if (!data || w <= 0 || h <= 0 || pitch <= 0) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }

    ps_image *img = (ps_image*)malloc(sizeof(ps_image));
    if (img) {
        img->refcount = 1;
		img->fmt = fmt;
		img->host = 0;
		img->transparent = false;
		img->colorkey = false;
		img->flage = buffer_alloc_none;
		new ((void*)&(img->key)) rgba8;
		new ((void*)&(img->buffer)) rendering_buffer; 
        img->buffer.attach(data, w, h, pitch);
    	global_status = STATUS_SUCCEED;
        return img;
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

ps_image* PICAPI ps_image_create_compatible(const ps_canvas* c, int w, int h)
{
    if (!c) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }

	if (w <= 0)
		w = c->buffer.width();

	if (h <= 0)
		h = c->buffer.height();

    ps_image *img = (ps_image*)malloc(sizeof(ps_image));
    if (img) {
        img->refcount = 1;
		img->fmt = c->fmt;
		img->host = 0;
		img->transparent = false;
		img->colorkey = false;
		new ((void*)&(img->key)) rgba8;
		new ((void*)&(img->buffer)) rendering_buffer; 
		int pitch = _byte_pre_color(c->fmt) * w;
		int8u* buf = 0;
		if ((buf = (int8u*)BufferAlloc(h * pitch))) {
			img->flage = buffer_alloc_surface;
			img->buffer.attach(buf, w, h, pitch);
    		global_status = STATUS_SUCCEED;
			return img;
		} else if ((buf = (int8u*)malloc(h * pitch))) {
			img->flage = buffer_alloc_malloc;
			img->buffer.attach(buf, w, h, pitch);
    		global_status = STATUS_SUCCEED;
			return img;
		} else {
			free(img);
        	global_status = STATUS_OUT_OF_MEMORY;
        	return 0;
		}
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

ps_image* PICAPI ps_image_create_from_image(ps_image* i, const ps_rect* r)
{
	if (!i) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
	}

	ps_rect rc = {0, 0, i->buffer.width(), i->buffer.height()}; 
	if (!r) {
		//Note: if rect is NULL, It equal reference.
    	global_status = STATUS_SUCCEED;
		return ps_image_ref(i);
	} else {
		if (r->x > 0)
			rc.x = r->x;
		if (r->y > 0)
			rc.y = r->y;
		if (r->w > 0)
			rc.w = r->w;
		if (r->h > 0)
			rc.h = r->h;
	}

    ps_image *img = (ps_image*)malloc(sizeof(ps_image));
    if (img) {
        img->refcount = 1;
		img->fmt = i->fmt;
		img->flage = buffer_alloc_image;
		img->host = (void*)ps_image_ref(i);
		img->transparent = i->transparent;
		img->colorkey = i->colorkey;
		img->key = i->key;
		int bpp = _byte_pre_color(i->fmt);
		new ((void*)&(img->buffer)) rendering_buffer; 
        img->buffer.attach(i->buffer.buf()+iround(float(rc.y*i->buffer.stride()+rc.x*bpp)), 
						   			iround((float)rc.w), iround((float)rc.h), i->buffer.stride());
    	global_status = STATUS_SUCCEED;
        return img;
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

ps_image* PICAPI ps_image_create_from_canvas(ps_canvas* c, const ps_rect* r)
{
	if (!c) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
	}

	ps_rect rc = {0, 0, c->buffer.width(), c->buffer.height()}; 
	if (r) {
		if (r->x > 0)
			rc.x = r->x;
		if (r->y > 0)
			rc.y = r->y;
		if (r->w > 0)
			rc.w = r->w;
		if (r->h > 0)
			rc.h = r->h;
	}

    ps_image *img = (ps_image*)malloc(sizeof(ps_image));
    if (img) {
        img->refcount = 1;
		img->fmt = c->fmt;
		img->flage = buffer_alloc_canvas;
		img->host = (void*)ps_canvas_ref(c);
		img->transparent = true;
		img->colorkey = false;
		int bpp = _byte_pre_color(c->fmt);
		new ((void*)&(img->key)) rgba8;
		new ((void*)&(img->buffer)) rendering_buffer; 
        img->buffer.attach(c->buffer.buf()+iround((float)(rc.y*c->buffer.stride()+rc.x*bpp)), 
						   			iround((float)rc.w), iround((float)rc.h), c->buffer.stride());
    	global_status = STATUS_SUCCEED;
        return img;
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}


ps_image* PICAPI ps_image_ref(ps_image* img)
{
    if (!img) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }

    img->refcount++;
    global_status = STATUS_SUCCEED;
    return img;
}

void PICAPI ps_image_unref(ps_image* img)
{
    if (!img) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    img->refcount--;
    if (img->refcount <= 0) {
		if (img->flage == buffer_alloc_surface)
			BufferFree(img->buffer.buf());
		else if (img->flage == buffer_alloc_malloc)
			free(img->buffer.buf());
		else if (img->flage == buffer_alloc_image)
			ps_image_unref(static_cast<ps_image*>(img->host));
		else if (img->flage == buffer_alloc_canvas)
			ps_canvas_unref(static_cast<ps_canvas*>(img->host));

        (&img->buffer)->rendering_buffer::~rendering_buffer();
		(&img->key)->rgba8::~rgba8();
        free(img);
    }
    global_status = STATUS_SUCCEED;
}

ps_size PICAPI ps_image_get_size(const ps_image* img)
{
	ps_size size = {0 , 0};
    if (!img) {
		global_status = STATUS_INVALID_ARGUMENT;
        return size;
    }

	size.w = img->buffer.width();
	size.h = img->buffer.height();
    global_status = STATUS_SUCCEED;
	return size;
}

ps_color_format PICAPI ps_image_get_format(const ps_image* img)
{
    if (!img) {
		global_status = STATUS_INVALID_ARGUMENT;
        return COLOR_FORMAT_UNKNOWN;
    }
    global_status = STATUS_SUCCEED;
	return img->fmt;
}

void PICAPI ps_image_set_allow_transparent(ps_image* img, ps_bool a)
{
    if (!img) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	img->transparent = a ? true : false;
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_image_set_transparent_color(ps_image* img, const ps_color* c)
{
    if (!img) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
	
	if (!c) {
		img->colorkey = false;
	} else {
		img->colorkey = true;
		img->key = rgba8(rgba((float)c->r,(float)c->g,(float)c->b,(float)c->a));
	}
    global_status = STATUS_SUCCEED;
}

#ifdef __cplusplus
}
#endif
