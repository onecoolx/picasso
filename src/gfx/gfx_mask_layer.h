/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2012 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_MASK_LAYER_H_
#define _GFX_MASK_LAYER_H_

#include "common.h"
#include "interfaces.h"

#include "picasso_mask.h"
#include "aggheader.h"

namespace gfx {

class gfx_mask_layer : public abstract_mask_layer
{
public:
    gfx_mask_layer(byte* buf, unsigned int width, unsigned int height, int stride) 
        : m_type(MASK_ALPHA)
    {
        attach(buf, width, height, stride);
    }

    virtual ~gfx_mask_layer() 
    {
        m_colors.free_all();
    }

    virtual void attach(byte* buf, unsigned int width, unsigned int height, int stride)
    {
        m_buffer.attach(buf, width, height, stride);
    }

    virtual void set_mask_type(int t)
    {
        m_type = t;
    }

    virtual void add_filter_color(const picasso::rgba& c)
    {
        m_colors.add(agg::rgba8(agg::rgba(c)));
    }

    virtual void clear_filter_colors(void)
    {
        m_colors.free_all();
    }

    int type(void) const { return m_type; }
    agg::rendering_buffer& buffer(void) { return m_buffer; } 
    agg::pod_bvector<agg::rgba8>& colors(void) { return m_colors; }
private:
    int m_type;
    agg::rendering_buffer m_buffer;
    agg::pod_bvector<agg::rgba8> m_colors;
};

}

#endif /*_GFX_MASK_LAYER_H_*/
