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

#ifndef AGG_ARC_INCLUDED
#define AGG_ARC_INCLUDED

#include <math.h>
#include "agg_basics.h"

namespace agg
{

    //=====================================================================arc
    //
    // See Implementation agg_arc.cpp 
    //
    class arc
    {
    public:
        arc() : m_scale(1.0), m_initialized(false) {}
        arc(float x,  float y, 
            float rx, float ry, 
            float a1, float a2, 
            bool ccw=true);

        void init(float x,  float y, 
                  float rx, float ry, 
                  float a1, float a2, 
                  bool ccw=true);

        void approximation_scale(float s);
        float approximation_scale() const { return m_scale;  }

        void rewind(unsigned);
        unsigned vertex(float* x, float* y);

    private:
        void normalize(float a1, float a2, bool ccw);

        float   m_x;
        float   m_y;
        float   m_rx;
        float   m_ry;
        float   m_angle;
        float   m_start;
        float   m_end;
        float   m_scale;
        float   m_da;
        bool     m_ccw;
        bool     m_initialized;
        unsigned m_path_cmd;
    };


}


#endif
