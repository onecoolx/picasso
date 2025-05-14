/* Picasso - a vector graphics library
 *
 * Copyright (C) 2014 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_RASTERIZER_CELL_H_
#define _GFX_RASTERIZER_CELL_H_

#include "common.h"

#include "graphic_base.h"

namespace gfx {

template <typename T>
static inline void swap_cells(T* a, T* b)
{
    T temp = *a;
    *a = *b;
    *b = temp;
}

const int32_t qsort_threshold = 9;

template <typename Cell>
void qsort_cells(Cell** start, uint32_t num)
{
    Cell** stack[80];
    Cell*** top;
    Cell** limit;
    Cell** base;

    limit = start + num;
    base = start;
    top = stack;

    for (;;) {
        int32_t len = (int32_t)(limit - base);

        Cell** i;
        Cell** j;
        Cell** pivot;

        if (len > qsort_threshold) {
            // we use base + len/2 as the pivot
            pivot = base + (len >> 1);
            swap_cells(base, pivot);

            i = base + 1;
            j = limit - 1;

            // now ensure that *i <= *base <= *j
            if ((*j)->x < (*i)->x) {
                swap_cells(i, j);
            }

            if ((*base)->x < (*i)->x) {
                swap_cells(base, i);
            }

            if ((*j)->x < (*base)->x) {
                swap_cells(base, j);
            }

            for (;;) {
                int32_t x = (*base)->x;
                do { i++; }
                while ( (*i)->x < x );
                do { j--; }
                while ( x < (*j)->x );

                if (i > j) {
                    break;
                }

                swap_cells(i, j);
            }

            swap_cells(base, j);

            // now, push the largest sub-array
            if (j - base > limit - i) {
                top[0] = base;
                top[1] = j;
                base = i;
            } else {
                top[0] = i;
                top[1] = limit;
                limit = j;
            }
            top += 2;
        } else {
            // the sub-array is small, perform insertion sort
            j = base;
            i = j + 1;

            for (; i < limit; j = i, i++) {
                for (; j[1]->x < (*j)->x; j--) {
                    swap_cells(j + 1, j);
                    if (j == base) {
                        break;
                    }
                }
            }

            if (top > stack) {
                top -= 2;
                base = top[0];
                limit = top[1];
            } else {
                break;
            }
        }
    }
}

// rasterizer cells anrialias
// An internal class that implements the main rasterization algorithm.
// Used in the rasterizer. Should not be used direcly.
template <typename Cell>
class gfx_rasterizer_cells_aa
{
public:
    typedef Cell cell_type;

    enum {
        cell_block_shift = 12,
        cell_block_size = 1 << cell_block_shift,
        cell_block_mask = cell_block_size - 1,
        cell_block_pool = 256,
        cell_block_limit = 1024,
    };

    enum {
        dx_limit = 16384 << poly_subpixel_shift,
    };

    struct sorted_y {
        uint32_t start;
        uint32_t num;
    };

    gfx_rasterizer_cells_aa()
        : m_num_blocks(0)
        , m_max_blocks(0)
        , m_curr_block(0)
        , m_num_cells(0)
        , m_cells(0)
        , m_curr_cell_ptr(0)
        , m_sorted_cells()
        , m_sorted_y()
        , m_min_x(0x7FFFFFFF)
        , m_min_y(0x7FFFFFFF)
        , m_max_x(-0x7FFFFFFF)
        , m_max_y(-0x7FFFFFFF)
        , m_sorted(false)
    {
        m_style_cell.initial();
        m_curr_cell.initial();
    }

    ~gfx_rasterizer_cells_aa()
    {
        if (m_num_blocks) {
            cell_type** ptr = m_cells + m_num_blocks - 1;
            while (m_num_blocks--) {
                pod_allocator<cell_type>::deallocate(*ptr, cell_block_size);
                ptr--;
            }
            pod_allocator<cell_type*>::deallocate(m_cells, m_max_blocks);
        }
    }

    void reset(void)
    {
        m_num_cells = 0;
        m_curr_block = 0;
        m_curr_cell.initial();
        m_style_cell.initial();
        m_sorted = false;
        m_min_x = 0x7FFFFFFF;
        m_min_y = 0x7FFFFFFF;
        m_max_x = -0x7FFFFFFF;
        m_max_y = -0x7FFFFFFF;
    }

    void style(const cell_type& style_cell)
    {
        m_style_cell.style(style_cell);
    }

    void line(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
    {
        int32_t dx = x2 - x1;

        if (dx >= dx_limit || dx <= -dx_limit) {
            int32_t cx = (x1 + x2) >> 1;
            int32_t cy = (y1 + y2) >> 1;
            line(x1, y1, cx, cy);
            line(cx, cy, x2, y2);
        }

        int32_t dy = y2 - y1;
        int32_t ex1 = x1 >> poly_subpixel_shift;
        int32_t ex2 = x2 >> poly_subpixel_shift;
        int32_t ey1 = y1 >> poly_subpixel_shift;
        int32_t ey2 = y2 >> poly_subpixel_shift;
        int32_t fy1 = y1 & poly_subpixel_mask;
        int32_t fy2 = y2 & poly_subpixel_mask;

        int32_t x_from, x_to;
        int32_t p, rem, mod, lift, delta, first, incr;

        if (ex1 < m_min_x) { m_min_x = ex1; }
        if (ex1 > m_max_x) { m_max_x = ex1; }
        if (ey1 < m_min_y) { m_min_y = ey1; }
        if (ey1 > m_max_y) { m_max_y = ey1; }
        if (ex2 < m_min_x) { m_min_x = ex2; }
        if (ex2 > m_max_x) { m_max_x = ex2; }
        if (ey2 < m_min_y) { m_min_y = ey2; }
        if (ey2 > m_max_y) { m_max_y = ey2; }

        set_curr_cell(ex1, ey1);

        //everything is on a single hline
        if (ey1 == ey2) {
            render_hline(ey1, x1, fy1, x2, fy2);
            return;
        }

        //Vertical line - we have to calculate start and end cells,
        //and then - the common values of the area and coverage for
        //all cells of the line. We know exactly there's only one
        //cell, so, we don't have to call render_hline().
        incr = 1;
        if (dx == 0) {
            int32_t ex = x1 >> poly_subpixel_shift;
            int32_t two_fx = (int32_t)((uint32_t)(x1 - (int32_t)((uint32_t)ex << poly_subpixel_shift)) << 1);
            int32_t area;

            first = poly_subpixel_scale;
            if (dy < 0) {
                first = 0;
                incr = -1;
            }

            x_from = x1;

            delta = first - fy1;
            m_curr_cell.cover += delta;
            m_curr_cell.area += two_fx * delta;

            ey1 += incr;
            set_curr_cell(ex, ey1);

            delta = first + first - poly_subpixel_scale;
            area = two_fx * delta;
            while (ey1 != ey2) {
                m_curr_cell.cover = delta;
                m_curr_cell.area = area;
                ey1 += incr;
                set_curr_cell(ex, ey1);
            }
            delta = fy2 - poly_subpixel_scale + first;
            m_curr_cell.cover += delta;
            m_curr_cell.area += two_fx * delta;
            return;
        }

        //ok, we have to render several hlines
        p = (poly_subpixel_scale - fy1) * dx;
        first = poly_subpixel_scale;

        if (dy < 0) {
            p = fy1 * dx;
            first = 0;
            incr = -1;
            dy = -dy;
        }

        delta = p / dy;
        mod = p % dy;

        if (mod < 0) {
            delta--;
            mod += dy;
        }

        x_from = x1 + delta;
        render_hline(ey1, x1, fy1, x_from, first);

        ey1 += incr;
        set_curr_cell(x_from >> poly_subpixel_shift, ey1);

        if (ey1 != ey2) {
            p = (int32_t)((uint32_t)dx << poly_subpixel_shift);
            lift = p / dy;
            rem = p % dy;

            if (rem < 0) {
                lift--;
                rem += dy;
            }
            mod -= dy;

            while (ey1 != ey2) {
                delta = lift;
                mod += rem;
                if (mod >= 0) {
                    mod -= dy;
                    delta++;
                }

                x_to = x_from + delta;
                render_hline(ey1, x_from, poly_subpixel_scale - first, x_to, first);
                x_from = x_to;

                ey1 += incr;
                set_curr_cell(x_from >> poly_subpixel_shift, ey1);
            }
        }
        render_hline(ey1, x_from, poly_subpixel_scale - first, x2, fy2);
    }

    int32_t min_x(void) const { return m_min_x; }
    int32_t min_y(void) const { return m_min_y; }
    int32_t max_x(void) const { return m_max_x; }
    int32_t max_y(void) const { return m_max_y; }

    void sort_cells(void)
    {
        if (m_sorted) {
            return; //Perform sort only the first time.
        }

        add_curr_cell();
        m_curr_cell.x = 0x7FFFFFFF;
        m_curr_cell.y = 0x7FFFFFFF;
        m_curr_cell.cover = 0;
        m_curr_cell.area = 0;

        if (m_num_cells == 0) {
            return;
        }

        // Allocate the array of cell pointers
        m_sorted_cells.allocate(m_num_cells + 16);

        // Allocate and zero the Y array
        m_sorted_y.allocate(m_max_y - m_min_y + 1 + 16);
        m_sorted_y.zero();

        // Create the Y-histogram (count the numbers of cells for each Y)
        cell_type** block_ptr = m_cells;
        cell_type* cell_ptr;
        uint32_t nb = m_num_cells >> cell_block_shift;
        uint32_t i;
        while (nb--) {
            cell_ptr = *block_ptr++;
            i = cell_block_size;
            while (i--) {
                m_sorted_y[cell_ptr->y - m_min_y].start++;
                ++cell_ptr;
            }
        }

        cell_ptr = *block_ptr++;
        i = m_num_cells & cell_block_mask;
        while (i--) {
            m_sorted_y[cell_ptr->y - m_min_y].start++;
            ++cell_ptr;
        }

        // Convert the Y-histogram into the array of starting indexes
        uint32_t start = 0;
        for (i = 0; i < m_sorted_y.size(); i++) {
            uint32_t v = m_sorted_y[i].start;
            m_sorted_y[i].start = start;
            start += v;
        }

        // Fill the cell pointer array sorted by Y
        block_ptr = m_cells;
        nb = m_num_cells >> cell_block_shift;
        while (nb--) {
            cell_ptr = *block_ptr++;
            i = cell_block_size;
            while (i--) {
                sorted_y& curr_y = m_sorted_y[cell_ptr->y - m_min_y];
                m_sorted_cells[curr_y.start + curr_y.num] = cell_ptr;
                ++curr_y.num;
                ++cell_ptr;
            }
        }

        cell_ptr = *block_ptr++;
        i = m_num_cells & cell_block_mask;
        while (i--) {
            sorted_y& curr_y = m_sorted_y[cell_ptr->y - m_min_y];
            m_sorted_cells[curr_y.start + curr_y.num] = cell_ptr;
            ++curr_y.num;
            ++cell_ptr;
        }

        // Finally arrange the X-arrays
        for (i = 0; i < m_sorted_y.size(); i++) {
            const sorted_y& curr_y = m_sorted_y[i];
            if (curr_y.num) {
                qsort_cells(m_sorted_cells.data() + curr_y.start, curr_y.num);
            }
        }
        m_sorted = true;
    }

    uint32_t total_cells(void) const
    {
        return m_num_cells;
    }

    uint32_t scanline_num_cells(uint32_t y) const
    {
        return m_sorted_y[y - m_min_y].num;
    }

    const cell_type* const* scanline_cells(uint32_t y) const
    {
        return m_sorted_cells.data() + m_sorted_y[y - m_min_y].start;
    }

    bool sorted(void) const { return m_sorted; }

private:
    void set_curr_cell(int32_t x, int32_t y)
    {
        if (m_curr_cell.not_equal(x, y, m_style_cell)) {
            add_curr_cell();
            m_curr_cell.style(m_style_cell);
            m_curr_cell.x = x;
            m_curr_cell.y = y;
            m_curr_cell.cover = 0;
            m_curr_cell.area = 0;
        }
    }

    void add_curr_cell(void)
    {
        if (m_curr_cell.area | m_curr_cell.cover) {
            if ((m_num_cells & cell_block_mask) == 0) {
                if (m_num_blocks >= cell_block_limit) {
                    return;
                }
                allocate_block();
            }
            *m_curr_cell_ptr++ = m_curr_cell;
            ++m_num_cells;
        }
    }

    void render_hline(int32_t ey, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
    {
        int32_t ex1 = x1 >> poly_subpixel_shift;
        int32_t ex2 = x2 >> poly_subpixel_shift;
        int32_t fx1 = x1 & poly_subpixel_mask;
        int32_t fx2 = x2 & poly_subpixel_mask;

        int32_t delta, p, first, dx;
        int32_t incr, lift, mod, rem;

        // trivial case. Happens often
        if (y1 == y2) {
            set_curr_cell(ex2, ey);
            return;
        }

        // everything is located in a single cell.
        if (ex1 == ex2) {
            delta = y2 - y1;
            m_curr_cell.cover += delta;
            m_curr_cell.area += (fx1 + fx2) * delta;
            return;
        }

        p = (poly_subpixel_scale - fx1) * (y2 - y1);
        first = poly_subpixel_scale;
        incr = 1;

        dx = x2 - x1;

        if (dx < 0) {
            p = fx1 * (y2 - y1);
            first = 0;
            incr = -1;
            dx = -dx;
        }

        delta = p / dx;
        mod = p % dx;

        if (mod < 0) {
            delta--;
            mod += dx;
        }

        m_curr_cell.cover += delta;
        m_curr_cell.area += (fx1 + first) * delta;

        ex1 += incr;
        set_curr_cell(ex1, ey);
        y1 += delta;

        if (ex1 != ex2) {
            p = (int32_t)((uint32_t)(y2 - y1 + delta) << poly_subpixel_shift);
            lift = p / dx;
            rem = p % dx;

            if (rem < 0) {
                lift--;
                rem += dx;
            }

            mod -= dx;

            while (ex1 != ex2) {
                delta = lift;
                mod += rem;
                if (mod >= 0) {
                    mod -= dx;
                    delta++;
                }

                m_curr_cell.cover += delta;
                m_curr_cell.area += (int32_t)((uint32_t)delta << poly_subpixel_shift);
                y1 += delta;
                ex1 += incr;
                set_curr_cell(ex1, ey);
            }
        }
        delta = y2 - y1;
        m_curr_cell.cover += delta;
        m_curr_cell.area += (fx2 + poly_subpixel_scale - first) * delta;
    }

    void allocate_block(void)
    {
        if (m_curr_block >= m_num_blocks) {
            if (m_num_blocks >= m_max_blocks) {
                cell_type** new_cells =
                    pod_allocator<cell_type*>::allocate(m_max_blocks + cell_block_pool);

                if (m_cells) {
                    mem_copy(new_cells, m_cells, m_max_blocks * sizeof(cell_type*));
                    pod_allocator<cell_type*>::deallocate(m_cells, m_max_blocks);
                }
                m_cells = new_cells;
                m_max_blocks += cell_block_pool;
            }

            m_cells[m_num_blocks++] =
                pod_allocator<cell_type>::allocate(cell_block_size);
        }
        m_curr_cell_ptr = m_cells[m_curr_block++];
    }

private:
    gfx_rasterizer_cells_aa(const gfx_rasterizer_cells_aa<Cell>&);
    const gfx_rasterizer_cells_aa<Cell>& operator = (const gfx_rasterizer_cells_aa<Cell>&);

    uint32_t m_num_blocks;
    uint32_t m_max_blocks;
    uint32_t m_curr_block;
    uint32_t m_num_cells;
    cell_type** m_cells;
    cell_type* m_curr_cell_ptr;
    pod_vector<cell_type*> m_sorted_cells;
    pod_vector<sorted_y> m_sorted_y;
    cell_type m_curr_cell;
    cell_type m_style_cell;
    int32_t m_min_x;
    int32_t m_min_y;
    int32_t m_max_x;
    int32_t m_max_y;
    bool m_sorted;
};

}
#endif /*_GFX_RASTERIZER_CELL_H_*/
