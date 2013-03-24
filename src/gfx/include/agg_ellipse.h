//----------------------------------------------------------------------------
// Anti-Grain Geometry (AGG) - Version 2.5
// A high quality rendering engine for C++
// Copyright (C) 2002-2006 Maxim Shemanarev
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://antigrain.com
// 
// AGG is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// AGG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AGG; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301, USA.
//----------------------------------------------------------------------------

#ifndef AGG_ELLIPSE_INCLUDED
#define AGG_ELLIPSE_INCLUDED

#include "agg_basics.h"
#include <math.h>

namespace agg
{

    //----------------------------------------------------------------ellipse
    class ellipse
    {
    public:
        ellipse() : 
            m_x(0.0), m_y(0.0), m_rx(1.0), m_ry(1.0), m_scale(1.0), 
            m_num(4), m_step(0), m_cw(false) {}

        ellipse(float x, float y, float rx, float ry, 
                unsigned num_steps=0, bool cw=false) :
            m_x(x), m_y(y), m_rx(rx), m_ry(ry), m_scale(1.0), 
            m_num(num_steps), m_step(0), m_cw(cw) 
        {
            if(m_num == 0) calc_num_steps();
        }

        void init(float x, float y, float rx, float ry, 
                  unsigned num_steps=0, bool cw=false);

        void approximation_scale(float scale);
        void rewind(unsigned path_id);
        unsigned vertex(float* x, float* y);

    private:
        void calc_num_steps();

        float m_x;
        float m_y;
        float m_rx;
        float m_ry;
        float m_scale;
        unsigned m_num;
        unsigned m_step;
        bool m_cw;
    };

    //------------------------------------------------------------------------
    inline void ellipse::init(float x, float y, float rx, float ry, 
                              unsigned num_steps, bool cw)
    {
        m_x = x;
        m_y = y;
        m_rx = rx;
        m_ry = ry;
        m_num = num_steps;
        m_step = 0;
        m_cw = cw;
        if(m_num == 0) calc_num_steps();
    }

    //------------------------------------------------------------------------
    inline void ellipse::approximation_scale(float scale)
    {   
        m_scale = scale;
        calc_num_steps();
    }

    //------------------------------------------------------------------------
    inline void ellipse::calc_num_steps()
    {
        float ra = ((float)fabs(m_rx) + (float)fabs(m_ry)) / 2;
        float da = (float)acos(ra / (ra + 0.125f / m_scale)) * 2;
        m_num = uround(2 * pi / da);
    }

    //------------------------------------------------------------------------
    inline void ellipse::rewind(unsigned)
    {
        m_step = 0;
    }

    //------------------------------------------------------------------------
    inline unsigned ellipse::vertex(float* x, float* y)
    {
        if(m_step == m_num) 
        {
            ++m_step;
            return path_cmd_end_poly | path_flags_close | path_flags_ccw;
        }
        if(m_step > m_num) return path_cmd_stop;
        float angle = float(m_step) / float(m_num) * 2.0f * pi;
        if(m_cw) angle = 2.0f * pi - angle;
        *x = m_x + (float)cos(angle) * m_rx;
        *y = m_y + (float)sin(angle) * m_ry;
        m_step++;
        return ((m_step == 1) ? path_cmd_move_to : path_cmd_line_to);
    }

}



#endif


