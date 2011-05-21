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
//
// Adaptation for high precision colors has been sponsored by 
// Liberty Technology Systems, Inc., visit http://lib-sys.com
//
// Liberty Technology Systems, Inc. is the provider of
// PostScript and PDF technology for software developers.
// 
//----------------------------------------------------------------------------

#ifndef AGG_PIXFMT_RGB_PACKED_INCLUDED
#define AGG_PIXFMT_RGB_PACKED_INCLUDED

#include <string.h>
#include "agg_basics.h"
#include "agg_color_rgba.h"
#include "agg_rendering_buffer.h"

namespace agg
{
    template<class ColorT, class Order, class Blender> struct comp_op_table_rgb_p
    {
        typedef int16u pixel_type;
        typedef typename ColorT::value_type value_type;
        typedef void (*comp_op_func_type)(pixel_type* p, 
                                          unsigned cr, 
                                          unsigned cg, 
                                          unsigned cb,
                                          unsigned ca,
                                          unsigned cover);
        static comp_op_func_type g_rgb_p_comp_op_func[];
    };

    //=========================================================comp_op_rgb_p_clear
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_clear
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned, unsigned, unsigned, unsigned,
                                         unsigned cover)
        {
            if(cover < 255)
            {
                cover = 255 - cover;
				color_type c = blender_type::make_color(*p);
                c.r = (value_type)((c.r * cover + 255) >> 8);
                c.g = (value_type)((c.g * cover + 255) >> 8);
                c.b = (value_type)((c.b * cover + 255) >> 8);
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
            else
            {
                *p = blender_type::make_pix(0, 0, 0); 
            }
        }
    };

    //===========================================================comp_op_rgb_p_src
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_src
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;

        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                unsigned alpha = 255 - cover;
				color_type c = blender_type::make_color(*p);
                c.r = (value_type)(((c.r * alpha + 255) >> 8) + ((sr * cover + 255) >> 8));
                c.g = (value_type)(((c.g * alpha + 255) >> 8) + ((sg * cover + 255) >> 8));
                c.b = (value_type)(((c.b * alpha + 255) >> 8) + ((sb * cover + 255) >> 8));
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
            else
            {
                *p = blender_type::make_pix(sr, sg, sb); 
            }
        }
    };

    //===========================================================comp_op_rgb_p_dst
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_dst
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;

        static AGG_INLINE void blend_pix(pixel_type*, 
                                         unsigned, unsigned, unsigned, 
                                         unsigned, unsigned)
        {
        }
    };

    //======================================================comp_op_rgb_src_over
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_src_over
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        //   Dca' = Sca + Dca.(1 - Sa)
        //   Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
			if (sa < 255)  {
				calc_type s1a = base_mask - sa;
				color_type c = blender_type::make_color(*p);
				c.r = (value_type)(sr + ((c.r * s1a + base_mask) >> base_shift));
				c.g = (value_type)(sg + ((c.g * s1a + base_mask) >> base_shift));
				c.b = (value_type)(sb + ((c.b * s1a + base_mask) >> base_shift));
				*p = blender_type::make_pix(c.r, c.g, c.b);
			} else { // fast for opaque
				*p = blender_type::make_pix(sr, sg, sb);
			}
        }
    };

    //======================================================comp_op_rgb_dst_over
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_dst_over
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Dca + Sca.(1 - Da)
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
			color_type c = blender_type::make_color(*p);
            c.r = (value_type)(c.r + (base_mask >> base_shift));
            c.g = (value_type)(c.g + (base_mask >> base_shift));
            c.b = (value_type)(c.b + (base_mask >> base_shift));
			*p = blender_type::make_pix(c.r, c.g, c.b);
        }
    };

    //======================================================comp_op_rgb_p_src_in
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_src_in
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca.Da
        // Da'  = Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            calc_type da = base_mask;
			color_type c = blender_type::make_color(*p);
            if(cover < 255)
            {
                unsigned alpha = 255 - cover;
                c.r = (value_type)(((c.r * alpha + 255) >> 8) + ((((sr * da + base_mask) >> base_shift) * cover + 255) >> 8));
                c.g = (value_type)(((c.g * alpha + 255) >> 8) + ((((sg * da + base_mask) >> base_shift) * cover + 255) >> 8));
                c.b = (value_type)(((c.b * alpha + 255) >> 8) + ((((sb * da + base_mask) >> base_shift) * cover + 255) >> 8));
            }
            else
            {
                c.r = (value_type)((sr * da + base_mask) >> base_shift);
                c.g = (value_type)((sg * da + base_mask) >> base_shift);
                c.b = (value_type)((sb * da + base_mask) >> base_shift);
            }
			*p = blender_type::make_pix(c.r, c.g, c.b);
        }
    };

    //======================================================comp_op_rgb_p_dst_in
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_dst_in
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Dca.Sa
        // Da'  = Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned, unsigned, unsigned, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sa = base_mask - ((cover * (base_mask - sa) + 255) >> 8);
            }
			color_type c = blender_type::make_color(*p);
            c.r = (value_type)((c.r * sa + base_mask) >> base_shift);
            c.g = (value_type)((c.g * sa + base_mask) >> base_shift);
            c.b = (value_type)((c.b * sa + base_mask) >> base_shift);
			*p = blender_type::make_pix(c.r, c.g, c.b);
        }
    };

    //======================================================comp_op_rgb_p_src_out
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_src_out
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca.(1 - Da)
        // Da'  = Sa.(1 - Da) 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            calc_type da = 0;
			color_type c = blender_type::make_color(*p);
            if(cover < 255)
            {
                unsigned alpha = 255 - cover;
                c.r = (value_type)(((c.r * alpha + 255) >> 8) + ((((sr * da + base_mask) >> base_shift) * cover + 255) >> 8));
                c.g = (value_type)(((c.g * alpha + 255) >> 8) + ((((sg * da + base_mask) >> base_shift) * cover + 255) >> 8));
                c.b = (value_type)(((c.b * alpha + 255) >> 8) + ((((sb * da + base_mask) >> base_shift) * cover + 255) >> 8));
            }
            else
            {
                c.r = (value_type)((sr * da + base_mask) >> base_shift);
                c.g = (value_type)((sg * da + base_mask) >> base_shift);
                c.b = (value_type)((sb * da + base_mask) >> base_shift);
            }
			*p = blender_type::make_pix(c.r, c.g, c.b);
        }
    };

    //======================================================comp_op_rgb_p_dst_out
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_dst_out
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Dca.(1 - Sa) 
        // Da'  = Da.(1 - Sa) 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned, unsigned, unsigned, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sa = (sa * cover + 255) >> 8;
            }
            sa = base_mask - sa;
			color_type c = blender_type::make_color(*p);
            c.r = (value_type)((c.r * sa + base_shift) >> base_shift);
            c.g = (value_type)((c.g * sa + base_shift) >> base_shift);
            c.b = (value_type)((c.b * sa + base_shift) >> base_shift);
			*p = blender_type::make_pix(c.r, c.g, c.b);
        }
    };

    //=====================================================comp_op_rgb_p_src_atop
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_src_atop
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca.Da + Dca.(1 - Sa)
        // Da'  = Da
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            calc_type da = base_mask;
            sa = base_mask - sa;
			color_type c = blender_type::make_color(*p);
            c.r = (value_type)((sr * da + c.r * sa + base_mask) >> base_shift);
            c.g = (value_type)((sg * da + c.g * sa + base_mask) >> base_shift);
            c.b = (value_type)((sb * da + c.b * sa + base_mask) >> base_shift);
			*p = blender_type::make_pix(c.r, c.g, c.b);
        }
    };

    //=====================================================comp_op_rgb_p_dst_atop
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_dst_atop
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Dca.Sa + Sca.(1 - Da)
        // Da'  = Sa 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            calc_type da = 0;
			color_type c = blender_type::make_color(*p);
            if(cover < 255)
            {
                unsigned alpha = 255 - cover;
                sr = (c.r * sa + sr * da + base_mask) >> base_shift;
                sg = (c.g * sa + sg * da + base_mask) >> base_shift;
                sb = (c.b * sa + sb * da + base_mask) >> base_shift;
                c.r = (value_type)(((c.r * alpha + 255) >> 8) + ((sr * cover + 255) >> 8));
                c.g = (value_type)(((c.g * alpha + 255) >> 8) + ((sg * cover + 255) >> 8));
                c.b = (value_type)(((c.b * alpha + 255) >> 8) + ((sb * cover + 255) >> 8));

            }
            else
            {
                c.r = (value_type)((c.r * sa + sr * da + base_mask) >> base_shift);
                c.g = (value_type)((c.g * sa + sg * da + base_mask) >> base_shift);
                c.b = (value_type)((c.b * sa + sb * da + base_mask) >> base_shift);
            }
			*p = blender_type::make_pix(c.r, c.g, c.b);
        }
    };

    //=========================================================comp_op_rgb_p_xor
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_xor
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
        // Da'  = Sa + Da - 2.Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
                calc_type s1a = base_mask - sa;
                calc_type d1a = 0;
				color_type c = blender_type::make_color(*p);
                c.r = (value_type)((c.r * s1a + sr * d1a + base_mask) >> base_shift);
                c.g = (value_type)((c.g * s1a + sg * d1a + base_mask) >> base_shift);
                c.b = (value_type)((c.b * s1a + sb * d1a + base_mask) >> base_shift);
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=========================================================comp_op_rgb_p_plus
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_plus
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca + Dca
        // Da'  = Sa + Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type dr = c.r + sr;
                calc_type dg = c.g + sg;
                calc_type db = c.b + sb;
                c.r = (dr > base_mask) ? (value_type)base_mask : dr;
                c.g = (dg > base_mask) ? (value_type)base_mask : dg;
                c.b = (db > base_mask) ? (value_type)base_mask : db;
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //========================================================comp_op_rgb_p_minus
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_minus
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Dca - Sca
        // Da' = 1 - (1 - Sa).(1 - Da)
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type dr = c.r - sr;
                calc_type dg = c.g - sg;
                calc_type db = c.b - sb;
                c.r = (dr > base_mask) ? 0 : dr;
                c.g = (dg > base_mask) ? 0 : dg;
                c.b = (db > base_mask) ? 0 : db;
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=====================================================comp_op_rgb_p_multiply
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_multiply
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type s1a = base_mask - sa;
                calc_type d1a = 0;
                calc_type dr = c.r;
                calc_type dg = c.g;
                calc_type db = c.b;
                c.r = (value_type)((sr * dr + sr * d1a + dr * s1a + base_mask) >> base_shift);
                c.g = (value_type)((sg * dg + sg * d1a + dg * s1a + base_mask) >> base_shift);
                c.b = (value_type)((sb * db + sb * d1a + db * s1a + base_mask) >> base_shift);
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=====================================================comp_op_rgb_p_screen
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_screen
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca + Dca - Sca.Dca
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type dr = c.r;
                calc_type dg = c.g;
                calc_type db = c.b;
                c.r = (value_type)(sr + dr - ((sr * dr + base_mask) >> base_shift));
                c.g = (value_type)(sg + dg - ((sg * dg + base_mask) >> base_shift));
                c.b = (value_type)(sb + db - ((sb * db + base_mask) >> base_shift));
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=====================================================comp_op_rgb_p_overlay
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_overlay
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // if 2.Dca < Da
        //   Dca' = 2.Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
        // otherwise
        //   Dca' = Sa.Da - 2.(Da - Dca).(Sa - Sca) + Sca.(1 - Da) + Dca.(1 - Sa)
        // 
        // Da' = Sa + Da - Sa.Da
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }

            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type d1a  = 0;
                calc_type s1a  = base_mask - sa;
                calc_type dr   = c.r;
                calc_type dg   = c.g;
                calc_type db   = c.b;
                calc_type da   = base_mask;
                calc_type sada = sa * base_mask;

                c.r = (value_type)(((2*dr < da) ? 
                    2*sr*dr + sr*d1a + dr*s1a : 
                    sada - 2*(da - dr)*(sa - sr) + sr*d1a + dr*s1a + base_mask) >> base_shift);

                c.g = (value_type)(((2*dg < da) ? 
                    2*sg*dg + sg*d1a + dg*s1a : 
                    sada - 2*(da - dg)*(sa - sg) + sg*d1a + dg*s1a + base_mask) >> base_shift);

                c.b = (value_type)(((2*db < da) ? 
                    2*sb*db + sb*d1a + db*s1a : 
                    sada - 2*(da - db)*(sa - sb) + sb*d1a + db*s1a + base_mask) >> base_shift);
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=====================================================comp_op_rgb_p_darken
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_darken
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = min(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type d1a = 0;
                calc_type s1a = base_mask - sa;
                calc_type dr  = c.r;
                calc_type dg  = c.g;
                calc_type db  = c.b;
                calc_type da  = base_mask;

                c.r = (value_type)((sd_min(sr * da, dr * sa) + sr * d1a + dr * s1a + base_mask) >> base_shift);
                c.g = (value_type)((sd_min(sg * da, dg * sa) + sg * d1a + dg * s1a + base_mask) >> base_shift);
                c.b = (value_type)((sd_min(sb * da, db * sa) + sb * d1a + db * s1a + base_mask) >> base_shift);
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=====================================================comp_op_rgb_p_lighten
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_lighten
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = max(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type d1a = 0;
                calc_type s1a = base_mask - sa;
                calc_type dr  = c.r;
                calc_type dg  = c.g;
                calc_type db  = c.b;
                calc_type da  = base_mask;

                c.r = (value_type)((sd_max(sr * da, dr * sa) + sr * d1a + dr * s1a + base_mask) >> base_shift);
                c.g = (value_type)((sd_max(sg * da, dg * sa) + sg * d1a + dg * s1a + base_mask) >> base_shift);
                c.b = (value_type)((sd_max(sb * da, db * sa) + sb * d1a + db * s1a + base_mask) >> base_shift);
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=====================================================comp_op_rgb_p_color_dodge
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_color_dodge
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        typedef typename color_type::long_type long_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // if Sca.Da + Dca.Sa >= Sa.Da
        //   Dca' = Sa.Da + Sca.(1 - Da) + Dca.(1 - Sa)
        // otherwise
        //   Dca' = Dca.Sa/(1-Sca/Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
        //
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type d1a  = 0;
                calc_type s1a  = base_mask - sa;
                calc_type dr   = c.r;
                calc_type dg   = c.g;
                calc_type db   = c.b;
                calc_type da   = base_mask;
                long_type drsa = dr * sa;
                long_type dgsa = dg * sa;
                long_type dbsa = db * sa;
                long_type srda = sr * da;
                long_type sgda = sg * da;
                long_type sbda = sb * da;
                long_type sada = sa * da;

                c.r = (value_type)((srda + drsa >= sada) ? 
                    (sada + sr * d1a + dr * s1a + base_mask) >> base_shift :
                    drsa / (base_mask - (sr << base_shift) / sa) + ((sr * d1a + dr * s1a + base_mask) >> base_shift));

                c.g = (value_type)((sgda + dgsa >= sada) ? 
                    (sada + sg * d1a + dg * s1a + base_mask) >> base_shift :
                    dgsa / (base_mask - (sg << base_shift) / sa) + ((sg * d1a + dg * s1a + base_mask) >> base_shift));

                c.b = (value_type)((sbda + dbsa >= sada) ? 
                    (sada + sb * d1a + db * s1a + base_mask) >> base_shift :
                    dbsa / (base_mask - (sb << base_shift) / sa) + ((sb * d1a + db * s1a + base_mask) >> base_shift));
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=====================================================comp_op_rgb_p_color_burn
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_color_burn
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        typedef typename color_type::long_type long_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // if Sca.Da + Dca.Sa <= Sa.Da
        //   Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
        // otherwise
        //   Dca' = Sa.(Sca.Da + Dca.Sa - Sa.Da)/Sca + Sca.(1 - Da) + Dca.(1 - Sa)
        // 
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type d1a  = 0;
                calc_type s1a  = base_mask - sa;
                calc_type dr   = c.r;
                calc_type dg   = c.g;
                calc_type db   = c.b;
                calc_type da   = base_mask;
                long_type drsa = dr * sa;
                long_type dgsa = dg * sa;
                long_type dbsa = db * sa;
                long_type srda = sr * da;
                long_type sgda = sg * da;
                long_type sbda = sb * da;
                long_type sada = sa * da;

                c.r = (value_type)(((srda + drsa <= sada) ? 
                    sr * d1a + dr * s1a :
                    sa * (srda + drsa - sada) / sr + sr * d1a + dr * s1a + base_mask) >> base_shift);

                c.g = (value_type)(((sgda + dgsa <= sada) ? 
                    sg * d1a + dg * s1a :
                    sa * (sgda + dgsa - sada) / sg + sg * d1a + dg * s1a + base_mask) >> base_shift);

                c.b = (value_type)(((sbda + dbsa <= sada) ? 
                    sb * d1a + db * s1a :
                    sa * (sbda + dbsa - sada) / sb + sb * d1a + db * s1a + base_mask) >> base_shift);
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=====================================================comp_op_rgb_p_hard_light
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_hard_light
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        typedef typename color_type::long_type long_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // if 2.Sca < Sa
        //    Dca' = 2.Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
        // otherwise
        //    Dca' = Sa.Da - 2.(Da - Dca).(Sa - Sca) + Sca.(1 - Da) + Dca.(1 - Sa)
        // 
        // Da'  = Sa + Da - Sa.Da
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type d1a  = 0;
                calc_type s1a  = base_mask - sa;
                calc_type dr   = c.r;
                calc_type dg   = c.g;
                calc_type db   = c.b;
                calc_type da   = base_mask;
                calc_type sada = sa * da;

                c.r = (value_type)(((2*sr < sa) ? 
                    2*sr*dr + sr*d1a + dr*s1a : 
                    sada - 2*(da - dr)*(sa - sr) + sr*d1a + dr*s1a + base_mask) >> base_shift);

                c.g = (value_type)(((2*sg < sa) ? 
                    2*sg*dg + sg*d1a + dg*s1a : 
                    sada - 2*(da - dg)*(sa - sg) + sg*d1a + dg*s1a + base_mask) >> base_shift);

                c.b = (value_type)(((2*sb < sa) ? 
                    2*sb*db + sb*d1a + db*s1a : 
                    sada - 2*(da - db)*(sa - sb) + sb*d1a + db*s1a + base_mask) >> base_shift);
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=====================================================comp_op_rgb_p_soft_light
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_soft_light
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        typedef typename color_type::long_type long_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // if 2.Sca < Sa
        //   Dca' = Dca.(Sa + (1 - Dca/Da).(2.Sca - Sa)) + Sca.(1 - Da) + Dca.(1 - Sa)
        // otherwise if 8.Dca <= Da
        //   Dca' = Dca.(Sa + (1 - Dca/Da).(2.Sca - Sa).(3 - 8.Dca/Da)) + Sca.(1 - Da) + Dca.(1 - Sa)
        // otherwise
        //   Dca' = (Dca.Sa + ((Dca/Da)^(0.5).Da - Dca).(2.Sca - Sa)) + Sca.(1 - Da) + Dca.(1 - Sa)
        // 
        // Da'  = Sa + Da - Sa.Da 

        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned r, unsigned g, unsigned b, 
                                         unsigned a, unsigned cover)
        {
            float sr = float(r * cover) / (base_mask * 255);
            float sg = float(g * cover) / (base_mask * 255);
            float sb = float(b * cover) / (base_mask * 255);
            float sa = float(a * cover) / (base_mask * 255);
            if(sa > 0)
            {
				color_type c = blender_type::make_color(*p);
                float dr = float(c.r) / base_mask;
                float dg = float(c.g) / base_mask;
                float db = float(c.b) / base_mask;
                float da = float(base_mask) / base_mask;
                if(cover < 255)
                {
                    a = (a * cover + 255) >> 8;
                }

                if(2*sr < sa)       dr = dr*(sa + (1 - dr/da)*(2*sr - sa)) + sr*(1 - da) + dr*(1 - sa);
                else if(8*dr <= da) dr = dr*(sa + (1 - dr/da)*(2*sr - sa)*(3 - 8*dr/da)) + sr*(1 - da) + dr*(1 - sa);
                else                dr = (dr*sa + ((float)sqrt(dr/da)*da - dr)*(2*sr - sa)) + sr*(1 - da) + dr*(1 - sa);

                if(2*sg < sa)       dg = dg*(sa + (1 - dg/da)*(2*sg - sa)) + sg*(1 - da) + dg*(1 - sa);
                else if(8*dg <= da) dg = dg*(sa + (1 - dg/da)*(2*sg - sa)*(3 - 8*dg/da)) + sg*(1 - da) + dg*(1 - sa);
                else                dg = (dg*sa + ((float)sqrt(dg/da)*da - dg)*(2*sg - sa)) + sg*(1 - da) + dg*(1 - sa);

                if(2*sb < sa)       db = db*(sa + (1 - db/da)*(2*sb - sa)) + sb*(1 - da) + db*(1 - sa);
                else if(8*db <= da) db = db*(sa + (1 - db/da)*(2*sb - sa)*(3 - 8*db/da)) + sb*(1 - da) + db*(1 - sa);
                else                db = (db*sa + ((float)sqrt(db/da)*da - db)*(2*sb - sa)) + sb*(1 - da) + db*(1 - sa);

                c.r = (value_type)uround(dr * base_mask);
                c.g = (value_type)uround(dg * base_mask);
                c.b = (value_type)uround(db * base_mask);
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=====================================================comp_op_rgb_p_difference
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_difference
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        typedef typename color_type::long_type long_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_scale = color_type::base_scale,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca + Dca - 2.min(Sca.Da, Dca.Sa)
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type dr = c.r;
                calc_type dg = c.g;
                calc_type db = c.b;
                calc_type da = base_mask;
                c.r = (value_type)(sr + dr - ((2 * sd_min(sr*da, dr*sa) + base_mask) >> base_shift));
                c.g = (value_type)(sg + dg - ((2 * sd_min(sg*da, dg*sa) + base_mask) >> base_shift));
                c.b = (value_type)(sb + db - ((2 * sd_min(sb*da, db*sa) + base_mask) >> base_shift));
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=====================================================comp_op_rgb_p_exclusion
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_exclusion
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        typedef typename color_type::long_type long_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = (Sca.Da + Dca.Sa - 2.Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type d1a = 0;
                calc_type s1a = base_mask - sa;
                calc_type dr = c.r;
                calc_type dg = c.g;
                calc_type db = c.b;
                calc_type da = base_mask;
                c.r = (value_type)((sr*da + dr*sa - 2*sr*dr + sr*d1a + dr*s1a + base_mask) >> base_shift);
                c.g = (value_type)((sg*da + dg*sa - 2*sg*dg + sg*d1a + dg*s1a + base_mask) >> base_shift);
                c.b = (value_type)((sb*da + db*sa - 2*sb*db + sb*d1a + db*s1a + base_mask) >> base_shift);
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=====================================================comp_op_rgb_p_contrast
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_contrast
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        typedef typename color_type::long_type long_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };


        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
			color_type c = blender_type::make_color(*p);
            long_type dr = c.r;
            long_type dg = c.g;
            long_type db = c.b;
            int       da = base_mask;
            long_type d2a = da >> 1;
            unsigned s2a = sa >> 1;

            int r = (int)((((dr - d2a) * int((sr - s2a)*2 + base_mask)) >> base_shift) + d2a); 
            int g = (int)((((dg - d2a) * int((sg - s2a)*2 + base_mask)) >> base_shift) + d2a); 
            int b = (int)((((db - d2a) * int((sb - s2a)*2 + base_mask)) >> base_shift) + d2a); 

            r = (r < 0) ? 0 : r;
            g = (g < 0) ? 0 : g;
            b = (b < 0) ? 0 : b;

            c.r = (value_type)((r > da) ? da : r);
            c.g = (value_type)((g > da) ? da : g);
            c.b = (value_type)((b > da) ? da : b);
			*p = blender_type::make_pix(c.r, c.g, c.b);
        }
    };

    //=====================================================comp_op_rgb_p_invert
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_invert
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        typedef typename color_type::long_type long_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            sa = (sa * cover + 255) >> 8;
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type da = base_mask;
                calc_type dr = ((da - c.r) * sa + base_mask) >> base_shift;
                calc_type dg = ((da - c.g) * sa + base_mask) >> base_shift;
                calc_type db = ((da - c.b) * sa + base_mask) >> base_shift;
                calc_type s1a = base_mask - sa;
                c.r = (value_type)(dr + ((c.r * s1a + base_mask) >> base_shift));
                c.g = (value_type)(dg + ((c.g * s1a + base_mask) >> base_shift));
                c.b = (value_type)(db + ((c.b * s1a + base_mask) >> base_shift));
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };

    //=================================================comp_op_rgb_p_invert_rgb
    template<class ColorT, class Order, class Blender> struct comp_op_rgb_p_invert_rgb
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Blender blender_type;
        typedef int16u pixel_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        typedef typename color_type::long_type long_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sr = (sr * cover + 255) >> 8;
                sg = (sg * cover + 255) >> 8;
                sb = (sb * cover + 255) >> 8;
                sa = (sa * cover + 255) >> 8;
            }
            if(sa)
            {
				color_type c = blender_type::make_color(*p);
                calc_type da = base_mask;
                calc_type dr = ((da - c.r) * sr + base_mask) >> base_shift;
                calc_type dg = ((da - c.g) * sg + base_mask) >> base_shift;
                calc_type db = ((da - c.b) * sb + base_mask) >> base_shift;
                calc_type s1a = base_mask - sa;
                c.r = (value_type)(dr + ((c.r * s1a + base_mask) >> base_shift));
                c.g = (value_type)(dg + ((c.g * s1a + base_mask) >> base_shift));
                c.b = (value_type)(db + ((c.b * s1a + base_mask) >> base_shift));
				*p = blender_type::make_pix(c.r, c.g, c.b);
            }
        }
    };
    //==========================================================g_rgb_p_comp_op_func
    template<class ColorT, class Order, class Blender> 
    typename comp_op_table_rgb_p<ColorT, Order, Blender>::comp_op_func_type
    comp_op_table_rgb_p<ColorT, Order, Blender>::g_rgb_p_comp_op_func[] = 
    {
        comp_op_rgb_p_clear      <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_src        <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_src_over   <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_src_in     <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_src_out    <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_src_atop   <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_dst        <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_dst_over   <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_dst_in     <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_dst_out    <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_dst_atop   <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_xor        <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_darken     <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_lighten    <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_overlay    <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_screen     <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_multiply   <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_plus       <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_minus      <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_exclusion  <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_difference <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_soft_light <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_hard_light <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_color_burn <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_color_dodge <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_contrast   <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_invert     <ColorT,Order,Blender>::blend_pix,
        comp_op_rgb_p_invert_rgb <ColorT,Order,Blender>::blend_pix,
        0
    };

    //=========================================================blender_rgb555
    struct blender_rgb555
    {
        typedef rgba8 color_type;
        typedef order_rgb555 order_type;
        typedef color_type::value_type value_type;
        typedef color_type::calc_type calc_type;
        typedef int16u pixel_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        static AGG_INLINE void blend_pix(unsigned op, pixel_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb, 
                                         unsigned alpha, 
                                         unsigned cover=0)
        {
            comp_op_table_rgb_p<color_type, order_type, blender_rgb555>::g_rgb_p_comp_op_func[op]
                (p, (cr * alpha + base_mask) >> base_shift, 
                    (cg * alpha + base_mask) >> base_shift,
                    (cb * alpha + base_mask) >> base_shift,
                     alpha, cover);
        }

        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb,
                                         unsigned alpha, 
                                         unsigned)
        {
            register pixel_type rgb = *p;
            calc_type r = (rgb >> 7) & 0xF8;
            calc_type g = (rgb >> 2) & 0xF8;
            calc_type b = (rgb << 3) & 0xF8;
            *p = (pixel_type)
               (((((cr - r) * alpha + (r << 8)) >> 1)  & 0x7C00) |
                ((((cg - g) * alpha + (g << 8)) >> 6)  & 0x03E0) |
                 (((cb - b) * alpha + (b << 8)) >> 11) | 0x8000);
        }

        static AGG_INLINE pixel_type make_pix(unsigned r, unsigned g, unsigned b)
        {
            return (pixel_type)(((r & 0xF8) << 7) | 
                                ((g & 0xF8) << 2) | 
                                 (b >> 3) | 0x8000);
        }

        static AGG_INLINE color_type make_color(pixel_type p)
        {
            return color_type((p >> 7) & 0xF8, 
                              (p >> 2) & 0xF8, 
                              (p << 3) & 0xF8);
        }
    };


    //=====================================================blender_rgb555_pre
    struct blender_rgb555_pre
    {
        typedef rgba8 color_type;
        typedef order_rgb555 order_type;
        typedef color_type::value_type value_type;
        typedef color_type::calc_type calc_type;
        typedef int16u pixel_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        static AGG_INLINE void blend_pix(unsigned op, pixel_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb, 
                                         unsigned alpha, 
                                         unsigned cover=0)
        {
            comp_op_table_rgb_p<color_type, order_type, blender_rgb555_pre>::g_rgb_p_comp_op_func[op]
                (p, (cr * alpha + base_mask) >> base_shift, 
                    (cg * alpha + base_mask) >> base_shift,
                    (cb * alpha + base_mask) >> base_shift,
                     alpha, cover);
        }

        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb,
                                         unsigned alpha, 
                                         unsigned cover)
        {
            alpha = color_type::base_mask - alpha;
            pixel_type rgb = *p;
            calc_type r = (rgb >> 7) & 0xF8;
            calc_type g = (rgb >> 2) & 0xF8;
            calc_type b = (rgb << 3) & 0xF8;
            *p = (pixel_type)
               ((((r * alpha + cr * cover) >> 1)  & 0x7C00) |
                (((g * alpha + cg * cover) >> 6)  & 0x03E0) |
                 ((b * alpha + cb * cover) >> 11) | 0x8000);
        }

        static AGG_INLINE pixel_type make_pix(unsigned r, unsigned g, unsigned b)
        {
            return (pixel_type)(((r & 0xF8) << 7) | 
                                ((g & 0xF8) << 2) | 
                                 (b >> 3) | 0x8000);
        }

        static AGG_INLINE color_type make_color(pixel_type p)
        {
            return color_type((p >> 7) & 0xF8, 
                              (p >> 2) & 0xF8, 
                              (p << 3) & 0xF8);
        }
    };




    //=====================================================blender_rgb555_gamma
    template<class Gamma> class blender_rgb555_gamma
    {
    public:
        typedef rgba8 color_type;
        typedef order_rgb555 order_type;
        typedef color_type::value_type value_type;
        typedef color_type::calc_type calc_type;
        typedef int16u pixel_type;
        typedef Gamma gamma_type;

        blender_rgb555_gamma() : m_gamma(0) {}
        void gamma(const gamma_type& g) { m_gamma = &g; }

        AGG_INLINE void blend_pix(pixel_type* p, 
                                  unsigned cr, unsigned cg, unsigned cb,
                                  unsigned alpha, 
                                  unsigned)
        {
            pixel_type rgb = *p;
            calc_type r = m_gamma->dir((rgb >> 7) & 0xF8);
            calc_type g = m_gamma->dir((rgb >> 2) & 0xF8);
            calc_type b = m_gamma->dir((rgb << 3) & 0xF8);
            *p = (pixel_type)
               (((m_gamma->inv(((m_gamma->dir(cr) - r) * alpha + (r << 8)) >> 8) << 7) & 0x7C00) |
                ((m_gamma->inv(((m_gamma->dir(cg) - g) * alpha + (g << 8)) >> 8) << 2) & 0x03E0) |
                 (m_gamma->inv(((m_gamma->dir(cb) - b) * alpha + (b << 8)) >> 8) >> 3) | 0x8000);
        }

        static AGG_INLINE pixel_type make_pix(unsigned r, unsigned g, unsigned b)
        {
            return (pixel_type)(((r & 0xF8) << 7) | 
                                ((g & 0xF8) << 2) | 
                                 (b >> 3) | 0x8000);
        }

        static AGG_INLINE color_type make_color(pixel_type p)
        {
            return color_type((p >> 7) & 0xF8, 
                              (p >> 2) & 0xF8, 
                              (p << 3) & 0xF8);
        }

    private:
        const Gamma* m_gamma;
    };





    //=========================================================blender_rgb565
    struct blender_rgb565
    {
        typedef rgba8 color_type;
        typedef order_rgb565 order_type;
        typedef color_type::value_type value_type;
        typedef color_type::calc_type calc_type;
        typedef int16u pixel_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        static AGG_INLINE void blend_pix(unsigned op, pixel_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb, 
                                         unsigned alpha, 
                                         unsigned cover=0)
        {
            comp_op_table_rgb_p<color_type, order_type, blender_rgb565>::g_rgb_p_comp_op_func[op]
                (p, (cr * alpha + base_mask) >> base_shift, 
                    (cg * alpha + base_mask) >> base_shift,
                    (cb * alpha + base_mask) >> base_shift,
                     alpha, cover);
        }

        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb,
                                         unsigned alpha, 
                                         unsigned)
        {
            register pixel_type rgb = *p;
            calc_type r = (rgb >> 8) & 0xF8;
            calc_type g = (rgb >> 3) & 0xFC;
            calc_type b = (rgb << 3) & 0xF8;
            *p = (pixel_type)
               (((((cr - r) * alpha + (r << 8))     ) & 0xF800) |
                ((((cg - g) * alpha + (g << 8)) >> 5) & 0x07E0) |
                 (((cb - b) * alpha + (b << 8)) >> 11));
        }

        static AGG_INLINE pixel_type make_pix(unsigned r, unsigned g, unsigned b)
        {
            return (pixel_type)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
        }

        static AGG_INLINE color_type make_color(pixel_type p)
        {
            return color_type((p >> 8) & 0xF8, 
                              (p >> 3) & 0xFC, 
                              (p << 3) & 0xF8);
        }
    };



    //=====================================================blender_rgb565_pre
    struct blender_rgb565_pre
    {
        typedef rgba8 color_type;
        typedef order_rgb565 order_type;
        typedef color_type::value_type value_type;
        typedef color_type::calc_type calc_type;
        typedef int16u pixel_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        static AGG_INLINE void blend_pix(unsigned op, pixel_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb, 
                                         unsigned alpha, 
                                         unsigned cover=0)
        {
            comp_op_table_rgb_p<color_type, order_type, blender_rgb565_pre>::g_rgb_p_comp_op_func[op]
                (p, (cr * alpha + base_mask) >> base_shift, 
                    (cg * alpha + base_mask) >> base_shift,
                    (cb * alpha + base_mask) >> base_shift,
                     alpha, cover);
        }

        static AGG_INLINE void blend_pix(pixel_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb,
                                         unsigned alpha, 
                                         unsigned cover)
        {
            alpha = color_type::base_mask - alpha;
            pixel_type rgb = *p;
            calc_type r = (rgb >> 8) & 0xF8;
            calc_type g = (rgb >> 3) & 0xFC;
            calc_type b = (rgb << 3) & 0xF8;
            *p = (pixel_type)
               ((((r * alpha + cr * cover)      ) & 0xF800) |
                (((g * alpha + cg * cover) >> 5 ) & 0x07E0) |
                 ((b * alpha + cb * cover) >> 11));
        }

        static AGG_INLINE pixel_type make_pix(unsigned r, unsigned g, unsigned b)
        {
            return (pixel_type)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
        }

        static AGG_INLINE color_type make_color(pixel_type p)
        {
            return color_type((p >> 8) & 0xF8, 
                              (p >> 3) & 0xFC, 
                              (p << 3) & 0xF8);
        }
    };


    //===========================================pixfmt_alpha_blend_comp_rgb_packed
    template<class Blender,  class RenBuf> class pixfmt_alpha_blend_comp_rgb_packed
    {
    public:
        typedef RenBuf   rbuf_type;
        typedef typename rbuf_type::row_data row_data;
        typedef Blender  blender_type;
        typedef typename blender_type::pixel_type pixel_type;
        typedef typename blender_type::color_type color_type;
        typedef typename blender_type::order_type   order_type;
        typedef typename color_type::value_type   value_type;
        typedef typename color_type::calc_type    calc_type;
        enum base_scale_e 
        {
            base_shift = color_type::base_shift,
            base_scale = color_type::base_scale,
            base_mask  = color_type::base_mask,
            pix_width  = sizeof(pixel_type)
        };

        //--------------------------------------------------------------------
        pixfmt_alpha_blend_comp_rgb_packed() : m_rbuf(0), m_comp_op(comp_op_src_over), alpha_seed(base_mask) {}
        explicit pixfmt_alpha_blend_comp_rgb_packed(rbuf_type& rb,
													unsigned comp_op=comp_op_src_over,unsigned alpha=base_mask)
		   	: m_rbuf(&rb),
            m_comp_op(comp_op),
			alpha_seed(alpha) 
		{}
        void attach(rbuf_type& rb) { m_rbuf = &rb; }

        //--------------------------------------------------------------------
        template<class PixFmt>
        bool attach(PixFmt& pixf, int x1, int y1, int x2, int y2)
        {
            rect_i r(x1, y1, x2, y2);
            if(r.clip(rect_i(0, 0, pixf.width()-1, pixf.height()-1)))
            {
                int stride = pixf.stride();
                m_rbuf->attach(pixf.pix_ptr(r.x1, stride < 0 ? r.y2 : r.y1), 
                               (r.x2 - r.x1) + 1,
                               (r.y2 - r.y1) + 1,
                               stride);
                return true;
            }
            return false;
        }

        //--------------------------------------------------------------------
        AGG_INLINE unsigned width()  const { return m_rbuf->width();  }
        AGG_INLINE unsigned height() const { return m_rbuf->height(); }
        AGG_INLINE int      stride() const { return m_rbuf->stride(); }

        //--------------------------------------------------------------------
        AGG_INLINE       int8u* row_ptr(int y)       { return m_rbuf->row_ptr(y); }
        AGG_INLINE const int8u* row_ptr(int y) const { return m_rbuf->row_ptr(y); }
        AGG_INLINE row_data     row(int y)     const { return m_rbuf->row(y); }

		AGG_INLINE void alpha(float a) { alpha_seed = uround(a * base_mask); }
		AGG_INLINE float alpha() const { return (float)alpha_seed / 255.0; }
		AGG_INLINE unsigned alpha_mul(unsigned a, unsigned s)
		{
			return (s == 255) ? a : ((a * s + base_mask) >> base_shift);
		}
        //--------------------------------------------------------------------
        AGG_INLINE int8u* pix_ptr(int x, int y)
        {
            return m_rbuf->row_ptr(y) + x * pix_width;
        }

        AGG_INLINE const int8u* pix_ptr(int x, int y) const
        {
            return m_rbuf->row_ptr(y) + x * pix_width;
        }

        //--------------------------------------------------------------------
        AGG_INLINE  void comp_op(unsigned op) { m_comp_op = op; }
        AGG_INLINE  unsigned comp_op() const  { return m_comp_op; }

        //--------------------------------------------------------------------
        AGG_INLINE static void make_pix(int8u* p, const color_type& c)
        {
            *(pixel_type*)p = blender_type::make_pix(c.r, c.g, c.b);
        }

        //--------------------------------------------------------------------
        AGG_INLINE color_type pixel(int x, int y) const
        {
            return blender_type::make_color(((pixel_type*)m_rbuf->row_ptr(y))[x]);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void copy_pixel(int x, int y, const color_type& c)
        {
            ((pixel_type*)
                m_rbuf->row_ptr(x, y, 1))[x] = 
                    blender_type::make_pix(c.r, c.g, c.b);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void blend_pixel(int x, int y, const color_type& c, int8u cover)
        {
            blender_type::blend_pix(
                m_comp_op, 
                (pixel_type*)m_rbuf->row_ptr(x, y, 1) + x,
                c.r, c.g, c.b, (value_type)alpha_mul(c.a, alpha_seed), 
                cover);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void copy_hline(int x, int y, 
                                   unsigned len, 
                                   const color_type& c)
        {
            pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y, len) + x;
            pixel_type v = blender_type::make_pix(c.r, c.g, c.b);
            do
            {
                *p++ = v;
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void copy_vline(int x, int y,
                                   unsigned len, 
                                   const color_type& c)
        {
            pixel_type v = blender_type::make_pix(c.r, c.g, c.b);
            do
            {
                pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y++, 1) + x;
                *p = v;
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void blend_hline(int x, int y, unsigned len, 
                         const color_type& c, int8u cover)
        {
            pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y, len) + x;
            do
            {
                blender_type::blend_pix(m_comp_op, p, c.r, c.g, c.b, (value_type)alpha_mul(c.a, alpha_seed), cover);
                p++;
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void blend_vline(int x, int y, unsigned len, 
                         const color_type& c, int8u cover)
        {
            do
            {
                blender_type::blend_pix(
                    m_comp_op, 
                    (pixel_type*)m_rbuf->row_ptr(x, y++, 1) + x, 
                    c.r, c.g, c.b, (value_type)alpha_mul(c.a, alpha_seed), 
                    cover);
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void blend_solid_hspan(int x, int y,
                               unsigned len, 
                               const color_type& c,
                               const int8u* covers)
        {
            pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y, len) + x;
            do 
            {
                blender_type::blend_pix(m_comp_op, 
                                        p, c.r, c.g, c.b, (value_type)alpha_mul(c.a, alpha_seed), 
                                        *covers++);
                p++;
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void blend_solid_vspan(int x, int y, unsigned len, 
                               const color_type& c, const int8u* covers)
        {
            do 
            {
                blender_type::blend_pix(
                    m_comp_op, 
                    (pixel_type*)m_rbuf->row_ptr(x, y++, 1) + x, 
                    c.r, c.g, c.b, (value_type)alpha_mul(c.a, alpha_seed), 
                    *covers++);
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        void copy_color_hspan(int x, int y,
                              unsigned len, 
                              const color_type* colors)
        {
            pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y, len) + x;
            do 
            {
                *p++ = blender_type::make_pix(colors->r, colors->g, colors->b);
                ++colors;
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        void copy_color_vspan(int x, int y,
                              unsigned len, 
                              const color_type* colors)
        {
            do 
            {
                pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y++, 1) + x;
                *p = blender_type::make_pix(colors->r, colors->g, colors->b);
                ++colors;
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        void blend_color_hspan(int x, int y,
                               unsigned len, 
                               const color_type* colors,
                               const int8u* covers,
                               int8u cover)
        {
            pixel_type* p = (pixel_type*)m_rbuf->row_ptr(x, y, len) + x;
            do 
            {
                blender_type::blend_pix(m_comp_op, 
                                        p, 
                                        colors->r, 
                                        colors->g, 
                                        colors->b, 
                                        (value_type)alpha_mul(colors->a, alpha_seed), 
                                        covers ? *covers++ : cover);
                p++;
                ++colors;
            }
            while(--len);
        }

        //--------------------------------------------------------------------
        void blend_color_vspan(int x, int y,
                               unsigned len, 
                               const color_type* colors,
                               const int8u* covers,
                               int8u cover)
        {
            do 
            {
                blender_type::blend_pix(
                    m_comp_op, 
                    (pixel_type*)m_rbuf->row_ptr(x, y++, 1) + x, 
                    colors->r,
                    colors->g,
                    colors->b,
                    (value_type)alpha_mul(colors->a, alpha_seed),
                    covers ? *covers++ : cover);
                ++colors;
            }
            while(--len);
        }
        
        //--------------------------------------------------------------------
        template<class RenBuf2>
        void copy_from(const RenBuf2& from, 
                       int xdst, int ydst,
                       int xsrc, int ysrc,
                       unsigned len)
        {
            const int8u* p = from.row_ptr(ysrc);
            if(p)
            {
                memmove(m_rbuf->row_ptr(xdst, ydst, len) + xdst * pix_width, 
                        p + xsrc * pix_width, 
                        len * pix_width);
            }
        }

        //--------------------------------------------------------------------
        template<class RenBuf2>
        void copy_point_from(const RenBuf2& from, 
                       int xdst, int ydst,
                       int xsrc, int ysrc)
        {
            const int8u* p = from.row_ptr(ysrc);
            if(p)
            {
                memmove(m_rbuf->row_ptr(xdst, ydst, 1) + xdst * pix_width, 
                        p + xsrc * pix_width, pix_width);
            }
        }

        //--------------------------------------------------------------------
        template<class SrcPixelFormatRenderer>
        void blend_from(const SrcPixelFormatRenderer& from, 
                        int xdst, int ydst,
                        int xsrc, int ysrc,
                        unsigned len,
                        int8u cover)
        {
            typedef typename SrcPixelFormatRenderer::order_type src_order;

            const value_type* psrc = (const value_type*)from.row_ptr(ysrc);
            if(psrc)
            {
                psrc += xsrc * 4;
                pixel_type* pdst = 
                    (pixel_type*)m_rbuf->row_ptr(xdst, ydst, len) + xdst;
                do 
                {
                    blender_type::blend_pix(m_comp_op, 
                                            pdst, 
                                            psrc[src_order::R],
                                            psrc[src_order::G],
                                            psrc[src_order::B],
                                            (value_type)alpha_mul(psrc[src_order::A], alpha_seed),
                                            cover);
                    psrc += 4;
                    ++pdst;
                }
                while(--len);
            }
        }

        //--------------------------------------------------------------------
        template<class SrcPixelFormatRenderer>
        void blend_point_from(const SrcPixelFormatRenderer& from, 
                        int xdst, int ydst,
                        int xsrc, int ysrc,
                        int8u cover)
        {
            typedef typename SrcPixelFormatRenderer::order_type src_order;

            const value_type* psrc = (const value_type*)from.row_ptr(ysrc);
            if(psrc)
            {
                psrc += xsrc * 4;
                pixel_type* pdst = 
                    (pixel_type*)m_rbuf->row_ptr(xdst, ydst, 1) + xdst;
                    blender_type::blend_pix(m_comp_op, 
                                            pdst, 
                                            psrc[src_order::R],
                                            psrc[src_order::G],
                                            psrc[src_order::B],
                                            (value_type)alpha_mul(psrc[src_order::A], alpha_seed),
                                            cover);
            }
        }

        //--------------------------------------------------------------------
        template<class SrcPixelFormatRenderer>
        void blend_from_color(const SrcPixelFormatRenderer& from, 
                              const color_type& color,
                              int xdst, int ydst,
                              int xsrc, int ysrc,
                              unsigned len,
                              int8u cover)
        {
            typedef typename SrcPixelFormatRenderer::value_type src_value_type;
            const src_value_type* psrc = (src_value_type*)from.row_ptr(ysrc);
            if(psrc)
            {
                pixel_type* pdst = 
                    (pixel_type*)m_rbuf->row_ptr(xdst, ydst, len) + xdst;
                do 
                {
                    blender_type::blend_pix(m_comp_op,
                                            pdst, 
                                            color.r, color.g, color.b, (value_type)alpha_mul(color.a, alpha_seed),
                                            (*psrc * cover + base_mask) >> base_shift);
                    ++psrc;
                    ++pdst;
                }
                while(--len);
            }
        }

        //--------------------------------------------------------------------
        template<class SrcPixelFormatRenderer>
        void blend_from_lut(const SrcPixelFormatRenderer& from, 
                            const color_type* color_lut,
                            int xdst, int ydst,
                            int xsrc, int ysrc,
                            unsigned len,
                            int8u cover)
        {
            typedef typename SrcPixelFormatRenderer::value_type src_value_type;
            const src_value_type* psrc = (src_value_type*)from.row_ptr(ysrc);
            if(psrc)
            {
                pixel_type* pdst = 
                    (pixel_type*)m_rbuf->row_ptr(xdst, ydst, len) + xdst;

                do 
                {
                    const color_type& color = color_lut[*psrc];
                    blender_type::blend_pix(m_comp_op,
											pdst, 
                                        	color.r, color.g, color.b, (value_type)alpha_mul(color.a, alpha_seed),
                                        	cover);
                    ++psrc;
                    ++pdst;
                }
                while(--len);
            }
        }

    private:
        rbuf_type* m_rbuf;
        unsigned m_comp_op;
		unsigned alpha_seed;
    };


    typedef pixfmt_alpha_blend_comp_rgb_packed<blender_rgb555, rendering_buffer> pixfmt_rgb555; //----pixfmt_rgb555
    typedef pixfmt_alpha_blend_comp_rgb_packed<blender_rgb565, rendering_buffer> pixfmt_rgb565; //----pixfmt_rgb565
    typedef pixfmt_alpha_blend_comp_rgb_packed<blender_rgb555_pre, rendering_buffer> pixfmt_rgb555_pre; //----pixfmt_rgb555_pre
    typedef pixfmt_alpha_blend_comp_rgb_packed<blender_rgb565_pre, rendering_buffer> pixfmt_rgb565_pre; //----pixfmt_rgb565_pre

}

#endif

