/* Picasso - a vector graphics library
 *
 * Copyright (C) 2024 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_COMPOSITE_PACKED_H_
#define _GFX_COMPOSITE_PACKED_H_

#include "common.h"

namespace gfx {

// composite_op_packed_clear
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_clear {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;

    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t, uint32_t,
                                         uint32_t, uint32_t, uint32_t cover)
    {
        if (cover < 255) {
            cover = 255 - cover;
            color_type c = blender_type::make_color(*p);
            c.r = (value_type)((c.r * cover + 255) >> 8);
            c.g = (value_type)((c.g * cover + 255) >> 8);
            c.b = (value_type)((c.b * cover + 255) >> 8);
            *p = blender_type::make_pix(c.r, c.g, c.b);
        } else {
            *p = blender_type::make_pix(0, 0, 0);
        }
    }
};

// composite_op_packed_src
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_src {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;

    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            uint32_t alpha = 255 - cover;
            color_type c = blender_type::make_color(*p);
            c.r = (value_type)(((c.r * alpha + 255) >> 8) + ((sr * cover + 255) >> 8));
            c.g = (value_type)(((c.g * alpha + 255) >> 8) + ((sg * cover + 255) >> 8));
            c.b = (value_type)(((c.b * alpha + 255) >> 8) + ((sb * cover + 255) >> 8));
            *p = blender_type::make_pix(c.r, c.g, c.b);
        } else {
            *p = blender_type::make_pix(sr, sg, sb);
        }
    }
};

// composite_op_packed_dst
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_dst {
    typedef typename Blender::pixel_type pixel_type;

    static _FORCE_INLINE_ void blend_pix(pixel_type*, uint32_t, uint32_t,
                                         uint32_t, uint32_t, uint32_t)
    {
    }
};

// comp_op_rgb_src_over
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_src_over {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa < 255) {
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

// comp_op_rgb_dst_over
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_dst_over {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Dca + Sca.(1 - Da)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
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

// composite_op_packed_src_in
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_src_in {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca.Da
    // Da'  = Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        calc_type da = base_mask;
        color_type c = blender_type::make_color(*p);
        if (cover < 255) {
            uint32_t alpha = 255 - cover;
            c.r = (value_type)(((c.r * alpha + 255) >> 8) +
                               ((((sr * da + base_mask) >> base_shift) * cover + 255) >> 8));
            c.g = (value_type)(((c.g * alpha + 255) >> 8) +
                               ((((sg * da + base_mask) >> base_shift) * cover + 255) >> 8));
            c.b = (value_type)(((c.b * alpha + 255) >> 8) +
                               ((((sb * da + base_mask) >> base_shift) * cover + 255) >> 8));
        } else {
            c.r = (value_type)((sr * da + base_mask) >> base_shift);
            c.g = (value_type)((sg * da + base_mask) >> base_shift);
            c.b = (value_type)((sb * da + base_mask) >> base_shift);
        }
        *p = blender_type::make_pix(c.r, c.g, c.b);
    }
};

// composite_op_packed_dst_in
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_dst_in {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Dca.Sa
    // Da'  = Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t, uint32_t, uint32_t,
                                         uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sa = base_mask - ((cover * (base_mask - sa) + 255) >> 8);
        }

        color_type c = blender_type::make_color(*p);
        c.r = (value_type)((c.r * sa + base_mask) >> base_shift);
        c.g = (value_type)((c.g * sa + base_mask) >> base_shift);
        c.b = (value_type)((c.b * sa + base_mask) >> base_shift);
        *p = blender_type::make_pix(c.r, c.g, c.b);
    }
};

// composite_op_packed_src_out
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_src_out {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca.(1 - Da)
    // Da'  = Sa.(1 - Da)
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        calc_type da = 0;
        color_type c = blender_type::make_color(*p);
        if (cover < 255) {
            uint32_t alpha = 255 - cover;
            c.r = (value_type)(((c.r * alpha + 255) >> 8) +
                               ((((sr * da + base_mask) >> base_shift) * cover + 255) >> 8));
            c.g = (value_type)(((c.g * alpha + 255) >> 8) +
                               ((((sg * da + base_mask) >> base_shift) * cover + 255) >> 8));
            c.b = (value_type)(((c.b * alpha + 255) >> 8) +
                               ((((sb * da + base_mask) >> base_shift) * cover + 255) >> 8));
        } else {
            c.r = (value_type)((sr * da + base_mask) >> base_shift);
            c.g = (value_type)((sg * da + base_mask) >> base_shift);
            c.b = (value_type)((sb * da + base_mask) >> base_shift);
        }
        *p = blender_type::make_pix(c.r, c.g, c.b);
    }
};

// composite_op_packed_dst_out
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_dst_out {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Dca.(1 - Sa)
    // Da'  = Da.(1 - Sa)
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t, uint32_t, uint32_t,
                                         uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
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

// composite_op_packed_src_atop
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_src_atop {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca.Da + Dca.(1 - Sa)
    // Da'  = Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
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

// composite_op_packed_dst_atop
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_dst_atop {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Dca.Sa + Sca.(1 - Da)
    // Da'  = Sa
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        calc_type da = 0;
        color_type c = blender_type::make_color(*p);

        if (cover < 255) {
            uint32_t alpha = 255 - cover;
            sr = (c.r * sa + sr * da + base_mask) >> base_shift;
            sg = (c.g * sa + sg * da + base_mask) >> base_shift;
            sb = (c.b * sa + sb * da + base_mask) >> base_shift;
            c.r = (value_type)(((c.r * alpha + 255) >> 8) + ((sr * cover + 255) >> 8));
            c.g = (value_type)(((c.g * alpha + 255) >> 8) + ((sg * cover + 255) >> 8));
            c.b = (value_type)(((c.b * alpha + 255) >> 8) + ((sb * cover + 255) >> 8));
        } else {
            c.r = (value_type)((c.r * sa + sr * da + base_mask) >> base_shift);
            c.g = (value_type)((c.g * sa + sg * da + base_mask) >> base_shift);
            c.b = (value_type)((c.b * sa + sb * da + base_mask) >> base_shift);
        }
        *p = blender_type::make_pix(c.r, c.g, c.b);
    }
};

// composite_op_packed_xor
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_xor {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa + Da - 2.Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
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

// composite_op_packed_plus
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_plus {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca + Dca
    // Da'  = Sa + Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
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

// composite_op_packed_minus
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_minus {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Dca - Sca
    // Da' = 1 - (1 - Sa).(1 - Da)
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
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

// composite_op_packed_multiply
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_multiply {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
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

// composite_op_packed_screen
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_screen {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca + Dca - Sca.Dca
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
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

// composite_op_packed_overlay
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_overlay {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // if 2.Dca < Da
    // Dca' = 2.Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
    // otherwise
    // Dca' = Sa.Da - 2.(Da - Dca).(Sa - Sca) + Sca.(1 - Da) + Dca.(1 - Sa)
    //
    // Da' = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            color_type c = blender_type::make_color(*p);
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = c.r;
            calc_type dg = c.g;
            calc_type db = c.b;
            calc_type da = base_mask;
            calc_type sada = sa * base_mask;

            c.r = (value_type)(((2 * dr < da) ?
                                2 * sr * dr + sr * d1a + dr* s1a :
                                sada - 2 * (da - dr) * (sa - sr) + sr * d1a + dr * s1a + base_mask) >> base_shift);

            c.g = (value_type)(((2 * dg < da) ?
                                2 * sg * dg + sg * d1a + dg* s1a :
                                sada - 2 * (da - dg) * (sa - sg) + sg * d1a + dg * s1a + base_mask) >> base_shift);

            c.b = (value_type)(((2 * db < da) ?
                                2 * sb * db + sb * d1a + db* s1a :
                                sada - 2 * (da - db) * (sa - sb) + sb * d1a + db * s1a + base_mask) >> base_shift);
            *p = blender_type::make_pix(c.r, c.g, c.b);
        }
    }
};

// composite_op_packed_darken
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_darken {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = min(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            color_type c = blender_type::make_color(*p);
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = c.r;
            calc_type dg = c.g;
            calc_type db = c.b;
            calc_type da = base_mask;

            c.r = (value_type)((Min(sr * da, dr * sa) + sr * d1a + dr * s1a + base_mask) >> base_shift);
            c.g = (value_type)((Min(sg * da, dg * sa) + sg * d1a + dg * s1a + base_mask) >> base_shift);
            c.b = (value_type)((Min(sb * da, db * sa) + sb * d1a + db * s1a + base_mask) >> base_shift);
            *p = blender_type::make_pix(c.r, c.g, c.b);
        }
    }
};

// composite_op_packed_lighten
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_lighten {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = max(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            color_type c = blender_type::make_color(*p);
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = c.r;
            calc_type dg = c.g;
            calc_type db = c.b;
            calc_type da = base_mask;

            c.r = (value_type)((Max(sr * da, dr * sa) + sr * d1a + dr * s1a + base_mask) >> base_shift);
            c.g = (value_type)((Max(sg * da, dg * sa) + sg * d1a + dg * s1a + base_mask) >> base_shift);
            c.b = (value_type)((Max(sb * da, db * sa) + sb * d1a + db * s1a + base_mask) >> base_shift);
            *p = blender_type::make_pix(c.r, c.g, c.b);
        }
    }
};

// composite_op_packed_color_dodge
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_color_dodge {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // if Sca.Da + Dca.Sa >= Sa.Da
    // Dca' = Sa.Da + Sca.(1 - Da) + Dca.(1 - Sa)
    // otherwise
    // Dca' = Dca.Sa/(1-Sca/Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
    //
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            color_type c = blender_type::make_color(*p);
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = c.r;
            calc_type dg = c.g;
            calc_type db = c.b;
            calc_type da = base_mask;
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

// composite_op_packed_color_burn
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_color_burn {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // if Sca.Da + Dca.Sa <= Sa.Da
    // Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
    // otherwise
    // Dca' = Sa.(Sca.Da + Dca.Sa - Sa.Da)/Sca + Sca.(1 - Da) + Dca.(1 - Sa)
    //
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            color_type c = blender_type::make_color(*p);
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = c.r;
            calc_type dg = c.g;
            calc_type db = c.b;
            calc_type da = base_mask;
            long_type drsa = dr * sa;
            long_type dgsa = dg * sa;
            long_type dbsa = db * sa;
            long_type srda = sr * da;
            long_type sgda = sg * da;
            long_type sbda = sb * da;
            long_type sada = sa * da;

            c.r = (value_type)(((srda + drsa <= sada) ?
                                sr * d1a + dr* s1a :
                                sa * (srda + drsa - sada) / sr + sr * d1a + dr * s1a + base_mask) >> base_shift);

            c.g = (value_type)(((sgda + dgsa <= sada) ?
                                sg * d1a + dg* s1a :
                                sa * (sgda + dgsa - sada) / sg + sg * d1a + dg * s1a + base_mask) >> base_shift);

            c.b = (value_type)(((sbda + dbsa <= sada) ?
                                sb * d1a + db* s1a :
                                sa * (sbda + dbsa - sada) / sb + sb * d1a + db * s1a + base_mask) >> base_shift);
            *p = blender_type::make_pix(c.r, c.g, c.b);
        }
    }
};

// composite_op_packed_hard_light
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_hard_light {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // if 2.Sca < Sa
    // Dca' = 2.Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
    // otherwise
    // Dca' = Sa.Da - 2.(Da - Dca).(Sa - Sca) + Sca.(1 - Da) + Dca.(1 - Sa)
    //
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            color_type c = blender_type::make_color(*p);
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = c.r;
            calc_type dg = c.g;
            calc_type db = c.b;
            calc_type da = base_mask;
            calc_type sada = sa * da;

            c.r = (value_type)(((2 * sr < sa) ?
                                2 * sr * dr + sr * d1a + dr* s1a :
                                sada - 2 * (da - dr) * (sa - sr) + sr * d1a + dr * s1a + base_mask) >> base_shift);

            c.g = (value_type)(((2 * sg < sa) ?
                                2 * sg * dg + sg * d1a + dg* s1a :
                                sada - 2 * (da - dg) * (sa - sg) + sg * d1a + dg * s1a + base_mask) >> base_shift);

            c.b = (value_type)(((2 * sb < sa) ?
                                2 * sb * db + sb * d1a + db* s1a :
                                sada - 2 * (da - db) * (sa - sb) + sb * d1a + db * s1a + base_mask) >> base_shift);
            *p = blender_type::make_pix(c.r, c.g, c.b);
        }
    }
};

// composite_op_packed_soft_light
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_soft_light {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // if 2.Sca < Sa
    // Dca' = Dca.(Sa + (1 - Dca/Da).(2.Sca - Sa)) + Sca.(1 - Da) + Dca.(1 - Sa)
    // otherwise if 8.Dca <= Da
    // Dca' = Dca.(Sa + (1 - Dca/Da).(2.Sca - Sa).(3 - 8.Dca/Da)) + Sca.(1 - Da) + Dca.(1 - Sa)
    // otherwise
    // Dca' = (Dca.Sa + ((Dca/Da)^(0.5).Da - Dca).(2.Sca - Sa)) + Sca.(1 - Da) + Dca.(1 - Sa)
    //
    // Da'  = Sa + Da - Sa.Da

    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t r, uint32_t g,
                                         uint32_t b, uint32_t a, uint32_t cover)
    {
        scalar sr = INT_TO_SCALAR(r * cover) / (base_mask * 255);
        scalar sg = INT_TO_SCALAR(g * cover) / (base_mask * 255);
        scalar sb = INT_TO_SCALAR(b * cover) / (base_mask * 255);
        scalar sa = INT_TO_SCALAR(a * cover) / (base_mask * 255);
        if (sa > 0) {
            color_type c = blender_type::make_color(*p);
            scalar dr = INT_TO_SCALAR(c.r) / base_mask;
            scalar dg = INT_TO_SCALAR(c.g) / base_mask;
            scalar db = INT_TO_SCALAR(c.b) / base_mask;
            scalar da = INT_TO_SCALAR(base_mask) / base_mask;
            if (cover < 255) {
                a = (a * cover + 255) >> 8;
            }

            if (2 * sr < sa) { dr = dr * (sa + (1 - dr / da) * (2 * sr - sa)) + sr * (1 - da) + dr * (1 - sa); }
            else if (8 * dr <= da) { dr = dr * (sa + (1 - dr / da) * (2 * sr - sa) * (3 - 8 * dr / da)) + sr * (1 - da) + dr * (1 - sa); }
            else { dr = (dr * sa + (Sqrt(dr / da) * da - dr) * (2 * sr - sa)) + sr * (1 - da) + dr * (1 - sa); }

            if (2 * sg < sa) { dg = dg * (sa + (1 - dg / da) * (2 * sg - sa)) + sg * (1 - da) + dg * (1 - sa); }
            else if (8 * dg <= da) { dg = dg * (sa + (1 - dg / da) * (2 * sg - sa) * (3 - 8 * dg / da)) + sg * (1 - da) + dg * (1 - sa); }
            else { dg = (dg * sa + (Sqrt(dg / da) * da - dg) * (2 * sg - sa)) + sg * (1 - da) + dg * (1 - sa); }

            if (2 * sb < sa) { db = db * (sa + (1 - db / da) * (2 * sb - sa)) + sb * (1 - da) + db * (1 - sa); }
            else if (8 * db <= da) { db = db * (sa + (1 - db / da) * (2 * sb - sa) * (3 - 8 * db / da)) + sb * (1 - da) + db * (1 - sa); }
            else { db = (db * sa + (Sqrt(db / da) * da - db) * (2 * sb - sa)) + sb * (1 - da) + db * (1 - sa); }

            c.r = (value_type)uround(dr * base_mask);
            c.g = (value_type)uround(dg * base_mask);
            c.b = (value_type)uround(db * base_mask);
            *p = blender_type::make_pix(c.r, c.g, c.b);
        }
    }
};

// composite_op_packed_difference
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_difference {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca + Dca - 2.min(Sca.Da, Dca.Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            color_type c = blender_type::make_color(*p);
            calc_type dr = c.r;
            calc_type dg = c.g;
            calc_type db = c.b;
            calc_type da = base_mask;
            c.r = (value_type)(sr + dr - ((2 * Min(sr * da, dr * sa) + base_mask) >> base_shift));
            c.g = (value_type)(sg + dg - ((2 * Min(sg * da, dg * sa) + base_mask) >> base_shift));
            c.b = (value_type)(sb + db - ((2 * Min(sb * da, db * sa) + base_mask) >> base_shift));
            *p = blender_type::make_pix(c.r, c.g, c.b);
        }
    }
};

// composite_op_packed_exclusion
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_exclusion {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = (Sca.Da + Dca.Sa - 2.Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            color_type c = blender_type::make_color(*p);
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = c.r;
            calc_type dg = c.g;
            calc_type db = c.b;
            calc_type da = base_mask;
            c.r = (value_type)((sr * da + dr * sa - 2 * sr * dr + sr * d1a + dr * s1a + base_mask) >> base_shift);
            c.g = (value_type)((sg * da + dg * sa - 2 * sg * dg + sg * d1a + dg * s1a + base_mask) >> base_shift);
            c.b = (value_type)((sb * da + db * sa - 2 * sb * db + sb * d1a + db * s1a + base_mask) >> base_shift);
            *p = blender_type::make_pix(c.r, c.g, c.b);
        }
    }
};

// composite_op_packed_contrast
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_contrast {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }
        color_type c = blender_type::make_color(*p);
        long_type dr = c.r;
        long_type dg = c.g;
        long_type db = c.b;
        int32_t da = base_mask;
        long_type d2a = da >> 1;
        uint32_t s2a = sa >> 1;

        int32_t r = (int32_t)((((dr - d2a) * int32_t((sr - s2a) * 2 + base_mask)) >> base_shift) + d2a);
        int32_t g = (int32_t)((((dg - d2a) * int32_t((sg - s2a) * 2 + base_mask)) >> base_shift) + d2a);
        int32_t b = (int32_t)((((db - d2a) * int32_t((sb - s2a) * 2 + base_mask)) >> base_shift) + d2a);

        r = (r < 0) ? 0 : r;
        g = (g < 0) ? 0 : g;
        b = (b < 0) ? 0 : b;

        c.r = (value_type)((r > da) ? da : r);
        c.g = (value_type)((g > da) ? da : g);
        c.b = (value_type)((b > da) ? da : b);
        *p = blender_type::make_pix(c.r, c.g, c.b);
    }
};

// composite_op_packed_invert
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_invert {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        sa = (sa * cover + 255) >> 8;
        if (sa) {
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

// composite_op_packed_invert_rgb
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_invert_rgb {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
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

// composite_op_packed_hue
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_hue {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // B(Cb, Cs) = setLum(setSat(Cs, SAT(Cb)), LUM(Cb))
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t r, uint32_t g,
                                         uint32_t b, uint32_t a, uint32_t cover)
    {
        scalar sr = INT_TO_SCALAR(r * cover) / (base_mask * 255);
        scalar sg = INT_TO_SCALAR(g * cover) / (base_mask * 255);
        scalar sb = INT_TO_SCALAR(b * cover) / (base_mask * 255);
        scalar sa = INT_TO_SCALAR(a * cover) / (base_mask * 255);

        if (sa > 0) {
            color_type c = blender_type::make_color(*p);
            scalar dr = INT_TO_SCALAR(c.r) / base_mask;
            scalar dg = INT_TO_SCALAR(c.g) / base_mask;
            scalar db = INT_TO_SCALAR(c.b) / base_mask;

            color_set_sat(&sr, &sg, &sb, C_SAT(dr, dg, db) * sa);
            color_set_lum(&sr, &sg, &sb, sa, C_LUM(dr, dg, db, scalar) * sa);

            c.r = (value_type)uround(sr * base_mask);
            c.g = (value_type)uround(sg * base_mask);
            c.b = (value_type)uround(sb * base_mask);
            *p = blender_type::make_pix(c.r, c.g, c.b);
        }
    }
};

// composite_op_packed_saturation
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_saturation {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // B(Cb, Cs) = setLum(setSat(Cb, SAT(Cs)), LUM(Cb))
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t r, uint32_t g,
                                         uint32_t b, uint32_t a, uint32_t cover)
    {
        scalar sr = INT_TO_SCALAR(r * cover) / (base_mask * 255);
        scalar sg = INT_TO_SCALAR(g * cover) / (base_mask * 255);
        scalar sb = INT_TO_SCALAR(b * cover) / (base_mask * 255);
        scalar sa = INT_TO_SCALAR(a * cover) / (base_mask * 255);

        if (sa > 0) {
            color_type c = blender_type::make_color(*p);
            scalar dr = INT_TO_SCALAR(c.r) / base_mask;
            scalar dg = INT_TO_SCALAR(c.g) / base_mask;
            scalar db = INT_TO_SCALAR(c.b) / base_mask;

            scalar sdr = dr * sa;
            scalar sdg = dg * sa;
            scalar sdb = db * sa;

            color_set_sat(&sdr, &sdg, &sdb, C_SAT(sr, sg, sb));
            color_set_lum(&sdr, &sdg, &sdb, sa, C_LUM(dr, dg, db, scalar) * sa);

            c.r = (value_type)uround(sdr * base_mask);
            c.g = (value_type)uround(sdg * base_mask);
            c.b = (value_type)uround(sdb * base_mask);
            *p = blender_type::make_pix(c.r, c.g, c.b);
        }
    }
};

// composite_op_packed_color
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_color {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // B(Cb, Cs) = setLum(Cs, LUM(Cb))
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t r, uint32_t g,
                                         uint32_t b, uint32_t a, uint32_t cover)
    {
        scalar sr = INT_TO_SCALAR(r * cover) / (base_mask * 255);
        scalar sg = INT_TO_SCALAR(g * cover) / (base_mask * 255);
        scalar sb = INT_TO_SCALAR(b * cover) / (base_mask * 255);
        scalar sa = INT_TO_SCALAR(a * cover) / (base_mask * 255);

        if (sa > 0) {
            color_type c = blender_type::make_color(*p);
            scalar dr = INT_TO_SCALAR(c.r) / base_mask;
            scalar dg = INT_TO_SCALAR(c.g) / base_mask;
            scalar db = INT_TO_SCALAR(c.b) / base_mask;

            color_set_lum(&sr, &sg, &sb, sa, C_LUM(dr, dg, db, scalar) * sa);

            c.r = (value_type)uround(sr * base_mask);
            c.g = (value_type)uround(sg * base_mask);
            c.b = (value_type)uround(sb * base_mask);
            *p = blender_type::make_pix(c.r, c.g, c.b);
        }
    }
};

// composite_op_packed_luminosity
template <typename ColorType, typename Order, typename Blender>
struct composite_op_packed_luminosity {
    typedef ColorType color_type;
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // B(Cb, Cs) = setLum(Cb, LUM(Cs))
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(pixel_type* p, uint32_t r, uint32_t g,
                                         uint32_t b, uint32_t a, uint32_t cover)
    {
        scalar sr = INT_TO_SCALAR(r * cover) / (base_mask * 255);
        scalar sg = INT_TO_SCALAR(g * cover) / (base_mask * 255);
        scalar sb = INT_TO_SCALAR(b * cover) / (base_mask * 255);
        scalar sa = INT_TO_SCALAR(a * cover) / (base_mask * 255);

        if (sa > 0) {
            color_type c = blender_type::make_color(*p);
            scalar dr = INT_TO_SCALAR(c.r) / base_mask;
            scalar dg = INT_TO_SCALAR(c.g) / base_mask;
            scalar db = INT_TO_SCALAR(c.b) / base_mask;

            scalar sdr = dr * sa;
            scalar sdg = dg * sa;
            scalar sdb = db * sa;

            color_set_lum(&sdr, &sdg, &sdb, sa, C_LUM(sr, sg, sb, scalar));

            c.r = (value_type)uround(sdr * base_mask);
            c.g = (value_type)uround(sdg * base_mask);
            c.b = (value_type)uround(sdb * base_mask);
            *p = blender_type::make_pix(c.r, c.g, c.b);
        }
    }
};

// composite operate table for blend rgb pixel format.
template <typename ColorType, typename Order, typename Blender>
struct blend_op_table_packed {
    typedef Blender blender_type;
    typedef typename blender_type::pixel_type pixel_type;
    typedef typename ColorType::value_type value_type;
    typedef void (*composite_op_func_type)(pixel_type* p,
                                           uint32_t cr,
                                           uint32_t cg,
                                           uint32_t cb,
                                           uint32_t ca,
                                           uint32_t cover);

    static composite_op_func_type g_packed_blend_op_func[];
};

// g_packed_comp_op_func
template <typename ColorType, typename Order, typename Blender>
typename blend_op_table_packed<ColorType, Order, Blender>::composite_op_func_type
blend_op_table_packed<ColorType, Order, Blender>::g_packed_blend_op_func[] = {
    composite_op_packed_clear <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_src <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_src_over <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_src_in <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_src_out <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_src_atop <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_dst <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_dst_over <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_dst_in <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_dst_out <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_dst_atop <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_xor <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_darken <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_lighten <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_overlay <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_screen <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_multiply <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_plus <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_minus <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_exclusion <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_difference <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_soft_light <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_hard_light <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_color_burn <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_color_dodge<ColorType, Order, Blender>::blend_pix,
    composite_op_packed_contrast <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_invert <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_invert_rgb <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_hue <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_saturation <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_color <ColorType, Order, Blender>::blend_pix,
    composite_op_packed_luminosity <ColorType, Order, Blender>::blend_pix,
    0
};

}
#endif /*_GFX_COMPOSITE_PACKED_H_*/
