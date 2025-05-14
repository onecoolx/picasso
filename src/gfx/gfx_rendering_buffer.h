/* Picasso - a vector graphics library
 *
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_RENDERING_BUFFER_H_
#define _GFX_RENDERING_BUFFER_H_

#include "common.h"
#include "interfaces.h"
#include "data_vector.h"
#include "color_type.h"

namespace gfx {

class gfx_rendering_buffer : public abstract_rendering_buffer
{
public:
    // const_row_info
    typedef struct const_row_info {
        int32_t x1, x2;
        const byte* ptr;

        const_row_info() : x1(0), x2(0), ptr(0) { }
        const_row_info(int32_t _x1, int32_t _x2, const byte* _ptr)
            : x1(_x1), x2(_x2), ptr(_ptr)
        {
        }
    } row_data;

    class gfx_buffer_observer
    {
    public:
        virtual ~gfx_buffer_observer() {}
        virtual void buffer_notify(void) = 0;
    };

    gfx_rendering_buffer();
    gfx_rendering_buffer(byte* ptr, uint32_t width, uint32_t height, int32_t stride);

    virtual void init(byte* ptr, uint32_t width, uint32_t height, int32_t stride);
    virtual void replace(byte* ptr, uint32_t width, uint32_t height, int32_t stride);

    virtual uint32_t width(void) const { return m_width; }
    virtual uint32_t height(void) const { return m_height; }
    virtual int32_t stride(void) const { return m_stride; }
    virtual byte* buffer(void) const { return m_buffer; }

    virtual bool is_transparent(void) const { return m_transparent; }
    virtual void set_transparent(bool t) { m_transparent = t; }

    virtual bool has_color_channel(void) const { return m_has_colorkey; }
    virtual void clear_color_channel(void)
    {
        m_has_colorkey = false;
        m_colorkey = rgba(0, 0, 0, 0);
    }

    virtual rgba get_color_channel(void) const { return m_colorkey; }
    virtual void set_color_channel(const rgba& c)
    {
        m_has_colorkey = true;
        m_colorkey = c;
    }

public:
    byte* row_ptr(int32_t y) const { return m_rows[y]; }
    byte* row_ptr(int32_t, int32_t y, uint32_t) const { return m_rows[y]; }
    row_data row (int32_t y) const { return row_data(0, m_width - 1, row_ptr(y)); }
    // for optimization
    uint32_t internal_width(void) const { return m_width; }
    uint32_t internal_height(void) const { return m_height; }
    int32_t internal_stride(void) const { return m_stride; }

    void set_buffer_observer(gfx_buffer_observer* o) { m_observer = o; }
    void notify_buffer_changed(void) { if (m_observer) m_observer->buffer_notify(); }
private:
    gfx_rendering_buffer(const gfx_rendering_buffer&);
    gfx_rendering_buffer& operator=(const gfx_rendering_buffer&);

    byte* m_buffer;
    gfx_buffer_observer* m_observer;
    pod_array<byte*> m_rows;
    uint32_t m_width; // width in pixels
    uint32_t m_height; // height in pixels
    int32_t m_stride; // number of bytes per row, link pitch. can < 0
    // extra property
    bool m_transparent;
    bool m_has_colorkey;
    rgba m_colorkey;
};

}
#endif /*_GFX_RENDERING_BUFFER_H_*/
