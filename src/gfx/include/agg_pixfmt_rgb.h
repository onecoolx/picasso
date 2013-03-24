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

#ifndef AGG_PIXFMT_RGB_INCLUDED
#define AGG_PIXFMT_RGB_INCLUDED

#include <string.h>
#include "agg_basics.h"
#include "agg_color_rgba.h"
#include "agg_rendering_buffer.h"

namespace agg
{

    //=====================================================apply_gamma_dir_rgb
    template<class ColorT, class Order, class GammaLut> class apply_gamma_dir_rgb
    {
    public:
        typedef typename ColorT::value_type value_type;

        apply_gamma_dir_rgb(const GammaLut& gamma) : m_gamma(gamma) {}

        AGG_INLINE void operator () (value_type* p)
        {
            p[Order::R] = m_gamma.dir(p[Order::R]);
            p[Order::G] = m_gamma.dir(p[Order::G]);
            p[Order::B] = m_gamma.dir(p[Order::B]);
        }

    private:
        const GammaLut& m_gamma;
    };



    //=====================================================apply_gamma_inv_rgb
    template<class ColorT, class Order, class GammaLut> class apply_gamma_inv_rgb
    {
    public:
        typedef typename ColorT::value_type value_type;

        apply_gamma_inv_rgb(const GammaLut& gamma) : m_gamma(gamma) {}

        AGG_INLINE void operator () (value_type* p)
        {
            p[Order::R] = m_gamma.inv(p[Order::R]);
            p[Order::G] = m_gamma.inv(p[Order::G]);
            p[Order::B] = m_gamma.inv(p[Order::B]);
        }

    private:
        const GammaLut& m_gamma;
    };


    //=========================================================blender_rgb
    template<class ColorT, class Order> struct blender_rgb
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e { base_shift = color_type::base_shift };

        //--------------------------------------------------------------------
        static AGG_INLINE void blend_pix(value_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb, 
                                         unsigned alpha, 
                                         unsigned cover=0)
        {
            p[Order::R] += (value_type)(((cr - p[Order::R]) * alpha) >> base_shift);
            p[Order::G] += (value_type)(((cg - p[Order::G]) * alpha) >> base_shift);
            p[Order::B] += (value_type)(((cb - p[Order::B]) * alpha) >> base_shift);
        }
    };


    //======================================================blender_rgb_pre
    template<class ColorT, class Order> struct blender_rgb_pre
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e { base_shift = color_type::base_shift };

        //--------------------------------------------------------------------
        static AGG_INLINE void blend_pix(value_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb,
                                         unsigned alpha,
                                         unsigned cover)
        {
            alpha = color_type::base_mask - alpha;
            cover = (cover + 1) << (base_shift - 8);
            p[Order::R] = (value_type)((p[Order::R] * alpha + cr * cover) >> base_shift);
            p[Order::G] = (value_type)((p[Order::G] * alpha + cg * cover) >> base_shift);
            p[Order::B] = (value_type)((p[Order::B] * alpha + cb * cover) >> base_shift);
        }

        //--------------------------------------------------------------------
        static AGG_INLINE void blend_pix(value_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb,
                                         unsigned alpha)
        {
            alpha = color_type::base_mask - alpha;
            p[Order::R] = (value_type)(((p[Order::R] * alpha) >> base_shift) + cr);
            p[Order::G] = (value_type)(((p[Order::G] * alpha) >> base_shift) + cg);
            p[Order::B] = (value_type)(((p[Order::B] * alpha) >> base_shift) + cb);
        }

    };

    //======================================================comp_op_table_rgba
    template<class ColorT, class Order> struct comp_op_table_rgb
    {
        typedef typename ColorT::value_type value_type;
        typedef void (*comp_op_func_type)(value_type* p, 
                                          unsigned cr, 
                                          unsigned cg, 
                                          unsigned cb,
                                          unsigned ca,
                                          unsigned cover);
        static comp_op_func_type g_rgb_comp_op_func[];
    };

    //=========================================================comp_op_rgb_clear
    template<class ColorT, class Order> struct comp_op_rgb_clear
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        static AGG_INLINE void blend_pix(value_type* p, 
                                         unsigned, unsigned, unsigned, unsigned,
                                         unsigned cover)
        {
            if(cover < 255)
            {
                cover = 255 - cover;
                p[Order::R] = (value_type)((p[Order::R] * cover + 255) >> 8);
                p[Order::G] = (value_type)((p[Order::G] * cover + 255) >> 8);
                p[Order::B] = (value_type)((p[Order::B] * cover + 255) >> 8);
            }
            else
            {
                p[Order::R] = p[Order::G] = p[Order::B] = 0; 
            }
        }
    };

    //===========================================================comp_op_rgb_src
    template<class ColorT, class Order> struct comp_op_rgb_src
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;

        static AGG_INLINE void blend_pix(value_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                unsigned alpha = 255 - cover;
                p[Order::R] = (value_type)(((p[Order::R] * alpha + 255) >> 8) + ((sr * cover + 255) >> 8));
                p[Order::G] = (value_type)(((p[Order::G] * alpha + 255) >> 8) + ((sg * cover + 255) >> 8));
                p[Order::B] = (value_type)(((p[Order::B] * alpha + 255) >> 8) + ((sb * cover + 255) >> 8));
            }
            else
            {
                p[Order::R] = sr;
                p[Order::G] = sg;
                p[Order::B] = sb;
            }
        }
    };

    //===========================================================comp_op_rgb_dst
    template<class ColorT, class Order> struct comp_op_rgb_dst
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;

        static AGG_INLINE void blend_pix(value_type*, 
                                         unsigned, unsigned, unsigned, 
                                         unsigned, unsigned)
        {
        }
    };

    //======================================================comp_op_rgb_src_over
    template<class ColorT, class Order> struct comp_op_rgb_src_over
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        //   Dca' = Sca + Dca.(1 - Sa)
        //   Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(value_type* p, 
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
				p[Order::R] = (value_type)(sr + ((p[Order::R] * s1a + base_mask) >> base_shift));
				p[Order::G] = (value_type)(sg + ((p[Order::G] * s1a + base_mask) >> base_shift));
				p[Order::B] = (value_type)(sb + ((p[Order::B] * s1a + base_mask) >> base_shift));
			} else { // fast for opaque
				p[Order::R] = (value_type)sr;
				p[Order::G] = (value_type)sg;
				p[Order::B] = (value_type)sb;
			}
        }
    };

    //======================================================comp_op_rgb_dst_over
    template<class ColorT, class Order> struct comp_op_rgb_dst_over
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Dca + Sca.(1 - Da)
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(value_type* p, 
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
            p[Order::R] = (value_type)(p[Order::R] + (base_mask >> base_shift));
            p[Order::G] = (value_type)(p[Order::G] + (base_mask >> base_shift));
            p[Order::B] = (value_type)(p[Order::B] + (base_mask >> base_shift));
        }
    };

    //======================================================comp_op_rgb_src_in
    template<class ColorT, class Order> struct comp_op_rgb_src_in
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca.Da
        // Da'  = Sa.Da 
        static AGG_INLINE void blend_pix(value_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            calc_type da = base_mask;
            if(cover < 255)
            {
                unsigned alpha = 255 - cover;
                p[Order::R] = (value_type)(((p[Order::R] * alpha + 255) >> 8) + ((((sr * da + base_mask) >> base_shift) * cover + 255) >> 8));
                p[Order::G] = (value_type)(((p[Order::G] * alpha + 255) >> 8) + ((((sg * da + base_mask) >> base_shift) * cover + 255) >> 8));
                p[Order::B] = (value_type)(((p[Order::B] * alpha + 255) >> 8) + ((((sb * da + base_mask) >> base_shift) * cover + 255) >> 8));
            }
            else
            {
                p[Order::R] = (value_type)((sr * da + base_mask) >> base_shift);
                p[Order::G] = (value_type)((sg * da + base_mask) >> base_shift);
                p[Order::B] = (value_type)((sb * da + base_mask) >> base_shift);
            }
        }
    };

    //======================================================comp_op_rgb_dst_in
    template<class ColorT, class Order> struct comp_op_rgb_dst_in
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Dca.Sa
        // Da'  = Sa.Da 
        static AGG_INLINE void blend_pix(value_type* p, 
                                         unsigned, unsigned, unsigned, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sa = base_mask - ((cover * (base_mask - sa) + 255) >> 8);
            }
            p[Order::R] = (value_type)((p[Order::R] * sa + base_mask) >> base_shift);
            p[Order::G] = (value_type)((p[Order::G] * sa + base_mask) >> base_shift);
            p[Order::B] = (value_type)((p[Order::B] * sa + base_mask) >> base_shift);
        }
    };

    //======================================================comp_op_rgb_src_out
    template<class ColorT, class Order> struct comp_op_rgb_src_out
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca.(1 - Da)
        // Da'  = Sa.(1 - Da) 
        static AGG_INLINE void blend_pix(value_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            calc_type da = 0;
            if(cover < 255)
            {
                unsigned alpha = 255 - cover;
                p[Order::R] = (value_type)(((p[Order::R] * alpha + 255) >> 8) + ((((sr * da + base_mask) >> base_shift) * cover + 255) >> 8));
                p[Order::G] = (value_type)(((p[Order::G] * alpha + 255) >> 8) + ((((sg * da + base_mask) >> base_shift) * cover + 255) >> 8));
                p[Order::B] = (value_type)(((p[Order::B] * alpha + 255) >> 8) + ((((sb * da + base_mask) >> base_shift) * cover + 255) >> 8));
            }
            else
            {
                p[Order::R] = (value_type)((sr * da + base_mask) >> base_shift);
                p[Order::G] = (value_type)((sg * da + base_mask) >> base_shift);
                p[Order::B] = (value_type)((sb * da + base_mask) >> base_shift);
            }
        }
    };

    //======================================================comp_op_rgb_dst_out
    template<class ColorT, class Order> struct comp_op_rgb_dst_out
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Dca.(1 - Sa) 
        // Da'  = Da.(1 - Sa) 
        static AGG_INLINE void blend_pix(value_type* p, 
                                         unsigned, unsigned, unsigned, 
                                         unsigned sa, unsigned cover)
        {
            if(cover < 255)
            {
                sa = (sa * cover + 255) >> 8;
            }
            sa = base_mask - sa;
            p[Order::R] = (value_type)((p[Order::R] * sa + base_shift) >> base_shift);
            p[Order::G] = (value_type)((p[Order::G] * sa + base_shift) >> base_shift);
            p[Order::B] = (value_type)((p[Order::B] * sa + base_shift) >> base_shift);
        }
    };

    //=====================================================comp_op_rgb_src_atop
    template<class ColorT, class Order> struct comp_op_rgb_src_atop
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca.Da + Dca.(1 - Sa)
        // Da'  = Da
        static AGG_INLINE void blend_pix(value_type* p, 
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
            p[Order::R] = (value_type)((sr * da + p[Order::R] * sa + base_mask) >> base_shift);
            p[Order::G] = (value_type)((sg * da + p[Order::G] * sa + base_mask) >> base_shift);
            p[Order::B] = (value_type)((sb * da + p[Order::B] * sa + base_mask) >> base_shift);
        }
    };

    //=====================================================comp_op_rgb_dst_atop
    template<class ColorT, class Order> struct comp_op_rgb_dst_atop
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Dca.Sa + Sca.(1 - Da)
        // Da'  = Sa 
        static AGG_INLINE void blend_pix(value_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            calc_type da = 0;
            if(cover < 255)
            {
                unsigned alpha = 255 - cover;
                sr = (p[Order::R] * sa + sr * da + base_mask) >> base_shift;
                sg = (p[Order::G] * sa + sg * da + base_mask) >> base_shift;
                sb = (p[Order::B] * sa + sb * da + base_mask) >> base_shift;
                p[Order::R] = (value_type)(((p[Order::R] * alpha + 255) >> 8) + ((sr * cover + 255) >> 8));
                p[Order::G] = (value_type)(((p[Order::G] * alpha + 255) >> 8) + ((sg * cover + 255) >> 8));
                p[Order::B] = (value_type)(((p[Order::B] * alpha + 255) >> 8) + ((sb * cover + 255) >> 8));

            }
            else
            {
                p[Order::R] = (value_type)((p[Order::R] * sa + sr * da + base_mask) >> base_shift);
                p[Order::G] = (value_type)((p[Order::G] * sa + sg * da + base_mask) >> base_shift);
                p[Order::B] = (value_type)((p[Order::B] * sa + sb * da + base_mask) >> base_shift);
            }
        }
    };

    //=========================================================comp_op_rgb_xor
    template<class ColorT, class Order> struct comp_op_rgb_xor
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
        // Da'  = Sa + Da - 2.Sa.Da 
        static AGG_INLINE void blend_pix(value_type* p, 
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
                p[Order::R] = (value_type)((p[Order::R] * s1a + sr * d1a + base_mask) >> base_shift);
                p[Order::G] = (value_type)((p[Order::G] * s1a + sg * d1a + base_mask) >> base_shift);
                p[Order::B] = (value_type)((p[Order::B] * s1a + sb * d1a + base_mask) >> base_shift);
            }
        }
    };

    //=========================================================comp_op_rgb_plus
    template<class ColorT, class Order> struct comp_op_rgb_plus
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca + Dca
        // Da'  = Sa + Da 
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type dr = p[Order::R] + sr;
                calc_type dg = p[Order::G] + sg;
                calc_type db = p[Order::B] + sb;
                p[Order::R] = (dr > base_mask) ? (value_type)base_mask : dr;
                p[Order::G] = (dg > base_mask) ? (value_type)base_mask : dg;
                p[Order::B] = (db > base_mask) ? (value_type)base_mask : db;
            }
        }
    };

    //========================================================comp_op_rgb_minus
    template<class ColorT, class Order> struct comp_op_rgb_minus
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Dca - Sca
        // Da' = 1 - (1 - Sa).(1 - Da)
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type dr = p[Order::R] - sr;
                calc_type dg = p[Order::G] - sg;
                calc_type db = p[Order::B] - sb;
                p[Order::R] = (dr > base_mask) ? 0 : dr;
                p[Order::G] = (dg > base_mask) ? 0 : dg;
                p[Order::B] = (db > base_mask) ? 0 : db;
            }
        }
    };

    //=====================================================comp_op_rgb_multiply
    template<class ColorT, class Order> struct comp_op_rgb_multiply
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type dr = p[Order::R];
                calc_type dg = p[Order::G];
                calc_type db = p[Order::B];
                p[Order::R] = (value_type)((sr * dr + sr * d1a + dr * s1a + base_mask) >> base_shift);
                p[Order::G] = (value_type)((sg * dg + sg * d1a + dg * s1a + base_mask) >> base_shift);
                p[Order::B] = (value_type)((sb * db + sb * d1a + db * s1a + base_mask) >> base_shift);
            }
        }
    };

    //=====================================================comp_op_rgb_screen
    template<class ColorT, class Order> struct comp_op_rgb_screen
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = Sca + Dca - Sca.Dca
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type dr = p[Order::R];
                calc_type dg = p[Order::G];
                calc_type db = p[Order::B];
                p[Order::R] = (value_type)(sr + dr - ((sr * dr + base_mask) >> base_shift));
                p[Order::G] = (value_type)(sg + dg - ((sg * dg + base_mask) >> base_shift));
                p[Order::B] = (value_type)(sb + db - ((sb * db + base_mask) >> base_shift));
            }
        }
    };

    //=====================================================comp_op_rgb_overlay
    template<class ColorT, class Order> struct comp_op_rgb_overlay
    {
        typedef ColorT color_type;
        typedef Order order_type;
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
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type d1a  = 0;
                calc_type s1a  = base_mask - sa;
                calc_type dr   = p[Order::R];
                calc_type dg   = p[Order::G];
                calc_type db   = p[Order::B];
                calc_type da   = base_mask;
                calc_type sada = sa * base_mask;

                p[Order::R] = (value_type)(((2*dr < da) ? 
                    2*sr*dr + sr*d1a + dr*s1a : 
                    sada - 2*(da - dr)*(sa - sr) + sr*d1a + dr*s1a + base_mask) >> base_shift);

                p[Order::G] = (value_type)(((2*dg < da) ? 
                    2*sg*dg + sg*d1a + dg*s1a : 
                    sada - 2*(da - dg)*(sa - sg) + sg*d1a + dg*s1a + base_mask) >> base_shift);

                p[Order::B] = (value_type)(((2*db < da) ? 
                    2*sb*db + sb*d1a + db*s1a : 
                    sada - 2*(da - db)*(sa - sb) + sb*d1a + db*s1a + base_mask) >> base_shift);
            }
        }
    };

    //=====================================================comp_op_rgb_darken
    template<class ColorT, class Order> struct comp_op_rgb_darken
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = min(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type d1a = 0;
                calc_type s1a = base_mask - sa;
                calc_type dr  = p[Order::R];
                calc_type dg  = p[Order::G];
                calc_type db  = p[Order::B];
                calc_type da  = base_mask;

                p[Order::R] = (value_type)((sd_min(sr * da, dr * sa) + sr * d1a + dr * s1a + base_mask) >> base_shift);
                p[Order::G] = (value_type)((sd_min(sg * da, dg * sa) + sg * d1a + dg * s1a + base_mask) >> base_shift);
                p[Order::B] = (value_type)((sd_min(sb * da, db * sa) + sb * d1a + db * s1a + base_mask) >> base_shift);
            }
        }
    };

    //=====================================================comp_op_rgb_lighten
    template<class ColorT, class Order> struct comp_op_rgb_lighten
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        // Dca' = max(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
        // Da'  = Sa + Da - Sa.Da 
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type d1a = 0;
                calc_type s1a = base_mask - sa;
                calc_type dr  = p[Order::R];
                calc_type dg  = p[Order::G];
                calc_type db  = p[Order::B];
                calc_type da  = base_mask;

                p[Order::R] = (value_type)((sd_max(sr * da, dr * sa) + sr * d1a + dr * s1a + base_mask) >> base_shift);
                p[Order::G] = (value_type)((sd_max(sg * da, dg * sa) + sg * d1a + dg * s1a + base_mask) >> base_shift);
                p[Order::B] = (value_type)((sd_max(sb * da, db * sa) + sb * d1a + db * s1a + base_mask) >> base_shift);
            }
        }
    };


    //=====================================================comp_op_rgb_color_dodge
    template<class ColorT, class Order> struct comp_op_rgb_color_dodge
    {
        typedef ColorT color_type;
        typedef Order order_type;
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
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type d1a  = 0;
                calc_type s1a  = base_mask - sa;
                calc_type dr   = p[Order::R];
                calc_type dg   = p[Order::G];
                calc_type db   = p[Order::B];
                calc_type da   = base_mask;
                long_type drsa = dr * sa;
                long_type dgsa = dg * sa;
                long_type dbsa = db * sa;
                long_type srda = sr * da;
                long_type sgda = sg * da;
                long_type sbda = sb * da;
                long_type sada = sa * da;

                p[Order::R] = (value_type)((srda + drsa >= sada) ? 
                    (sada + sr * d1a + dr * s1a + base_mask) >> base_shift :
                    drsa / (base_mask - (sr << base_shift) / sa) + ((sr * d1a + dr * s1a + base_mask) >> base_shift));

                p[Order::G] = (value_type)((sgda + dgsa >= sada) ? 
                    (sada + sg * d1a + dg * s1a + base_mask) >> base_shift :
                    dgsa / (base_mask - (sg << base_shift) / sa) + ((sg * d1a + dg * s1a + base_mask) >> base_shift));

                p[Order::B] = (value_type)((sbda + dbsa >= sada) ? 
                    (sada + sb * d1a + db * s1a + base_mask) >> base_shift :
                    dbsa / (base_mask - (sb << base_shift) / sa) + ((sb * d1a + db * s1a + base_mask) >> base_shift));
            }
        }
    };

    //=====================================================comp_op_rgb_color_burn
    template<class ColorT, class Order> struct comp_op_rgb_color_burn
    {
        typedef ColorT color_type;
        typedef Order order_type;
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
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type d1a  = 0;
                calc_type s1a  = base_mask - sa;
                calc_type dr   = p[Order::R];
                calc_type dg   = p[Order::G];
                calc_type db   = p[Order::B];
                calc_type da   = base_mask;
                long_type drsa = dr * sa;
                long_type dgsa = dg * sa;
                long_type dbsa = db * sa;
                long_type srda = sr * da;
                long_type sgda = sg * da;
                long_type sbda = sb * da;
                long_type sada = sa * da;

                p[Order::R] = (value_type)(((srda + drsa <= sada) ? 
                    sr * d1a + dr * s1a :
                    sa * (srda + drsa - sada) / sr + sr * d1a + dr * s1a + base_mask) >> base_shift);

                p[Order::G] = (value_type)(((sgda + dgsa <= sada) ? 
                    sg * d1a + dg * s1a :
                    sa * (sgda + dgsa - sada) / sg + sg * d1a + dg * s1a + base_mask) >> base_shift);

                p[Order::B] = (value_type)(((sbda + dbsa <= sada) ? 
                    sb * d1a + db * s1a :
                    sa * (sbda + dbsa - sada) / sb + sb * d1a + db * s1a + base_mask) >> base_shift);
            }
        }
    };

    //=====================================================comp_op_rgb_hard_light
    template<class ColorT, class Order> struct comp_op_rgb_hard_light
    {
        typedef ColorT color_type;
        typedef Order order_type;
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
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type d1a  = 0;
                calc_type s1a  = base_mask - sa;
                calc_type dr   = p[Order::R];
                calc_type dg   = p[Order::G];
                calc_type db   = p[Order::B];
                calc_type da   = base_mask;
                calc_type sada = sa * da;

                p[Order::R] = (value_type)(((2*sr < sa) ? 
                    2*sr*dr + sr*d1a + dr*s1a : 
                    sada - 2*(da - dr)*(sa - sr) + sr*d1a + dr*s1a + base_mask) >> base_shift);

                p[Order::G] = (value_type)(((2*sg < sa) ? 
                    2*sg*dg + sg*d1a + dg*s1a : 
                    sada - 2*(da - dg)*(sa - sg) + sg*d1a + dg*s1a + base_mask) >> base_shift);

                p[Order::B] = (value_type)(((2*sb < sa) ? 
                    2*sb*db + sb*d1a + db*s1a : 
                    sada - 2*(da - db)*(sa - sb) + sb*d1a + db*s1a + base_mask) >> base_shift);
            }
        }
    };

    //=====================================================comp_op_rgb_soft_light
    template<class ColorT, class Order> struct comp_op_rgb_soft_light
    {
        typedef ColorT color_type;
        typedef Order order_type;
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

        static AGG_INLINE void blend_pix(value_type* p, 
                                         unsigned r, unsigned g, unsigned b, 
                                         unsigned a, unsigned cover)
        {
            float sr = float(r * cover) / (base_mask * 255);
            float sg = float(g * cover) / (base_mask * 255);
            float sb = float(b * cover) / (base_mask * 255);
            float sa = float(a * cover) / (base_mask * 255);
            if(sa > 0)
            {
                float dr = float(p[Order::R]) / base_mask;
                float dg = float(p[Order::G]) / base_mask;
                float db = float(p[Order::B]) / base_mask;
                float da = float(base_mask) / base_mask;
                if(cover < 255)
                {
                    a = (a * cover + 255) >> 8;
                }

                if(2*sr < sa)       dr = dr*(sa + (1 - dr/da)*(2*sr - sa)) + sr*(1 - da) + dr*(1 - sa);
                else if(8*dr <= da) dr = dr*(sa + (1 - dr/da)*(2*sr - sa)*(3 - 8*dr/da)) + sr*(1 - da) + dr*(1 - sa);
                else                dr = (dr*sa + (sqrtf(dr/da)*da - dr)*(2*sr - sa)) + sr*(1 - da) + dr*(1 - sa);

                if(2*sg < sa)       dg = dg*(sa + (1 - dg/da)*(2*sg - sa)) + sg*(1 - da) + dg*(1 - sa);
                else if(8*dg <= da) dg = dg*(sa + (1 - dg/da)*(2*sg - sa)*(3 - 8*dg/da)) + sg*(1 - da) + dg*(1 - sa);
                else                dg = (dg*sa + (sqrtf(dg/da)*da - dg)*(2*sg - sa)) + sg*(1 - da) + dg*(1 - sa);

                if(2*sb < sa)       db = db*(sa + (1 - db/da)*(2*sb - sa)) + sb*(1 - da) + db*(1 - sa);
                else if(8*db <= da) db = db*(sa + (1 - db/da)*(2*sb - sa)*(3 - 8*db/da)) + sb*(1 - da) + db*(1 - sa);
                else                db = (db*sa + (sqrtf(db/da)*da - db)*(2*sb - sa)) + sb*(1 - da) + db*(1 - sa);

                p[Order::R] = (value_type)uround(dr * base_mask);
                p[Order::G] = (value_type)uround(dg * base_mask);
                p[Order::B] = (value_type)uround(db * base_mask);
            }
        }
    };

    //=====================================================comp_op_rgb_difference
    template<class ColorT, class Order> struct comp_op_rgb_difference
    {
        typedef ColorT color_type;
        typedef Order order_type;
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
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type dr = p[Order::R];
                calc_type dg = p[Order::G];
                calc_type db = p[Order::B];
                calc_type da = base_mask;
                p[Order::R] = (value_type)(sr + dr - ((2 * sd_min(sr*da, dr*sa) + base_mask) >> base_shift));
                p[Order::G] = (value_type)(sg + dg - ((2 * sd_min(sg*da, dg*sa) + base_mask) >> base_shift));
                p[Order::B] = (value_type)(sb + db - ((2 * sd_min(sb*da, db*sa) + base_mask) >> base_shift));
            }
        }
    };

    //=====================================================comp_op_rgb_exclusion
    template<class ColorT, class Order> struct comp_op_rgb_exclusion
    {
        typedef ColorT color_type;
        typedef Order order_type;
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
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type d1a = 0;
                calc_type s1a = base_mask - sa;
                calc_type dr = p[Order::R];
                calc_type dg = p[Order::G];
                calc_type db = p[Order::B];
                calc_type da = base_mask;
                p[Order::R] = (value_type)((sr*da + dr*sa - 2*sr*dr + sr*d1a + dr*s1a + base_mask) >> base_shift);
                p[Order::G] = (value_type)((sg*da + dg*sa - 2*sg*dg + sg*d1a + dg*s1a + base_mask) >> base_shift);
                p[Order::B] = (value_type)((sb*da + db*sa - 2*sb*db + sb*d1a + db*s1a + base_mask) >> base_shift);
            }
        }
    };

    //=====================================================comp_op_rgb_contrast
    template<class ColorT, class Order> struct comp_op_rgb_contrast
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        typedef typename color_type::long_type long_type;
        enum base_scale_e
        { 
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };


        static AGG_INLINE void blend_pix(value_type* p, 
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
            long_type dr = p[Order::R];
            long_type dg = p[Order::G];
            long_type db = p[Order::B];
            int       da = base_mask;
            long_type d2a = da >> 1;
            unsigned s2a = sa >> 1;

            int r = (int)((((dr - d2a) * int((sr - s2a)*2 + base_mask)) >> base_shift) + d2a); 
            int g = (int)((((dg - d2a) * int((sg - s2a)*2 + base_mask)) >> base_shift) + d2a); 
            int b = (int)((((db - d2a) * int((sb - s2a)*2 + base_mask)) >> base_shift) + d2a); 

            r = (r < 0) ? 0 : r;
            g = (g < 0) ? 0 : g;
            b = (b < 0) ? 0 : b;

            p[Order::R] = (value_type)((r > da) ? da : r);
            p[Order::G] = (value_type)((g > da) ? da : g);
            p[Order::B] = (value_type)((b > da) ? da : b);
        }
    };

    //=====================================================comp_op_rgb_invert
    template<class ColorT, class Order> struct comp_op_rgb_invert
    {
        typedef ColorT color_type;
        typedef Order order_type;
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
        static AGG_INLINE void blend_pix(value_type* p, 
                                         unsigned sr, unsigned sg, unsigned sb, 
                                         unsigned sa, unsigned cover)
        {
            sa = (sa * cover + 255) >> 8;
            if(sa)
            {
                calc_type da = base_mask;
                calc_type dr = ((da - p[Order::R]) * sa + base_mask) >> base_shift;
                calc_type dg = ((da - p[Order::G]) * sa + base_mask) >> base_shift;
                calc_type db = ((da - p[Order::B]) * sa + base_mask) >> base_shift;
                calc_type s1a = base_mask - sa;
                p[Order::R] = (value_type)(dr + ((p[Order::R] * s1a + base_mask) >> base_shift));
                p[Order::G] = (value_type)(dg + ((p[Order::G] * s1a + base_mask) >> base_shift));
                p[Order::B] = (value_type)(db + ((p[Order::B] * s1a + base_mask) >> base_shift));
            }
        }
    };

    //=================================================comp_op_rgb_invert_rgb
    template<class ColorT, class Order> struct comp_op_rgb_invert_rgb
    {
        typedef ColorT color_type;
        typedef Order order_type;
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
        static AGG_INLINE void blend_pix(value_type* p, 
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
                calc_type da = base_mask;
                calc_type dr = ((da - p[Order::R]) * sr + base_mask) >> base_shift;
                calc_type dg = ((da - p[Order::G]) * sg + base_mask) >> base_shift;
                calc_type db = ((da - p[Order::B]) * sb + base_mask) >> base_shift;
                calc_type s1a = base_mask - sa;
                p[Order::R] = (value_type)(dr + ((p[Order::R] * s1a + base_mask) >> base_shift));
                p[Order::G] = (value_type)(dg + ((p[Order::G] * s1a + base_mask) >> base_shift));
                p[Order::B] = (value_type)(db + ((p[Order::B] * s1a + base_mask) >> base_shift));
            }
        }
    };
    //==========================================================g_rgb_comp_op_func
    template<class ColorT, class Order> 
    typename comp_op_table_rgb<ColorT, Order>::comp_op_func_type
    comp_op_table_rgb<ColorT, Order>::g_rgb_comp_op_func[] = 
    {
        comp_op_rgb_clear      <ColorT,Order>::blend_pix,
        comp_op_rgb_src        <ColorT,Order>::blend_pix,
        comp_op_rgb_src_over   <ColorT,Order>::blend_pix,
        comp_op_rgb_src_in     <ColorT,Order>::blend_pix,
        comp_op_rgb_src_out    <ColorT,Order>::blend_pix,
        comp_op_rgb_src_atop   <ColorT,Order>::blend_pix,
        comp_op_rgb_dst        <ColorT,Order>::blend_pix,
        comp_op_rgb_dst_over   <ColorT,Order>::blend_pix,
        comp_op_rgb_dst_in     <ColorT,Order>::blend_pix,
        comp_op_rgb_dst_out    <ColorT,Order>::blend_pix,
        comp_op_rgb_dst_atop   <ColorT,Order>::blend_pix,
        comp_op_rgb_xor        <ColorT,Order>::blend_pix,
        comp_op_rgb_darken     <ColorT,Order>::blend_pix,
        comp_op_rgb_lighten    <ColorT,Order>::blend_pix,
        comp_op_rgb_overlay    <ColorT,Order>::blend_pix,
        comp_op_rgb_screen     <ColorT,Order>::blend_pix,
        comp_op_rgb_multiply   <ColorT,Order>::blend_pix,
        comp_op_rgb_plus       <ColorT,Order>::blend_pix,
        comp_op_rgb_minus      <ColorT,Order>::blend_pix,
        comp_op_rgb_exclusion  <ColorT,Order>::blend_pix,
        comp_op_rgb_difference <ColorT,Order>::blend_pix,
        comp_op_rgb_soft_light <ColorT,Order>::blend_pix,
        comp_op_rgb_hard_light <ColorT,Order>::blend_pix,
        comp_op_rgb_color_burn <ColorT,Order>::blend_pix,
        comp_op_rgb_color_dodge <ColorT,Order>::blend_pix,
        comp_op_rgb_contrast   <ColorT,Order>::blend_pix,
        comp_op_rgb_invert     <ColorT,Order>::blend_pix,
        comp_op_rgb_invert_rgb <ColorT,Order>::blend_pix,
        0
    };


    //=========================================================comp_op_adaptor_rgb
    template<class ColorT, class Order> struct comp_op_adaptor_rgb
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        enum base_scale_e 
		{
		   	base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask 
		};

        static AGG_INLINE void blend_pix(unsigned op, value_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb, 
                                         unsigned alpha, 
                                         unsigned cover=0)
        {
            comp_op_table_rgb<ColorT, Order>::g_rgb_comp_op_func[op]
                (p, (cr * alpha + base_mask) >> base_shift, 
                    (cg * alpha + base_mask) >> base_shift,
                    (cb * alpha + base_mask) >> base_shift,
                     alpha, cover);
        }
    };

    //=========================================================comp_op_adaptor_rgb_pre
    template<class ColorT, class Order> struct comp_op_adaptor_rgb_pre
    {
        typedef ColorT color_type;
        typedef Order order_type;
        typedef typename color_type::value_type value_type;
        enum base_scale_e 
		{
		   	base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask 
		};

        static AGG_INLINE void blend_pix(unsigned op, value_type* p, 
                                         unsigned cr, unsigned cg, unsigned cb, 
                                         unsigned alpha, 
                                         unsigned cover)
        {
            comp_op_table_rgb<ColorT, Order>::g_rgb_comp_op_func[op](p, cr, cg, cb, alpha, cover);
        }
    };



    //===================================================blender_rgb_gamma
    template<class ColorT, class Order, class Gamma> class blender_rgb_gamma
    {
    public:
        typedef ColorT color_type;
        typedef Order order_type;
        typedef Gamma gamma_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e { base_shift = color_type::base_shift };

        //--------------------------------------------------------------------
        blender_rgb_gamma() : m_gamma(0) {}
        void gamma(const gamma_type& g) { m_gamma = &g; }

        //--------------------------------------------------------------------
        AGG_INLINE void blend_pix(value_type* p, 
                                  unsigned cr, unsigned cg, unsigned cb,
                                  unsigned alpha, 
                                  unsigned cover=0)
        {
            calc_type r = m_gamma->dir(p[Order::R]);
            calc_type g = m_gamma->dir(p[Order::G]);
            calc_type b = m_gamma->dir(p[Order::B]);
            p[Order::R] = m_gamma->inv((((m_gamma->dir(cr) - r) * alpha) >> base_shift) + r);
            p[Order::G] = m_gamma->inv((((m_gamma->dir(cg) - g) * alpha) >> base_shift) + g);
            p[Order::B] = m_gamma->inv((((m_gamma->dir(cb) - b) * alpha) >> base_shift) + b);
        }

    private:
        const gamma_type* m_gamma;
    };


    //==================================================pixfmt_alpha_blend_comp_rgb
    template<class Blender, class RenBuf> class pixfmt_alpha_blend_comp_rgb
    {
    public:
        typedef RenBuf   rbuf_type;
        typedef typename rbuf_type::row_data row_data;
        typedef int32u   pixel_type;
        typedef Blender  blender_type;
        typedef typename blender_type::color_type color_type;
        typedef typename blender_type::order_type order_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e 
        {
            base_shift = color_type::base_shift,
            base_scale = color_type::base_scale,
            base_mask  = color_type::base_mask,
            pix_width  = sizeof(value_type) * 3
        };


        //--------------------------------------------------------------------
        pixfmt_alpha_blend_comp_rgb() : m_rbuf(0), m_comp_op(comp_op_src_over), alpha_seed(base_mask){}
        explicit pixfmt_alpha_blend_comp_rgb(rbuf_type& rb, unsigned comp_op=comp_op_src_over, unsigned alpha=base_mask)
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
            ((value_type*)p)[order_type::R] = c.r;
            ((value_type*)p)[order_type::G] = c.g;
            ((value_type*)p)[order_type::B] = c.b;
        }

        //--------------------------------------------------------------------
        AGG_INLINE color_type pixel(int x, int y) const
        {
            const value_type* p = (value_type*)m_rbuf->row_ptr(y) + x + x + x;
            return color_type(p[order_type::R], 
                              p[order_type::G], 
                              p[order_type::B]);
        }

        //--------------------------------------------------------------------
        AGG_INLINE void copy_pixel(int x, int y, const color_type& c)
        {
            value_type* p = (value_type*)m_rbuf->row_ptr(x, y, 1) + x + x + x;
            p[order_type::R] = c.r;
            p[order_type::G] = c.g;
            p[order_type::B] = c.b;
        }

        //--------------------------------------------------------------------
        AGG_INLINE void blend_pixel(int x, int y, const color_type& c, int8u cover)
        {
            blender_type::blend_pix(
                m_comp_op, 
                (value_type*)m_rbuf->row_ptr(x, y, 1) + x + x + x,
                c.r, c.g, c.b, (value_type)alpha_mul(c.a, alpha_seed), 
                cover);
        }


        //--------------------------------------------------------------------
        AGG_INLINE void copy_hline(int x, int y, 
                                   unsigned len, 
                                   const color_type& c)
        {
            value_type* p = (value_type*)m_rbuf->row_ptr(x, y, len) + x + x + x;
            do
            {
                p[order_type::R] = c.r; 
                p[order_type::G] = c.g; 
                p[order_type::B] = c.b;
                p += 3;
            }
            while(--len);
        }


        //--------------------------------------------------------------------
        AGG_INLINE void copy_vline(int x, int y,
                                   unsigned len, 
                                   const color_type& c)
        {
            do
            {
                value_type* p = (value_type*)
                    m_rbuf->row_ptr(x, y++, 1) + x + x + x;
                p[order_type::R] = c.r; 
                p[order_type::G] = c.g; 
                p[order_type::B] = c.b;
            }
            while(--len);
        }


        //--------------------------------------------------------------------
        AGG_INLINE void blend_hline(int x, int y, unsigned len, 
                         const color_type& c, int8u cover)
        {
            value_type* p = (value_type*)m_rbuf->row_ptr(x, y, len) + x + x + x;
            do
            {
                blender_type::blend_pix(m_comp_op, p, c.r, c.g, c.b, (value_type)alpha_mul(c.a, alpha_seed), cover);
                p += 3;
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
                    (value_type*)m_rbuf->row_ptr(x, y++, 1) + x + x + x, 
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
            value_type* p = (value_type*)m_rbuf->row_ptr(x, y, len) + x + x + x;
            do 
            {
                blender_type::blend_pix(m_comp_op, 
                                        p, c.r, c.g, c.b, (value_type)alpha_mul(c.a, alpha_seed), 
                                        *covers++);
                p += 3;
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
                    (value_type*)m_rbuf->row_ptr(x, y++, 1) + x + x + x, 
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
            value_type* p = (value_type*)m_rbuf->row_ptr(x, y, len) + x + x + x;
            do 
            {
                p[order_type::R] = colors->r;
                p[order_type::G] = colors->g;
                p[order_type::B] = colors->b;
                ++colors;
                p += 3;
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
                value_type* p = (value_type*)m_rbuf->row_ptr(x, y++, 1) + x + x + x;
                p[order_type::R] = colors->r;
                p[order_type::G] = colors->g;
                p[order_type::B] = colors->b;
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
            value_type* p = (value_type*)m_rbuf->row_ptr(x, y, len) + x + x + x;
            do 
            {
                blender_type::blend_pix(m_comp_op, 
                                        p, 
                                        colors->r, 
                                        colors->g, 
                                        colors->b, 
                                        (value_type)alpha_mul(colors->a, alpha_seed), 
                                        covers ? *covers++ : cover);
                p += 3;
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
                    (value_type*)m_rbuf->row_ptr(x, y++, 1) + x + x + x,
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
        template<class Function> void for_each_pixel(Function f)
        {
            unsigned y;
            for(y = 0; y < height(); ++y)
            {
                row_data r = m_rbuf->row(y);
                if(r.ptr)
                {
                    unsigned len = r.x2 - r.x1 + 1;
                    value_type* p = (value_type*)
                        m_rbuf->row_ptr(r.x1, y, len) + r.x1 * 3;
                    do
                    {
                        f(p);
                        p += 3;
                    }
                    while(--len);
                }
            }
        }

        //--------------------------------------------------------------------
        template<class GammaLut> void apply_gamma_dir(const GammaLut& g)
        {
            for_each_pixel(apply_gamma_dir_rgb<color_type, order_type, GammaLut>(g));
        }

        //--------------------------------------------------------------------
        template<class GammaLut> void apply_gamma_inv(const GammaLut& g)
        {
            for_each_pixel(apply_gamma_inv_rgb<color_type, order_type, GammaLut>(g));
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
                value_type* pdst = 
                    (value_type*)m_rbuf->row_ptr(xdst, ydst, len) + xdst * 3;   

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
					pdst += 3;
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
                value_type* pdst = 
                    (value_type*)m_rbuf->row_ptr(xdst, ydst, 1) + xdst * 3;   

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
                value_type* pdst = 
                    (value_type*)m_rbuf->row_ptr(xdst, ydst, len) + xdst * 3;
                do 
                {
                    blender_type::blend_pix(m_comp_op,
                                            pdst, 
                                            color.r, color.g, color.b, alpha_mul(color.a, alpha_seed),
                                            (*psrc * cover + base_mask) >> base_shift);
                    ++psrc;
                    pdst += 3;
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
                value_type* pdst = 
                    (value_type*)m_rbuf->row_ptr(xdst, ydst, len) + xdst * 3;

				do 
				{
					const color_type& color = color_lut[*psrc];
                    blender_type::blend_pix(m_comp_op,
                                            pdst, 
                                            color.r, color.g, color.b, alpha_mul(color.a, alpha_seed),
                                            cover);
					++psrc;
					pdst += 3;
				}
				while(--len);
            }
        }

    private:
        rbuf_type* m_rbuf;
        unsigned m_comp_op;
		unsigned alpha_seed;
    };

	typedef comp_op_adaptor_rgb<rgba8, order_rgb> blender_rgb24;
	typedef comp_op_adaptor_rgb<rgba8, order_bgr> blender_bgr24;
	typedef comp_op_adaptor_rgb_pre<rgba8, order_rgb> blender_rgb24_pre;
	typedef comp_op_adaptor_rgb_pre<rgba8, order_bgr> blender_bgr24_pre;

    typedef pixfmt_alpha_blend_comp_rgb<blender_rgb24, rendering_buffer> pixfmt_rgb24;    //----pixfmt_rgb24
    typedef pixfmt_alpha_blend_comp_rgb<blender_bgr24, rendering_buffer> pixfmt_bgr24;    //----pixfmt_bgr24
    typedef pixfmt_alpha_blend_comp_rgb<blender_rgb24_pre, rendering_buffer> pixfmt_rgb24_pre; //----pixfmt_rgb24_pre
    typedef pixfmt_alpha_blend_comp_rgb<blender_bgr24_pre, rendering_buffer> pixfmt_bgr24_pre; //----pixfmt_bgr24_pre

}

#endif

