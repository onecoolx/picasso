/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2013 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "gfx_gradient_adapter.h"

namespace gfx {

void gfx_gradient_adapter::init_linear(int spread, scalar x1, scalar y1, scalar x2, scalar y2)
{
    if (!m_wrapper) {
        switch (spread) {
            case SPREAD_PAD:
                m_wrapper = new gradient_wrapper_adaptor<agg::gradient_x, 
                                        agg::gradient_pad_adaptor<agg::gradient_x> >;
                break;
            case SPREAD_REPEAT:
                m_wrapper = new gradient_wrapper_adaptor<agg::gradient_x, 
                                        agg::gradient_repeat_adaptor<agg::gradient_x> >;
                break;
            case SPREAD_REFLECT:
                m_wrapper = new gradient_wrapper_adaptor<agg::gradient_x, 
                                        agg::gradient_reflect_adaptor<agg::gradient_x> >;
                break;
        };

        if (!m_wrapper)
            return;
        
        float len = agg::calc_distance(SCALAR_TO_FLT(x1), SCALAR_TO_FLT(y1), SCALAR_TO_FLT(x2), SCALAR_TO_FLT(y2));

        agg::trans_affine mtx;
        if (len) {
            if (x2 < x1)
                mtx.rotate(SCALAR_TO_FLT(PI - Asin((y2 - y1) / FLT_TO_SCALAR(len))));
            else
                mtx.rotate(SCALAR_TO_FLT(Asin((y2 - y1) / FLT_TO_SCALAR(len))));
        } else
            len = 2.0f; // len can not be zero

	    mtx.translate(SCALAR_TO_FLT(x1), SCALAR_TO_FLT(y1));

        m_start = 0.0f;
        m_length = len;
        m_matrix = mtx;
    }
}

void gfx_gradient_adapter::init_radial(int spread, scalar x1, scalar y1, scalar radius1, 
                                           scalar x2, scalar y2, scalar radius2)
{
    if (!m_wrapper) {
        if ((x1 == x2) && (y1 == y2)) {
            switch (spread) {
                case SPREAD_PAD:
                    m_wrapper = new gradient_wrapper_adaptor<agg::gradient_radial, 
                                            agg::gradient_pad_adaptor<agg::gradient_radial> >;
                    break;
                case SPREAD_REPEAT:
                    m_wrapper = new gradient_wrapper_adaptor<agg::gradient_radial, 
                                            agg::gradient_repeat_adaptor<agg::gradient_radial> >;
                    break;
                case SPREAD_REFLECT:
                    m_wrapper = new gradient_wrapper_adaptor<agg::gradient_radial, 
                                            agg::gradient_reflect_adaptor<agg::gradient_radial> >;
                    break;
            }
        } else {
            switch (spread) {
                case SPREAD_PAD:
                    m_wrapper = new gradient_wrapper_adaptor<agg::gradient_radial_focus, 
                                            agg::gradient_pad_adaptor<agg::gradient_radial_focus> >;
                    break;
                case SPREAD_REPEAT:
                    m_wrapper = new gradient_wrapper_adaptor<agg::gradient_radial_focus, 
                                            agg::gradient_repeat_adaptor<agg::gradient_radial_focus> >;
                    break;
                case SPREAD_REFLECT:
                    m_wrapper = new gradient_wrapper_adaptor<agg::gradient_radial_focus, 
                                            agg::gradient_reflect_adaptor<agg::gradient_radial_focus> >;
                    break;
            }

        }

        if (!m_wrapper)
            return;

        float len = fabsf(SCALAR_TO_FLT(radius2));
        float fx = SCALAR_TO_FLT(x1 - x2);
        float fy = SCALAR_TO_FLT(y1 - y2);

        m_wrapper->init(len, fx, fy);

        if (!len) 
            len = 2.0f;// len can not be zero

        agg::trans_affine mtx;
        mtx.translate(SCALAR_TO_FLT(x1)-fx, SCALAR_TO_FLT(y1)-fy);


        m_start = fabsf(SCALAR_TO_FLT(radius1));
        m_length = len;
        m_matrix = mtx;
    }
}

void gfx_gradient_adapter::init_conic(int spread, scalar x, scalar y, scalar angle)
{
    if (!m_wrapper) {
	    // only support reflect 
	    m_wrapper = new gradient_wrapper_adaptor<agg::gradient_conic, 
                                agg::gradient_reflect_adaptor<agg::gradient_conic> >;

        if (!m_wrapper) 
            return;

        agg::trans_affine mtx;
        mtx.translate(SCALAR_TO_FLT(x), SCALAR_TO_FLT(y));
        mtx.translate(-SCALAR_TO_FLT(x), -SCALAR_TO_FLT(y));
        mtx.rotate(SCALAR_TO_FLT(angle));
        mtx.translate(SCALAR_TO_FLT(x), SCALAR_TO_FLT(y));

        m_start = 0;
        m_length = 128;
        m_matrix = mtx;
    }
}

}
