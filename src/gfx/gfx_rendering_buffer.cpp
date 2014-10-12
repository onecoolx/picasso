/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "gfx_rendering_buffer.h"

namespace gfx {

gfx_rendering_buffer::gfx_rendering_buffer(byte* ptr, unsigned int width, unsigned int height, int stride)
    : m_transparent(false)
    , m_has_colorkey(false)
    , m_colorkey(0,0,0,0)
{
    init(ptr, width, height, stride);
}

void gfx_rendering_buffer::init(byte* ptr, unsigned int width, unsigned int height, int stride)
{
    m_impl.attach(ptr, width, height, stride);
}

unsigned int gfx_rendering_buffer::width(void) const
{
    return m_impl.width();
}

unsigned int gfx_rendering_buffer::height(void) const
{
    return m_impl.height();
}

int gfx_rendering_buffer::stride(void) const
{
    return m_impl.stride();
}

byte* gfx_rendering_buffer::buffer(void) const
{
    return const_cast<byte*>(m_impl.buf());
}

bool gfx_rendering_buffer::is_transparent(void) const
{
    return m_transparent;
}

void gfx_rendering_buffer::set_transparent(bool b)
{
    m_transparent = b;
}

bool gfx_rendering_buffer::has_color_channel(void) const
{
    return m_has_colorkey;
}

void gfx_rendering_buffer::clear_color_channel(void)
{
    m_has_colorkey = false;
    m_colorkey = rgba(0,0,0,0);
}

void gfx_rendering_buffer::set_color_channel(const rgba& c)
{
    m_has_colorkey = true;
    m_colorkey = c;
}

rgba gfx_rendering_buffer::get_color_channel(void) const
{
    return m_colorkey;
}

}
