/* Picasso - a vector graphics library
 *
 * Copyright (C) 2013 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PICASSO_MASK_H_
#define _PICASSO_MASK_H_

#include "common.h"
#include "device.h"
#include "interfaces.h"

enum {
    MASK_ALPHA,
    MASK_COLORS,
};

namespace picasso {

class mask_layer
{
public:
    mask_layer(byte* buf, uint32_t width, uint32_t height, int32_t stride, int32_t type = MASK_ALPHA);
    ~mask_layer();
public:
    void attach(byte* buf, uint32_t width, uint32_t height, int32_t stride);

    void set_mask_type(int32_t type);
    void add_filter_color(const rgba& c);
    void clear_filter_colors(void);

    uint32_t width(void) const { return m_width; }
    uint32_t height(void) const { return m_height; }
    int32_t stride(void) const { return m_stride; }
    byte* buffer(void) const { return m_buf; }

    abstract_mask_layer* impl(void) const { return m_impl; }
private:
    mask_layer(const mask_layer&);
    mask_layer& operator = (const mask_layer&);
    abstract_mask_layer* m_impl;
    byte* m_buf;
    uint32_t m_width;
    uint32_t m_height;
    int32_t m_stride;
};

}
#endif /*_PICASSO_MASK_H_*/
