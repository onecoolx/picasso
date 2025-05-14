/* Picasso - a vector graphics library
 *
 * Copyright (C) 2014 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_PIXFMT_RGB_H_
#define _GFX_PIXFMT_RGB_H_

#include "common.h"
#include "gfx_rendering_buffer.h"

namespace gfx {

// composite_op_rgb_clear
template <typename ColorType, typename Order>
struct composite_op_rgb_clear {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;

    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t, uint32_t,
                                         uint32_t, uint32_t, uint32_t cover)
    {
        if (cover < 255) {
            cover = 255 - cover;
            p[Order::R] = (value_type)((p[Order::R] * cover + 255) >> 8);
            p[Order::G] = (value_type)((p[Order::G] * cover + 255) >> 8);
            p[Order::B] = (value_type)((p[Order::B] * cover + 255) >> 8);
        } else {
            p[Order::R] = p[Order::G] = p[Order::B] = 0;
        }
    }
};

// composite_op_rgb_src
template <typename ColorType, typename Order>
struct composite_op_rgb_src {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;

    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            uint32_t alpha = 255 - cover;
            p[Order::R] = (value_type)(((p[Order::R] * alpha + 255) >> 8) + ((sr * cover + 255) >> 8));
            p[Order::G] = (value_type)(((p[Order::G] * alpha + 255) >> 8) + ((sg * cover + 255) >> 8));
            p[Order::B] = (value_type)(((p[Order::B] * alpha + 255) >> 8) + ((sb * cover + 255) >> 8));
        } else {
            p[Order::R] = sr;
            p[Order::G] = sg;
            p[Order::B] = sb;
        }
    }
};

// composite_op_rgb_dst
template <typename ColorType, typename Order>
struct composite_op_rgb_dst {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;

    static _FORCE_INLINE_ void blend_pix(value_type*, uint32_t, uint32_t,
                                         uint32_t, uint32_t, uint32_t)
    {
    }
};

// composite_op_rgb_src_over
template <typename ColorType, typename Order>
struct composite_op_rgb_src_over {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
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

// composite_op_rgb_dst_over
template <typename ColorType, typename Order>
struct composite_op_rgb_dst_over {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Dca + Sca.(1 - Da)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
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

// composite_op_rgb_src_in
template <typename ColorType, typename Order>
struct composite_op_rgb_src_in {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca.Da
    // Da'  = Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        calc_type da = base_mask;
        if (cover < 255) {
            uint32_t alpha = 255 - cover;
            p[Order::R] = (value_type)(((p[Order::R] * alpha + 255) >> 8) +
                                       ((((sr * da + base_mask) >> base_shift) * cover + 255) >> 8));
            p[Order::G] = (value_type)(((p[Order::G] * alpha + 255) >> 8) +
                                       ((((sg * da + base_mask) >> base_shift) * cover + 255) >> 8));
            p[Order::B] = (value_type)(((p[Order::B] * alpha + 255) >> 8) +
                                       ((((sb * da + base_mask) >> base_shift) * cover + 255) >> 8));
        } else {
            p[Order::R] = (value_type)((sr * da + base_mask) >> base_shift);
            p[Order::G] = (value_type)((sg * da + base_mask) >> base_shift);
            p[Order::B] = (value_type)((sb * da + base_mask) >> base_shift);
        }
    }
};

// composite_op_rgb_dst_in
template <typename ColorType, typename Order>
struct composite_op_rgb_dst_in {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Dca.Sa
    // Da'  = Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t, uint32_t, uint32_t,
                                         uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sa = base_mask - ((cover * (base_mask - sa) + 255) >> 8);
        }

        p[Order::R] = (value_type)((p[Order::R] * sa + base_mask) >> base_shift);
        p[Order::G] = (value_type)((p[Order::G] * sa + base_mask) >> base_shift);
        p[Order::B] = (value_type)((p[Order::B] * sa + base_mask) >> base_shift);
    }
};

// composite_op_rgb_src_out
template <typename ColorType, typename Order>
struct composite_op_rgb_src_out {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca.(1 - Da)
    // Da'  = Sa.(1 - Da)
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        calc_type da = 0;
        if (cover < 255) {
            uint32_t alpha = 255 - cover;
            p[Order::R] = (value_type)(((p[Order::R] * alpha + 255) >> 8) +
                                       ((((sr * da + base_mask) >> base_shift) * cover + 255) >> 8));
            p[Order::G] = (value_type)(((p[Order::G] * alpha + 255) >> 8) +
                                       ((((sg * da + base_mask) >> base_shift) * cover + 255) >> 8));
            p[Order::B] = (value_type)(((p[Order::B] * alpha + 255) >> 8) +
                                       ((((sb * da + base_mask) >> base_shift) * cover + 255) >> 8));
        } else {
            p[Order::R] = (value_type)((sr * da + base_mask) >> base_shift);
            p[Order::G] = (value_type)((sg * da + base_mask) >> base_shift);
            p[Order::B] = (value_type)((sb * da + base_mask) >> base_shift);
        }
    }
};

// composite_op_rgb_dst_out
template <typename ColorType, typename Order>
struct composite_op_rgb_dst_out {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Dca.(1 - Sa)
    // Da'  = Da.(1 - Sa)
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t, uint32_t, uint32_t,
                                         uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sa = (sa * cover + 255) >> 8;
        }

        sa = base_mask - sa;
        p[Order::R] = (value_type)((p[Order::R] * sa + base_shift) >> base_shift);
        p[Order::G] = (value_type)((p[Order::G] * sa + base_shift) >> base_shift);
        p[Order::B] = (value_type)((p[Order::B] * sa + base_shift) >> base_shift);
    }
};

// composite_op_rgb_src_atop
template <typename ColorType, typename Order>
struct composite_op_rgb_src_atop {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca.Da + Dca.(1 - Sa)
    // Da'  = Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
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
        p[Order::R] = (value_type)((sr * da + p[Order::R] * sa + base_mask) >> base_shift);
        p[Order::G] = (value_type)((sg * da + p[Order::G] * sa + base_mask) >> base_shift);
        p[Order::B] = (value_type)((sb * da + p[Order::B] * sa + base_mask) >> base_shift);
    }
};

// composite_op_rgb_dst_atop
template <typename ColorType, typename Order>
struct composite_op_rgb_dst_atop {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Dca.Sa + Sca.(1 - Da)
    // Da'  = Sa
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        calc_type da = 0;
        if (cover < 255) {
            uint32_t alpha = 255 - cover;
            sr = (p[Order::R] * sa + sr * da + base_mask) >> base_shift;
            sg = (p[Order::G] * sa + sg * da + base_mask) >> base_shift;
            sb = (p[Order::B] * sa + sb * da + base_mask) >> base_shift;
            p[Order::R] = (value_type)(((p[Order::R] * alpha + 255) >> 8) + ((sr * cover + 255) >> 8));
            p[Order::G] = (value_type)(((p[Order::G] * alpha + 255) >> 8) + ((sg * cover + 255) >> 8));
            p[Order::B] = (value_type)(((p[Order::B] * alpha + 255) >> 8) + ((sb * cover + 255) >> 8));

        } else {
            p[Order::R] = (value_type)((p[Order::R] * sa + sr * da + base_mask) >> base_shift);
            p[Order::G] = (value_type)((p[Order::G] * sa + sg * da + base_mask) >> base_shift);
            p[Order::B] = (value_type)((p[Order::B] * sa + sb * da + base_mask) >> base_shift);
        }
    }
};

// composite_op_rgb_xor
template <typename ColorType, typename Order>
struct composite_op_rgb_xor {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa + Da - 2.Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
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
            p[Order::R] = (value_type)((p[Order::R] * s1a + sr * d1a + base_mask) >> base_shift);
            p[Order::G] = (value_type)((p[Order::G] * s1a + sg * d1a + base_mask) >> base_shift);
            p[Order::B] = (value_type)((p[Order::B] * s1a + sb * d1a + base_mask) >> base_shift);
        }
    }
};

// composite_op_rgb_plus
template <typename ColorType, typename Order>
struct composite_op_rgb_plus {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca + Dca
    // Da'  = Sa + Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            calc_type dr = p[Order::R] + sr;
            calc_type dg = p[Order::G] + sg;
            calc_type db = p[Order::B] + sb;
            p[Order::R] = (dr > base_mask) ? (value_type)base_mask : dr;
            p[Order::G] = (dg > base_mask) ? (value_type)base_mask : dg;
            p[Order::B] = (db > base_mask) ? (value_type)base_mask : db;
        }
    }
};

// composite_op_rgb_minus
template <typename ColorType, typename Order>
struct composite_op_rgb_minus {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Dca - Sca
    // Da' = 1 - (1 - Sa).(1 - Da)
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            calc_type dr = p[Order::R] - sr;
            calc_type dg = p[Order::G] - sg;
            calc_type db = p[Order::B] - sb;
            p[Order::R] = (dr > base_mask) ? 0 : dr;
            p[Order::G] = (dg > base_mask) ? 0 : dg;
            p[Order::B] = (db > base_mask) ? 0 : db;
        }
    }
};

// composite_op_rgb_multiply
template <typename ColorType, typename Order>
struct composite_op_rgb_multiply {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
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
            calc_type dr = p[Order::R];
            calc_type dg = p[Order::G];
            calc_type db = p[Order::B];
            p[Order::R] = (value_type)((sr * dr + sr * d1a + dr * s1a + base_mask) >> base_shift);
            p[Order::G] = (value_type)((sg * dg + sg * d1a + dg * s1a + base_mask) >> base_shift);
            p[Order::B] = (value_type)((sb * db + sb * d1a + db * s1a + base_mask) >> base_shift);
        }
    }
};

// composite_op_rgb_screen
template <typename ColorType, typename Order>
struct composite_op_rgb_screen {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca + Dca - Sca.Dca
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            calc_type dr = p[Order::R];
            calc_type dg = p[Order::G];
            calc_type db = p[Order::B];
            p[Order::R] = (value_type)(sr + dr - ((sr * dr + base_mask) >> base_shift));
            p[Order::G] = (value_type)(sg + dg - ((sg * dg + base_mask) >> base_shift));
            p[Order::B] = (value_type)(sb + db - ((sb * db + base_mask) >> base_shift));
        }
    }
};

// composite_op_rgb_overlay
template <typename ColorType, typename Order>
struct composite_op_rgb_overlay {
    typedef ColorType color_type;
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
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = p[Order::R];
            calc_type dg = p[Order::G];
            calc_type db = p[Order::B];
            calc_type da = base_mask;
            calc_type sada = sa * base_mask;

            p[Order::R] = (value_type)(((2 * dr < da) ?
                                        2 * sr * dr + sr * d1a + dr* s1a :
                                        sada - 2 * (da - dr) * (sa - sr) + sr * d1a + dr * s1a + base_mask) >> base_shift);

            p[Order::G] = (value_type)(((2 * dg < da) ?
                                        2 * sg * dg + sg * d1a + dg* s1a :
                                        sada - 2 * (da - dg) * (sa - sg) + sg * d1a + dg * s1a + base_mask) >> base_shift);

            p[Order::B] = (value_type)(((2 * db < da) ?
                                        2 * sb * db + sb * d1a + db* s1a :
                                        sada - 2 * (da - db) * (sa - sb) + sb * d1a + db * s1a + base_mask) >> base_shift);
        }
    }
};

// composite_op_rgb_darken
template <typename ColorType, typename Order>
struct composite_op_rgb_darken {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = min(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = p[Order::R];
            calc_type dg = p[Order::G];
            calc_type db = p[Order::B];
            calc_type da = base_mask;

            p[Order::R] = (value_type)((Min(sr * da, dr * sa) + sr * d1a + dr * s1a + base_mask) >> base_shift);
            p[Order::G] = (value_type)((Min(sg * da, dg * sa) + sg * d1a + dg * s1a + base_mask) >> base_shift);
            p[Order::B] = (value_type)((Min(sb * da, db * sa) + sb * d1a + db * s1a + base_mask) >> base_shift);
        }
    }
};

// composite_op_rgb_lighten
template <typename ColorType, typename Order>
struct composite_op_rgb_lighten {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = max(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = p[Order::R];
            calc_type dg = p[Order::G];
            calc_type db = p[Order::B];
            calc_type da = base_mask;

            p[Order::R] = (value_type)((Max(sr * da, dr * sa) + sr * d1a + dr * s1a + base_mask) >> base_shift);
            p[Order::G] = (value_type)((Max(sg * da, dg * sa) + sg * d1a + dg * s1a + base_mask) >> base_shift);
            p[Order::B] = (value_type)((Max(sb * da, db * sa) + sb * d1a + db * s1a + base_mask) >> base_shift);
        }
    }
};

// composite_op_rgb_color_dodge
template <typename ColorType, typename Order>
struct composite_op_rgb_color_dodge {
    typedef ColorType color_type;
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
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = p[Order::R];
            calc_type dg = p[Order::G];
            calc_type db = p[Order::B];
            calc_type da = base_mask;
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

// composite_op_rgb_color_burn
template <typename ColorType, typename Order>
struct composite_op_rgb_color_burn {
    typedef ColorType color_type;
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
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = p[Order::R];
            calc_type dg = p[Order::G];
            calc_type db = p[Order::B];
            calc_type da = base_mask;
            long_type drsa = dr * sa;
            long_type dgsa = dg * sa;
            long_type dbsa = db * sa;
            long_type srda = sr * da;
            long_type sgda = sg * da;
            long_type sbda = sb * da;
            long_type sada = sa * da;

            p[Order::R] = (value_type)(((srda + drsa <= sada) ?
                                        sr * d1a + dr* s1a :
                                        sa * (srda + drsa - sada) / sr + sr * d1a + dr * s1a + base_mask) >> base_shift);

            p[Order::G] = (value_type)(((sgda + dgsa <= sada) ?
                                        sg * d1a + dg* s1a :
                                        sa * (sgda + dgsa - sada) / sg + sg * d1a + dg * s1a + base_mask) >> base_shift);

            p[Order::B] = (value_type)(((sbda + dbsa <= sada) ?
                                        sb * d1a + db* s1a :
                                        sa * (sbda + dbsa - sada) / sb + sb * d1a + db * s1a + base_mask) >> base_shift);
        }
    }
};

// composite_op_rgb_hard_light
template <typename ColorType, typename Order>
struct composite_op_rgb_hard_light {
    typedef ColorType color_type;
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
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = p[Order::R];
            calc_type dg = p[Order::G];
            calc_type db = p[Order::B];
            calc_type da = base_mask;
            calc_type sada = sa * da;

            p[Order::R] = (value_type)(((2 * sr < sa) ?
                                        2 * sr * dr + sr * d1a + dr* s1a :
                                        sada - 2 * (da - dr) * (sa - sr) + sr * d1a + dr * s1a + base_mask) >> base_shift);

            p[Order::G] = (value_type)(((2 * sg < sa) ?
                                        2 * sg * dg + sg * d1a + dg* s1a :
                                        sada - 2 * (da - dg) * (sa - sg) + sg * d1a + dg * s1a + base_mask) >> base_shift);

            p[Order::B] = (value_type)(((2 * sb < sa) ?
                                        2 * sb * db + sb * d1a + db* s1a :
                                        sada - 2 * (da - db) * (sa - sb) + sb * d1a + db * s1a + base_mask) >> base_shift);
        }
    }
};

// composite_op_rgb_soft_light
template <typename ColorType, typename Order>
struct composite_op_rgb_soft_light {
    typedef ColorType color_type;
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

    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t r, uint32_t g,
                                         uint32_t b, uint32_t a, uint32_t cover)
    {
        scalar sr = INT_TO_SCALAR(r * cover) / (base_mask * 255);
        scalar sg = INT_TO_SCALAR(g * cover) / (base_mask * 255);
        scalar sb = INT_TO_SCALAR(b * cover) / (base_mask * 255);
        scalar sa = INT_TO_SCALAR(a * cover) / (base_mask * 255);
        if (sa > 0) {
            scalar dr = INT_TO_SCALAR(p[Order::R]) / base_mask;
            scalar dg = INT_TO_SCALAR(p[Order::G]) / base_mask;
            scalar db = INT_TO_SCALAR(p[Order::B]) / base_mask;
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

            p[Order::R] = (value_type)uround(dr * base_mask);
            p[Order::G] = (value_type)uround(dg * base_mask);
            p[Order::B] = (value_type)uround(db * base_mask);
        }
    }
};

// composite_op_rgb_difference
template <typename ColorType, typename Order>
struct composite_op_rgb_difference {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = Sca + Dca - 2.min(Sca.Da, Dca.Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            calc_type dr = p[Order::R];
            calc_type dg = p[Order::G];
            calc_type db = p[Order::B];
            calc_type da = base_mask;
            p[Order::R] = (value_type)(sr + dr - ((2 * Min(sr * da, dr * sa) + base_mask) >> base_shift));
            p[Order::G] = (value_type)(sg + dg - ((2 * Min(sg * da, dg * sa) + base_mask) >> base_shift));
            p[Order::B] = (value_type)(sb + db - ((2 * Min(sb * da, db * sa) + base_mask) >> base_shift));
        }
    }
};

// composite_op_rgb_exclusion
template <typename ColorType, typename Order>
struct composite_op_rgb_exclusion {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = (Sca.Da + Dca.Sa - 2.Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
            calc_type d1a = 0;
            calc_type s1a = base_mask - sa;
            calc_type dr = p[Order::R];
            calc_type dg = p[Order::G];
            calc_type db = p[Order::B];
            calc_type da = base_mask;
            p[Order::R] = (value_type)((sr * da + dr * sa - 2 * sr * dr + sr * d1a + dr * s1a + base_mask) >> base_shift);
            p[Order::G] = (value_type)((sg * da + dg * sa - 2 * sg * dg + sg * d1a + dg * s1a + base_mask) >> base_shift);
            p[Order::B] = (value_type)((sb * da + db * sa - 2 * sb * db + sb * d1a + db * s1a + base_mask) >> base_shift);
        }
    }
};

// composite_op_rgb_contrast
template <typename ColorType, typename Order>
struct composite_op_rgb_contrast {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        long_type dr = p[Order::R];
        long_type dg = p[Order::G];
        long_type db = p[Order::B];
        int32_t da = base_mask;
        long_type d2a = da >> 1;
        uint32_t s2a = sa >> 1;

        int32_t r = (int32_t)((((dr - d2a) * int32_t((sr - s2a) * 2 + base_mask)) >> base_shift) + d2a);
        int32_t g = (int32_t)((((dg - d2a) * int32_t((sg - s2a) * 2 + base_mask)) >> base_shift) + d2a);
        int32_t b = (int32_t)((((db - d2a) * int32_t((sb - s2a) * 2 + base_mask)) >> base_shift) + d2a);

        r = (r < 0) ? 0 : r;
        g = (g < 0) ? 0 : g;
        b = (b < 0) ? 0 : b;

        p[Order::R] = (value_type)((r > da) ? da : r);
        p[Order::G] = (value_type)((g > da) ? da : g);
        p[Order::B] = (value_type)((b > da) ? da : b);
    }
};

// composite_op_rgb_invert
template <typename ColorType, typename Order>
struct composite_op_rgb_invert {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        sa = (sa * cover + 255) >> 8;
        if (sa) {
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

// composite_op_rgb_invert_rgb
template <typename ColorType, typename Order>
struct composite_op_rgb_invert_rgb {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t sr, uint32_t sg,
                                         uint32_t sb, uint32_t sa, uint32_t cover)
    {
        if (cover < 255) {
            sr = (sr * cover + 255) >> 8;
            sg = (sg * cover + 255) >> 8;
            sb = (sb * cover + 255) >> 8;
            sa = (sa * cover + 255) >> 8;
        }

        if (sa) {
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

// composite_op_rgb_hue
template <typename ColorType, typename Order>
struct composite_op_rgb_hue {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // B(Cb, Cs) = setLum(setSat(Cs, SAT(Cb)), LUM(Cb))
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t r, uint32_t g,
                                         uint32_t b, uint32_t a, uint32_t cover)
    {
        scalar sr = INT_TO_SCALAR(r * cover) / (base_mask * 255);
        scalar sg = INT_TO_SCALAR(g * cover) / (base_mask * 255);
        scalar sb = INT_TO_SCALAR(b * cover) / (base_mask * 255);
        scalar sa = INT_TO_SCALAR(a * cover) / (base_mask * 255);

        if (sa > 0) {
            scalar dr = INT_TO_SCALAR(p[Order::R]) / base_mask;
            scalar dg = INT_TO_SCALAR(p[Order::G]) / base_mask;
            scalar db = INT_TO_SCALAR(p[Order::B]) / base_mask;

            color_set_sat(&sr, &sg, &sb, C_SAT(dr, dg, db) * sa);
            color_set_lum(&sr, &sg, &sb, sa, C_LUM(dr, dg, db, scalar) * sa);

            p[Order::R] = (value_type)uround(sr * base_mask);
            p[Order::G] = (value_type)uround(sg * base_mask);
            p[Order::B] = (value_type)uround(sb * base_mask);
        }
    }
};

// composite_op_rgb_saturation
template <typename ColorType, typename Order>
struct composite_op_rgb_saturation {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // B(Cb, Cs) = setLum(setSat(Cb, SAT(Cs)), LUM(Cb))
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t r, uint32_t g,
                                         uint32_t b, uint32_t a, uint32_t cover)
    {
        scalar sr = INT_TO_SCALAR(r * cover) / (base_mask * 255);
        scalar sg = INT_TO_SCALAR(g * cover) / (base_mask * 255);
        scalar sb = INT_TO_SCALAR(b * cover) / (base_mask * 255);
        scalar sa = INT_TO_SCALAR(a * cover) / (base_mask * 255);

        if (sa > 0) {
            scalar dr = INT_TO_SCALAR(p[Order::R]) / base_mask;
            scalar dg = INT_TO_SCALAR(p[Order::G]) / base_mask;
            scalar db = INT_TO_SCALAR(p[Order::B]) / base_mask;

            scalar sdr = dr * sa;
            scalar sdg = dg * sa;
            scalar sdb = db * sa;

            color_set_sat(&sdr, &sdg, &sdb, C_SAT(sr, sg, sb));
            color_set_lum(&sdr, &sdg, &sdb, sa, C_LUM(dr, dg, db, scalar) * sa);

            p[Order::R] = (value_type)uround(sdr * base_mask);
            p[Order::G] = (value_type)uround(sdg * base_mask);
            p[Order::B] = (value_type)uround(sdb * base_mask);
        }
    }
};

// composite_op_rgb_color
template <typename ColorType, typename Order>
struct composite_op_rgb_color {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;
    typedef typename color_type::long_type long_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // B(Cb, Cs) = setLum(Cs, LUM(Cb))
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t r, uint32_t g,
                                         uint32_t b, uint32_t a, uint32_t cover)
    {
        scalar sr = INT_TO_SCALAR(r * cover) / (base_mask * 255);
        scalar sg = INT_TO_SCALAR(g * cover) / (base_mask * 255);
        scalar sb = INT_TO_SCALAR(b * cover) / (base_mask * 255);
        scalar sa = INT_TO_SCALAR(a * cover) / (base_mask * 255);

        if (sa > 0) {
            scalar dr = INT_TO_SCALAR(p[Order::R]) / base_mask;
            scalar dg = INT_TO_SCALAR(p[Order::G]) / base_mask;
            scalar db = INT_TO_SCALAR(p[Order::B]) / base_mask;

            color_set_lum(&sr, &sg, &sb, sa, C_LUM(dr, dg, db, scalar) * sa);

            p[Order::R] = (value_type)uround(sr * base_mask);
            p[Order::G] = (value_type)uround(sg * base_mask);
            p[Order::B] = (value_type)uround(sb * base_mask);
        }
    }
};

// composite_op_rgb_luminosity
template <typename ColorType, typename Order>
struct composite_op_rgb_luminosity {
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef typename color_type::calc_type calc_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    // B(Cb, Cs) = setLum(Cb, LUM(Cs))
    // Da'  = Sa + Da - Sa.Da
    static _FORCE_INLINE_ void blend_pix(value_type* p, uint32_t r, uint32_t g,
                                         uint32_t b, uint32_t a, uint32_t cover)
    {
        scalar sr = INT_TO_SCALAR(r * cover) / (base_mask * 255);
        scalar sg = INT_TO_SCALAR(g * cover) / (base_mask * 255);
        scalar sb = INT_TO_SCALAR(b * cover) / (base_mask * 255);
        scalar sa = INT_TO_SCALAR(a * cover) / (base_mask * 255);

        if (sa > 0) {
            scalar dr = INT_TO_SCALAR(p[Order::R]) / base_mask;
            scalar dg = INT_TO_SCALAR(p[Order::G]) / base_mask;
            scalar db = INT_TO_SCALAR(p[Order::B]) / base_mask;

            scalar sdr = dr * sa;
            scalar sdg = dg * sa;
            scalar sdb = db * sa;

            color_set_lum(&sdr, &sdg, &sdb, sa, C_LUM(sr, sg, sb, scalar));

            p[Order::R] = (value_type)uround(sdr * base_mask);
            p[Order::G] = (value_type)uround(sdg * base_mask);
            p[Order::B] = (value_type)uround(sdb * base_mask);
        }
    }
};

// composite operate table for blend rgb pixel format.
template <typename ColorType, typename Order>
struct blend_op_table_rgb {
    typedef typename ColorType::value_type value_type;
    typedef void (*composite_op_func_type)(value_type* p,
                                           uint32_t cr,
                                           uint32_t cg,
                                           uint32_t cb,
                                           uint32_t ca,
                                           uint32_t cover);

    static composite_op_func_type g_rgb_blend_op_func[];
};

// g_rgb_comp_op_func
template <typename ColorType, typename Order>
typename blend_op_table_rgb<ColorType, Order>::composite_op_func_type
blend_op_table_rgb<ColorType, Order>::g_rgb_blend_op_func[] = {
    composite_op_rgb_clear <ColorType, Order>::blend_pix,
    composite_op_rgb_src <ColorType, Order>::blend_pix,
    composite_op_rgb_src_over <ColorType, Order>::blend_pix,
    composite_op_rgb_src_in <ColorType, Order>::blend_pix,
    composite_op_rgb_src_out <ColorType, Order>::blend_pix,
    composite_op_rgb_src_atop <ColorType, Order>::blend_pix,
    composite_op_rgb_dst <ColorType, Order>::blend_pix,
    composite_op_rgb_dst_over <ColorType, Order>::blend_pix,
    composite_op_rgb_dst_in <ColorType, Order>::blend_pix,
    composite_op_rgb_dst_out <ColorType, Order>::blend_pix,
    composite_op_rgb_dst_atop <ColorType, Order>::blend_pix,
    composite_op_rgb_xor <ColorType, Order>::blend_pix,
    composite_op_rgb_darken <ColorType, Order>::blend_pix,
    composite_op_rgb_lighten <ColorType, Order>::blend_pix,
    composite_op_rgb_overlay <ColorType, Order>::blend_pix,
    composite_op_rgb_screen <ColorType, Order>::blend_pix,
    composite_op_rgb_multiply <ColorType, Order>::blend_pix,
    composite_op_rgb_plus <ColorType, Order>::blend_pix,
    composite_op_rgb_minus <ColorType, Order>::blend_pix,
    composite_op_rgb_exclusion <ColorType, Order>::blend_pix,
    composite_op_rgb_difference <ColorType, Order>::blend_pix,
    composite_op_rgb_soft_light <ColorType, Order>::blend_pix,
    composite_op_rgb_hard_light <ColorType, Order>::blend_pix,
    composite_op_rgb_color_burn <ColorType, Order>::blend_pix,
    composite_op_rgb_color_dodge<ColorType, Order>::blend_pix,
    composite_op_rgb_contrast <ColorType, Order>::blend_pix,
    composite_op_rgb_invert <ColorType, Order>::blend_pix,
    composite_op_rgb_invert_rgb <ColorType, Order>::blend_pix,
    composite_op_rgb_hue <ColorType, Order>::blend_pix,
    composite_op_rgb_saturation <ColorType, Order>::blend_pix,
    composite_op_rgb_color <ColorType, Order>::blend_pix,
    composite_op_rgb_luminosity <ColorType, Order>::blend_pix,
    0
};

// blend operate adaptor for rgb
template <typename ColorType, typename Order>
class blend_op_adaptor_rgb
{
public:
    typedef ColorType color_type;
    typedef typename color_type::value_type value_type;
    typedef Order order_type;

    enum {
        base_shift = color_type::base_shift,
        base_mask = color_type::base_mask,
    };

    static _FORCE_INLINE_ void blend_pix(uint32_t op, value_type* p,
                                         uint32_t cr, uint32_t cg,
                                         uint32_t cb, uint32_t ca, uint32_t cover)
    {
        blend_op_table_rgb<ColorType, Order>::g_rgb_blend_op_func[op]
        (p, (cr * ca + base_mask) >> base_shift,
         (cg * ca + base_mask) >> base_shift,
         (cb * ca + base_mask) >> base_shift,
         ca, cover);
    }
};

// pixfmt blender rgb
template <typename Blender, typename RenBuffer>
class pixfmt_blender_rgb
{
public:
    typedef RenBuffer buffer_type;
    typedef typename buffer_type::row_data row_data;
    typedef uint32_t pixel_type;
    typedef Blender blender_type;
    typedef typename blender_type::color_type color_type;
    typedef typename blender_type::order_type order_type;
    typedef typename color_type::value_type value_type;

    enum {
        base_shift = color_type::base_shift,
        base_scale = color_type::base_scale,
        base_mask = color_type::base_mask,
        pix_width = sizeof(value_type) * 3
    };

    pixfmt_blender_rgb()
        : m_buffer(0)
        , m_blend_op(comp_op_src_over)
        , m_alpha_factor(base_mask)
    {
    }

    explicit pixfmt_blender_rgb(buffer_type& rb, uint32_t op = comp_op_src_over, uint32_t alpha = base_mask)
        : m_buffer(&rb)
        , m_blend_op(op)
        , m_alpha_factor(alpha)
    {
    }

    void attach(buffer_type& rb) { m_buffer = &rb; }

    uint32_t width(void) const { return m_buffer->internal_width(); }
    uint32_t height(void) const { return m_buffer->internal_height(); }
    int32_t stride(void) const { return m_buffer->internal_stride(); }

    byte* row_ptr(int32_t y) { return m_buffer->row_ptr(y); }
    const byte* row_ptr(int32_t y) const { return m_buffer->row_ptr(y); }
    row_data row(int32_t y) const { return m_buffer->row(y); }

    void alpha(scalar a) { m_alpha_factor = uround(a * base_mask); }
    scalar alpha(void) const { return INT_TO_SCALAR(m_alpha_factor) / FLT_TO_SCALAR(255.0f); }

    byte* pix_zero(void) const
    {
        static pixel_type zero = 0;
        return (byte*)&zero;
    }

    byte* pix_ptr(int32_t x, int32_t y) const
    {
        return m_buffer->row_ptr(y) + x * pix_width;
    }

    void blend_op(uint32_t op) { m_blend_op = op; }
    uint32_t blend_op(void) const { return m_blend_op; }

    color_type pixel(int32_t x, int32_t y) const
    {
        const value_type* p = (value_type*)m_buffer->row_ptr(y) + x + x + x;
        return color_type(p[order_type::R],
                          p[order_type::G],
                          p[order_type::B]);
    }

    void copy_pixel(int32_t x, int32_t y, const color_type& c)
    {
        value_type* p = (value_type*)m_buffer->row_ptr(x, y, 1) + x + x + x;
        p[order_type::R] = c.r;
        p[order_type::G] = c.g;
        p[order_type::B] = c.b;
    }

    void blend_pixel(int32_t x, int32_t y, const color_type& c, uint8_t cover)
    {
        blender_type::blend_pix(m_blend_op,
                                (value_type*)m_buffer->row_ptr(x, y, 1) + x + x + x,
                                c.r, c.g, c.b, (value_type)alpha_mul(c.a, m_alpha_factor), cover);
    }

    void copy_hline(int32_t x, int32_t y, uint32_t len, const color_type& c)
    {
        value_type* p = (value_type*)m_buffer->row_ptr(x, y, len) + x + x + x;
        do {
            p[order_type::R] = c.r;
            p[order_type::G] = c.g;
            p[order_type::B] = c.b;
            p += 3;
        } while (--len);
    }

    void copy_vline(int32_t x, int32_t y, uint32_t len, const color_type& c)
    {
        do {
            value_type* p = (value_type*)m_buffer->row_ptr(x, y++, 1) + x + x + x;
            p[order_type::R] = c.r;
            p[order_type::G] = c.g;
            p[order_type::B] = c.b;
        } while (--len);
    }

    void blend_hline(int32_t x, int32_t y, uint32_t len, const color_type& c, uint8_t cover)
    {
        value_type* p = (value_type*)m_buffer->row_ptr(x, y, len) + x + x + x;
        _REGISTER_ value_type alpha = (value_type)alpha_mul(c.a, m_alpha_factor);

        if ((m_blend_op == comp_op_src_over) && (alpha == base_mask) && (cover == 255)) {
            // optimization.
            do {
                p[order_type::R] = c.r;
                p[order_type::G] = c.g;
                p[order_type::B] = c.b;
                p += 3;
            } while (--len);
        } else {
            do {
                blender_type::blend_pix(m_blend_op, p, c.r, c.g, c.b, alpha, cover);
                p += 3;
            } while (--len);
        }
    }

    void blend_vline(int32_t x, int32_t y, uint32_t len, const color_type& c, uint8_t cover)
    {
        _REGISTER_ value_type alpha = (value_type)alpha_mul(c.a, m_alpha_factor);
        do {
            blender_type::blend_pix(m_blend_op,
                                    (value_type*)m_buffer->row_ptr(x, y++, 1) + x + x + x,
                                    c.r, c.g, c.b, alpha, cover);
        } while (--len);
    }

    void blend_solid_hspan(int32_t x, int32_t y, uint32_t len, const color_type& c, const uint8_t* covers)
    {
        value_type* p = (value_type*)m_buffer->row_ptr(x, y, len) + x + x + x;
        _REGISTER_ value_type alpha = (value_type)alpha_mul(c.a, m_alpha_factor);
        do {
            blender_type::blend_pix(m_blend_op,
                                    p, c.r, c.g, c.b, alpha, *covers++);
            p += 3;
        } while (--len);
    }

    void blend_solid_vspan(int32_t x, int32_t y, uint32_t len, const color_type& c, const uint8_t* covers)
    {
        _REGISTER_ value_type alpha = (value_type)alpha_mul(c.a, m_alpha_factor);
        do {
            blender_type::blend_pix(m_blend_op,
                                    (value_type*)m_buffer->row_ptr(x, y++, 1) + x + x + x,
                                    c.r, c.g, c.b, alpha, *covers++);
        } while (--len);
    }

    void copy_color_hspan(int32_t x, int32_t y, uint32_t len, const color_type* colors)
    {
        value_type* p = (value_type*)m_buffer->row_ptr(x, y, len) + x + x + x;
        do {
            p[order_type::R] = colors->r;
            p[order_type::G] = colors->g;
            p[order_type::B] = colors->b;
            ++colors;
            p += 3;
        } while (--len);
    }

    void copy_color_vspan(int32_t x, int32_t y, uint32_t len, const color_type* colors)
    {
        do {
            value_type* p = (value_type*)m_buffer->row_ptr(x, y++, 1) + x + x + x;
            p[order_type::R] = colors->r;
            p[order_type::G] = colors->g;
            p[order_type::B] = colors->b;
            ++colors;
        } while (--len);
    }

    void blend_color_hspan(int32_t x, int32_t y, uint32_t len,
                           const color_type* colors, const uint8_t* covers, uint8_t cover)
    {
        value_type* p = (value_type*)m_buffer->row_ptr(x, y, len) + x + x + x;
        do {
            blender_type::blend_pix(m_blend_op, p,
                                    colors->r,
                                    colors->g,
                                    colors->b,
                                    (value_type)alpha_mul(colors->a, m_alpha_factor),
                                    covers ? *covers++ : cover);
            p += 3;
            ++colors;
        } while (--len);
    }

    void blend_color_vspan(int32_t x, int32_t y, uint32_t len,
                           const color_type* colors, const uint8_t* covers, uint8_t cover)
    {
        do {
            blender_type::blend_pix(m_blend_op,
                                    (value_type*)m_buffer->row_ptr(x, y++, 1) + x + x + x,
                                    colors->r,
                                    colors->g,
                                    colors->b,
                                    (value_type)alpha_mul(colors->a, m_alpha_factor),
                                    covers ? *covers++ : cover);
            ++colors;
        } while (--len);
    }

    static void make_pix(byte* p, const color_type& c)
    {
        ((value_type*)p)[order_type::R] = c.r;
        ((value_type*)p)[order_type::G] = c.g;
        ((value_type*)p)[order_type::B] = c.b;
    }

    template <typename RenBuffer2>
    void copy_point_from(const RenBuffer2& from, int32_t xdst, int32_t ydst, int32_t xsrc, int32_t ysrc)
    {
        const byte* p = from.row_ptr(ysrc);
        if (p) {
            mem_deep_copy(m_buffer->row_ptr(xdst, ydst, 1) + xdst * pix_width,
                          p + xsrc * pix_width, pix_width);
        }
    }

    template <typename RenBuffer2>
    void copy_from(const RenBuffer2& from, int32_t xdst, int32_t ydst, int32_t xsrc, int32_t ysrc, uint32_t len)
    {
        const byte* p = from.row_ptr(ysrc);
        if (p) {
            mem_deep_copy(m_buffer->row_ptr(xdst, ydst, len) + xdst * pix_width,
                          p + xsrc * pix_width, len * pix_width);
        }
    }

    template <typename SrcPixelFormatRenderer>
    void blend_from(const SrcPixelFormatRenderer& from, int32_t xdst, int32_t ydst,
                    int32_t xsrc, int32_t ysrc, uint32_t len, uint8_t cover)
    {
        typedef typename SrcPixelFormatRenderer::order_type src_order;
        const value_type* psrc = (const value_type*)from.row_ptr(ysrc);
        if (psrc) {
            psrc += xsrc << 2;
            value_type* pdst =
                (value_type*)m_buffer->row_ptr(xdst, ydst, len) + xdst * 3;

            do {
                blender_type::blend_pix(m_blend_op, pdst,
                                        psrc[src_order::R],
                                        psrc[src_order::G],
                                        psrc[src_order::B],
                                        (value_type)alpha_mul(psrc[src_order::A], m_alpha_factor), cover);
                psrc += 4;
                pdst += 3;
            } while (--len);
        }
    }

    template <typename SrcPixelFormatRenderer>
    void blend_point_from(const SrcPixelFormatRenderer& from, int32_t xdst, int32_t ydst,
                          int32_t xsrc, int32_t ysrc, uint8_t cover)
    {
        typedef typename SrcPixelFormatRenderer::order_type src_order;
        const value_type* psrc = (const value_type*)from.row_ptr(ysrc);
        if (psrc) {
            psrc += xsrc << 2;
            value_type* pdst = (value_type*)m_buffer->row_ptr(xdst, ydst, 1) + xdst * 3;

            blender_type::blend_pix(m_blend_op, pdst,
                                    psrc[src_order::R],
                                    psrc[src_order::G],
                                    psrc[src_order::B],
                                    (value_type)alpha_mul(psrc[src_order::A], m_alpha_factor), cover);
        }
    }

    template <typename SrcPixelFormatRenderer>
    void blend_from_color(const SrcPixelFormatRenderer& from, const color_type& color,
                          int32_t xdst, int32_t ydst, int32_t xsrc, int32_t ysrc, uint32_t len, uint8_t cover)
    {
        typedef typename SrcPixelFormatRenderer::value_type src_value_type;
        const src_value_type* psrc = (src_value_type*)from.row_ptr(ysrc);
        _REGISTER_ value_type alpha = (value_type)alpha_mul(color.a, m_alpha_factor);
        if (psrc) {
            value_type* pdst = (value_type*)m_buffer->row_ptr(xdst, ydst, len) + xdst * 3;

            do {
                blender_type::blend_pix(m_blend_op, pdst,
                                        color.r, color.g, color.b, alpha,
                                        (*psrc * cover + base_mask) >> base_shift);
                ++psrc;
                pdst += 3;
            } while (--len);
        }
    }

    template <typename SrcPixelFormatRenderer>
    void blend_from_lut(const SrcPixelFormatRenderer& from, const color_type* color_lut,
                        int32_t xdst, int32_t ydst, int32_t xsrc, int32_t ysrc, uint32_t len, uint8_t cover)
    {
        typedef typename SrcPixelFormatRenderer::value_type src_value_type;
        const src_value_type* psrc = (src_value_type*)from.row_ptr(ysrc);
        if (psrc) {
            value_type* pdst = (value_type*)m_buffer->row_ptr(xdst, ydst, len) + xdst * 3;

            do {
                const color_type& color = color_lut[*psrc];
                blender_type::blend_pix(m_blend_op, pdst,
                                        color.r, color.g, color.b, alpha_mul(color.a, m_alpha_factor), cover);
                ++psrc;
                pdst += 3;
            } while (--len);
        }
    }

private:
    uint32_t alpha_mul(uint32_t a, uint32_t s)
    {
        return (s == 255) ? a : ((a * s + base_mask) >> base_shift);
    }

    pixfmt_blender_rgb(const pixfmt_blender_rgb&);
    pixfmt_blender_rgb& operator=(const pixfmt_blender_rgb&);
private:
    buffer_type* m_buffer;
    uint32_t m_blend_op;
    uint32_t m_alpha_factor;
};

typedef blend_op_adaptor_rgb<rgba8, order_rgb> blender_rgb24; // blender_rgb24
typedef blend_op_adaptor_rgb<rgba8, order_bgr> blender_bgr24; // blender_bgr24

typedef pixfmt_blender_rgb<blender_rgb24, gfx_rendering_buffer> pixfmt_rgb24; // pixfmt_rgb24
typedef pixfmt_blender_rgb<blender_bgr24, gfx_rendering_buffer> pixfmt_bgr24; // pixfmt_bgr24

}
#endif /*_GFX_PIXFMT_RGB_H_*/
