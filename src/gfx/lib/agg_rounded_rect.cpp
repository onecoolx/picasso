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

#include <math.h>
#include "agg_rounded_rect.h"


namespace agg
{
    //------------------------------------------------------------------------
    rounded_rect::rounded_rect(float x1, float y1, float x2, float y2, float r) :
        m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2),
        m_rx1(r), m_ry1(r), m_rx2(r), m_ry2(r), 
        m_rx3(r), m_ry3(r), m_rx4(r), m_ry4(r)
    {
        if(x1 > x2) { m_x1 = x2; m_x2 = x1; }
        if(y1 > y2) { m_y1 = y2; m_y2 = y1; }
    }

    //--------------------------------------------------------------------
    void rounded_rect::rect(float x1, float y1, float x2, float y2)
    {
        m_x1 = x1;
        m_y1 = y1;
        m_x2 = x2;
        m_y2 = y2;
        if(x1 > x2) { m_x1 = x2; m_x2 = x1; }
        if(y1 > y2) { m_y1 = y2; m_y2 = y1; }
    }

    //--------------------------------------------------------------------
    void rounded_rect::radius(float r)
    {
        m_rx1 = m_ry1 = m_rx2 = m_ry2 = m_rx3 = m_ry3 = m_rx4 = m_ry4 = r; 
    }

    //--------------------------------------------------------------------
    void rounded_rect::radius(float rx, float ry)
    {
        m_rx1 = m_rx2 = m_rx3 = m_rx4 = rx; 
        m_ry1 = m_ry2 = m_ry3 = m_ry4 = ry; 
    }

    //--------------------------------------------------------------------
    void rounded_rect::radius(float rx_bottom, float ry_bottom, 
                              float rx_top,    float ry_top)
    {
        m_rx1 = m_rx2 = rx_bottom; 
        m_rx3 = m_rx4 = rx_top; 
        m_ry1 = m_ry2 = ry_bottom; 
        m_ry3 = m_ry4 = ry_top; 
    }

    //--------------------------------------------------------------------
    void rounded_rect::radius(float rx1, float ry1, float rx2, float ry2, 
                              float rx3, float ry3, float rx4, float ry4)
    {
        m_rx1 = rx1; m_ry1 = ry1; m_rx2 = rx2; m_ry2 = ry2; 
        m_rx3 = rx3; m_ry3 = ry3; m_rx4 = rx4; m_ry4 = ry4;
    }

    //--------------------------------------------------------------------
    void rounded_rect::normalize_radius()
    {
        float dx = (float)fabs(m_y2 - m_y1);
        float dy = (float)fabs(m_x2 - m_x1);

        float k = 1.0f;
        float t;
        t = dx / (m_rx1 + m_rx2); if(t < k) k = t; 
        t = dx / (m_rx3 + m_rx4); if(t < k) k = t; 
        t = dy / (m_ry1 + m_ry2); if(t < k) k = t; 
        t = dy / (m_ry3 + m_ry4); if(t < k) k = t; 

        if(k < 1.0)
        {
            m_rx1 *= k; m_ry1 *= k; m_rx2 *= k; m_ry2 *= k;
            m_rx3 *= k; m_ry3 *= k; m_rx4 *= k; m_ry4 *= k;
        }
    }

    //--------------------------------------------------------------------
    void rounded_rect::rewind(unsigned)
    {
        m_status = 0;
    }

    //--------------------------------------------------------------------
    unsigned rounded_rect::vertex(float* x, float* y)
    {
        unsigned cmd = path_cmd_stop;
        switch(m_status)
        {
        case 0:
            m_arc.init(m_x1 + m_rx1, m_y1 + m_ry1, m_rx1, m_ry1,
                       pi, pi+pi*0.5f);
            m_arc.rewind(0);
            m_status++;

        case 1:
            cmd = m_arc.vertex(x, y);
            if(is_stop(cmd)) m_status++;
            else return cmd;

        case 2:
            m_arc.init(m_x2 - m_rx2, m_y1 + m_ry2, m_rx2, m_ry2,
                       pi+pi*0.5f, 0.0f);
            m_arc.rewind(0);
            m_status++;

        case 3:
            cmd = m_arc.vertex(x, y);
            if(is_stop(cmd)) m_status++;
            else return path_cmd_line_to;

        case 4:
            m_arc.init(m_x2 - m_rx3, m_y2 - m_ry3, m_rx3, m_ry3,
                       0.0f, pi*0.5f);
            m_arc.rewind(0);
            m_status++;

        case 5:
            cmd = m_arc.vertex(x, y);
            if(is_stop(cmd)) m_status++;
            else return path_cmd_line_to;

        case 6:
            m_arc.init(m_x1 + m_rx4, m_y2 - m_ry4, m_rx4, m_ry4,
                       pi*0.5f, pi);
            m_arc.rewind(0);
            m_status++;

        case 7:
            cmd = m_arc.vertex(x, y);
            if(is_stop(cmd)) m_status++;
            else return path_cmd_line_to;

        case 8:
            cmd = path_cmd_end_poly | path_flags_close | path_flags_ccw;
            m_status++;
            break;
        }
        return cmd;
    }


}

