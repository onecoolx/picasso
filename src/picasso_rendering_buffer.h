/* Picasso - a vector graphics library
 *
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PICASSO_RENDERING_BUFFER_H_
#define _PICASSO_RENDERING_BUFFER_H_

#include "common.h"
#include "device.h"
#include "color_type.h"
#include "interfaces.h"

namespace picasso {

class rendering_buffer
{
public:
    rendering_buffer();
    rendering_buffer(byte* buf, uint32_t width, uint32_t height, int32_t stride);
    ~rendering_buffer();
public:
    void attach(byte* buf, uint32_t width, uint32_t height, int32_t stride);
    void replace(byte* buf, uint32_t width, uint32_t height, int32_t stride);
    bool is_empty(void) const;

    byte* buffer(void) const;
    uint32_t width(void) const;
    uint32_t height(void) const;
    int32_t stride(void) const;

    bool is_transparent(void) const;
    void set_transparent(bool b);

    bool has_color_channel(void) const;
    void clear_color_channel(void);
    void set_color_channel(const rgba& c);
    rgba get_color_channel(void) const;

    abstract_rendering_buffer* impl(void) const { return m_impl; }
private:
    friend class painter;
    abstract_rendering_buffer* m_impl;
};

}
#endif/*_PICASSO_RENDERING_BUFFER_H_*/
