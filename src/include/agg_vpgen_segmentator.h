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

#ifndef AGG_VPGEN_SEGMENTATOR_INCLUDED
#define AGG_VPGEN_SEGMENTATOR_INCLUDED

#include <math.h>
#include "agg_basics.h"

namespace agg
{

    //=======================================================vpgen_segmentator
    // 
    // See Implementation agg_vpgen_segmentator.cpp
    //
    class vpgen_segmentator
    {
    public:
        vpgen_segmentator() : m_approximation_scale(1.0) {}

        void approximation_scale(float s) { m_approximation_scale = s;     }
        float approximation_scale() const { return m_approximation_scale;  }

        static bool auto_close()   { return false; }
        static bool auto_unclose() { return false; }

        void reset() { m_cmd = path_cmd_stop; }
        void move_to(float x, float y);
        void line_to(float x, float y);
        unsigned vertex(float* x, float* y);

    private:
        float   m_approximation_scale;
        float   m_x1;
        float   m_y1;
        float   m_dx;
        float   m_dy;
        float   m_dl;
        float   m_ddl;
        unsigned m_cmd;
    };



}

#endif

