/* Picasso - a vector graphics library
 *
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "convert.h"
#include "matrix.h"

#include "gfx_gamma_function.h"
#include "gfx_raster_adapter.h"

#include "picasso_raster_adapter.h"

namespace gfx {

class gfx_raster_adapter_impl
{
public:
    enum {
        aa_shift = 8,
        aa_scale = 1 << aa_shift,
        aa_mask = aa_scale - 1,
    };

    gfx_raster_adapter_impl()
        : m_source(0)
        , m_transform(0)
        , m_method(raster_fill)
        , m_antialias(true)
        , m_dashline(false)
        , m_dash_start(0)
        , m_dash_data(0)
        , m_dash_num(0)
        , m_line_width(FLT_TO_SCALAR(1.0f))
        , m_miter_limit(FLT_TO_SCALAR(4.0f))
        , m_line_cap(butt_cap)
        , m_line_join(miter_join)
        , m_inner_join(inner_miter)
        , m_filling_rule(fill_non_zero)
    {
        for (int32_t i = 0; i < aa_scale; i++) {
            m_gamma[i] = i;
        }
    }

    ~gfx_raster_adapter_impl()
    {
        reset();
    }

    void reset(void)
    {
        m_source = 0;
        m_transform = 0;
        m_method = raster_fill;
        m_antialias = true;
        m_dashline = false;
        m_dash_start = 0;
        m_dash_data = 0;
        m_dash_num = 0;
        m_line_width = FLT_TO_SCALAR(1.0f);
        m_miter_limit = FLT_TO_SCALAR(4.0f);
        m_line_cap = butt_cap;
        m_line_join = miter_join;
        m_inner_join = inner_miter;
        m_filling_rule = fill_non_zero;
    }

    template <typename GammaFunc>
    void gamma(const GammaFunc& gamma_function)
    {
        for (int32_t i = 0; i < aa_scale; i++) {
            m_gamma[i] = uround(gamma_function(INT_TO_SCALAR(i) / aa_mask) * aa_mask);
        }
    }

    const vertex_source* m_source;
    const trans_affine* m_transform;
    uint32_t m_method;
    bool m_antialias;
    bool m_dashline;
    scalar m_dash_start;
    const scalar* m_dash_data;
    uint32_t m_dash_num;
    //stroke attributes
    scalar m_line_width;
    scalar m_miter_limit;
    line_cap m_line_cap;
    line_join m_line_join;
    inner_join m_inner_join;
    //fill attributes
    filling_rule m_filling_rule;
    // gamma table
    int32_t m_gamma[aa_scale];
};

gfx_raster_adapter::gfx_raster_adapter()
    : m_impl(new gfx_raster_adapter_impl)
    , m_sraster(m_impl->m_gamma)
    , m_fraster(m_impl->m_gamma)
{
}

gfx_raster_adapter::~gfx_raster_adapter()
{
    delete m_impl;
}

void gfx_raster_adapter::set_gamma_power(scalar g)
{
    if (m_impl->m_antialias) {
        m_impl->gamma(gamma_power(SCALAR_TO_FLT(g)));
    } else {
        m_impl->gamma(gamma_threshold(0.5f));
    }
}

void gfx_raster_adapter::set_antialias(bool b)
{
    m_impl->m_antialias = b;
}

void gfx_raster_adapter::set_transform(const trans_affine* mtx)
{
    m_impl->m_transform = mtx;
}

trans_affine gfx_raster_adapter::transformation(void) const
{
    if (m_impl->m_transform) {
        return *(m_impl->m_transform);
    } else {
        return trans_affine();
    }
}

void gfx_raster_adapter::set_raster_method(uint32_t m)
{
    m_impl->m_method = m;
}

uint32_t gfx_raster_adapter::raster_method(void) const
{
    return m_impl->m_method;
}

void gfx_raster_adapter::reset(void)
{
    m_impl->reset();
    m_sraster.reset();
    m_fraster.reset();
}

void gfx_raster_adapter::set_stroke_dashes(scalar start, const scalar* dashes, uint32_t num)
{
    m_impl->m_dashline = true;
    m_impl->m_dash_start = start;
    m_impl->m_dash_data = dashes;
    m_impl->m_dash_num = num;
}

void gfx_raster_adapter::set_stroke_attr(int32_t idx, int32_t val)
{
    switch (idx) {
        case STA_LINE_CAP:
            m_impl->m_line_cap = (line_cap)val;
            break;
        case STA_LINE_JOIN:
            m_impl->m_line_join = (line_join)val;
            break;
        case STA_INNER_JOIN:
            m_impl->m_inner_join = (inner_join)val;
            break;
        default:
            break;
    }
}

void gfx_raster_adapter::set_stroke_attr_val(int32_t idx, scalar val)
{
    switch (idx) {
        case STA_WIDTH:
            m_impl->m_line_width = val;
            break;
        case STA_MITER_LIMIT:
            m_impl->m_miter_limit = val;
            break;
        default:
            break;
    }
}

void gfx_raster_adapter::set_fill_attr(int32_t idx, int32_t val)
{
    switch (idx) {
        case FIA_FILL_RULE:
            m_impl->m_filling_rule = (filling_rule)val;
            break;
        default:
            break;
    }
}

bool gfx_raster_adapter::is_empty(void)
{
    return m_sraster.initial() && m_fraster.initial();
}

void gfx_raster_adapter::setup_stroke_raster(void)
{
    if (m_impl->m_dashline) {
        picasso::conv_dash c(*const_cast<vertex_source*>(m_impl->m_source));

        for (uint32_t i = 0; i < m_impl->m_dash_num; i += 2) {
            c.add_dash(m_impl->m_dash_data[i], m_impl->m_dash_data[i + 1]);
        }

        c.dash_start(m_impl->m_dash_start);

        picasso::conv_stroke p(c);

        bool adjust;
        trans_affine adjmtx = stable_matrix(*const_cast<trans_affine*>(m_impl->m_transform), &adjust);
        if (adjust) {
            adjmtx *= trans_affine_translation(FLT_TO_SCALAR(0.5f), FLT_TO_SCALAR(0.5f)); //adjust edge
        }

        picasso::conv_transform t(p, &adjmtx);

        p.set_width(SCALAR_TO_FLT(m_impl->m_line_width));
        p.set_line_cap(m_impl->m_line_cap);
        p.set_line_join(m_impl->m_line_join);
        p.set_inner_join(m_impl->m_inner_join);
        p.set_miter_limit(SCALAR_TO_FLT(m_impl->m_miter_limit));

        m_sraster.add_path(t);
    } else {
        picasso::conv_curve c(*const_cast<vertex_source*>(m_impl->m_source));
        picasso::conv_stroke p(c);

        bool adjust;
        trans_affine adjmtx = stable_matrix(*const_cast<trans_affine*>(m_impl->m_transform), &adjust);
        if (adjust) {
            adjmtx *= trans_affine_translation(FLT_TO_SCALAR(0.5f), FLT_TO_SCALAR(0.5f)); //adjust edge
        }

        picasso::conv_transform t(p, &adjmtx);

        p.set_width(SCALAR_TO_FLT(m_impl->m_line_width));
        p.set_line_cap(m_impl->m_line_cap);
        p.set_line_join(m_impl->m_line_join);
        p.set_inner_join(m_impl->m_inner_join);
        p.set_miter_limit(SCALAR_TO_FLT(m_impl->m_miter_limit));

        m_sraster.add_path(t);
    }
}

void gfx_raster_adapter::setup_fill_raster(void)
{
    m_fraster.filling(m_impl->m_filling_rule);
    trans_affine adjmtx = stable_matrix(*const_cast<trans_affine*>(m_impl->m_transform));

    conv_transform mt(*const_cast<vertex_source*>(m_impl->m_source), &adjmtx);
    m_fraster.add_path(mt);
}

void gfx_raster_adapter::commit(void)
{
    if (m_impl->m_source) {
        if (m_impl->m_method & raster_stroke) {
            setup_stroke_raster();
        }

        if (m_impl->m_method & raster_fill) {
            setup_fill_raster();
        }
    }
}

void gfx_raster_adapter::add_shape(const vertex_source& vs, uint32_t id)
{
    m_impl->m_source = &vs;
}

bool gfx_raster_adapter::contains(scalar x, scalar y)
{
    if (m_impl->m_source) {
        if (m_impl->m_method & raster_stroke) {
            return m_sraster.hit_test(iround(x), iround(y));
        } else if (m_impl->m_method & raster_fill) {
            return m_fraster.hit_test(iround(x), iround(y));
        } else {
            return false;
        }
    } else {
        return false;
    }
}

}
