/* Picasso - a vector graphics library
 *
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_RASTER_ADAPTER_H_
#define _GFX_RASTER_ADAPTER_H_

#include "common.h"
#include "interfaces.h"
#include "matrix.h"

#include "gfx_rasterizer_scanline.h"

namespace gfx {

class gfx_raster_adapter_impl;

class gfx_raster_adapter : public abstract_raster_adapter
{
public:
    gfx_raster_adapter();
    virtual ~gfx_raster_adapter();

    virtual void set_gamma_power(scalar g);
    virtual void set_antialias(bool b);
    virtual void set_transform(const trans_affine* mtx);
    virtual void set_raster_method(uint32_t m);

    virtual void add_shape(const vertex_source& vs, uint32_t id);
    virtual void reset(void);

    virtual void set_stroke_dashes(scalar start, const scalar* dashes, uint32_t num);
    virtual void set_stroke_attr(int32_t idx, int32_t val);
    virtual void set_stroke_attr_val(int32_t idx, scalar val);
    virtual void set_fill_attr(int32_t idx, int32_t val);

    virtual void commit(void);
    virtual bool is_empty(void);
    virtual bool contains(scalar x, scalar y);

    uint32_t raster_method(void) const;
    gfx_rasterizer_scanline_aa<>& stroke_impl(void) { return m_sraster; }
    gfx_rasterizer_scanline_aa<>& fill_impl(void) { return m_fraster; }
    trans_affine transformation(void) const;
private:
    void setup_stroke_raster(void);
    void setup_fill_raster(void);

    gfx_raster_adapter_impl* m_impl;
    gfx_rasterizer_scanline_aa<> m_sraster;
    gfx_rasterizer_scanline_aa<> m_fraster;
};

}
#endif /*_GFX_RASTER_ADAPTER_H_*/
