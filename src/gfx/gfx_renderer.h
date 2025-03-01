/* Picasso - a vector graphics library
 *
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_RENDERER_H_
#define _GFX_RENDERER_H_

#include "common.h"
#include "graphic_helper.h"

namespace gfx {

// graphics renderer with clip.
template <typename PixelFormat>
class gfx_renderer : public gfx_rendering_buffer::gfx_buffer_observer
{
public:
    typedef PixelFormat pixfmt_type;
    typedef typename pixfmt_type::color_type color_type;

    gfx_renderer()
        : m_pixfmt(0)
        , m_clip_rect(1, 1, 0, 0)
        , m_is_path_clip(false)
    {
    }

    explicit gfx_renderer(pixfmt_type& fmt)
        : m_pixfmt(&fmt)
        , m_clip_rect(0, 0, fmt.width() - 1, fmt.height() - 1)
        , m_is_path_clip(false)
    {
    }

    virtual ~gfx_renderer()
    {
        m_clip_path.reset();
    }

    virtual void buffer_notify(void)
    {
        // reset clip_rect
        reset_clipping(true);
    }

    void attach(pixfmt_type& fmt)
    {
        m_pixfmt = &fmt;
        m_clip_rect = rect(0, 0, fmt.width() - 1, fmt.height() - 1);
        m_clip_path.reset();
        m_is_path_clip = false;
    }

    const rect& clip_rect(void) const { return m_clip_rect; }

    int32_t xmin(void) const { return m_clip_rect.x1; }
    int32_t ymin(void) const { return m_clip_rect.y1; }
    int32_t xmax(void) const { return m_clip_rect.x2; }
    int32_t ymax(void) const { return m_clip_rect.y2; }

    uint32_t width(void) const { return m_pixfmt->width(); }
    uint32_t height(void) const { return m_pixfmt->height(); }

    bool clip_rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
    {
        rect cb(x1, y1, x2, y2);
        cb.normalize();

        if (cb.clip(rect(0, 0, width() - 1, height() - 1))) {
            m_clip_rect = cb;
            return true;
        }

        m_clip_rect.x1 = 1;
        m_clip_rect.y1 = 1;
        m_clip_rect.x2 = 0;
        m_clip_rect.y2 = 0;
        return false;
    }

    void add_clipping(vertex_source& p, filling_rule f)
    {
        m_is_path_clip = true;
        m_clip_path.reset();
        m_clip_path.filling(f);
        m_clip_path.add_path(p);

        rect_s cb;
        picasso::bounding_rect(p, 0, &cb.x1, &cb.y1, &cb.x2, &cb.y2);
        rect cbi(SCALAR_TO_INT(cb.x1), SCALAR_TO_INT(cb.y1),
                 SCALAR_TO_INT(cb.x2), SCALAR_TO_INT(cb.y2));
        m_clip_rect.clip(cbi);
    }

    void reset_clipping(bool visibility)
    {
        m_clip_rect = rect(0, 0, width() - 1, height() - 1);
        m_clip_path.reset();
        m_is_path_clip = false;
    }

    void clear(const color_type& c)
    {
        if (m_is_path_clip) { // copy for per pixel.
            for (uint32_t i = 0; i < height(); i++)
                for (uint32_t j = 0; j < width(); j++)
                    if (pixel_in_path(j, i)) {
                        m_pixfmt->copy_pixel(j, i, c);
                    }
        } else {
            int32_t x = m_clip_rect.x1;
            int32_t y = m_clip_rect.y1;
            int32_t w = m_clip_rect.x2 - m_clip_rect.x1 + 1;
            int32_t h = m_clip_rect.y2 - m_clip_rect.y1 + 1;

            if (w) {
                for (int32_t i = 0; i < h; i++) {
                    m_pixfmt->copy_hline(x, y + i, w, c);
                }
            }
        }
    }

    void copy_absolute_from(const gfx_rendering_buffer& from,
                            const rect* rect_src_ptr = 0, int32_t dx = 0, int32_t dy = 0)
    {
        rect rsrc(0, 0, from.width(), from.height());
        if (rect_src_ptr) {
            rsrc.x1 = rect_src_ptr->x1;
            rsrc.y1 = rect_src_ptr->y1;
            rsrc.x2 = rect_src_ptr->x2 + 1;
            rsrc.y2 = rect_src_ptr->y2 + 1;
        }

        rect rdst(dx, dy, rsrc.x2 - rsrc.x1 + dx, rsrc.y2 - rsrc.y1 + dy);
        rect rc = clip_rect_area(rdst, rsrc, from.width(), from.height());

        if (m_is_path_clip) {
            if (rc.x2 > 0 && rc.y2 > 0) {
                int32_t incy = 1;
                int32_t incx = 1;

                if (rdst.y1 > rsrc.y1) {
                    rsrc.y1 += rc.y2 - 1;
                    rdst.y1 += rc.y2 - 1;
                    incy = -1;
                }

                if (rdst.x1 > rsrc.x1) {
                    rsrc.x1 += rc.x2 - 1;
                    rdst.x1 += rc.x2 - 1;
                    incx = -1;
                }

                while (rc.x2 > 0) {
                    int32_t y2 = rc.y2;
                    int32_t dy1 = rdst.y1;
                    int32_t sy1 = rsrc.y1;
                    while (y2 > 0) {
                        if (pixel_in_path(rdst.x1, dy1)) {
                            m_pixfmt->copy_point_from(from, rdst.x1, dy1, rsrc.x1, sy1);
                        }
                        dy1 += incy;
                        sy1 += incy;
                        --y2;
                    }
                    rdst.x1 += incx;
                    rsrc.x1 += incx;
                    --rc.x2;
                }
            }
        } else {
            if (rc.x2 > 0) {
                int32_t incy = 1;

                if (rdst.y1 > rsrc.y1) {
                    rsrc.y1 += rc.y2 - 1;
                    rdst.y1 += rc.y2 - 1;
                    incy = -1;
                }

                while (rc.y2 > 0) {
                    m_pixfmt->copy_from(from, rdst.x1, rdst.y1, rsrc.x1, rsrc.y1, rc.x2);

                    rdst.y1 += incy;
                    rsrc.y1 += incy;
                    --rc.y2;
                }
            }
        }
    }

    template <typename SrcPixelFormatRenderer>
    void blend_from(const SrcPixelFormatRenderer& from,
                    const rect* rect_src_ptr = 0, int32_t dx = 0, int32_t dy = 0,
                    cover_type cover = cover_full)
    {
        rect rsrc(0, 0, from.width(), from.height());
        if (rect_src_ptr) {
            rsrc.x1 = rect_src_ptr->x1;
            rsrc.y1 = rect_src_ptr->y1;
            rsrc.x2 = rect_src_ptr->x2 + 1;
            rsrc.y2 = rect_src_ptr->y2 + 1;
        }

        rect rdst(rsrc.x1 + dx, rsrc.y1 + dy, rsrc.x2 + dx, rsrc.y2 + dy);
        rect rc = clip_rect_area(rdst, rsrc, from.width(), from.height());

        if (m_is_path_clip) {
            if (rc.x2 > 0 && rc.y2 > 0) {
                int32_t incy = 1;
                int32_t incx = 1;

                if (rdst.y1 > rsrc.y1) {
                    rsrc.y1 += rc.y2 - 1;
                    rdst.y1 += rc.y2 - 1;
                    incy = -1;
                }

                if (rdst.x1 > rsrc.x1) {
                    rsrc.x1 += rc.x2 - 1;
                    rdst.x1 += rc.x2 - 1;
                    incx = -1;
                }

                while (rc.x2 > 0) {
                    int32_t y2 = rc.y2;
                    int32_t dy1 = rdst.y1;
                    int32_t sy1 = rsrc.y1;
                    while (y2 > 0) {
                        if (pixel_in_path(rdst.x1, dy1)) {
                            m_pixfmt->blend_point_from(from, rdst.x1, dy1, rsrc.x1, sy1, cover);
                        }
                        dy1 += incy;
                        sy1 += incy;
                        --y2;
                    }
                    rdst.x1 += incx;
                    rsrc.x1 += incx;
                    --rc.x2;
                }
            }
        } else {
            if (rc.x2 > 0) {
                int32_t incy = 1;

                if (rdst.y1 > rsrc.y1) {
                    rsrc.y1 += rc.y2 - 1;
                    rdst.y1 += rc.y2 - 1;
                    incy = -1;
                }

                while (rc.y2 > 0) {
                    typename SrcPixelFormatRenderer::row_data rw = from.row(rsrc.y1);
                    if (rw.ptr) {
                        int32_t x1src = rsrc.x1;
                        int32_t x1dst = rdst.x1;
                        int32_t len = rc.x2;

                        if (rw.x1 > x1src) {
                            x1dst += rw.x1 - x1src;
                            len -= rw.x1 - x1src;
                            x1src = rw.x1;
                        }

                        if (len > 0) {
                            if (x1src + len - 1 > rw.x2) {
                                len -= x1src + len - rw.x2 - 1;
                            }

                            if (len > 0) {
                                m_pixfmt->blend_from(from, x1dst, rdst.y1, x1src, rsrc.y1, len, cover);
                            }
                        }
                    }

                    rdst.y1 += incy;
                    rsrc.y1 += incy;
                    --rc.y2;
                }
            }
        }
    }

    void blend_hline(int32_t x1, int32_t y, int32_t x2, const color_type& c, cover_type cover)
    {
        normalize(x1, x2);
        if (m_is_path_clip) { // blend for per pixel.
            for (int32_t i = 0; i < (x2 - x1) + 1; i++)
                if (pixel_in_path(x1 + i, y)) {
                    m_pixfmt->blend_pixel(x1 + i, y, c, cover);
                }
        } else {
            if (y > ymax() || y < ymin()) {
                return;
            }

            if (x1 > xmax() || x2 < xmin()) {
                return;
            }

            if (x1 < xmin()) {
                x1 = xmin();
            }

            if (x2 > xmax()) {
                x2 = xmax();
            }

            m_pixfmt->blend_hline(x1, y, x2 - x1 + 1, c, cover);
        }
    }

    void blend_solid_hspan(int32_t x, int32_t y, int32_t len, const color_type& c, const cover_type* covers)
    {
        if (m_is_path_clip) {
            for (int32_t i = 0; i < len; i++)
                if (pixel_in_path(x + i, y)) {
                    m_pixfmt->blend_pixel(x + i, y, c, covers[i]);
                }
        } else {
            if (y > ymax() || y < ymin()) {
                return;
            }

            if (x < xmin()) {
                len -= (xmin() - x);
                if (len <= 0) {
                    return;
                }

                covers += (xmin() - x);
                x = xmin();
            }

            if (x + len > xmax()) {
                len = xmax() - x + 1;
                if (len <= 0) {
                    return;
                }
            }

            m_pixfmt->blend_solid_hspan(x, y, len, c, covers);
        }
    }

    void blend_color_hspan(int32_t x, int32_t y, int32_t len, const color_type* colors,
                           const cover_type* covers, cover_type cover = cover_full)
    {
        if (m_is_path_clip) {
            for (int32_t i = 0; i < len; i++)
                if (pixel_in_path(x + i, y)) {
                    m_pixfmt->blend_pixel(x + i, y, colors[i], covers[i]);
                }
        } else {
            if (y > ymax() || y < ymin()) {
                return;
            }

            if (x < xmin()) {
                int32_t d = xmin() - x;
                len -= d;
                if (len <= 0) {
                    return;
                }

                if (covers) {
                    covers += d;
                }

                colors += d;
                x = xmin();
            }

            if (x + len > xmax()) {
                len = xmax() - x + 1;
                if (len <= 0) {
                    return;
                }
            }

            m_pixfmt->blend_color_hspan(x, y, len, colors, covers, cover);
        }
    }

private:
    void normalize(int32_t& min, int32_t& max)
    {
        if (min > max) {
            int32_t tmp = max;
            max = min;
            min = tmp;
        }
    }

    bool inbox(int32_t x, int32_t y) const
    {
        return x >= m_clip_rect.x1 && y >= m_clip_rect.y1
               && x <= m_clip_rect.x2 && y <= m_clip_rect.y2;
    }

    bool pixel_in_path(int32_t x, int32_t y)
    {
        if (!inbox(x, y)) {
            return false;
        }

        if (!m_clip_path.hit_test(x, y)) {
            return false;
        }

        return true;
    }

    rect clip_rect_area(rect& dst, rect& src, int32_t wsrc, int32_t hsrc) const
    {
        rect rc(0, 0, 0, 0);
        rect cb = clip_rect();
        ++cb.x2;
        ++cb.y2;

        if (src.x1 < 0) {
            dst.x1 -= src.x1;
            src.x1 = 0;
        }

        if (src.y1 < 0) {
            dst.y1 -= src.y1;
            src.y1 = 0;
        }

        if (src.x2 > wsrc) { src.x2 = wsrc; }
        if (src.y2 > hsrc) { src.y2 = hsrc; }

        if (dst.x1 < cb.x1) {
            src.x1 += cb.x1 - dst.x1;
            dst.x1 = cb.x1;
        }

        if (dst.y1 < cb.y1) {
            src.y1 += cb.y1 - dst.y1;
            dst.y1 = cb.y1;
        }

        if (dst.x2 > cb.x2) { dst.x2 = cb.x2; }
        if (dst.y2 > cb.y2) { dst.y2 = cb.y2; }

        rc.x2 = dst.x2 - dst.x1;
        rc.y2 = dst.y2 - dst.y1;

        if (rc.x2 > src.x2 - src.x1) { rc.x2 = src.x2 - src.x1; }
        if (rc.y2 > src.y2 - src.y1) { rc.y2 = src.y2 - src.y1; }

        return rc;
    }

private:
    gfx_renderer(const gfx_renderer&);
    gfx_renderer& operator=(const gfx_renderer&);

    pixfmt_type* m_pixfmt;
    rect m_clip_rect;
    bool m_is_path_clip;
    gfx_rasterizer_scanline_aa<> m_clip_path;
};

}
#endif /*_GFX_RENDERER_H_*/
