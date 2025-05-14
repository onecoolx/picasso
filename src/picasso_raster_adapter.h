/* Picasso - a vector graphics library
 *
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PICASSO_RASTER_ADAPTER_H_
#define _PICASSO_RASTER_ADAPTER_H_

#include "common.h"

enum {
    STA_WIDTH,
    STA_LINE_CAP,
    STA_LINE_JOIN,
    STA_INNER_JOIN,
    STA_MITER_LIMIT,
};

enum {
    FIA_FILL_RULE,
};

namespace picasso {

class trans_affine;

class raster_adapter
{
public:
    raster_adapter();
    ~raster_adapter();

    void set_gamma_power(scalar g);
    void set_antialias(bool b);
    void set_transform(const trans_affine& mtx);
    void set_raster_method(uint32_t methods);

    void set_stroke_dashes(scalar start, const scalar* dashes, uint32_t num);
    void set_stroke_attr(int32_t idx, int32_t val);
    void set_stroke_attr_val(int32_t idx, scalar val);
    void set_fill_attr(int32_t idx, int32_t val);

    void add_shape(const vertex_source& vs, uint32_t id = 0);
    void reset(void);
    void commit(void);

    bool is_empty(void) const;
public:
    static bool fill_contents_point(const vertex_source& vs, scalar x, scalar y, filling_rule rule);
    static bool stroke_contents_point(const vertex_source& vs, scalar x, scalar y, scalar w);
public:
    abstract_raster_adapter* impl(void) const { return m_impl; }
private:
    abstract_raster_adapter* m_impl;
};

}
#endif/*_PICASSO_RASTER_ADAPTER_H_*/

