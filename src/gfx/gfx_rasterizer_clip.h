/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2014 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_RASTERIZER_CLIP_H_
#define _GFX_RASTERIZER_CLIP_H_

#include "common.h"

#include "graphic_base.h"

namespace gfx {

// rasterizer scanline clip
class gfx_rasterizer_sl_clip
{
public:
    typedef int coord_type;

    gfx_rasterizer_sl_clip()
        : m_clip_box(0,0,0,0)
        , m_x1(0)
        , m_y1(0)
        , m_f1(0)
        , m_clipping(false) 
    {
    }

    void reset_clipping(void)
    {
        m_clipping = false;
    }

    void clip_box(int x1, int y1, int x2, int y2)
    {
        m_clip_box = rect(x1, y1, x2, y2);
        m_clip_box.normalize();
        m_clipping = true;
    }

    void move_to(int x1, int y1)
    {
        m_x1 = x1;
        m_y1 = y1;
        if (m_clipping) 
            m_f1 = clipping_flags(x1, y1, m_clip_box);
    }

    static int upscale(scalar v) { return iround(v * poly_subpixel_scale); }
    static int downscale(int v) { return v; }
private:
    int mul_div(scalar a, scalar b, scalar c) const
    {
        return iround(a * b / c);
    }

    unsigned int clipping_flags(int x, int y, const rect& clip_box)
    {
        return (x > clip_box.x2) |
               ((y > clip_box.y2) << 1) |
               ((x < clip_box.x1) << 2) |
               ((y < clip_box.y1) << 3);
    }

    unsigned int clipping_flags_x(int x, const rect& clip_box)
    {
        return  (x > clip_box.x2) | ((x < clip_box.x1) << 2);
    }


    unsigned int clipping_flags_y(int y, const rect& clip_box)
    {
        return ((y > clip_box.y2) << 1) | ((y < clip_box.y1) << 3);
    }

    template <typename Rasterizer>
    void line_clip_y(Rasterizer& ras,int x1, int y1, int x2, int y2, 
                                    unsigned int f1, unsigned int f2) const
    {
        f1 &= 10;
        f2 &= 10;
        if ((f1 | f2) == 0) {
            // Fully visible
            ras.line(x1, y1, x2, y2); 
        } else {
            if (f1 == f2) {
                // Invisible by Y
                return;
            }

            int tx1 = x1;
            int ty1 = y1;
            int tx2 = x2;
            int ty2 = y2;

            if (f1 & 8) { // y1 < clip.y1
                tx1 = x1 + mul_div(INT_TO_SCALAR(m_clip_box.y1-y1), 
                                   INT_TO_SCALAR(x2-x1), INT_TO_SCALAR(y2-y1));
                ty1 = m_clip_box.y1;
            }

            if (f1 & 2) { // y1 > clip.y2
                tx1 = x1 + mul_div(INT_TO_SCALAR(m_clip_box.y2-y1),
                                   INT_TO_SCALAR(x2-x1), INT_TO_SCALAR(y2-y1));
                ty1 = m_clip_box.y2;
            }

            if (f2 & 8) { // y2 < clip.y1
                tx2 = x1 + mul_div(INT_TO_SCALAR(m_clip_box.y1-y1),
                                   INT_TO_SCALAR(x2-x1), INT_TO_SCALAR(y2-y1));
                ty2 = m_clip_box.y1;
            }

            if (f2 & 2) { // y2 > clip.y2
                tx2 = x1 + mul_div(INT_TO_SCALAR(m_clip_box.y2-y1),
                                   INT_TO_SCALAR(x2-x1), INT_TO_SCALAR(y2-y1));
                ty2 = m_clip_box.y2;
            }
            ras.line(tx1, ty1, tx2, ty2);
        }
    }

public:
    template <typename Rasterizer>
    void line_to(Rasterizer& ras, int x2, int y2)
    {
        if (m_clipping) {
            unsigned int f2 = clipping_flags(x2, y2, m_clip_box);

            if ((m_f1 & 10) == (f2 & 10) && (m_f1 & 10) != 0) {
                // Invisible by Y
                m_x1 = x2;
                m_y1 = y2;
                m_f1 = f2;
                return;
            }

            int x1 = m_x1;
            int y1 = m_y1;
            unsigned int f1 = m_f1;
            int y3, y4;
            unsigned int f3, f4;

            switch (((f1 & 5) << 1) | (f2 & 5))
            {
            case 0: // Visible by X
                line_clip_y(ras, x1, y1, x2, y2, f1, f2);
                break;

            case 1: // x2 > clip.x2
                y3 = y1 + mul_div(INT_TO_SCALAR(m_clip_box.x2-x1), 
                                  INT_TO_SCALAR(y2-y1), INT_TO_SCALAR(x2-x1));
                f3 = clipping_flags_y(y3, m_clip_box);
                line_clip_y(ras, x1, y1, m_clip_box.x2, y3, f1, f3);
                line_clip_y(ras, m_clip_box.x2, y3, m_clip_box.x2, y2, f3, f2);
                break;

            case 2: // x1 > clip.x2
                y3 = y1 + mul_div(INT_TO_SCALAR(m_clip_box.x2-x1), 
                                  INT_TO_SCALAR(y2-y1), INT_TO_SCALAR(x2-x1));
                f3 = clipping_flags_y(y3, m_clip_box);
                line_clip_y(ras, m_clip_box.x2, y1, m_clip_box.x2, y3, f1, f3);
                line_clip_y(ras, m_clip_box.x2, y3, x2, y2, f3, f2);
                break;

            case 3: // x1 > clip.x2 && x2 > clip.x2
                line_clip_y(ras, m_clip_box.x2, y1, m_clip_box.x2, y2, f1, f2);
                break;

            case 4: // x2 < clip.x1
                y3 = y1 + mul_div(INT_TO_SCALAR(m_clip_box.x1-x1), 
                                  INT_TO_SCALAR(y2-y1), INT_TO_SCALAR(x2-x1));
                f3 = clipping_flags_y(y3, m_clip_box);
                line_clip_y(ras, x1, y1, m_clip_box.x1, y3, f1, f3);
                line_clip_y(ras, m_clip_box.x1, y3, m_clip_box.x1, y2, f3, f2);
                break;

            case 6: // x1 > clip.x2 && x2 < clip.x1
                y3 = y1 + mul_div(INT_TO_SCALAR(m_clip_box.x2-x1), 
                                  INT_TO_SCALAR(y2-y1), INT_TO_SCALAR(x2-x1));
                y4 = y1 + mul_div(INT_TO_SCALAR(m_clip_box.x1-x1), 
                                  INT_TO_SCALAR(y2-y1), INT_TO_SCALAR(x2-x1));
                f3 = clipping_flags_y(y3, m_clip_box);
                f4 = clipping_flags_y(y4, m_clip_box);
                line_clip_y(ras, m_clip_box.x2, y1, m_clip_box.x2, y3, f1, f3);
                line_clip_y(ras, m_clip_box.x2, y3, m_clip_box.x1, y4, f3, f4);
                line_clip_y(ras, m_clip_box.x1, y4, m_clip_box.x1, y2, f4, f2);
                break;

            case 8: // x1 < clip.x1
                y3 = y1 + mul_div(INT_TO_SCALAR(m_clip_box.x1-x1), 
                                  INT_TO_SCALAR(y2-y1), INT_TO_SCALAR(x2-x1));
                f3 = clipping_flags_y(y3, m_clip_box);
                line_clip_y(ras, m_clip_box.x1, y1, m_clip_box.x1, y3, f1, f3);
                line_clip_y(ras, m_clip_box.x1, y3, x2, y2, f3, f2);
                break;

            case 9:  // x1 < clip.x1 && x2 > clip.x2
                y3 = y1 + mul_div(INT_TO_SCALAR(m_clip_box.x1-x1), 
                                  INT_TO_SCALAR(y2-y1), INT_TO_SCALAR(x2-x1));
                y4 = y1 + mul_div(INT_TO_SCALAR(m_clip_box.x2-x1), 
                                  INT_TO_SCALAR(y2-y1), INT_TO_SCALAR(x2-x1));
                f3 = clipping_flags_y(y3, m_clip_box);
                f4 = clipping_flags_y(y4, m_clip_box);
                line_clip_y(ras, m_clip_box.x1, y1, m_clip_box.x1, y3, f1, f3);
                line_clip_y(ras, m_clip_box.x1, y3, m_clip_box.x2, y4, f3, f4);
                line_clip_y(ras, m_clip_box.x2, y4, m_clip_box.x2, y2, f4, f2);
                break;

            case 12: // x1 < clip.x1 && x2 < clip.x1
                line_clip_y(ras, m_clip_box.x1, y1, m_clip_box.x1, y2, f1, f2);
                break;
            }
            m_f1 = f2;
        } else {
            ras.line(m_x1, m_y1, x2, y2); 
        }
        m_x1 = x2;
        m_y1 = y2;
    }

private:
    rect m_clip_box;
    int m_x1;
    int m_y1;
    unsigned int m_f1;
    bool m_clipping;
};

}
#endif /*_GFX_RASTERIZER_CLIP_H_*/
