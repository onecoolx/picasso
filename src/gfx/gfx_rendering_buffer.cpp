/* Picasso - a vector graphics library
 *
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "gfx_rendering_buffer.h"

namespace gfx {

gfx_rendering_buffer::gfx_rendering_buffer()
    : m_buffer(0)
    , m_width(0)
    , m_height(0)
    , m_stride(0)
    , m_transparent(false)
    , m_has_colorkey(false)
    , m_colorkey(0, 0, 0, 0)
{
}

gfx_rendering_buffer::gfx_rendering_buffer(byte* ptr, uint32_t width, uint32_t height, int32_t stride)
    : m_buffer(0)
    , m_observer(0)
    , m_width(0)
    , m_height(0)
    , m_stride(0)
    , m_transparent(false)
    , m_has_colorkey(false)
    , m_colorkey(0, 0, 0, 0)
{
    init(ptr, width, height, stride);
}

void gfx_rendering_buffer::replace(byte* ptr, uint32_t width, uint32_t height, int32_t stride)
{
    init(ptr, width, height, stride);
    notify_buffer_changed();
}

void gfx_rendering_buffer::init(byte* ptr, uint32_t width, uint32_t height, int32_t stride)
{
    m_buffer = ptr;
    m_width = width;
    m_height = height;
    m_stride = stride;

    if (height > m_rows.size()) {
        m_rows.resize(height);
    }

    byte* p = m_buffer;
    if (stride < 0) {
        p = m_buffer - (int32_t)(height - 1) * stride;
    }

    byte** rows = &m_rows[0];
    while (height--) {
        *rows++ = p;
        p += stride;
    }
}

}
