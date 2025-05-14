/* Picasso - a vector graphics library
 *
 * Copyright (C) 2014 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_RASTERIZER_SCANLINE_H_
#define _GFX_RASTERIZER_SCANLINE_H_

#include "common.h"
#include "gfx_rasterizer_cell.h"

namespace gfx {

// A pixel cell. There're no constructors defined and it was done
// intentionally in order to avoid extra overhead when allocating an array of cells.
struct cell {
    int32_t x;
    int32_t y;
    int32_t cover;
    int32_t area;

    void initial(void)
    {
        x = INT32_MAX;
        y = INT32_MAX;
        cover = 0;
        area = 0;
    }

    void style(const cell&) {}

    int32_t not_equal(int32_t ex, int32_t ey, const cell&) const
    {
        return (ex - x) | (ey - y);
    }
};

// scanline hit test
class scanline_hit_test
{
public:
    scanline_hit_test(int32_t x)
        : m_x(x), m_hit(false)
    {
    }

    void reset_spans(void) { }
    void finalize(int32_t) { }

    void add_cell(int32_t x, int32_t)
    {
        if (m_x == x) {
            m_hit = true;
        }
    }

    void add_span(int32_t x, int32_t len, int32_t)
    {
        if (m_x >= x && m_x < (x + len)) {
            m_hit = true;
        }
    }

    uint32_t num_spans(void) const { return 1; }
    bool hit(void) const { return m_hit; }
    int32_t x(void) const { return m_x; }
private:
    int32_t m_x;
    bool m_hit;
};

// scanline generator
class scanline_generator
{
public:
    typedef int32_t coord_type;

    scanline_generator()
        : m_x1(0)
        , m_y1(0)
    {
    }

    void move_to(int32_t x1, int32_t y1)
    {
        m_x1 = x1;
        m_y1 = y1;
    }

    template <typename Rasterizer>
    void line_to(Rasterizer& ras, int32_t x2, int32_t y2)
    {
        ras.line(m_x1, m_y1, x2, y2);
        m_x1 = x2;
        m_y1 = y2;
    }

    static int32_t upscale(scalar v) { return iround(v * poly_subpixel_scale); }
    static int32_t downscale(int32_t v) { return v; }

private:
    int32_t m_x1;
    int32_t m_y1;
};

// rasterizer scanline antialias
// Polygon rasterizer that is used to render filled polygons with
// high-quality Anti-Aliasing. Internally, by default, the class uses
// integer coordinates in format 24.8, i.e. 24 bits for integer part
// and 8 bits for fractional - see poly_subpixel_shift. This class can be
// used in the following  way:
//
// 1. filling_rule(filling_rule ft) - optional.
//
// 3. reset()
//
// 4. move_to(x, y) / line_to(x, y) - make the polygon. One can create
// more than one contour, but each contour must consist of at least 3
// vertices, i.e. move_to(x1, y1); line_to(x2, y2); line_to(x3, y3);
// is the absolute minimum of vertices that define a triangle.
// The algorithm does not check either the number of vertices nor
// coincidence of their coordinates, but in the worst case it just
// won't draw anything.
// The orger of the vertices (clockwise or counterclockwise)
// is important when using the non-zero filling rule (fill_non_zero).
// In this case the vertex order of all the contours must be the same
// if you want your intersecting polygons to be without "holes".
// You actually can use different vertices order. If the contours do not
// intersect each other the order is not important anyway. If they do,
// contours with the same vertex order will be rendered without "holes"
// while the intersecting contours with different orders will have "holes".
//
// filling_rule() can be called anytime before "sweeping".

template <typename Gen = scanline_generator>
class gfx_rasterizer_scanline_aa
{
public:
    typedef Gen gen_type;
    typedef typename Gen::coord_type coord_type;

    enum {
        status_initial,
        status_move_to,
        status_line_to,
        status_closed
    };

    enum {
        aa_shift = 8,
        aa_scale = 1 << aa_shift,
        aa_mask = aa_scale - 1,
        aa_scale2 = aa_scale << 1,
        aa_mask2 = aa_scale2 - 1
    };

    gfx_rasterizer_scanline_aa(int32_t* gamma_table = 0)
        : m_gamma(gamma_table)
        , m_filling_rule(fill_non_zero)
        , m_start_x(0)
        , m_start_y(0)
        , m_status(status_initial)
        , m_scan_y(0)
        , m_auto_close(true)
    {
        if (!m_gamma) {
            for (int32_t i = 0; i < aa_scale; i++) {
                m_gamma_table[i] = i;
            }

            m_gamma = m_gamma_table;
        }
    }

    void reset(void)
    {
        m_outline.reset();
        m_status = status_initial;
    }

    void filling(filling_rule rule)
    {
        m_filling_rule = rule;
    }

    void auto_close(bool flag)
    {
        m_auto_close = flag;
    }

    bool initial(void)
    {
        return m_status == status_initial;
    }

    uint32_t apply_gamma(uint32_t cover) const
    {
        return m_gamma[cover];
    }

    void move_to(int32_t x, int32_t y)
    {
        if (m_outline.sorted()) {
            reset();
        }
        if (m_auto_close) {
            close_polygon();
        }

        m_gen.move_to(m_start_x = gen_type::downscale(x),
                      m_start_y = gen_type::downscale(y));
        m_status = status_move_to;
    }

    void line_to(int32_t x, int32_t y)
    {
        m_gen.line_to(m_outline, gen_type::downscale(x),
                      gen_type::downscale(y));
        m_status = status_line_to;
    }

    void move_to_d(scalar x, scalar y)
    {
        if (m_outline.sorted()) {
            reset();
        }
        if (m_auto_close) {
            close_polygon();
        }

        m_gen.move_to(m_start_x = gen_type::upscale(x),
                      m_start_y = gen_type::upscale(y));
        m_status = status_move_to;
    }

    void line_to_d(scalar x, scalar y)
    {
        m_gen.line_to(m_outline, gen_type::upscale(x),
                      gen_type::upscale(y));
        m_status = status_line_to;
    }

    void close_polygon(void)
    {
        if (m_status == status_line_to) {
            m_gen.line_to(m_outline, m_start_x, m_start_y);
            m_status = status_closed;
        }
    }

    void add_vertex(scalar x, scalar y, uint32_t cmd)
    {
        if (is_move_to(cmd)) {
            move_to_d(x, y);
        } else if (is_vertex(cmd)) {
            line_to_d(x, y);
        } else if (is_close(cmd)) {
            close_polygon();
        }
    }

    void edge(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
    {
        if (m_outline.sorted()) {
            reset();
        }

        m_gen.move_to(gen_type::downscale(x1), gen_type::downscale(y1));
        m_gen.line_to(m_outline, gen_type::downscale(x2), gen_type::downscale(y2));
        m_status = status_move_to;
    }

    void edge_d(scalar x1, scalar y1, scalar x2, scalar y2)
    {
        if (m_outline.sorted()) {
            reset();
        }

        m_gen.move_to(gen_type::upscale(x1), gen_type::upscale(y1));
        m_gen.line_to(m_outline, gen_type::upscale(x2), gen_type::upscale(y2));
        m_status = status_move_to;
    }

    void add_path(vertex_source& vs, uint32_t path_id = 0)
    {
        scalar x;
        scalar y;

        uint32_t cmd;
        vs.rewind(path_id);
        if (m_outline.sorted()) {
            reset();
        }

        while (!is_stop(cmd = vs.vertex(&x, &y))) {
            add_vertex(x, y, cmd);
        }
    }

    int32_t min_x(void) const { return m_outline.min_x(); }
    int32_t min_y(void) const { return m_outline.min_y(); }
    int32_t max_x(void) const { return m_outline.max_x(); }
    int32_t max_y(void) const { return m_outline.max_y(); }

    void sort(void)
    {
        if (m_auto_close) {
            close_polygon();
        }

        m_outline.sort_cells();
    }

    bool rewind_scanlines(void)
    {
        if (m_auto_close) {
            close_polygon();
        }

        m_outline.sort_cells();
        if (m_outline.total_cells() == 0) {
            return false;
        }

        m_scan_y = m_outline.min_y();
        return true;
    }

    bool navigate_scanline(int32_t y)
    {
        if (m_auto_close) {
            close_polygon();
        }

        m_outline.sort_cells();
        if (m_outline.total_cells() == 0
            || y < m_outline.min_y()
            || y > m_outline.max_y()) {
            return false;
        }
        m_scan_y = y;
        return true;
    }

    uint32_t calculate_alpha(int32_t area) const
    {
        int32_t cover = area >> ((poly_subpixel_shift << 1) + 1 - aa_shift);

        if (cover < 0) {
            cover = -cover;
        }

        if (m_filling_rule == fill_even_odd) {
            cover &= aa_mask2;
            if (cover > aa_scale) {
                cover = aa_scale2 - cover;
            }
        }

        if (cover > aa_mask) {
            cover = aa_mask;
        }

        return m_gamma[cover];
    }

    template <typename Scanline>
    bool sweep_scanline(Scanline& sl)
    {
        for (;;) {
            if (m_scan_y > m_outline.max_y()) {
                return false;
            }

            sl.reset_spans();
            uint32_t num_cells = m_outline.scanline_num_cells(m_scan_y);
            const cell* const* cells = m_outline.scanline_cells(m_scan_y);
            int32_t cover = 0;

            while (num_cells) {
                const cell* cur_cell = *cells;
                int32_t x = cur_cell->x;
                int32_t area = cur_cell->area;
                uint32_t alpha;

                cover += cur_cell->cover;

                // accumulate all cells with the same X
                while (--num_cells) {
                    cur_cell = *++cells;
                    if (cur_cell->x != x) {
                        break;
                    }
                    area += cur_cell->area;
                    cover += cur_cell->cover;
                }

                if (area) {
                    alpha = calculate_alpha((int32_t)((uint32_t)cover << (poly_subpixel_shift + 1)) - area);
                    if (alpha) {
                        sl.add_cell(x, alpha);
                    }
                    x++;
                }

                if (num_cells && cur_cell->x > x) {
                    alpha = calculate_alpha((int32_t)((uint32_t)cover << (poly_subpixel_shift + 1)));
                    if (alpha) {
                        sl.add_span(x, cur_cell->x - x, alpha);
                    }
                }
            }

            if (sl.num_spans()) {
                break;
            }
            ++m_scan_y;
        }

        sl.finalize(m_scan_y);
        ++m_scan_y;
        return true;
    }

    template <typename Scanline>
    bool sweep_scanline_hit(Scanline& sl)
    {
        if (m_scan_y > m_outline.max_y()) {
            return false;
        }

        uint32_t num_cells = m_outline.scanline_num_cells(m_scan_y);
        const cell* const* cells = m_outline.scanline_cells(m_scan_y);
        int32_t cover = 0;
        int32_t tx = sl.x();

        while (num_cells) {
            const cell* cur_cell = *cells;
            int32_t x = cur_cell->x;
            int32_t area = cur_cell->area;
            uint32_t alpha;

            if (tx < x) {
                return false;
            }

            cover += cur_cell->cover;

            // accumulate all cells with the same X
            while (--num_cells) {
                cur_cell = *++cells;
                if (cur_cell->x != x) {
                    break;
                }
                area += cur_cell->area;
                cover += cur_cell->cover;
            }

            if (area) {
                if (tx == x) {
                    alpha = calculate_alpha((int32_t)((uint32_t)cover << (poly_subpixel_shift + 1)) - area);
                    if (alpha) {
                        sl.add_cell(x, alpha);
                        if (sl.hit()) {
                            break;
                        }
                    }
                }
                x++;
            }

            if (num_cells && cur_cell->x > x) {
                int32_t sx = cur_cell->x - x;
                if ((tx >= x) && (tx < (x + sx))) {
                    alpha = calculate_alpha((int32_t)((uint32_t)cover << (poly_subpixel_shift + 1)));
                    if (alpha) {
                        sl.add_span(x, sx, alpha);
                        if (sl.hit()) {
                            break;
                        }
                    }
                }
            }
        }
        return true;
    }

    bool hit_test(int32_t tx, int32_t ty)
    {
        if (!navigate_scanline(ty)) {
            return false;
        }
        scanline_hit_test sl(tx);
        sweep_scanline_hit(sl);
        return sl.hit();
    }

private:
    gfx_rasterizer_cells_aa<cell> m_outline;
    gen_type m_gen;
    int32_t m_gamma_table[aa_scale];
    int32_t* m_gamma;
    filling_rule m_filling_rule;
    coord_type m_start_x;
    coord_type m_start_y;
    uint32_t m_status;
    int32_t m_scan_y;
    bool m_auto_close;
};

}
#endif /*_GFX_RASTERIZER_SCANLINE_H_*/
