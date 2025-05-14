/* Picasso - a vector graphics library
 *
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PICASSO_PAINTER_H_
#define _PICASSO_PAINTER_H_

#include "common.h"
#include "device.h"
#include "vertex.h"
#include "color_type.h"
#include "interfaces.h"

namespace picasso {

class context_state;

class rendering_buffer;
class raster_adapter;
class font_adapter;
class graphic_path;
class trans_affine;
class mask_layer;
class font;

class painter
{
public:
    painter(pix_fmt fmt);
    ~painter();
public:
    void attach(rendering_buffer& buf);

    void render_stroke(context_state* state, raster_adapter& raster, const graphic_path& p);
    void render_fill(context_state* state, raster_adapter& raster, const graphic_path& p);
    void render_paint(context_state* state, raster_adapter& raster, const graphic_path& p);
    void render_clear(context_state* state);
    void render_blur(context_state* state);
    void render_gamma(context_state* state, raster_adapter& raster);
    void render_clip(context_state* state, bool clip);
    void render_shadow(context_state* state, const graphic_path& p, bool fill, bool stroke);

    void render_mask(const mask_layer& m, bool mask);
    void render_copy(rendering_buffer& src, const rect* rect, const painter* dst, int32_t off_x, int32_t off_y);

    void render_glyph(context_state* state, raster_adapter& raster, const font* f, int32_t type);
    void render_glyphs_raster(context_state* state, raster_adapter& raster, int32_t style);
private:
    void init_raster_data(context_state*, uint32_t, raster_adapter&, const vertex_source&, const trans_affine&);
    void init_source_data(context_state*, uint32_t, const graphic_path&);
private:
    abstract_painter* m_impl;
};

}
#endif /*_PICASSO_PAINTER_H_*/
