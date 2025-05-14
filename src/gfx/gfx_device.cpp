/* Picasso - a vector graphics library
 *
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "math_type.h"
#include "color_type.h"

#include "gfx_device.h"
#include "gfx_painter.h"
#include "gfx_raster_adapter.h"
#include "gfx_gradient_adapter.h"
#include "gfx_mask_layer.h"
#include "gfx_pixfmt_rgba.h"
#include "gfx_pixfmt_rgb.h"
#include "gfx_pixfmt_rgb16.h"
#include "gfx_pixfmt_gray.h"

namespace gfx {

gfx_device* gfx_device::create(void)
{
    return new gfx_device;
}

gfx_device::gfx_device()
{
}

gfx_device::~gfx_device()
{
}

abstract_painter* gfx_device::create_painter(pix_fmt fmt)
{
    switch (fmt) {
#if ENABLE(FORMAT_RGBA)
        case pix_fmt_rgba:
            return new gfx_painter<pixfmt_rgba32>;
#endif
#if ENABLE(FORMAT_ARGB)
        case pix_fmt_argb:
            return new gfx_painter<pixfmt_argb32>;
#endif
#if ENABLE(FORMAT_ABGR)
        case pix_fmt_abgr:
            return new gfx_painter<pixfmt_abgr32>;
#endif
#if ENABLE(FORMAT_BGRA)
        case pix_fmt_bgra:
            return new gfx_painter<pixfmt_bgra32>;
#endif
#if ENABLE(FORMAT_RGB)
        case pix_fmt_rgb:
            return new gfx_painter<pixfmt_rgb24>;
#endif
#if ENABLE(FORMAT_BGR)
        case pix_fmt_bgr:
            return new gfx_painter<pixfmt_bgr24>;
#endif
#if ENABLE(FORMAT_RGB565)
        case pix_fmt_rgb565:
            return new gfx_painter<pixfmt_rgb565>;
#endif
#if ENABLE(FORMAT_RGB555)
        case pix_fmt_rgb555:
            return new gfx_painter<pixfmt_rgb555>;
#endif
#if ENABLE(FORMAT_A8)
        case pix_fmt_gray8:
            return new gfx_painter<pixfmt_gray8>;
#endif
        default:
            return 0;
    }
}

void gfx_device::destroy_painter(abstract_painter* p)
{
    delete p;
}

abstract_raster_adapter* gfx_device::create_raster_adapter(void)
{
    return new gfx_raster_adapter;
}

void gfx_device::destroy_raster_adapter(abstract_raster_adapter* d)
{
    delete d;
}

abstract_rendering_buffer* gfx_device::create_rendering_buffer(byte* buf,
                                                               uint32_t width, uint32_t height, int32_t stride)
{
    return new gfx_rendering_buffer(buf, width, height, stride);
}

void gfx_device::destroy_rendering_buffer(abstract_rendering_buffer* b)
{
    delete b;
}

abstract_mask_layer* gfx_device::create_mask_layer(byte* buf,
                                                   uint32_t width, uint32_t height, int32_t stride)
{
    return new gfx_mask_layer(buf, width, height, stride);
}

void gfx_device::destroy_mask_layer(abstract_mask_layer* m)
{
    delete m;
}

abstract_gradient_adapter* gfx_device::create_gradient_adapter(void)
{
    return new gfx_gradient_adapter();
}

void gfx_device::destroy_gradient_adapter(abstract_gradient_adapter* g)
{
    delete g;
}

} //namespace gfx
