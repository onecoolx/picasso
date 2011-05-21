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

#ifndef AGG_TRANS_DOUBLE_PATH_INCLUDED
#define AGG_TRANS_DOUBLE_PATH_INCLUDED

#include "agg_basics.h"
#include "agg_vertex_sequence.h"

namespace agg
{
    
    // See also: agg_trans_float_path.cpp
    //
    //-------------------------------------------------------trans_float_path
    class trans_float_path
    {
        enum status_e
        {
            initial,
            making_path,
            ready
        };

    public:
        typedef vertex_sequence<vertex_dist, 6> vertex_storage;

        trans_float_path();

        //--------------------------------------------------------------------
        void   base_length(float v)  { m_base_length = v; }
        float base_length() const { return m_base_length; }

        //--------------------------------------------------------------------
        void   base_height(float v)  { m_base_height = v; }
        float base_height() const { return m_base_height; }

        //--------------------------------------------------------------------
        void preserve_x_scale(bool f) { m_preserve_x_scale = f;    }
        bool preserve_x_scale() const { return m_preserve_x_scale; }

        //--------------------------------------------------------------------
        void reset();
        void move_to1(float x, float y);
        void line_to1(float x, float y);
        void move_to2(float x, float y);
        void line_to2(float x, float y);
        void finalize_paths();

        //--------------------------------------------------------------------
        template<class VertexSource1, class VertexSource2> 
        void add_paths(VertexSource1& vs1, VertexSource2& vs2, 
                       unsigned path1_id=0, unsigned path2_id=0)
        {
            float x;
            float y;

            unsigned cmd;

            vs1.rewind(path1_id);
            while(!is_stop(cmd = vs1.vertex(&x, &y)))
            {
                if(is_move_to(cmd)) 
                {
                    move_to1(x, y);
                }
                else 
                {
                    if(is_vertex(cmd))
                    {
                        line_to1(x, y);
                    }
                }
            }

            vs2.rewind(path2_id);
            while(!is_stop(cmd = vs2.vertex(&x, &y)))
            {
                if(is_move_to(cmd)) 
                {
                    move_to2(x, y);
                }
                else 
                {
                    if(is_vertex(cmd))
                    {
                        line_to2(x, y);
                    }
                }
            }
            finalize_paths();
        }

        //--------------------------------------------------------------------
        float total_length1() const;
        float total_length2() const;
        void transform(float *x, float *y) const;

    private:
        float finalize_path(vertex_storage& vertices);
        void transform1(const vertex_storage& vertices, 
                        float kindex, float kx,
                        float *x, float* y) const;

        vertex_storage m_src_vertices1;
        vertex_storage m_src_vertices2;
        float         m_base_length;
        float         m_base_height;
        float         m_kindex1;
        float         m_kindex2;
        status_e       m_status1;
        status_e       m_status2;
        bool           m_preserve_x_scale;
    };

}


#endif
