/* Picasso - a vector graphics library
 *
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _OBJ_INTERFACES_H_
#define _OBJ_INTERFACES_H_

#include "common.h"
#include "math_type.h"
#include "matrix.h"
#include "vertex.h"
#include "color_type.h"
#include "graphic_base.h"

namespace picasso {

// Rendering buffer interface
class abstract_rendering_buffer
{
public:
    virtual ~abstract_rendering_buffer() {}

    virtual void init(byte* ptr, unsigned int width, unsigned int height, int stride) = 0;
    virtual void replace(byte* ptr, unsigned int width, unsigned int height, int stride) = 0;

    virtual unsigned int width(void) const = 0;
    virtual unsigned int height(void) const = 0;
    virtual int stride(void) const = 0;
    virtual byte* buffer(void) const = 0;

    virtual bool is_transparent(void) const = 0;
    virtual void set_transparent(bool b) = 0;

    virtual bool has_color_channel(void) const = 0;
    virtual void clear_color_channel(void) = 0;
    virtual void set_color_channel(const rgba&) = 0;
    virtual rgba get_color_channel(void) const = 0;
protected:
    abstract_rendering_buffer() {}
private:
    abstract_rendering_buffer(const abstract_rendering_buffer&);
    abstract_rendering_buffer& operator=(const abstract_rendering_buffer&);
};

//Raster adapter interface
class abstract_raster_adapter
{
public:
    virtual ~abstract_raster_adapter() {}

    virtual void set_gamma_power(scalar) = 0;
    virtual void set_antialias(bool) = 0;
    virtual void set_transform(const trans_affine* mtx) = 0;
    virtual void set_raster_method(unsigned int methods) = 0;
    virtual void set_stroke_dashes(scalar start, const scalar* dashes, unsigned int num) = 0;
    virtual void set_stroke_attr(int idx, int val) = 0;
    virtual void set_stroke_attr_val(int idx, scalar val) = 0;

    virtual void set_fill_attr(int idx, int val) = 0;

    virtual void add_shape(const vertex_source& vs, unsigned int id) = 0;
    virtual void reset(void) = 0;
    virtual void commit(void) = 0;
    virtual bool is_empty(void) = 0;
    virtual bool contains(scalar x, scalar y) = 0;
protected:
    abstract_raster_adapter() {}
private:
    abstract_raster_adapter(const abstract_raster_adapter&);
    abstract_raster_adapter& operator=(const abstract_raster_adapter&);
};

//Masking layer interface
class abstract_mask_layer
{
public:
    virtual ~abstract_mask_layer() {}

    virtual void attach(byte* buf, unsigned int width, unsigned int height, int stride) = 0;
    virtual void set_mask_type(int type) = 0;
    virtual void add_filter_color(const rgba& c) = 0;
    virtual void clear_filter_colors(void) = 0;
protected:
    abstract_mask_layer() {}
private:
    abstract_mask_layer(const abstract_mask_layer&);
    abstract_mask_layer& operator=(const abstract_mask_layer&);
};

//Gradient interface
class abstract_gradient_adapter
{
public:
    virtual ~abstract_gradient_adapter() {}

    virtual void init_linear(int spread, scalar x1, scalar y1, scalar x2, scalar y2) = 0;
    virtual void init_radial(int spread, scalar x1, scalar y1, scalar radius1,
                             scalar x2, scalar y2, scalar radius2) = 0;
    virtual void init_conic(int spread, scalar x, scalar y, scalar angle) = 0;

    virtual void add_color_stop(scalar offset, const rgba& c) = 0;
    virtual void clear_stops(void) = 0;

    virtual void transform(const trans_affine* mtx) = 0;
protected:
    abstract_gradient_adapter() {}
private:
    abstract_gradient_adapter(const abstract_gradient_adapter&);
    abstract_gradient_adapter& operator=(const abstract_gradient_adapter&);
};

// Painter interface
class abstract_painter
{
public:
    virtual ~abstract_painter() {}

    virtual void attach(abstract_rendering_buffer* b) = 0;
    virtual pix_fmt pixel_format(void) const = 0;

    virtual void set_alpha(scalar a) = 0;
    virtual void set_composite(comp_op op) = 0;
    // stroke
    virtual void set_stroke_color(const rgba& c) = 0;
    virtual void set_stroke_image(const abstract_rendering_buffer* img, pix_fmt format, int filter, const rect_s& rc) = 0;
    virtual void set_stroke_canvas(const abstract_rendering_buffer* img, pix_fmt format, int filter, const rect_s& rc) = 0;
    virtual void set_stroke_pattern(const abstract_rendering_buffer* img, pix_fmt format, int filter, const rect_s& rc,
                                    int xtype, int ytype, const trans_affine* mtx) = 0;
    virtual void set_stroke_gradient(const abstract_gradient_adapter* g) = 0;
    // fill
    virtual void set_fill_color(const rgba& c) = 0;
    virtual void set_fill_image(const abstract_rendering_buffer* img, pix_fmt format, int filter, const rect_s& rc) = 0;
    virtual void set_fill_canvas(const abstract_rendering_buffer* img, pix_fmt format, int filter, const rect_s& rc) = 0;
    virtual void set_fill_pattern(const abstract_rendering_buffer* img, pix_fmt format, int filter, const rect_s& rc,
                                  int xtype, int ytype, const trans_affine* mtx) = 0;
    virtual void set_fill_gradient(const abstract_gradient_adapter* g) = 0;
    virtual void set_font_fill_color(const rgba& c) = 0;

    virtual void apply_stroke(abstract_raster_adapter* rs) = 0;
    virtual void apply_fill(abstract_raster_adapter* rs) = 0;

    virtual void apply_text_fill(abstract_raster_adapter* rs, text_style style) = 0;

    // FIXME: own mono storage implements needed!
    virtual void apply_mono_text_fill(void* storage) = 0;

    // clear
    virtual void apply_clear(const rgba& c) = 0;

    // blur
    virtual void apply_blur(scalar blur) = 0;

    // clipping
    virtual void apply_clip_path(const vertex_source& v, int rule, const trans_affine* mtx) = 0;
    virtual void apply_clip_device(const rect_s& rc, scalar xoffset, scalar yoffset) = 0;
    virtual void clear_clip(void) = 0;

    // masking
    virtual void apply_masking(abstract_mask_layer*) = 0;
    virtual void clear_masking(void) = 0;

    // shadow
    virtual bool begin_shadow(const rect_s& rc) = 0;
    virtual void apply_shadow(abstract_raster_adapter* rs, const rect_s& r,
                              const rgba& c, scalar x, scalar y, scalar b) = 0;

    //data copy
    virtual void copy_rect_from(abstract_rendering_buffer* src, const rect& rc, int x, int y) = 0;
protected:
    abstract_painter() {}
private:
    abstract_painter(const abstract_painter&);
    abstract_painter& operator=(const abstract_painter&);
};

}

using picasso::abstract_rendering_buffer;
using picasso::abstract_raster_adapter;
using picasso::abstract_painter;
using picasso::abstract_mask_layer;
using picasso::abstract_gradient_adapter;

#endif/*_OBJ_INTERFACES_H_*/
