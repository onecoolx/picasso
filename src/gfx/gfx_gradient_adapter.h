/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2013 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_GRADIENT_ADAPTER_H_
#define _GFX_GRADIENT_ADAPTER_H_

#include "common.h"
#include "interfaces.h"

#include "picasso_gradient.h"
#include "aggheader.h"
#include "pixfmt_wrapper.h"
#include "gfx_trans_affine.h"

namespace gfx {

class gfx_gradient_adapter : public abstract_gradient_adapter
{
public:
    gfx_gradient_adapter()
        : m_wrapper(0)
        , m_start(0)
        , m_length(0)
        , m_build(false)
    {
    }

    virtual ~gfx_gradient_adapter()
    {
        if (m_wrapper)
            delete m_wrapper;
    }

    virtual void init_linear(int spread, scalar x1, scalar y1, scalar x2, scalar y2);

    virtual void init_radial(int spread, scalar x1, scalar y1, scalar radius1, 
                                               scalar x2, scalar y2, scalar radius2);

    virtual void init_conic(int spread, scalar x, scalar y, scalar angle);

    virtual void add_color_stop(scalar offset, const picasso::rgba& c)
    {
        m_colors.add_color(SCALAR_TO_FLT(offset), agg::rgba8(agg::rgba(c)));
        m_build = false;
    }

    virtual void clear_stops(void)
    {
        m_colors.remove_all();
        m_build = false;
    }

    virtual void transform(const abstract_trans_affine* mtx)
    {
        register const gfx_trans_affine* m = static_cast<const gfx_trans_affine*>(mtx);
        m_matrix *= const_cast<gfx_trans_affine*>(m)->impl();
    }

    void build(void) 
    {
        if (!m_build) {
            m_colors.build_lut();
            m_build = true;
        }
    }

    gradient_wrapper* wrapper(void) { return m_wrapper; }
    float start(void) { return m_start; }
    float length(void) { return m_length; }
    agg::gradient_lut<agg::color_interpolator<agg::rgba8> > & colors(void) { return m_colors; } 
    agg::trans_affine& matrix(void) { return m_matrix; }
private:
    gradient_wrapper* m_wrapper;
	float m_start;
	float m_length;
    agg::trans_affine m_matrix;
	agg::gradient_lut<agg::color_interpolator<agg::rgba8> > m_colors;
	bool m_build;
};

}
#endif /*_GFX_GRADIENT_ADAPTER_H_*/
