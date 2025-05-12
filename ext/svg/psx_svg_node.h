/*
 * Copyright (c) 2025, Zhang Ji Peng
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

#ifndef _PSX_SVG_NODE_H_
#define _PSX_SVG_NODE_H_

#include "picasso.h"

#include "psx_common.h"
#include "psx_array.h"
#include "psx_tree.h"

#define PSX_SVG_VERSION "Tiny 1.2"

// svg tag
typedef enum {
    SVG_TAG_INVALID = -1,
    SVG_TAG_CONTENT,
    SVG_TAG_ENTITY,
    SVG_TAG_SVG,
    SVG_TAG_USE,
    SVG_TAG_A,
    SVG_TAG_G,
    SVG_TAG_PATH,
    SVG_TAG_RECT,
    SVG_TAG_CIRCLE,
    SVG_TAG_ELLIPSE,
    SVG_TAG_LINE,
    SVG_TAG_POLYLINE,
    SVG_TAG_POLYGON,
    SVG_TAG_SOLID_COLOR,
    SVG_TAG_LINEAR_GRADIENT,
    SVG_TAG_RADIAL_GRADIENT,
    SVG_TAG_STOP,
    SVG_TAG_DEFS,
    SVG_TAG_IMAGE,
    SVG_TAG_MPATH,
    SVG_TAG_SET,
    SVG_TAG_ANIMATE,
    SVG_TAG_ANIMATE_COLOR,
    SVG_TAG_ANIMATE_TRANSFORM,
    SVG_TAG_ANIMATE_MOTION,
    SVG_TAG_ANIMATION,
    SVG_TAG_TEXT,
    SVG_TAG_TSPAN,
    SVG_TAG_TEXT_AREA,
    SVG_TAG_TBREAK,
} psx_svg_tag;

// attributes
typedef enum {
    SVG_ATTR_INVALID = -1,
    SVG_ATTR_ID,
    SVG_ATTR_XML_ID,
    SVG_ATTR_VERSION,
    SVG_ATTR_BASE_PROFILE,
    SVG_ATTR_VIEWBOX,
    SVG_ATTR_PRESERVE_ASPECT_RATIO,
    SVG_ATTR_VIEWPORT_FILL,
    SVG_ATTR_VIEWPORT_FILL_OPACITY,
    SVG_ATTR_DISPLAY,
    SVG_ATTR_VISIBILITY,
    SVG_ATTR_X,
    SVG_ATTR_Y,
    SVG_ATTR_WIDTH,
    SVG_ATTR_HEIGHT,
    SVG_ATTR_RX,
    SVG_ATTR_RY,
    SVG_ATTR_CX,
    SVG_ATTR_CY,
    SVG_ATTR_R,
    SVG_ATTR_X1,
    SVG_ATTR_Y1,
    SVG_ATTR_X2,
    SVG_ATTR_Y2,
    SVG_ATTR_POINTS,
    SVG_ATTR_D,
    SVG_ATTR_PATH_LENGTH,
    SVG_ATTR_XLINK_HREF,
    SVG_ATTR_FILL,
    SVG_ATTR_FILL_RULE,
    SVG_ATTR_FILL_OPACITY,
    SVG_ATTR_STROKE,
    SVG_ATTR_STROKE_WIDTH,
    SVG_ATTR_STROKE_LINECAP,
    SVG_ATTR_STROKE_LINEJOIN,
    SVG_ATTR_STROKE_MITER_LIMIT,
    SVG_ATTR_STROKE_DASH_ARRAY,
    SVG_ATTR_STROKE_DASH_OFFSET,
    SVG_ATTR_STROKE_OPACITY,
    SVG_ATTR_OPACITY,
    SVG_ATTR_SOLID_COLOR,
    SVG_ATTR_SOLID_OPACITY,
    SVG_ATTR_GRADIENT_UNITS,
    SVG_ATTR_GRADIENT_STOP_OFFSET,
    SVG_ATTR_GRADIENT_STOP_COLOR,
    SVG_ATTR_GRADIENT_STOP_OPACITY,
    SVG_ATTR_FONT_FAMILY,
    SVG_ATTR_FONT_STYLE,
    SVG_ATTR_FONT_VARIANT,
    SVG_ATTR_FONT_WEIGHT,
    SVG_ATTR_FONT_SIZE,
    SVG_ATTR_TRANSFORM,
    SVG_ATTR_TEXT_ANCHOR,
    SVG_ATTR_ATTRIBUTE_NAME,
    SVG_ATTR_ATTRIBUTE_TYPE,
    SVG_ATTR_BEGIN,
    SVG_ATTR_END,
    SVG_ATTR_DUR,
    SVG_ATTR_MIN,
    SVG_ATTR_MAX,
    SVG_ATTR_RESTART,
    SVG_ATTR_REPEAT_COUNT,
    SVG_ATTR_REPEAT_DUR,
    SVG_ATTR_CALC_MODE,
    SVG_ATTR_VALUES,
    SVG_ATTR_KEY_TIMES,
    SVG_ATTR_KEY_SPLINES,
    SVG_ATTR_KEY_POINTS,
    SVG_ATTR_FROM,
    SVG_ATTR_TO,
    SVG_ATTR_BY,
    SVG_ATTR_ADDITIVE,
    SVG_ATTR_ACCUMULATE,
    SVG_ATTR_PATH,
    SVG_ATTR_ROTATE,
    SVG_ATTR_TRANSFORM_TYPE,
} psx_svg_attr_type;

// transform type
enum {
    SVG_TRANSFORM_TYPE_MATRIX = 1,
    SVG_TRANSFORM_TYPE_TRANSLATE,
    SVG_TRANSFORM_TYPE_ROTATE,
    SVG_TRANSFORM_TYPE_SCALE,
    SVG_TRANSFORM_TYPE_SKEW_X,
    SVG_TRANSFORM_TYPE_SKEW_Y,
};

enum {
    SVG_ASPECT_RATIO_NONE = 0,
    SVG_ASPECT_RATIO_XMIN_YMIN = (1 << 1),
    SVG_ASPECT_RATIO_XMID_YMIN = (2 << 1),
    SVG_ASPECT_RATIO_XMAX_YMIN = (3 << 1),
    SVG_ASPECT_RATIO_XMIN_YMID = (4 << 1),
    SVG_ASPECT_RATIO_XMID_YMID = (5 << 1),
    SVG_ASPECT_RATIO_XMAX_YMID = (6 << 1),
    SVG_ASPECT_RATIO_XMIN_YMAX = (7 << 1),
    SVG_ASPECT_RATIO_XMID_YMAX = (8 << 1),
    SVG_ASPECT_RATIO_XMAX_YMAX = (9 << 1),
};

enum {
    SVG_ASPECT_RATIO_OPT_MEET = 0,
    SVG_ASPECT_RATIO_OPT_SLICE,
};

// animation
enum {
    SVG_ANIMATION_REMOVE = 0,
    SVG_ANIMATION_FREEZE,
};

enum {
    SVG_ANIMATION_RESTART_ALWAYS = 0,
    SVG_ANIMATION_RESTART_WHEN_NOT_ACTIVE,
    SVG_ANIMATION_RESTART_NEVER,
};

enum {
    SVG_ANIMATION_CALC_MODE_LINEAR = 0,
    SVG_ANIMATION_CALC_MODE_PACED,
    SVG_ANIMATION_CALC_MODE_SPLINE,
    SVG_ANIMATION_CALC_MODE_DISCRETE,
};

enum {
    SVG_ANIMATION_ADDITIVE_REPLACE = 0,
    SVG_ANIMATION_ADDITIVE_SUM,
};

enum {
    SVG_ANIMATION_ACCUMULATE_NONE = 0,
    SVG_ANIMATION_ACCUMULATE_SUM,
};

// graphic
enum {
    SVG_GRADIENT_UNITS_OBJECT = 0,
    SVG_GRADIENT_UNITS_USER_SPACE,
};

typedef struct _ps_point psx_svg_point;

typedef struct {
    float m[3][3];
} psx_svg_matrix;

enum {
    SVG_ATTR_VALUE_DATA = 0,
    SVG_ATTR_VALUE_PTR,
    SVG_ATTR_VALUE_PATH_PTR,
};

enum {
    SVG_ATTR_VALUE_NONE = 0,
    SVG_ATTR_VALUE_INITIAL,
    SVG_ATTR_VALUE_INHERIT,
};

typedef union {
    int32_t ival;
    uint32_t uval;
    float fval;
    char* sval;
    void* val;
} psx_svg_attr_value;

/*
 * to simplify list, allocate enough memory for all data and length.
 * | size | data[0] | data[1] | data[2] | ... |
 */
typedef struct {
    uint32_t length;
    uint8_t data[1];
} psx_svg_attr_values_list;

typedef struct {
    int32_t attr_id : 8;
    int32_t val_type : 8;
    int32_t class_type : 8;
    psx_svg_attr_value value;
} psx_svg_attr;

class psx_svg_render_obj;

// svg dom node
class psx_svg_node : public psx_tree_node
{
public:
    psx_svg_node(psx_svg_node* parent);
    virtual ~psx_svg_node();

    psx_svg_tag type(void) const { return m_tag; }
    void set_type(psx_svg_tag tag) { m_tag = tag; }

    const char* content(uint32_t* rlen = NULL) const
    {
        if (rlen) {
            *rlen = m_len;
        }
        return m_data;
    }

    bool has_content(void) const { return m_data != NULL; }
    void set_content(const char* data, uint32_t len);

    uint32_t attr_count(void) const { return psx_array_size(&m_attrs); }
    psx_svg_attr* attr_at(uint32_t idx) const
    {
        return psx_array_get(&m_attrs, idx, psx_svg_attr);
    }

    psx_array* attrs(void) { return &m_attrs; }

    psx_svg_node* get_child(uint32_t idx) const
    {
        return (psx_svg_node*)psx_tree_node::get_child(idx);
    }

    psx_svg_node* parent(void) const { return (psx_svg_node*)psx_tree_node::parent(); }

    void set_render(psx_svg_render_obj* render) { m_render_obj = render; }
    const psx_svg_render_obj* render(void) const { return m_render_obj; }

    NON_COPYABLE_CLASS(psx_svg_node);
private:
    char* m_data; // id or content
    uint32_t m_len;
    psx_svg_tag m_tag;
    psx_array m_attrs;
    psx_svg_render_obj* m_render_obj;
};

#ifdef __cplusplus
extern "C" {
#endif

psx_svg_node* psx_svg_load_data(const char* svg_data, uint32_t len);

psx_svg_node* psx_svg_node_create(psx_svg_node* parent);

void psx_svg_node_destroy(psx_svg_node* node);

// svg matrix function
void psx_svg_matrix_identity(psx_svg_matrix* matrix);

void psx_svg_matrix_init(psx_svg_matrix* matrix, float a, float b, float c, float d, float e, float f);

void psx_svg_matrix_translate(psx_svg_matrix* matrix, float tx, float ty);

void psx_svg_matrix_scale(psx_svg_matrix* matrix, float sx, float sy);

void psx_svg_matrix_rotate(psx_svg_matrix* matrix, float rad);

void psx_svg_matrix_skew(psx_svg_matrix* matrix, float shx, float shy);

#ifdef __cplusplus
}
#endif

#endif /* _PSX_SVG_NODE_H_ */
