//----------------------------------------------------------------------------
// Anti-Grain Geometry (AGG) - Version 2.5
// A high quality rendering engine for C++
// Copyright (C) 2002-2006 Maxim Shemanarev
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://antigrain.com
// Changed by Zhang Ji Peng (onecoolx@gmail.com)
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
//
// Adaptation for high precision colors has been sponsored by 
// Liberty Technology Systems, Inc., visit http://lib-sys.com
//
// Liberty Technology Systems, Inc. is the provider of
// PostScript and PDF technology for software developers.
// 
//----------------------------------------------------------------------------


#ifndef AGG_SPAN_PATTERN_RGB_PACKED_INCLUDED
#define AGG_SPAN_PATTERN_RGB_PACKED_INCLUDED

#include "agg_basics.h"

namespace agg
{
    //========================================================span_pattern_rgb_packed
    template<class Source> class span_pattern_rgb_packed
    {
    public:
        typedef Source source_type;
        typedef typename source_type::color_type color_type;
        typedef typename source_type::order_type order_type;
		typedef typename source_type::pixel_type pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;

        //--------------------------------------------------------------------
        span_pattern_rgb_packed() {}
        span_pattern_rgb_packed(source_type& src, 
                         unsigned offset_x, unsigned offset_y) :
            m_src(&src),
            m_offset_x(offset_x),
            m_offset_y(offset_y),
            m_alpha(color_type::base_mask)
        {}

        //--------------------------------------------------------------------
        void   attach(source_type& v)      { m_src = &v; }
               source_type& source()       { return *m_src; }
        const  source_type& source() const { return *m_src; }

        //--------------------------------------------------------------------
        void       offset_x(unsigned v) { m_offset_x = v; }
        void       offset_y(unsigned v) { m_offset_y = v; }
        unsigned   offset_x() const { return m_offset_x; }
        unsigned   offset_y() const { return m_offset_y; }
        void       alpha(value_type v) { m_alpha = v; }
        value_type alpha() const { return m_alpha; }

        //--------------------------------------------------------------------
        void prepare() {}
        void generate(color_type* span, int x, int y, unsigned len)
        {   
            x += m_offset_x;
            y += m_offset_y;
            const value_type* p = (const value_type*)m_src->span(x, y, len);
            pixel_type rgb_pixel;
			pixel_type seed = 0;
			pixel_type r_mask = ((~seed)>>(order_type::G + order_type::B))<<(order_type::G + order_type::B);
			pixel_type g_mask = (((~seed)&(~r_mask))>>(order_type::B))<<(order_type::B);
			pixel_type b_mask = ((~seed)&(~(r_mask|g_mask)));
            do
            {
            	rgb_pixel = *reinterpret_cast<const pixel_type*>(p);
                span->r = (rgb_pixel&r_mask)>>(color_type::base_shift-
										(16-order_type::R-order_type::G-order_type::B));
                span->g = (rgb_pixel&g_mask) >> (order_type::G+order_type::B-color_type::base_shift);
                span->b = (rgb_pixel&b_mask) << (color_type::base_shift-order_type::B);
                span->a = m_alpha;
                p = m_src->next_x();
                ++span;
            }
            while(--len);
        }

    private:
        source_type* m_src;
        unsigned     m_offset_x;
        unsigned     m_offset_y;
        value_type   m_alpha;

    };

}

#endif

