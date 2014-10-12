/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "gfx_raster_adapter.h"
#include "gfx_trans_affine.h"
#include "picasso_raster_adapter.h"

namespace gfx {

class gfx_raster_adapter_impl
{
public:
    gfx_raster_adapter_impl()
        : m_source(0)
        , m_transform(0)
        , m_method(raster_fill)
        , m_antialias(true)
        , m_dashline(false)
        , m_dash_start(0)
        , m_dash_data(0)
        , m_dash_num(0)
         , line_width(FLT_TO_SCALAR(1.0f))
         , miter_limit(FLT_TO_SCALAR(4.0f))
         , line_cap(butt_cap)
         , line_join(miter_join)
         , inner_join(inner_miter)
        , filling_rule(fill_non_zero)
    {
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
         line_width = FLT_TO_SCALAR(1.0f);
         miter_limit = FLT_TO_SCALAR(4.0f);
         line_cap = butt_cap;
         line_join = miter_join;
         inner_join = inner_miter;
        filling_rule = fill_non_zero;
    }

    const vertex_source* m_source;
    const gfx_trans_affine* m_transform;
    unsigned int m_method;
    bool m_antialias;
    bool m_dashline;
    scalar m_dash_start;
    const scalar* m_dash_data;
    unsigned int m_dash_num;
    //stroke attributes
    scalar line_width;
    scalar miter_limit;
    int line_cap;
    int line_join;
    int inner_join;
    //fill attributes
    int filling_rule;
};


gfx_raster_adapter::gfx_raster_adapter()
    : m_impl(new gfx_raster_adapter_impl)
{
}

gfx_raster_adapter::~gfx_raster_adapter()
{
    delete m_impl;
}

void gfx_raster_adapter::set_gamma_power(scalar g)
{
    if (m_impl->m_antialias) {
        m_sraster.gamma(agg::gamma_power(SCALAR_TO_FLT(g)));
        m_fraster.gamma(agg::gamma_power(SCALAR_TO_FLT(g)));
    } else {
        m_sraster.gamma(agg::gamma_threshold(0.5f));
        m_fraster.gamma(agg::gamma_threshold(0.5f));
    }
}

void gfx_raster_adapter::set_antialias(bool b)
{
    m_impl->m_antialias = b;
}

void gfx_raster_adapter::set_transform(const abstract_trans_affine* mtx)
{
    m_impl->m_transform = static_cast<const gfx_trans_affine*>(mtx);
}

agg::trans_affine gfx_raster_adapter::transformation(void) 
{
    if (m_impl->m_transform)
        return const_cast<gfx_trans_affine*>(m_impl->m_transform)->impl();
    else
        return agg::trans_affine();
}

void gfx_raster_adapter::set_raster_method(unsigned int m)
{
    m_impl->m_method = m;
}

unsigned int gfx_raster_adapter::raster_method(void) const
{
    return m_impl->m_method;
}

void gfx_raster_adapter::reset(void)
{
    m_impl->reset(); 
    m_sraster.reset();
    m_fraster.reset();
}

void gfx_raster_adapter::set_stroke_dashes(scalar start, const scalar* dashes, unsigned int num)
{
    m_impl->m_dashline = true;
    m_impl->m_dash_start = start;
    m_impl->m_dash_data = dashes;
    m_impl->m_dash_num = num;
}

void gfx_raster_adapter::set_stroke_attr(int idx, int val)
{
    switch (idx) {
        case STA_LINE_CAP:
            m_impl->line_cap = val;
            break;
        case STA_LINE_JOIN:
            m_impl->line_join = val;
            break;
        case STA_INNER_JOIN:
            m_impl->inner_join = val;
            break;
        default:
            break;
    }
}

void gfx_raster_adapter::set_stroke_attr_val(int idx, scalar val)
{
    switch (idx) {
        case STA_WIDTH:
            m_impl->line_width = val;
            break;
        case STA_MITER_LIMIT:
            m_impl->miter_limit = val;
            break;
        default:
            break;
    }
}

void gfx_raster_adapter::set_fill_attr(int idx, int val)
{
    switch (idx) {
        case FIA_FILL_RULE:
            m_impl->filling_rule = val;
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
        agg::conv_dash<vertex_source> c(*const_cast<vertex_source*>(m_impl->m_source));

        for (unsigned int i = 0; i < m_impl->m_dash_num; i += 2)
            c.add_dash(m_impl->m_dash_data[i], m_impl->m_dash_data[i+1]);

        c.dash_start(m_impl->m_dash_start);

        agg::conv_stroke<agg::conv_dash<vertex_source> > p(c); 

        agg::trans_affine adjmtx = stable_matrix(const_cast<gfx_trans_affine*>(m_impl->m_transform)->impl());
        adjmtx *= agg::trans_affine_translation(0.5f, 0.5f); //adjust edge

        agg::conv_transform<agg::conv_stroke<agg::conv_dash<vertex_source> > > t(p, adjmtx);

        p.width(SCALAR_TO_FLT(m_impl->line_width));
        p.line_cap((agg::line_cap_e)m_impl->line_cap);
        p.line_join((agg::line_join_e)m_impl->line_join);
        p.inner_join((agg::inner_join_e)m_impl->inner_join);
        p.miter_limit(SCALAR_TO_FLT(m_impl->miter_limit));

        m_sraster.add_path(t);
    } else {
        agg::conv_curve<vertex_source> c(*const_cast<vertex_source*>(m_impl->m_source));
        agg::conv_stroke<agg::conv_curve<vertex_source> > p(c); 

        agg::trans_affine adjmtx = stable_matrix(const_cast<gfx_trans_affine*>(m_impl->m_transform)->impl());
        adjmtx *= agg::trans_affine_translation(0.5f, 0.5f); //adjust edge

        agg::conv_transform<agg::conv_stroke<agg::conv_curve<vertex_source> > > t(p, adjmtx);

        p.width(SCALAR_TO_FLT(m_impl->line_width));
        p.line_cap((agg::line_cap_e)m_impl->line_cap);
        p.line_join((agg::line_join_e)m_impl->line_join);
        p.inner_join((agg::inner_join_e)m_impl->inner_join);
        p.miter_limit(SCALAR_TO_FLT(m_impl->miter_limit));

        m_sraster.add_path(t);
    }
}

void gfx_raster_adapter::setup_fill_raster(void)
{
    m_fraster.filling_rule((agg::filling_rule_e)m_impl->filling_rule);
    agg::trans_affine adjmtx = stable_matrix(const_cast<gfx_trans_affine*>(m_impl->m_transform)->impl());

    agg::conv_transform<vertex_source> t(*const_cast<vertex_source*>(m_impl->m_source), adjmtx);
    m_fraster.add_path(t);
}

void gfx_raster_adapter::commit(void)
{
    if (m_impl->m_source) {
        if (m_impl->m_method & raster_stroke)
            setup_stroke_raster();

        if (m_impl->m_method & raster_fill)
            setup_fill_raster();
    }
}

void gfx_raster_adapter::add_shape(const vertex_source& vs, unsigned int id)
{
    m_impl->m_source = &vs;
}

bool gfx_raster_adapter::contains(scalar x, scalar y)
{
    if (m_impl->m_source) {
        if (m_impl->m_method & raster_stroke)
            return m_sraster.hit_test(agg::iround(x), agg::iround(y));
        else if (m_impl->m_method & raster_fill)
            return m_fraster.hit_test(agg::iround(x), agg::iround(y));
        else
            return false;
    } else {
        return false;
    }
}

}
