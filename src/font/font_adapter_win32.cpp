/*
 * Copyright (c) 2024, Zhang Ji Peng
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "common.h"

#if defined(WIN32) && !ENABLE(FREE_TYPE2)
#include "convert.h"
#include "matrix.h"
#include "font_adapter.h"

#include "gfx_rasterizer_scanline.h"
#include "gfx_scanline.h"
#include "gfx_scanline_renderer.h"
#include "gfx_scanline_storage.h"

#include <windows.h>
#include "graphic_path.h"
#include "graphic_helper.h"

using namespace gfx;

namespace picasso {

class font_adapter_impl
{
public:
    font_adapter_impl()
        : font(0)
        , antialias(false)
        , flip_y(false)
        , hinting(false)
        , height(0)
        , ascent(0)
        , descent(0)
        , leading(0)
        , units_per_em(0)
        , dc(0)
        , old_font(0)
        , buf_size(32736) // 32768-32
        , buf(new char[buf_size])
        , cur_glyph_index(0)
        , cur_data_size(0)
        , cur_data_type(glyph_type_outline)
        , cur_bound_rect(0, 0, 0, 0)
        , cur_advance_x(0)
        , cur_advance_y(0)
        , num_kerning_pairs(0)
        , max_kerning_pairs(0)
        , kerning_loaded(false)
        , kerning_pairs(0)
    {
        memset(&mat, 0, sizeof(MAT2));
        mat.eM11.value = 1;
        mat.eM22.value = 1;
    }

    ~font_adapter_impl()
    {
        if (font) {
            ::DeleteObject(font);
            font = 0;
        }

        if (kerning_pairs) {
            delete [] kerning_pairs;
            kerning_pairs = 0;
        }

        delete [] buf;
    }

    void load_kerning_pairs(void);
    void sort_kerning_pairs(void);

    HFONT font;
    bool antialias;
    bool flip_y;
    bool hinting;
    trans_affine matrix;
    scalar height;
    scalar ascent;
    scalar descent;
    scalar leading;
    uint32_t units_per_em;
    //font special
    HDC dc;
    HFONT old_font;
    MAT2 mat;
    //current glyph
    uint32_t buf_size;
    char* buf;
    uint32_t cur_glyph_index;
    uint32_t cur_data_size;
    glyph_type cur_data_type;
    rect cur_bound_rect;
    scalar cur_advance_x;
    scalar cur_advance_y;
    picasso::graphic_path cur_font_path;
    gfx_scanline_storage_bin cur_font_scanlines_bin;
    gfx_serialized_scanlines_adaptor_bin cur_font_storage_bin;
    //kerning
    uint32_t num_kerning_pairs;
    uint32_t max_kerning_pairs;
    bool kerning_loaded;
    KERNINGPAIR* kerning_pairs;
};

font_adapter::font_adapter(const char* name, int32_t charset, scalar height, scalar weight,
                           bool italic, bool hint, bool flip, bool a, const trans_affine* mtx)
    : m_impl(new font_adapter_impl)
{
    int32_t char_set = (charset == charset_latin1) ? ANSI_CHARSET : DEFAULT_CHARSET;
    m_impl->antialias = a;
    m_impl->flip_y = flip;
    m_impl->hinting = hint;
    m_impl->font = ::CreateFontA(-iround(height), // height of font
                                 0, // average character width
                                 0, // angle of escapement
                                 0, // base-line orientation angle
                                 iround(weight), // font weight
                                 italic ? TRUE : FALSE, // italic attribute option
                                 0, // underline attribute option
                                 0, // strikeout attribute option
                                 char_set, // character set identifier
                                 OUT_DEFAULT_PRECIS, // output precision
                                 CLIP_DEFAULT_PRECIS, // clipping precision
                                 ANTIALIASED_QUALITY, // output quality
                                 FF_DONTCARE, // pitch and family
                                 name); // typeface name

    m_impl->matrix = *mtx;

    OUTLINETEXTMETRIC omt;
    HDC dc = ::GetDC(0);
    HFONT old_font = (HFONT)::SelectObject(dc, m_impl->font);

    ::GetOutlineTextMetrics(dc, sizeof(omt), &omt);
    TEXTMETRIC& mt = omt.otmTextMetrics;
    m_impl->height = INT_TO_SCALAR(mt.tmHeight);
    m_impl->ascent = INT_TO_SCALAR(mt.tmAscent);
    m_impl->descent = INT_TO_SCALAR(mt.tmDescent);
    m_impl->leading = INT_TO_SCALAR(mt.tmExternalLeading);
    m_impl->units_per_em = omt.otmEMSquare;

    ::SelectObject(dc, old_font);
    ::ReleaseDC(0, dc);
}

font_adapter::~font_adapter()
{
    if (m_impl->dc && m_impl->old_font) {
        deactive();
    }

    delete m_impl;
}

void font_adapter::active(void)
{
    m_impl->dc = ::GetDC(0);
    m_impl->old_font = (HFONT)::SelectObject(m_impl->dc, m_impl->font);
}

void font_adapter::deactive(void)
{
    ::SelectObject(m_impl->dc, m_impl->old_font);
    ::ReleaseDC(0, m_impl->dc);

    m_impl->dc = 0;
    m_impl->old_font = 0;
}

scalar font_adapter::height(void) const
{
    return m_impl->height;
}

scalar font_adapter::ascent(void) const
{
    return m_impl->ascent;
}

scalar font_adapter::descent(void) const
{
    return m_impl->descent;
}

scalar font_adapter::leading(void) const
{
    return m_impl->leading;
}

uint32_t font_adapter::units_per_em(void) const
{
    return m_impl->units_per_em;
}

static int32_t pair_less(const void* v1, const void* v2)
{
    if (((KERNINGPAIR*)v1)->wFirst != ((KERNINGPAIR*)v2)->wFirst) {
        return ((KERNINGPAIR*)v1)->wFirst - ((KERNINGPAIR*)v2)->wFirst;
    }
    return ((KERNINGPAIR*)v1)->wSecond - ((KERNINGPAIR*)v2)->wSecond;
}

void font_adapter_impl::load_kerning_pairs(void)
{
    if (dc) {
        if (!kerning_pairs) {
            kerning_pairs = new KERNINGPAIR [16384 - 16];
            max_kerning_pairs = 16384 - 16;
        }

        num_kerning_pairs = ::GetKerningPairs(dc, max_kerning_pairs, kerning_pairs);

        if (num_kerning_pairs) {
            for (uint32_t i = 1; i < num_kerning_pairs; ++i) {
                if (pair_less(&kerning_pairs[i - 1], &kerning_pairs[i]) >= 0) {
                    sort_kerning_pairs();
                    break;
                }
            }
        }
        kerning_loaded = true;
    }
}

void font_adapter_impl::sort_kerning_pairs(void)
{
    qsort(kerning_pairs, num_kerning_pairs, sizeof(KERNINGPAIR), pair_less);
}

void font_adapter::add_kerning(uint32_t first, uint32_t second, scalar* x, scalar* y)
{
    if (m_impl->dc) {
        if (!m_impl->kerning_loaded) {
            m_impl->load_kerning_pairs();
        }

        int32_t end = m_impl->num_kerning_pairs - 1;
        int32_t beg = 0;
        KERNINGPAIR t;
        t.wFirst = (WORD)first;
        t.wSecond = (WORD)second;

        while (beg <= end) {
            int32_t mid = (end + beg) / 2;
            if (m_impl->kerning_pairs[mid].wFirst == t.wFirst &&
                m_impl->kerning_pairs[mid].wSecond == t.wSecond) {
                scalar dx = INT_TO_SCALAR(m_impl->kerning_pairs[mid].iKernAmount);
                scalar dy = 0;
                m_impl->matrix.transform_2x2(&dx, &dy);
                *x += dx;
                *y += dy;
                return;
            } else {
                if (pair_less(&t, &m_impl->kerning_pairs[mid]) < 0) {
                    end = mid - 1;
                } else {
                    beg = mid + 1;
                }
            }
        }
    }
}

static inline float fx_to_flt(const FIXED& p)
{
    return float(p.value) + float(p.fract) * (1.0f / 65536.0f);
}

static void decompose_win32_glyph_bitmap_mono(const char* gbuf, int32_t w, int32_t h, int32_t x, int32_t y,
                                              bool flip_y, gfx_scanline_bin& sl, gfx_scanline_storage_bin& storage)
{
    int32_t pitch = ((w + 31) >> 5) << 2;
    const byte* buf = (const byte*)gbuf;
    sl.reset(x, x + w);
    storage.prepare();

    if (flip_y) {
        buf += pitch * (h - 1);
        y += h;
        pitch = -pitch;
    }

    for (int32_t i = 0; i < h; i++) {
        sl.reset_spans();
        bitset_iterator bits(buf, 0);
        int32_t j;
        for (j = 0; j < w; j++) {
            if (bits.bit()) { sl.add_cell(x + j, cover_full); }
            ++bits;
        }

        buf += pitch;
        if (sl.num_spans()) {
            sl.finalize(y - i - 1);
            storage.render(sl);
        }
    }
}

static bool decompose_win32_glyph_outline(const char* gbuf, uint32_t total_size,
                                          bool flip_y, const trans_affine& mtx, graphic_path& path)
{
    const char* cur_glyph = gbuf;
    const char* end_glyph = gbuf + total_size;
    scalar x, y;

    bool closed = true;
    while (cur_glyph < end_glyph) {
        const TTPOLYGONHEADER* th = (TTPOLYGONHEADER*)cur_glyph;

        const char* end_poly = cur_glyph + th->cb;
        const char* cur_poly = cur_glyph + sizeof(TTPOLYGONHEADER);

        x = fx_to_flt(th->pfxStart.x);
        y = fx_to_flt(th->pfxStart.y);
        //FIXME: it is needed ?
        if (flip_y) {
            y = -y;
        }

        if (!closed) {
            path.close_polygon();
        }

        mtx.transform(&x, &y);
        path.move_to(x, y);

        while (cur_poly < end_poly) {
            const TTPOLYCURVE* pc = (const TTPOLYCURVE*)cur_poly;

            if (pc->wType == TT_PRIM_LINE) {
                for (int32_t i = 0; i < pc->cpfx; i++) {
                    x = fx_to_flt(pc->apfx[i].x);
                    y = fx_to_flt(pc->apfx[i].y);
                    //FIXME: it is needed ?
                    if (flip_y) {
                        y = -y;
                    }
                    mtx.transform(&x, &y);
                    path.line_to(x, y);
                }
            }

            if (pc->wType == TT_PRIM_QSPLINE) {
                for (int32_t u = 0; u < pc->cpfx - 1; u++) { // Walk through points in spline
                    POINTFX pnt_b = pc->apfx[u]; // B is always the current point
                    POINTFX pnt_c = pc->apfx[u + 1];

                    if (u < pc->cpfx - 2) { // If not on last spline, compute C
                        // midpoint (x,y)
                        *(int32_t*)&pnt_c.x = (*(int32_t*)&pnt_b.x + * (int32_t*)&pnt_c.x) / 2;
                        *(int32_t*)&pnt_c.y = (*(int32_t*)&pnt_b.y + * (int32_t*)&pnt_c.y) / 2;
                    }

                    scalar x2, y2;
                    x = fx_to_flt(pnt_b.x);
                    y = fx_to_flt(pnt_b.y);
                    x2 = fx_to_flt(pnt_c.x);
                    y2 = fx_to_flt(pnt_c.y);
                    //FIXME: it is needed ?
                    if (flip_y) {
                        y = -y;
                        y2 = -y2;
                    }
                    mtx.transform(&x, &y);
                    mtx.transform(&x2, &y2);
                    path.curve3(x, y, x2, y2);
                }
            }
            cur_poly += sizeof(WORD) * 2 + sizeof(POINTFX) * pc->cpfx;
        }
        cur_glyph += th->cb;
        closed = false;
    }
    path.close_polygon();
    return true;
}

static rect get_bounding_rect(graphic_path& path)
{
    rect_s rc(0, 0, 0, 0);
    bounding_rect(path, 0, &rc.x1, &rc.y1, &rc.x2, &rc.y2);
    return rect((int32_t)Floor(rc.x1), (int32_t)Floor(rc.y1), (int32_t)Ceil(rc.x2), (int32_t)Ceil(rc.y2));
}

#ifndef GGO_UNHINTED         // For compatibility with old SDKs.
    #define GGO_UNHINTED 0x0100
#endif

bool font_adapter::prepare_glyph(uint32_t code)
{
    if (m_impl->dc) {

        bool sys_bitmap = false;
        int32_t format = GGO_NATIVE;

        if (!m_impl->antialias && m_impl->matrix.is_identity()) { //matrix is identity
            sys_bitmap = true;
            format = GGO_BITMAP;
        }

        if (!m_impl->hinting) { format |= GGO_UNHINTED; }

        GLYPHMETRICS gm;
        int32_t total_size = GetGlyphOutlineW(m_impl->dc, code, format, &gm,
                                              m_impl->buf_size, m_impl->buf, &m_impl->mat);
        if (total_size < 0) {
            int32_t total_size = GetGlyphOutlineW(m_impl->dc, code, GGO_METRICS, &gm,
                                                  m_impl->buf_size, m_impl->buf, &m_impl->mat);

            if (total_size < 0) {
                return false;
            }
            gm.gmBlackBoxX = gm.gmBlackBoxY = 0;
            total_size = 0;
        }

        m_impl->cur_glyph_index = code;
        m_impl->cur_advance_x = INT_TO_SCALAR(gm.gmCellIncX);
        m_impl->cur_advance_y = -INT_TO_SCALAR(gm.gmCellIncY);

        if (m_impl->antialias) {
            // outline text
            m_impl->cur_data_type = glyph_type_outline;
            m_impl->matrix.transform(&m_impl->cur_advance_x, &m_impl->cur_advance_y);
            m_impl->cur_font_path.remove_all();

            if (decompose_win32_glyph_outline(m_impl->buf, total_size,
                                              m_impl->flip_y, m_impl->matrix, m_impl->cur_font_path)) {
                m_impl->cur_bound_rect = get_bounding_rect(m_impl->cur_font_path);
                m_impl->cur_data_size = m_impl->cur_font_path.total_byte_size() + sizeof(uint32_t); //count data
                return true;
            }
        } else {
            m_impl->cur_data_type = glyph_type_mono;
            // bitmap text
            if (sys_bitmap) { // bitmap create by system.
                gfx_scanline_bin sl;
                decompose_win32_glyph_bitmap_mono(m_impl->buf, gm.gmBlackBoxX, gm.gmBlackBoxY,
                                                  gm.gmptGlyphOrigin.x, m_impl->flip_y ? -gm.gmptGlyphOrigin.y : gm.gmptGlyphOrigin.y,
                                                  m_impl->flip_y, sl, m_impl->cur_font_scanlines_bin);

                m_impl->cur_bound_rect = rect(m_impl->cur_font_scanlines_bin.min_x(),
                                              m_impl->cur_font_scanlines_bin.min_y(),
                                              m_impl->cur_font_scanlines_bin.max_x() + 1,
                                              m_impl->cur_font_scanlines_bin.max_y() + 1);
                m_impl->cur_data_size = m_impl->cur_font_scanlines_bin.byte_size();
                return true;
            } else { // bitmap create by own rastering
                m_impl->cur_font_path.remove_all();

                if (decompose_win32_glyph_outline(m_impl->buf, total_size,
                                                  m_impl->flip_y, m_impl->matrix, m_impl->cur_font_path)) {
                    gfx_rasterizer_scanline_aa<> rasterizer;
                    conv_curve curves(m_impl->cur_font_path);
                    curves.approximation_scale(4.0);
                    rasterizer.add_path(curves);

                    gfx_scanline_bin sl;
                    m_impl->cur_font_scanlines_bin.prepare(); // Remove all
                    gfx_render_scanlines(rasterizer, sl, m_impl->cur_font_scanlines_bin);
                    m_impl->cur_bound_rect = rect(m_impl->cur_font_scanlines_bin.min_x(),
                                                  m_impl->cur_font_scanlines_bin.min_y(),
                                                  m_impl->cur_font_scanlines_bin.max_x() + 1,
                                                  m_impl->cur_font_scanlines_bin.max_y() + 1);
                    m_impl->cur_data_size = m_impl->cur_font_scanlines_bin.byte_size();
                    return true;
                }
            }
        }
    }
    return false;
}

void font_adapter::write_glyph_to(byte* buffer)
{
    if (buffer && m_impl->cur_data_size) {
        if (m_impl->cur_data_type == glyph_type_outline) {
            uint32_t count = m_impl->cur_font_path.total_vertices();
            mem_copy(buffer, &count, sizeof(uint32_t));
            buffer += sizeof(uint32_t);
            m_impl->cur_font_path.serialize_to(buffer);
        } else { // mono glyph
            m_impl->cur_font_scanlines_bin.serialize(buffer);
        }
    }
}

uint32_t font_adapter::glyph_index(void) const
{
    return m_impl->cur_glyph_index;
}

uint32_t font_adapter::data_size(void) const
{
    return m_impl->cur_data_size;
}

glyph_type font_adapter::data_type(void) const
{
    return m_impl->cur_data_type;
}

const rect& font_adapter::bounds(void) const
{
    return m_impl->cur_bound_rect;
}

scalar font_adapter::advance_x(void) const
{
    return m_impl->cur_advance_x;
}

scalar font_adapter::advance_y(void) const
{
    return m_impl->cur_advance_y;
}

void* font_adapter::create_storage(byte* buf, uint32_t len, scalar x, scalar y)
{
    m_impl->cur_font_storage_bin.init(buf, len, SCALAR_TO_FLT(x), SCALAR_TO_FLT(y));
    return (void*)&m_impl->cur_font_storage_bin;
}

void font_adapter::destroy_storage(void*)
{
    // do nothing
}

void font_adapter::translate_storage(void* storage, scalar x, scalar y)
{
    gfx_serialized_scanlines_adaptor_bin* sd = (gfx_serialized_scanlines_adaptor_bin*)storage;
    int32_t ox = sd->x();
    int32_t oy = sd->y();
    sd->setX(ox + SCALAR_TO_INT(x));
    sd->setY(oy + SCALAR_TO_INT(y));
}

}

bool platform_font_init(void)
{
    // win32 do nothing
    return true;
}

void platform_font_shutdown(void)
{
    // win32 do nothing
}

#endif /* WIN32 */
