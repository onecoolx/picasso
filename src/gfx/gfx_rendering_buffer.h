/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_RENDERING_BUFFER_H_
#define _GFX_RENDERING_BUFFER_H_

#include "common.h"
#include "interfaces.h"
#include "aggheader.h"

namespace gfx {

class gfx_rendering_buffer : public abstract_rendering_buffer
{
public:
    gfx_rendering_buffer(byte* ptr, unsigned int width, unsigned int height, int stride);

    virtual void init(byte* ptr, unsigned int width, unsigned int height, int stride);

    virtual unsigned int width(void) const;
    virtual unsigned int height(void) const;
    virtual int stride(void) const;

    virtual byte* buffer(void) const;
    virtual bool is_transparent(void) const;
    virtual void set_transparent(bool b);

    virtual bool has_color_channel(void) const;
    virtual void clear_color_channel(void);
    virtual void set_color_channel(const rgba&);
    virtual rgba get_color_channel(void) const;
    agg::rendering_buffer& impl(void) { return m_impl; }
private:
    agg::rendering_buffer m_impl;
    bool m_transparent;
    bool m_has_colorkey;
    rgba m_colorkey;
};

}
#endif /*_GFX_RENDERING_BUFFER_H_*/
