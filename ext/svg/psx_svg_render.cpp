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

#include "psx_svg_render.h"
#include "psx_linear_allocator.h"
#include "images/psx_image.h"

#include <new>
#include <math.h>

#ifndef M_PI
    #define M_PI 3.1415926f
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ABS(a) fabsf(a)
#define PCT_TO_PX(v, base) ((v) > 1 ? (v) : ((v) * (base)))

#define DEFAULT_FONT_FAMILY "sans-serif"
#define DEFAULT_FONT_SIZE 16

class render_obj_base;

class svg_render_list_impl : public psx_svg_render_list
{
public:
    svg_render_list_impl()
        : m_alloc(NULL)
        , m_head(NULL)
    {
        m_alloc = psx_linear_allocator_create(PSX_LINEAR_ALIGN_DEFAULT);
    }

    virtual ~svg_render_list_impl()
    {
        psx_linear_allocator_destroy(m_alloc);
    }

    uint32_t get_bytes_size(void) const
    {
        return (uint32_t)m_alloc->total_memory;
    }

    template <typename T>
    T* new_obj(const psx_svg_node* node)
    {
        void* mem = m_alloc->alloc(m_alloc, sizeof(T));
        return new (mem) T(node, m_alloc);
    }

    template <typename T>
    void delete_obj(T* v)
    {
        v->destroy_obj();
    }

    void* alloc(size_t size)
    {
        return m_alloc->alloc(m_alloc, size);
    }

    void set_head(render_obj_base* head) { m_head = head; }
    render_obj_base* head(void) const { return m_head; }
private:
    psx_linear_allocator* m_alloc;
    render_obj_base* m_head;
};

enum {
    RENDER_FILL = 1,
    RENDER_STROKE = 2,
};

enum {
    RENDER_ATTR_FILL = (4 << 1),
    RENDER_ATTR_REF_FILL = (4 << 2),
    RENDER_ATTR_FILL_RULE = (4 << 3),
    RENDER_ATTR_FILL_OPACITY = (4 << 4),
    RENDER_ATTR_STROKE = (4 << 5),
    RENDER_ATTR_REF_STROKE = (4 << 6),
    RENDER_ATTR_STROKE_OPACITY = (4 << 7),
    RENDER_ATTR_STROKE_WIDTH = (4 << 8),
    RENDER_ATTR_STROKE_LINECAP = (4 << 9),
    RENDER_ATTR_STROKE_LINEJOIN = (4 << 10),
    RENDER_ATTR_STROKE_MITER_LIMIT = (4 << 11),
    RENDER_ATTR_STROKE_DASH_ARRAY = (4 << 12),
    RENDER_ATTR_STROKE_DASH_OFFSET = (4 << 13),
};

struct ps_draw_attrs {
    int32_t ref_count;
    uint32_t flags;
    ps_color fill_color;
    ps_color stroke_color;
    float fill_opacity;
    int32_t fill_rule;
    float stroke_opacity;
    float stroke_line_width;
    int32_t stroke_line_cap;
    int32_t stroke_line_join;
    float stroke_miter_limit;
    float stroke_dash_offset;
    float* stroke_dash_array;
    uint32_t stroke_dash_num;
    char* fill_ref;
    char* stroke_ref;
};

// draw build state
struct _svg_list_builder_state {
    const psx_svg_node* doc;
    svg_render_list_impl* list;
    ps_draw_attrs* draw_attrs;
    ps_matrix* global_matrix;
    render_obj_base* tail;
    int32_t in_group_deps;
    bool in_defs;
    const psx_svg_node* cur_text;
    bool in_text;
};

static INLINE void svg_to_pcolor(ps_color* val, uint32_t color)
{
    val->r = ((color >> 16) & 0xff) / 255.0f;
    val->g = ((color >> 8) & 0xff) / 255.0f;
    val->b = (color & 0xff) / 255.0f;
    val->a = 1.0f;
}

static INLINE void init_draw_attrs(ps_draw_attrs* attrs)
{
    ps_color c = { 0.0f, 0.0f, 0.0f, 1.0f };
    attrs->ref_count = 0;
    attrs->flags = RENDER_FILL;
    attrs->fill_color = c;
    attrs->fill_opacity = 1.0f;
    attrs->fill_rule = FILL_RULE_WINDING;
    attrs->stroke_color = c;
    attrs->stroke_opacity = 1.0f;
    attrs->stroke_line_width = 1.0f;
    attrs->stroke_line_cap = LINE_CAP_BUTT;
    attrs->stroke_line_join = LINE_JOIN_MITER;
    attrs->stroke_miter_limit = 4.0f;
    attrs->stroke_dash_offset = 0.0f;
    attrs->stroke_dash_array = NULL;
    attrs->stroke_dash_num = 0;
    attrs->fill_ref = NULL;
    attrs->stroke_ref = NULL;
}

static INLINE void copy_draw_attrs(ps_draw_attrs* attrs, const ps_draw_attrs* other)
{
    attrs->ref_count = 0;
    attrs->flags = other->flags;
    attrs->fill_color = other->fill_color;
    attrs->fill_opacity = other->fill_opacity;
    attrs->fill_rule = other->fill_rule;
    attrs->stroke_color = other->stroke_color;
    attrs->stroke_opacity = other->stroke_opacity;
    attrs->stroke_line_width = other->stroke_line_width;
    attrs->stroke_line_cap = other->stroke_line_cap;
    attrs->stroke_line_join = other->stroke_line_join;
    attrs->stroke_miter_limit = other->stroke_miter_limit;
    attrs->stroke_dash_offset = other->stroke_dash_offset;
    attrs->stroke_dash_array = other->stroke_dash_array; // shared data
    attrs->stroke_dash_num = other->stroke_dash_num;
    attrs->fill_ref = other->fill_ref;
    attrs->stroke_ref = other->stroke_ref;
}

static INLINE void set_draw_attr(const _svg_list_builder_state* state, ps_draw_attrs* attrs, int32_t attr_id, const psx_svg_attr* val)
{
    switch (attr_id) {
        case SVG_ATTR_FILL: {
                if (val->class_type == SVG_ATTR_VALUE_NONE) {
                    attrs->flags &= ~RENDER_FILL;
                    return;
                } else if (val->class_type == SVG_ATTR_VALUE_INHERIT) {
                    attrs->flags &= ~RENDER_ATTR_FILL;
                    attrs->flags &= ~RENDER_ATTR_REF_FILL;
                    return;
                } else {
                    if (val->val_type == SVG_ATTR_VALUE_PTR) {
                        // FIXME: reduce string dup function
                        size_t len = strlen(val->value.sval);
                        char* link = (char*)state->list->alloc(len + 1);
                        mem_copy(link, val->value.sval, len);
                        link[len] = '\0';

                        attrs->fill_ref = link;

                        attrs->flags &= ~RENDER_ATTR_FILL;
                        attrs->flags |= RENDER_ATTR_REF_FILL;
                    } else {
                        svg_to_pcolor(&attrs->fill_color, val->value.uval);
                        attrs->flags &= ~RENDER_ATTR_REF_FILL;
                        attrs->flags |= RENDER_ATTR_FILL;
                    }
                    attrs->flags |= RENDER_FILL;
                }
                break;
            }
        case SVG_ATTR_FILL_RULE: {
                if (val->class_type == SVG_ATTR_VALUE_INHERIT) {
                    attrs->flags &= ~RENDER_ATTR_FILL_RULE;
                    return;
                }
                attrs->fill_rule = val->value.ival;
                attrs->flags |= RENDER_ATTR_FILL_RULE;
                break;
            }
        case SVG_ATTR_FILL_OPACITY: {
                if (val->class_type == SVG_ATTR_VALUE_INHERIT) {
                    attrs->flags &= ~RENDER_ATTR_FILL_OPACITY;
                    return;
                }
                attrs->fill_opacity = val->value.fval;
                attrs->flags |= RENDER_ATTR_FILL_OPACITY;
                break;
            }
        case SVG_ATTR_STROKE: {
                if (val->class_type == SVG_ATTR_VALUE_NONE) {
                    attrs->flags &= ~RENDER_STROKE;
                    return;
                } else if (val->class_type == SVG_ATTR_VALUE_INHERIT) {
                    attrs->flags &= ~RENDER_ATTR_STROKE;
                    attrs->flags &= ~RENDER_ATTR_REF_STROKE;
                    return;
                } else {
                    if (val->val_type == SVG_ATTR_VALUE_PTR) {
                        //FIXME: reduce string dup function.
                        size_t len = strlen(val->value.sval);
                        char* link = (char*)state->list->alloc(len + 1);
                        mem_copy(link, val->value.sval, len);
                        link[len] = '\0';

                        attrs->stroke_ref = link;

                        attrs->flags &= ~RENDER_ATTR_STROKE;
                        attrs->flags |= RENDER_ATTR_REF_STROKE;
                    } else {
                        svg_to_pcolor(&attrs->stroke_color, val->value.uval);
                        attrs->flags &= ~RENDER_ATTR_REF_STROKE;
                        attrs->flags |= RENDER_ATTR_STROKE;
                    }
                    attrs->flags |= RENDER_STROKE;
                }
                break;
            }
        case SVG_ATTR_STROKE_OPACITY: {
                if (val->class_type == SVG_ATTR_VALUE_INHERIT) {
                    attrs->flags &= ~RENDER_ATTR_STROKE_OPACITY;
                    return;
                }
                attrs->stroke_opacity = val->value.fval;
                attrs->flags |= RENDER_ATTR_STROKE_OPACITY;
                break;
            }
        case SVG_ATTR_STROKE_WIDTH: {
                if (val->class_type == SVG_ATTR_VALUE_INHERIT) {
                    attrs->flags &= ~RENDER_ATTR_STROKE_WIDTH;
                    return;
                }
                attrs->stroke_line_width = val->value.fval;
                attrs->flags |= RENDER_ATTR_STROKE_WIDTH;
                break;
            }
        case SVG_ATTR_STROKE_LINECAP: {
                if (val->class_type == SVG_ATTR_VALUE_INHERIT) {
                    attrs->flags &= ~RENDER_ATTR_STROKE_LINECAP;
                    return;
                }
                attrs->stroke_line_cap = val->value.ival;
                attrs->flags |= RENDER_ATTR_STROKE_LINECAP;
                break;
            }
        case SVG_ATTR_STROKE_LINEJOIN: {
                if (val->class_type == SVG_ATTR_VALUE_INHERIT) {
                    attrs->flags &= ~RENDER_ATTR_STROKE_LINEJOIN;
                    return;
                }
                attrs->stroke_line_join = val->value.ival;
                attrs->flags |= RENDER_ATTR_STROKE_LINEJOIN;
                break;
            }
        case SVG_ATTR_STROKE_MITER_LIMIT: {
                if (val->class_type == SVG_ATTR_VALUE_INHERIT) {
                    attrs->flags &= ~RENDER_ATTR_STROKE_MITER_LIMIT;
                    return;
                }
                attrs->stroke_miter_limit = val->value.fval;
                attrs->flags |= RENDER_ATTR_STROKE_MITER_LIMIT;
                break;
            }
        case SVG_ATTR_STROKE_DASH_ARRAY: {
                if (val->class_type == SVG_ATTR_VALUE_NONE) {
                    attrs->stroke_dash_array = NULL;
                    attrs->stroke_dash_num = 0;
                    attrs->flags |= RENDER_ATTR_STROKE_DASH_ARRAY;
                    return;
                } else if (val->class_type == SVG_ATTR_VALUE_INHERIT) {
                    attrs->flags &= ~RENDER_ATTR_STROKE_DASH_ARRAY;
                    return;
                } else {
                    psx_svg_attr_values_list* vals = (psx_svg_attr_values_list*)(val->value.val);
                    uint32_t len = vals->length;
                    float* dashs = (float*)(&vals->data);

                    attrs->stroke_dash_array = (float*)state->list->alloc(len * sizeof(float));
                    for (uint32_t i = 0; i < len; i++) {
                        attrs->stroke_dash_array[i] = dashs[i];
                    }

                    attrs->stroke_dash_num = len;
                    attrs->flags |= RENDER_ATTR_STROKE_DASH_ARRAY;
                }
                break;
            }
        case SVG_ATTR_STROKE_DASH_OFFSET: {
                if (val->class_type == SVG_ATTR_VALUE_INHERIT) {
                    attrs->flags &= ~RENDER_ATTR_STROKE_DASH_OFFSET;
                    return;
                }
                attrs->stroke_dash_offset = val->value.fval;
                attrs->flags |= RENDER_ATTR_STROKE_DASH_OFFSET;
                break;
            }
    }
}

static INLINE ps_draw_attrs* draw_attrs_create(svg_render_list_impl* list)
{
    ps_draw_attrs* attrs = (ps_draw_attrs*)list->alloc(sizeof(ps_draw_attrs));
    // init attrs
    init_draw_attrs(attrs);
    return attrs;
}

static INLINE ps_draw_attrs* get_current_attrs(_svg_list_builder_state* state)
{
    ps_draw_attrs* attrs = state->draw_attrs;
    if (attrs->ref_count == 0) {
        return attrs;
    }

    attrs = (ps_draw_attrs*)state->list->alloc(sizeof(ps_draw_attrs));
    // copy attrs
    copy_draw_attrs(attrs, state->draw_attrs);
    attrs->ref_count = 0;
    state->draw_attrs = attrs;
    return attrs;
}

typedef enum {
    RENDER_NORMAL = 0,
    RENDER_IN_DEFS = 1,
    RENDER_IN_GROUP = 2,
    RENDER_IN_TEXT = 4,
} svg_render_type;

class render_obj_base : public psx_svg_render_obj
{
public:
    render_obj_base(const psx_svg_node* node)
        : m_next(NULL)
        , m_render_type(RENDER_NORMAL)
        , m_id(NULL)
        , m_head(NULL)
        , m_draw_attrs(NULL)
        , m_global_matrix(NULL)
    {
        m_tag = node->type();
        m_matrix = ps_matrix_create();
    }

    virtual ~render_obj_base()
    {
        ps_matrix_unref(m_matrix);
    }

    virtual void destroy_obj(void)
    {
        this->render_obj_base::~render_obj_base();
    }

    virtual void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state);
    virtual void render(ps_context* ctx, const ps_matrix* matrix) { }
    virtual void get_bounding_rect(ps_rect* rc) const { }
    virtual void update(const psx_svg_node* node) { }
    virtual void set_fill_attrs(ps_context* ctx, const render_obj_base* obj) { }
    virtual void set_stroke_attrs(ps_context* ctx, const render_obj_base* obj) { }

    virtual void prepare(ps_context* ctx) const
    {
        if (m_draw_attrs) {
            if (m_draw_attrs->flags & RENDER_ATTR_FILL) {
                // color fill
                ps_color color = m_draw_attrs->fill_color;
                if (m_draw_attrs->flags & RENDER_ATTR_FILL_OPACITY) {
                    color.a *= m_draw_attrs->fill_opacity;
                }
                ps_set_source_color(ctx, &color);
            }

            if (m_draw_attrs->flags & RENDER_ATTR_FILL_RULE) {
                ps_set_fill_rule(ctx, (ps_fill_rule)m_draw_attrs->fill_rule);
            }

            if (m_draw_attrs->flags & RENDER_ATTR_STROKE) {
                // color stroke
                ps_color color = m_draw_attrs->stroke_color;
                if (m_draw_attrs->flags & RENDER_ATTR_STROKE_OPACITY) {
                    color.a *= m_draw_attrs->stroke_opacity;
                }
                ps_set_stroke_color(ctx, &color);
            }

            if (m_draw_attrs->flags & RENDER_ATTR_STROKE_WIDTH) {
                ps_set_line_width(ctx, m_draw_attrs->stroke_line_width);
            }

            if (m_draw_attrs->flags & RENDER_ATTR_STROKE_LINECAP) {
                ps_set_line_cap(ctx, (ps_line_cap)m_draw_attrs->stroke_line_cap);
            }

            if (m_draw_attrs->flags & RENDER_ATTR_STROKE_LINEJOIN) {
                ps_set_line_join(ctx, (ps_line_join)m_draw_attrs->stroke_line_join);
            }

            if (m_draw_attrs->flags & RENDER_ATTR_STROKE_MITER_LIMIT) {
                ps_set_miter_limit(ctx, m_draw_attrs->stroke_miter_limit);
            }

            if (m_draw_attrs->flags & RENDER_ATTR_STROKE_DASH_ARRAY) {
                float start = 0.0f;
                if (m_draw_attrs->flags & RENDER_ATTR_STROKE_DASH_OFFSET) {
                    start = m_draw_attrs->stroke_dash_offset;
                }

                if (m_draw_attrs->stroke_dash_array) {
                    ps_set_line_dash(ctx, start, m_draw_attrs->stroke_dash_array, m_draw_attrs->stroke_dash_num);
                } else {
                    ps_reset_line_dash(ctx);
                }
            }
        }
    }

    void paint(ps_context* ctx) const
    {
        if (m_draw_attrs) {
            ps_draw_attrs* attrs = m_draw_attrs;

            if (attrs->flags & RENDER_ATTR_REF_FILL) {
                render_obj_base* head = m_head;
                while (head) {
                    if (head->id()) {
                        if (strcmp(m_draw_attrs->fill_ref, head->id()) == 0) {
                            head->set_fill_attrs(ctx, this);
                            break;
                        }
                    }
                    head = head->next();
                }
            }

            if (attrs->flags & RENDER_ATTR_REF_STROKE) {
                render_obj_base* head = m_head;
                while (head) {
                    if (head->id()) {
                        if (strcmp(m_draw_attrs->stroke_ref, head->id()) == 0) {
                            head->set_stroke_attrs(ctx, this);
                            break;
                        }
                    }
                    head = head->next();
                }
            }

            if (m_global_matrix) {
                ps_transform(ctx, m_global_matrix);
            }

            if ((attrs->flags & RENDER_FILL) && (attrs->flags & RENDER_STROKE)) {
                ps_paint(ctx);
            } else {
                if (attrs->flags & RENDER_FILL) {
                    ps_fill(ctx);
                } else if (attrs->flags & RENDER_STROKE) {
                    ps_stroke(ctx);
                }
            }
        }
    }

    void set_draw_attrs(ps_draw_attrs* attrs)
    {
        m_draw_attrs = attrs;
        attrs->ref_count++;
    }
    ps_draw_attrs* get_draw_attrs(void) const { return m_draw_attrs; }

    psx_svg_tag type(void) const { return m_tag; }
    svg_render_type render_type(void) const { return m_render_type; }
    void set_render_type(svg_render_type render_type) { m_render_type = render_type; }

    void set_matrix(const psx_svg_matrix* mtx)
    {
        ps_matrix_init(m_matrix,
                       mtx->m[0][0],
                       mtx->m[1][0],
                       mtx->m[0][1],
                       mtx->m[1][1],
                       mtx->m[0][2],
                       mtx->m[1][2]
                      );
    }

    void set_global_matrix(ps_matrix* matrix) { m_global_matrix = matrix; }

    void set_next(render_obj_base* obj) { m_next = obj; }
    render_obj_base* next(void) const { return m_next; }

    void set_id(const char* id) { m_id = id; }
    const char* id(void) const { return m_id; }

    void set_head(render_obj_base* head) { m_head = head; }
protected:
    render_obj_base* m_next;
    psx_svg_tag m_tag;
    svg_render_type m_render_type;
    const char* m_id;
    render_obj_base* m_head;
    ps_matrix* m_matrix;
    ps_draw_attrs* m_draw_attrs;
    ps_matrix* m_global_matrix;
};

void render_obj_base::set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
{
    if (attr->attr_id == SVG_ATTR_TRANSFORM) {
        if (attr->class_type == SVG_ATTR_VALUE_NONE) {
            return;
        }
        set_matrix((psx_svg_matrix*)attr->value.val);
    } else {
        ps_draw_attrs* draw_attrs = get_current_attrs(state);
        set_draw_attr(state, draw_attrs, attr->attr_id, attr);
    }
}

// viewport
class svg_render_viewport : public render_obj_base
{
public:
    svg_render_viewport(const psx_svg_node* node, psx_linear_allocator* allocator)
        : render_obj_base(node)
        , m_width(0)
        , m_height(0)
        , m_fill(false)
    {
    }

    void render(ps_context* ctx, const ps_matrix*)
    {
        if (m_fill) {
            ps_save(ctx);
            ps_transform(ctx, m_matrix);
            ps_rect rc = { 0, 0, m_width, m_height };
            prepare(ctx);
            ps_set_composite_operator(ctx, COMPOSITE_SRC);
            ps_rectangle(ctx, &rc);
            ps_fill(ctx);
            ps_restore(ctx);
        }
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        switch (attr->attr_id) {
            case SVG_ATTR_WIDTH:
                m_width = attr->value.fval;
                break;
            case SVG_ATTR_HEIGHT:
                m_height = attr->value.fval;
                break;
            case SVG_ATTR_VIEWBOX: {
                    if (attr->class_type == SVG_ATTR_VALUE_INITIAL) {
                        float* vals = (float*)attr->value.val;
                        float scale_x = 1.0f;
                        float scale_y = 1.0f;
                        float trans_x = vals[0];
                        float trans_y = vals[1];

                        if (m_width > 0 && vals[2] > 0) {
                            scale_x = m_width / vals[2];
                        }
                        if (m_height > 0 && vals[3] > 0) {
                            scale_y = m_height / vals[3];
                        }
                        m_width = scale_x * vals[2];
                        m_height = scale_y * vals[3];

                        ps_matrix_translate(m_matrix, -trans_x, -trans_y);
                        ps_matrix_scale(m_matrix, scale_x, scale_y);
                        state->global_matrix = m_matrix;
                    }
                }
                break;
            case SVG_ATTR_VIEWPORT_FILL: {
                    if (attr->class_type == SVG_ATTR_VALUE_INITIAL
                        && attr->val_type == SVG_ATTR_VALUE_DATA) {
                        ps_draw_attrs* draw_attrs = get_current_attrs(state);
                        set_draw_attr(state, draw_attrs, SVG_ATTR_FILL, attr);
                        m_fill = true;
                    } else if (attr->class_type == SVG_ATTR_VALUE_NONE) {
                        m_fill = false;
                    }
                }
                break;
            case SVG_ATTR_VIEWPORT_FILL_OPACITY: {
                    if (attr->class_type == SVG_ATTR_VALUE_INITIAL) {
                        ps_draw_attrs* draw_attrs = get_current_attrs(state);
                        set_draw_attr(state, draw_attrs, SVG_ATTR_FILL_OPACITY, attr);
                    }
                }
                break;
        }
    }

    void get_bounding_rect(ps_rect* rc) const
    {
        if (rc) {
            rc->x = 0;
            rc->y = 0;
            rc->w = m_width;
            rc->h = m_height;
        }
    }
private:
    float m_width;
    float m_height;
    bool m_fill;
};

// rect
class svg_render_rect : public render_obj_base
{
public:
    svg_render_rect(const psx_svg_node* node, psx_linear_allocator* allocator)
        : render_obj_base(node)
        , m_rx(0.0f)
        , m_ry(0.0f)
    {
        m_rc.x = m_rc.y = m_rc.w = m_rc.h = 0.0f;
    }

    void render(ps_context* ctx, const ps_matrix* matrix)
    {
        ps_save(ctx);
        ps_transform(ctx, m_matrix);
        if (matrix) {
            ps_transform(ctx, matrix);
        }
        prepare(ctx);

        float rx = m_rx;
        float ry = m_ry;

        if (rx == 0.0f && ry == 0.0f) {
            ps_rectangle(ctx, &m_rc);
        } else {
            if (rx > 0.0f && ry == 0.0f) {
                ry = rx;
            } else if (ry > 0.0f && rx == 0.0f ) {
                rx = ry;
            }
            ps_rounded_rect(ctx, &m_rc, rx, ry, rx, ry, rx, ry, rx, ry);
        }
        paint(ctx);
        ps_restore(ctx);
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        render_obj_base::set_attr(attr, state);
        switch (attr->attr_id) {
            case SVG_ATTR_X:
                m_rc.x = attr->value.fval;
                break;
            case SVG_ATTR_Y:
                m_rc.y = attr->value.fval;
                break;
            case SVG_ATTR_WIDTH:
                m_rc.w = attr->value.fval;
                break;
            case SVG_ATTR_HEIGHT:
                m_rc.h = attr->value.fval;
                break;
            case SVG_ATTR_RX:
                m_rx = attr->value.fval;
                break;
            case SVG_ATTR_RY:
                m_ry = attr->value.fval;
                break;
        }
    }

    void get_bounding_rect(ps_rect* rc) const
    {
        rc->x = m_rc.x;
        rc->y = m_rc.y;
        rc->w = m_rc.w;
        rc->h = m_rc.h;
    }
private:
    ps_rect m_rc;
    float m_rx;
    float m_ry;
};

// circle
class svg_render_circle : public render_obj_base
{
public:
    svg_render_circle(const psx_svg_node* node, psx_linear_allocator* allocator)
        : render_obj_base(node)
        , m_cx(0)
        , m_cy(0)
        , m_r(0)
    {
    }

    void render(ps_context* ctx, const ps_matrix* matrix)
    {
        ps_save(ctx);
        ps_transform(ctx, m_matrix);
        if (matrix) {
            ps_transform(ctx, matrix);
        }
        prepare(ctx);

        ps_rect rc = { m_cx - m_r, m_cy - m_r, m_r + m_r, m_r + m_r };
        ps_ellipse(ctx, &rc);

        paint(ctx);
        ps_restore(ctx);
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        render_obj_base::set_attr(attr, state);
        switch (attr->attr_id) {
            case SVG_ATTR_CX:
                m_cx = attr->value.fval;
                break;
            case SVG_ATTR_CY:
                m_cy = attr->value.fval;
                break;
            case SVG_ATTR_R:
                m_r = attr->value.fval;
                break;
        }
    }

    void get_bounding_rect(ps_rect* rc) const
    {
        rc->x = m_cx - m_r;
        rc->y = m_cy - m_r;
        rc->w = m_r + m_r;
        rc->h = rc->w;
    }

private:
    float m_cx;
    float m_cy;
    float m_r;
};

// ellipse
class svg_render_ellipse : public render_obj_base
{
public:
    svg_render_ellipse(const psx_svg_node* node, psx_linear_allocator* allocator)
        : render_obj_base(node)
        , m_cx(0)
        , m_cy(0)
        , m_rx(0)
        , m_ry(0)
    {
    }

    void render(ps_context* ctx, const ps_matrix* matrix)
    {
        ps_save(ctx);
        ps_transform(ctx, m_matrix);
        if (matrix) {
            ps_transform(ctx, matrix);
        }
        prepare(ctx);

        ps_rect rc = { m_cx - m_rx, m_cy - m_ry, m_rx + m_rx, m_ry + m_ry };
        ps_ellipse(ctx, &rc);

        paint(ctx);
        ps_restore(ctx);
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        render_obj_base::set_attr(attr, state);
        switch (attr->attr_id) {
            case SVG_ATTR_CX:
                m_cx = attr->value.fval;
                break;
            case SVG_ATTR_CY:
                m_cy = attr->value.fval;
                break;
            case SVG_ATTR_RX:
                m_rx = attr->value.fval;
                break;
            case SVG_ATTR_RY:
                m_ry = attr->value.fval;
                break;
        }
    }

    void get_bounding_rect(ps_rect* rc) const
    {
        rc->x = m_cx - m_rx;
        rc->y = m_cy - m_ry;
        rc->w = m_rx + m_rx;
        rc->h = m_ry + m_ry;
    }

private:
    float m_cx;
    float m_cy;
    float m_rx;
    float m_ry;
};

// line
class svg_render_line : public render_obj_base
{
public:
    svg_render_line(const psx_svg_node* node, psx_linear_allocator* allocator)
        : render_obj_base(node)
        , m_x1(0.0f)
        , m_y1(0.0f)
        , m_x2(0.0f)
        , m_y2(0.0f)
    {
    }

    void render(ps_context* ctx, const ps_matrix* matrix)
    {
        ps_save(ctx);
        ps_transform(ctx, m_matrix);
        if (matrix) {
            ps_transform(ctx, matrix);
        }
        prepare(ctx);

        ps_point p1 = { m_x1, m_y1 };
        ps_move_to(ctx, &p1);
        ps_point p2 = { m_x2, m_y2 };
        ps_line_to(ctx, &p2);

        paint(ctx);
        ps_restore(ctx);
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        render_obj_base::set_attr(attr, state);
        switch (attr->attr_id) {
            case SVG_ATTR_X1:
                m_x1 = attr->value.fval;
                break;
            case SVG_ATTR_Y1:
                m_y1 = attr->value.fval;
                break;
            case SVG_ATTR_X2:
                m_x2 = attr->value.fval;
                break;
            case SVG_ATTR_Y2:
                m_y2 = attr->value.fval;
                break;
        }
    }

    void get_bounding_rect(ps_rect* rc) const
    {
        rc->x = MIN(m_x1, m_x2);
        rc->y = MIN(m_y1, m_y2);
        rc->w = ABS(m_x2 - m_x1);
        rc->h = ABS(m_y2 - m_y1);
    }

private:
    float m_x1;
    float m_y1;
    float m_x2;
    float m_y2;
};

// polyline
class svg_render_polyline : public render_obj_base
{
public:
    svg_render_polyline(const psx_svg_node* node, psx_linear_allocator* allocator)
        : render_obj_base(node)
    {
        m_path = ps_path_create();
    }

    virtual ~svg_render_polyline()
    {
        ps_path_unref(m_path);
    }

    virtual void destroy_obj(void)
    {
        this->svg_render_polyline::~svg_render_polyline();
    }

    void render(ps_context* ctx, const ps_matrix* matrix)
    {
        ps_save(ctx);
        ps_transform(ctx, m_matrix);
        if (matrix) {
            ps_transform(ctx, matrix);
        }
        prepare(ctx);

        ps_set_path(ctx, m_path);

        paint(ctx);
        ps_restore(ctx);
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        render_obj_base::set_attr(attr, state);

        if (attr->attr_id == SVG_ATTR_POINTS) {
            ps_path_clear(m_path);

            psx_svg_attr_values_list* vals = (psx_svg_attr_values_list*)(attr->value.val);
            uint32_t len = vals->length;
            psx_svg_point* points = (psx_svg_point*)(&vals->data);

            ps_point pt = { points[0].x, points[0].y };
            ps_path_move_to(m_path, &pt);
            for (uint32_t i = 1; i < len; i++) {
                pt.x = points[i].x;
                pt.y = points[i].y;
                ps_path_line_to(m_path, &pt);
            }
        }
    }

    void get_bounding_rect(ps_rect* rc) const
    {
        ps_path_bounding_rect(m_path, rc);
    }

protected:
    ps_path* m_path;
};

// polygon
class svg_render_polygon : public svg_render_polyline
{
public:
    svg_render_polygon(const psx_svg_node* node, psx_linear_allocator* allocator)
        : svg_render_polyline(node, allocator)
    {
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        svg_render_polyline::set_attr(attr, state);

        if (attr->attr_id == SVG_ATTR_POINTS) {
            ps_path_sub_close(m_path);
        }
    }
};

// path
class svg_render_path : public svg_render_polyline
{
public:
    svg_render_path(const psx_svg_node* node, psx_linear_allocator* allocator)
        : svg_render_polyline(node, allocator)
    {
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        render_obj_base::set_attr(attr, state);

        if (attr->attr_id == SVG_ATTR_D) {
            ps_path_unref(m_path);
            ps_path* path = (ps_path*)(attr->value.val);
            m_path = ps_path_ref(path);
        }
    }
};

// use
class svg_render_use : public render_obj_base
{
public:
    svg_render_use(const psx_svg_node* node, psx_linear_allocator* allocator)
        : render_obj_base(node)
        , m_x(0)
        , m_y(0)
        , m_xlink(NULL)
        , m_linked(NULL)
    {
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        render_obj_base::set_attr(attr, state);

        switch (attr->attr_id) {
            case SVG_ATTR_X:
                m_x = attr->value.fval;
                break;
            case SVG_ATTR_Y:
                m_y = attr->value.fval;
                break;
            case SVG_ATTR_XLINK_HREF: {
                    //FIXME: reduce string dup function
                    size_t len = strlen(attr->value.sval);
                    char* xlink = (char*)state->list->alloc(len + 1);
                    mem_copy(xlink, attr->value.sval, len);
                    xlink[len] = '\0';

                    m_xlink = xlink;
                }
                break;
        }
    }

    void render(ps_context* ctx, const ps_matrix* matrix)
    {
        ps_save(ctx);
        ps_transform(ctx, m_matrix);

        ps_matrix* mtx = ps_matrix_create();
        ps_matrix_translate(mtx, m_x, m_y);

        get_xlink();

        if (m_linked) {
            m_linked->prepare(ctx);
            prepare(ctx);
            m_linked->render(ctx, mtx);
        }

        ps_matrix_unref(mtx);
        ps_restore(ctx);
    }

    void get_xlink(void) const
    {
        if (!m_linked) {
            render_obj_base* head = m_head;
            while (head) {
                if (head->id() && strcmp(m_xlink, head->id()) == 0) {
                    m_linked = head;
                    break;
                }
                head = head->next();
            }
        }
    }

    void get_bounding_rect(ps_rect* rc) const
    {
        get_xlink();
        if (m_linked) {
            m_linked->get_bounding_rect(rc);
        } else {
            rc->x = rc->y = rc->w = rc->h = 0;
        }
    }

private:
    float m_x;
    float m_y;
    char* m_xlink;
    mutable render_obj_base* m_linked;
};

// group
class svg_render_group : public render_obj_base
{
public:
    svg_render_group(const psx_svg_node* node, psx_linear_allocator* allocator)
        : render_obj_base(node)
    {
        psx_array_capacity_init(&m_items, node->child_count(), sizeof(render_obj_base*));
    }

    virtual ~svg_render_group()
    {
        psx_array_destroy(&m_items);
    }

    virtual void destroy_obj(void)
    {
        this->svg_render_group::~svg_render_group();
    }

    void render(ps_context* ctx, const ps_matrix* matrix)
    {
        ps_save(ctx);
        ps_transform(ctx, m_matrix);
        prepare(ctx);

        for (uint32_t i = 0; i < psx_array_size(&m_items); i++) {
            render_obj_base* item = *(psx_array_get(&m_items, i, render_obj_base*));

            if (item->render_type() == RENDER_IN_GROUP) {
                ps_save(ctx);
                item->prepare(ctx);
                item->render(ctx, matrix);
                ps_restore(ctx);
            }
        }
        ps_restore(ctx);
    }

    void get_bounding_rect(ps_rect* rc) const
    {
        float x1 = 0;
        float y1 = 0;
        float x2 = 0;
        float y2 = 0;

        for (uint32_t i = 0; i < psx_array_size(&m_items); i++) {
            ps_rect tc = { 0, 0, 0, 0 };
            render_obj_base* item = *(psx_array_get(&m_items, i, render_obj_base*));
            item->get_bounding_rect(&tc);
            x1 = MIN(tc.x, x1);
            y1 = MIN(tc.y, y1);
            x2 = MAX(tc.x + tc.w, x2);
            y2 = MAX(tc.y + tc.h, y2);
        }
        rc->x = x1;
        rc->y = y1;
        rc->w = x2 - x1;
        rc->h = y2 - y1;
    }

    void add_item(render_obj_base* item)
    {
        psx_array_push_back(&m_items, item);
    }

private:
    psx_array m_items;
};

// text
#define SET_FONT_ATTRS(attr, state) \
    do { \
        switch(attr->attr_id) { \
            case SVG_ATTR_FONT_FAMILY: \
                if (attr->val_type == SVG_ATTR_VALUE_PTR) { \
                    ps_path_clear(m_path); \
                    ps_font_unref(m_font); \
                    m_font = ps_font_create(attr->value.sval, CHARSET_UNICODE, m_font_size, m_font_weight, m_font_italic); \
                } \
                break; \
            case SVG_ATTR_FONT_SIZE: \
                if (attr->class_type == SVG_ATTR_VALUE_INITIAL) { \
                    if (attr->val_type == SVG_ATTR_VALUE_DATA) { \
                        ps_path_clear(m_path); \
                        m_font_size = attr->value.fval; \
                        ps_font_set_size(m_font, m_font_size); \
                    } \
                } \
                break; \
            case SVG_ATTR_FONT_STYLE: \
                if (attr->class_type == SVG_ATTR_VALUE_INITIAL) { \
                    if (attr->val_type == SVG_ATTR_VALUE_PTR) { \
                        ps_path_clear(m_path); \
                        if (strncmp(attr->value.sval, "italic", 6) == 0) { \
                            m_font_italic = True; \
                        } else { \
                            m_font_italic = False; \
                        } \
                        ps_font_set_italic(m_font, m_font_italic); \
                    } \
                } \
                break; \
            case SVG_ATTR_FONT_WEIGHT: \
                if (attr->class_type == SVG_ATTR_VALUE_INITIAL) { \
                    if (attr->val_type == SVG_ATTR_VALUE_PTR) { \
                        ps_path_clear(m_path); \
                        if (strncmp(attr->value.sval, "bold", 4) == 0) { \
                            m_font_weight = FONT_WEIGHT_BOLD; \
                        } else { \
                            m_font_weight = FONT_WEIGHT_REGULAR; \
                        } \
                        ps_font_set_weight(m_font, m_font_weight); \
                    } \
                } \
                break; \
            case SVG_ATTR_FONT_VARIANT: \
                if (attr->class_type == SVG_ATTR_VALUE_INITIAL) { \
                    if (attr->val_type == SVG_ATTR_VALUE_PTR) { \
                        ps_path_clear(m_path); \
                        if (strncmp(attr->value.sval, "small-caps", 10) == 0) { \
                            ps_font_set_size(m_font, m_font_size / 2); \
                        } else { \
                            ps_font_set_size(m_font, m_font_size); \
                        } \
                    } \
                } \
                break; \
        } \
    } while(0)

class svg_render_content : public render_obj_base
{
public:
    svg_render_content(const psx_svg_node* node, psx_linear_allocator* allocator)
        : render_obj_base(node)
        , m_letters(NULL)
        , m_letter_count(0)
    {
    }

    virtual void render_content(ps_context* ctx, ps_path* contents, ps_matrix* offset_matrix, float yoffset)
    {
        ps_path* glyph_path = ps_path_create();

        ps_matrix* mtx = ps_matrix_create_copy(offset_matrix);
        ps_matrix_translate(mtx, 0, yoffset);

        for (uint32_t i = 0; i < m_letter_count; i++) {
            uint32_t letter = m_letters[i];
            ps_glyph g;
            ps_get_glyph(ctx, letter, &g);
            ps_path_clear(glyph_path);
            ps_get_path_from_glyph(ctx, &g, glyph_path);
            ps_size s;
            ps_glyph_get_extent(&g, &s);
            ps_matrix_transform_path(mtx, glyph_path);
            ps_path_add_sub_path(contents, glyph_path);
            ps_matrix_translate(mtx, s.w, 0);
            ps_matrix_translate(offset_matrix, s.w, 0);
        }

        ps_matrix_unref(mtx);
        ps_path_unref(glyph_path);
    }

    void set_contents(uint32_t* letters, uint32_t count)
    {
        m_letters = letters;
        m_letter_count = count;
    }

    void get_bounding_rect(ps_rect* rc) const
    {
        rc->x = 0;
        rc->y = 0;
        rc->w = 0;
        rc->h = 0;
    }
protected:
    uint32_t* m_letters;
    uint32_t m_letter_count;
};

class svg_render_text : public render_obj_base
{
public:
    svg_render_text(const psx_svg_node* node, psx_linear_allocator* allocator)
        : render_obj_base(node)
        , m_font_size(DEFAULT_FONT_SIZE)
        , m_font_weight(FONT_WEIGHT_REGULAR)
        , m_font_italic(False)
        , m_x(0)
        , m_y(0)
    {
        m_font = ps_font_create(DEFAULT_FONT_FAMILY, CHARSET_UNICODE, m_font_size, FONT_WEIGHT_REGULAR, False);
        m_path = ps_path_create();
        psx_array_capacity_init(&m_contents, node->child_count(), sizeof(svg_render_content*));
    }

    virtual ~svg_render_text()
    {
        psx_array_destroy(&m_contents);
        ps_path_unref(m_path);
        ps_font_unref(m_font);
    }

    virtual void destroy_obj(void)
    {
        this->svg_render_text::~svg_render_text();
    }

    void render(ps_context* ctx, const ps_matrix* matrix)
    {
        ps_save(ctx);
        ps_transform(ctx, m_matrix);
        if (matrix) {
            ps_transform(ctx, matrix);
        }
        prepare(ctx);

        ps_font* old_font = ps_set_font(ctx, m_font);
        ps_font_info info;
        ps_get_font_info(ctx, &info);

        if (ps_path_is_empty(m_path)) {
            ps_matrix* mtx = ps_matrix_create();
            ps_matrix_translate(mtx, m_x, m_y);

            // draw text contents and spans
            for (uint32_t i = 0; i < psx_array_size(&m_contents); i++) {
                svg_render_content* content = *(psx_array_get(&m_contents, i, svg_render_content*));
                content->render_content(ctx, m_path, mtx, -info.ascent);
            }
            ps_matrix_unref(mtx);
        }

        ps_set_font(ctx, old_font);

        ps_set_path(ctx, m_path);

        paint(ctx);
        ps_restore(ctx);
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        render_obj_base::set_attr(attr, state);

        SET_FONT_ATTRS(attr, state);

        switch (attr->attr_id) {
            case SVG_ATTR_X:
                m_x = attr->value.fval;
                break;
            case SVG_ATTR_Y:
                m_y = attr->value.fval;
                break;
        }
    }

    void get_bounding_rect(ps_rect* rc) const
    {
        ps_path_bounding_rect(m_path, rc);
        for (uint32_t i = 0; i < psx_array_size(&m_contents); i++) {
            ps_rect crc;
            svg_render_content* content = *(psx_array_get(&m_contents, i, svg_render_content*));
            content->get_bounding_rect(&crc);
            if (crc.w != 0 && crc.h != 0) {
                rc->x = MIN(rc->x, crc.x);
                rc->y = MIN(rc->y, crc.y);
                rc->w = MAX(rc->x + rc->w, crc.x + crc.w) - rc->x;
                rc->h = MAX(rc->y + rc->h, crc.y + crc.h) - rc->y;
            }
        }
    }

    void add_content(svg_render_content* content)
    {
        psx_array_push_back(&m_contents, content);
    }

    const ps_font* font(void) const { return m_font; }
    float font_size(void) const { return m_font_size; }
    int32_t font_weight(void) const { return m_font_weight; }
    ps_bool font_italic(void) const { return m_font_italic; }
private:
    ps_font* m_font;
    ps_path* m_path;
    psx_array m_contents;
    float m_font_size;
    int32_t m_font_weight;
    ps_bool m_font_italic;
    float m_x; // FIXME: need support pos array
    float m_y;
};

class svg_render_tspan : public svg_render_content
{
public:
    svg_render_tspan(const psx_svg_node* node, psx_linear_allocator* allocator)
        : svg_render_content(node, allocator)
        , m_font_size(DEFAULT_FONT_SIZE)
        , m_font_weight(FONT_WEIGHT_REGULAR)
        , m_font_italic(False)
    {
        psx_svg_node* parent = node->parent();
        if (parent && parent->type() == SVG_TAG_TEXT) {
            svg_render_text* text = (svg_render_text*)parent->render();
            m_font = ps_font_create_copy(text->font());
            m_font_size = text->font_size();
            m_font_weight = text->font_weight();
            m_font_italic = text->font_italic();
        } else {
            m_font = ps_font_create(DEFAULT_FONT_FAMILY, CHARSET_UNICODE, m_font_size, FONT_WEIGHT_REGULAR, False);
        }
        m_path = ps_path_create();
    }

    virtual ~svg_render_tspan()
    {
        ps_path_unref(m_path);
        ps_font_unref(m_font);
    }

    virtual void destroy_obj(void)
    {
        this->svg_render_tspan::~svg_render_tspan();
    }

    virtual void render_content(ps_context* ctx, ps_path*, ps_matrix* offset_matrix, float)
    {
        ps_save(ctx);
        prepare(ctx);

        ps_font* old_font = ps_set_font(ctx, m_font);
        ps_font_info info;
        ps_get_font_info(ctx, &info);

        if (ps_path_is_empty(m_path)) {

            ps_path* glyph_path = ps_path_create();
            ps_matrix* mtx = ps_matrix_create_copy(offset_matrix);
            ps_matrix_translate(mtx, 0, -info.ascent);

            for (uint32_t i = 0; i < m_letter_count; i++) {
                uint32_t letter = m_letters[i];
                ps_glyph g;
                ps_get_glyph(ctx, letter, &g);
                ps_path_clear(glyph_path);
                ps_get_path_from_glyph(ctx, &g, glyph_path);
                ps_size s;
                ps_glyph_get_extent(&g, &s);
                ps_matrix_transform_path(mtx, glyph_path);
                ps_path_add_sub_path(m_path, glyph_path);
                ps_matrix_translate(mtx, s.w, 0);
                ps_matrix_translate(offset_matrix, s.w, 0);
            }

            ps_matrix_unref(mtx);
            ps_path_unref(glyph_path);
        }

        ps_set_font(ctx, old_font);

        ps_set_path(ctx, m_path);

        paint(ctx);
        ps_restore(ctx);
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        render_obj_base::set_attr(attr, state);

        SET_FONT_ATTRS(attr, state);
    }

    void get_bounding_rect(ps_rect* rc) const
    {
        ps_path_bounding_rect(m_path, rc);
    }

private:
    ps_font* m_font;
    ps_path* m_path;
    float m_font_size;
    int32_t m_font_weight;
    ps_bool m_font_italic;
};

// image
class svg_render_image : public render_obj_base
{
public:
    svg_render_image(const psx_svg_node* node, psx_linear_allocator* allocator)
        : render_obj_base(node)
        , m_image(NULL)
        , m_opacity(1.0f)
        , m_ratio(SVG_ASPECT_RATIO_XMID_YMID | SVG_ASPECT_RATIO_OPT_MEET)
    {
        m_rc.x = m_rc.y = m_rc.w = m_rc.h = 0;
    }

    virtual ~svg_render_image()
    {
        if (m_image) {
            psx_image_destroy(m_image);
        }
    }

    virtual void destroy_obj(void)
    {
        this->svg_render_image::~svg_render_image();
    }

    void render(ps_context* ctx, const ps_matrix* matrix)
    {
        if (!m_image) {
            return;
        }

        ps_save(ctx);
        ps_transform(ctx, m_matrix);
        if (matrix) {
            ps_transform(ctx, matrix);
        }
        prepare(ctx);

        float img_w = (float)m_image->width;
        float img_h = (float)m_image->height;

        ps_rect img_rc;
        img_rc.x = m_rc.x;
        img_rc.y = m_rc.y;
        img_rc.w = img_w;
        img_rc.h = img_h;

        float tx = 0.0f;
        float ty = 0.0f;

        float scale_x = m_rc.w / img_w;
        float scale_y = m_rc.h / img_h;
        float scale = 1.0f;

        if ((m_ratio & 0x1) == SVG_ASPECT_RATIO_OPT_SLICE) {
            scale = MAX(scale_x, scale_y);
        } else if ((m_ratio & 0x1) == SVG_ASPECT_RATIO_OPT_MEET) {
            scale = MIN(scale_x, scale_y);
        }

        if ((m_ratio & ~0x1) == SVG_ASPECT_RATIO_NONE) {
            img_rc = m_rc;
        } else {
            switch (m_ratio & ~0x1) {
                case SVG_ASPECT_RATIO_XMID_YMIN: {
                        tx = (m_rc.w - img_w * scale) / 2;
                    }
                    break;
                case SVG_ASPECT_RATIO_XMAX_YMIN: {
                        tx = m_rc.w - img_w * scale;
                    }
                    break;
                case SVG_ASPECT_RATIO_XMIN_YMID: {
                        ty = (m_rc.h - img_h * scale) / 2;
                    }
                    break;
                case SVG_ASPECT_RATIO_XMID_YMID: {
                        tx = (m_rc.w - img_w * scale) / 2;
                        ty = (m_rc.h - img_h * scale) / 2;
                    }
                    break;
                case SVG_ASPECT_RATIO_XMAX_YMID: {
                        tx = m_rc.w - img_w * scale;
                        ty = (m_rc.h - img_h * scale) / 2;
                    }
                    break;
                case SVG_ASPECT_RATIO_XMIN_YMAX: {
                        ty = m_rc.h - img_h * scale;
                    }
                    break;
                case SVG_ASPECT_RATIO_XMID_YMAX: {
                        tx = (m_rc.w - img_w * scale) / 2;
                        ty = m_rc.h - img_h * scale;
                    }
                    break;
                case SVG_ASPECT_RATIO_XMAX_YMAX: {
                        tx = m_rc.w - img_w * scale;
                        ty = m_rc.h - img_h * scale;
                    }
                    break;
                case SVG_ASPECT_RATIO_XMIN_YMIN:
                case SVG_ASPECT_RATIO_NONE:
                    break;
            }
            img_rc.x += tx;
            img_rc.y += ty;
            img_rc.w *= scale;
            img_rc.h *= scale;
        }

        ps_rectangle(ctx, &img_rc);
        ps_set_source_image(ctx, IMG_OBJ_AT_INDEX(m_image, 0));
        ps_set_alpha(ctx, m_opacity);
        ps_clip_rect(ctx, &m_rc);
        paint(ctx);
        ps_restore(ctx);
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        render_obj_base::set_attr(attr, state);

        switch (attr->attr_id) {
            case SVG_ATTR_X:
                m_rc.x = attr->value.fval;
                break;
            case SVG_ATTR_Y:
                m_rc.y = attr->value.fval;
                break;
            case SVG_ATTR_HEIGHT:
                m_rc.h = attr->value.fval;
                break;
            case SVG_ATTR_WIDTH:
                m_rc.w = attr->value.fval;
                break;
            case SVG_ATTR_OPACITY:
                m_opacity = attr->value.fval;
                break;
            case SVG_ATTR_XLINK_HREF: {
                    const char* xlink = attr->value.sval;
                    if (m_image) {
                        psx_image_destroy(m_image);
                    }
                    int err = 0;
                    m_image = psx_image_load(xlink, &err);
                    if (err != S_OK) {
                        LOG_ERROR("Image load failed!\n");
                        m_image = NULL;
                    }
                    break;
                }
            case SVG_ATTR_PRESERVE_ASPECT_RATIO: {
                    if (attr->class_type == SVG_ATTR_VALUE_INITIAL) {
                        m_ratio = attr->value.uval;
                    }
                    break;
                }
        }
    }

    void get_bounding_rect(ps_rect* rc) const
    {
        *rc = m_rc;
    }
private:
    ps_rect m_rc;
    psx_image* m_image;
    float m_opacity;
    uint32_t m_ratio;
};

// defs
class svg_render_defs : public render_obj_base
{
public:
    typedef struct {
        ps_color color;
        float opacity;
        float start;
    } stop_color;

    svg_render_defs(const psx_svg_node* node, psx_linear_allocator* allocator)
        : render_obj_base(node)
        , m_tag(node->type())
    {
        if (m_tag != SVG_TAG_SOLID_COLOR) {
            init_gradient(node, allocator);
        } else {
            m_color.r = m_color.g = m_color.b = 0.0f;
            m_color.a = m_opacity = 1.0f;
        }
    }

    virtual ~svg_render_defs()
    {
    }

    virtual void destroy_obj(void)
    {
        this->svg_render_defs::~svg_render_defs();
    }

    void init_gradient(const psx_svg_node* node, psx_linear_allocator* allocator)
    {
        m_units = SVG_GRADIENT_UNITS_OBJECT;
        m_cx = 0.5f;
        m_cy = 0.5f;
        m_cr = 0.5f;
        m_x1 = 0.0f;
        m_y1 = 0.0f;
        m_x2 = 1.0f;
        m_y2 = 0.0f;

        uint32_t count = node->child_count();
        m_stops = (stop_color*)allocator->alloc(allocator, count * sizeof(stop_color));
        m_stop_count = count;

        for (uint32_t i = 0; i < count; i++) {
            psx_svg_node* child_node = node->get_child(i);
            uint32_t attr_num = child_node->attr_count();

            m_stops[i].color.r = m_stops[i].color.g = m_stops[i].color.b = 0.0f;
            m_stops[i].color.a = m_stops[i].opacity = 1.0f;
            m_stops[i].start = 0.0f;

            for (uint32_t j = 0; j < attr_num; j++) {
                psx_svg_attr* attr = child_node->attr_at(j);
                switch (attr->attr_id) {
                    case SVG_ATTR_GRADIENT_STOP_COLOR: {
                            svg_to_pcolor(&m_stops[i].color, attr->value.uval);
                        }
                        break;
                    case SVG_ATTR_GRADIENT_STOP_OPACITY: {
                            m_stops[i].opacity = attr->value.fval;
                        }
                        break;
                    case SVG_ATTR_GRADIENT_STOP_OFFSET: {
                            m_stops[i].start = attr->value.fval;
                        }
                        break;
                }
            }
        }
    }

    void setup_gradient(ps_context* ctx, const render_obj_base* obj, bool fill)
    {
        ps_gradient* gradient = NULL;
        ps_rect bound;
        obj->get_bounding_rect(&bound);

        ps_matrix* mtx = ps_matrix_create();

        if (m_tag == SVG_TAG_RADIAL_GRADIENT) {
            float cx = m_cx;
            float cy = m_cy;
            float cr = m_cr;

            if (m_units == SVG_GRADIENT_UNITS_OBJECT) {
                cx = PCT_TO_PX(m_cx, bound.w);
                cy = PCT_TO_PX(m_cy, bound.h);
                cr = PCT_TO_PX(m_cr, MAX(bound.w, bound.h));
                ps_matrix_translate(mtx, bound.x, bound.y);
            }
            ps_point c = { cx, cy };
            gradient = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &c, 0, &c, cr);
            ps_gradient_transform(gradient, mtx);
        } else {
            float x1 = m_x1;
            float y1 = m_y1;
            float x2 = m_x2;
            float y2 = m_y2;

            if (m_units == SVG_GRADIENT_UNITS_OBJECT) {
                x1 = PCT_TO_PX(m_x1, bound.w);
                y1 = PCT_TO_PX(m_y1, bound.h);
                x2 = PCT_TO_PX(m_x2, bound.w);
                y2 = PCT_TO_PX(m_y2, bound.h);
                ps_matrix_translate(mtx, bound.x, bound.y);
            }
            ps_point s = { x1, y1 };
            ps_point e = { x2, y2 };
            gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &s, &e);
            ps_gradient_transform(gradient, mtx);
        }

        for (uint32_t i = 0; i < m_stop_count; i++) {
            ps_color c = m_stops[i].color;
            c.a *= m_stops[i].opacity;
            ps_gradient_add_color_stop(gradient, m_stops[i].start, &c);
        }

        if (fill) {
            ps_set_source_gradient(ctx, gradient);
        } else {
            ps_set_stroke_gradient(ctx, gradient);
        }

        ps_matrix_unref(mtx);
        ps_gradient_unref(gradient);
    }

    void set_fill_attrs(ps_context* ctx, const render_obj_base* obj)
    {
        if (m_tag == SVG_TAG_SOLID_COLOR) {
            ps_color color = m_color;
            color.a *= m_opacity;
            ps_set_source_color(ctx, &color);
        } else {
            setup_gradient(ctx, obj, true);
        }
    }

    void set_stroke_attrs(ps_context* ctx, const render_obj_base* obj)
    {
        if (m_tag == SVG_TAG_SOLID_COLOR) {
            ps_color color = m_color;
            color.a *= m_opacity;
            ps_set_stroke_color(ctx, &color);
        } else {
            setup_gradient(ctx, obj, false);
        }
    }

    void set_attr(const psx_svg_attr* attr, _svg_list_builder_state* state)
    {
        render_obj_base::set_attr(attr, state);

        switch (attr->attr_id) {
            case SVG_ATTR_SOLID_COLOR:
                svg_to_pcolor(&m_color, attr->value.uval);
                break;
            case SVG_ATTR_SOLID_OPACITY:
                m_opacity = attr->value.fval;
                break;
            case SVG_ATTR_CX:
                m_cx = attr->value.fval;
                break;
            case SVG_ATTR_CY:
                m_cy = attr->value.fval;
                break;
            case SVG_ATTR_R:
                m_cr = attr->value.fval;
                break;
            case SVG_ATTR_X1:
                m_x1 = attr->value.fval;
                break;
            case SVG_ATTR_Y1:
                m_y1 = attr->value.fval;
                break;
            case SVG_ATTR_X2:
                m_x2 = attr->value.fval;
                break;
            case SVG_ATTR_Y2:
                m_y2 = attr->value.fval;
                break;
            case SVG_ATTR_GRADIENT_UNITS:
                m_units = attr->value.ival;
                break;
        }
    }

private:
    psx_svg_tag m_tag;
    ps_color m_color;
    float m_opacity;
    stop_color* m_stops;
    uint32_t m_stop_count;
    int32_t m_units;
    float m_cx;
    float m_cy;
    float m_cr;
    float m_x1;
    float m_y1;
    float m_x2;
    float m_y2;
};

static INLINE uint32_t utf8_text_encode_next(const char* str, uint32_t len, uint32_t* i)
{
    uint32_t result = 0;
    if (str[*i] == '\0' || (*i) >= len) {
        return result;
    }

    uint8_t ch = str[*i];
    if (ch < 0x80) { // 1 bytes code
        result = ch;
        (*i)++;
    } else if ((ch & 0xE0) == 0xC0) { // 2 bytes code
        result = ((ch & 0x1F) << 6) | (str[(*i) + 1] & 0x3F);
        (*i) += 2;
    } else if ((ch & 0xF0) == 0xE0) { // 3 bytes code
        result = ((ch & 0x0F) << 12) | ((str[(*i) + 1] & 0x3F) << 6) | (str[(*i) + 2] & 0x3F);
        (*i) += 3;
    } else if ((ch & 0xF8) == 0xF0) { // 4 bytes code
        result = ((ch & 0x07) << 18) | ((str[(*i) + 1] & 0x3F) << 12)
                 | ((str[(*i) + 2] & 0x3F) << 6) | (str[(*i) + 3] & 0x3F);
        (*i) += 4;
    } else {
        (*i)++;
    }
    return result;
}

static INLINE void init_contents(svg_render_content* content, const psx_svg_node* node, _svg_list_builder_state* state)
{
    uint32_t len;
    const char* str = node->content(&len);
    if (str) {
        uint32_t* letters = (uint32_t*)state->list->alloc(len * sizeof(uint32_t));
        uint32_t offset = 0;
        for (uint32_t i = 0; i < len; i++) {
            letters[i] = utf8_text_encode_next(str, len, &offset);
        }
        content->set_contents(letters, len);
    }
}

static INLINE void set_render_attrs(render_obj_base* obj, const psx_svg_node* node, _svg_list_builder_state* state)
{
    if ((node->type() != SVG_TAG_CONTENT) && node->has_content()) {
        uint32_t len;
        const char* content = node->content(&len);
        char* id = (char*)state->list->alloc(len + 1);
        mem_copy(id, content, len);
        id[len] = '\0';
        obj->set_id(id);
    } else if ((node->type() == SVG_TAG_CONTENT) || (node->type() == SVG_TAG_TSPAN)) {
        svg_render_content* cobj = (svg_render_content*)obj;
        const psx_svg_node* content_node = node;
        if (node->type() == SVG_TAG_TSPAN) {
            content_node = node->get_child(0);
        }
        init_contents(cobj, content_node, state);
    }

    uint32_t len = node->attr_count();
    for (uint32_t i = 0; i < len; i++) {
        psx_svg_attr* attr = node->attr_at(i);
        obj->set_attr(attr, state);
    }
    obj->set_draw_attrs(state->draw_attrs);
}

static INLINE render_obj_base* render_obj_create(const psx_svg_node* node, _svg_list_builder_state* state)
{
    svg_render_list_impl* list = state->list;
    switch (node->type()) {
        case SVG_TAG_SVG: {
                svg_render_viewport* view = list->new_obj<svg_render_viewport>(node);
                set_render_attrs(view, node, state);
                return view;
            }
        case SVG_TAG_RECT: {
                svg_render_rect* rect = list->new_obj<svg_render_rect>(node);
                set_render_attrs(rect, node, state);
                return rect;
            }
        case SVG_TAG_CIRCLE: {
                svg_render_circle* circle = list->new_obj<svg_render_circle>(node);
                set_render_attrs(circle, node, state);
                return circle;
            }
        case SVG_TAG_ELLIPSE: {
                svg_render_ellipse* ellipse = list->new_obj<svg_render_ellipse>(node);
                set_render_attrs(ellipse, node, state);
                return ellipse;
            }
        case SVG_TAG_LINE: {
                svg_render_line* line = list->new_obj<svg_render_line>(node);
                set_render_attrs(line, node, state);
                return line;
            }
        case SVG_TAG_POLYLINE: {
                svg_render_polyline* poly = list->new_obj<svg_render_polyline>(node);
                set_render_attrs(poly, node, state);
                return poly;
            }
        case SVG_TAG_POLYGON: {
                svg_render_polygon* poly = list->new_obj<svg_render_polygon>(node);
                set_render_attrs(poly, node, state);
                return poly;
            }
        case SVG_TAG_PATH: {
                svg_render_path* path = list->new_obj<svg_render_path>(node);
                set_render_attrs(path, node, state);
                return path;
            }
        case SVG_TAG_TEXT: {
                svg_render_text* txt = list->new_obj<svg_render_text>(node);
                set_render_attrs(txt, node, state);
                return txt;
            }
        case SVG_TAG_CONTENT: {
                svg_render_content* content = list->new_obj<svg_render_content>(node);
                set_render_attrs(content, node, state);
                return content;
            }
        case SVG_TAG_TSPAN: {
                svg_render_tspan* span = list->new_obj<svg_render_tspan>(node);
                set_render_attrs(span, node, state);
                return span;
            }
        case SVG_TAG_USE: {
                svg_render_use* use = list->new_obj<svg_render_use>(node);
                set_render_attrs(use, node, state);
                return use;
            }
        case SVG_TAG_G: {
                svg_render_group* group = list->new_obj<svg_render_group>(node);
                set_render_attrs(group, node, state);
                return group;
            }
        case SVG_TAG_IMAGE: {
                svg_render_image* image = list->new_obj<svg_render_image>(node);
                set_render_attrs(image, node, state);
                return image;
            }
        case SVG_TAG_SOLID_COLOR:
        case SVG_TAG_RADIAL_GRADIENT:
        case SVG_TAG_LINEAR_GRADIENT: {
                svg_render_defs* defs = list->new_obj<svg_render_defs>(node);
                set_render_attrs(defs, node, state);
                return defs;
            }
        default:
            return NULL;
    }
}

static bool svg_doc_walk(const psx_tree_node* node, void* data)
{
    _svg_list_builder_state* state = (_svg_list_builder_state*)data;
    psx_svg_node* svg_node = (psx_svg_node*)node;

    render_obj_base* obj = render_obj_create(svg_node, state);
    if (!obj) {
        return true;
    }

    if (state->in_defs) {
        obj->set_render_type(RENDER_IN_DEFS);
    }

    if (state->in_group_deps > 0) {
        obj->set_render_type(RENDER_IN_GROUP);
    }

    if (state->in_text) {
        obj->set_render_type(RENDER_IN_TEXT);
    }

    if (state->tail == NULL) {
        state->tail = obj;
        state->list->set_head(obj);
    } else {
        state->tail->set_next(obj);
        state->tail = obj;
    }

    obj->set_head(state->list->head());
    obj->set_global_matrix(state->global_matrix);
    svg_node->set_render(obj);
    return true;
}

static bool svg_doc_walk_before(const psx_tree_node* node, void* data)
{
    _svg_list_builder_state* state = (_svg_list_builder_state*)data;
    psx_svg_node* svg_node = (psx_svg_node*)node;

    if (svg_node->type() == SVG_TAG_TEXT) {
        state->cur_text = svg_node;
        state->in_text = true;
    }

    if (svg_node->type() == SVG_TAG_DEFS) {
        state->in_defs = true;
    }

    if (svg_node->type() == SVG_TAG_G) {
        state->in_group_deps++;
    }
    return true;
}

static bool svg_doc_walk_after(const psx_tree_node* node, void* data)
{
    _svg_list_builder_state* state = (_svg_list_builder_state*)data;
    psx_svg_node* svg_node = (psx_svg_node*)node;

    if (state->in_text) {
        if (svg_node->type() == SVG_TAG_TSPAN || svg_node->type() == SVG_TAG_CONTENT) {
            if (svg_node->parent() == state->cur_text) {
                svg_render_text* text = (svg_render_text*)state->cur_text->render();
                text->add_content((svg_render_content*)svg_node->render());
            }
        }
    }

    if (svg_node->type() == SVG_TAG_TEXT) {
        svg_render_text* text = (svg_render_text*)svg_node->render();
        text->set_render_type(RENDER_NORMAL);
        state->cur_text = NULL;
        state->in_text = false;
    }

    if (svg_node->type() == SVG_TAG_G) {
        svg_render_group* group = (svg_render_group*)svg_node->render();
        uint32_t count = svg_node->child_count();
        for (uint32_t i = 0; i < count; i++) {
            psx_svg_node* child = svg_node->get_child(i);
            if (child->render()) { // not defs
                group->add_item((render_obj_base*)child->render());
            }
        }

        state->in_group_deps--;
        if (state->in_group_deps == 0) {
            group->set_render_type(RENDER_NORMAL);
        }
    }

    if (svg_node->type() == SVG_TAG_DEFS) {
        state->in_defs = false;
    }

    psx_svg_node* parent = (psx_svg_node*)node->parent();
    if (parent && parent->render()) {
        render_obj_base* obj = (render_obj_base*)parent->render();
        state->draw_attrs = obj->get_draw_attrs();
    }
    return true;
}

#ifdef __cplusplus
extern "C" {
#endif

psx_svg_render_list* psx_svg_render_list_create(const psx_svg_node* doc)
{
    if (!doc) {
        LOG_ERROR("Invalid parameters!\n");
        return NULL;
    }

    svg_render_list_impl* list = new svg_render_list_impl();
    if (!list) {
        LOG_ERROR("Not enough memory for create render list!\n");
        return NULL;
    }

    ps_draw_attrs* attrs = draw_attrs_create(list);
    if (!attrs) {
        LOG_ERROR("Not enough memory for create render attributes!\n");
        return NULL;
    }

    _svg_list_builder_state state;
    state.doc = doc;
    state.list = list;
    state.draw_attrs = attrs;
    state.global_matrix = NULL;
    state.tail = NULL;
    state.in_group_deps = 0;
    state.in_defs = false;
    state.cur_text = NULL;
    state.in_text = false;

    if (!psx_tree_traversal<PSX_TREE_WALK_PRE_ORDER>(doc, &state, svg_doc_walk, svg_doc_walk_before, svg_doc_walk_after)) {
        delete list;
        LOG_ERROR("Parser SVG dom fail!\n");
        return NULL;
    }
    return list;
}

void psx_svg_render_list_destroy(psx_svg_render_list* list)
{
    if (list) {
        svg_render_list_impl* svg_list = (svg_render_list_impl*)list;
        const render_obj_base* head = svg_list->head();
        while (head) {
            render_obj_base* obj = (render_obj_base*)head;
            head = head->next();
            svg_list->delete_obj(obj);
        }
        delete list;
    }
}

bool psx_svg_render_list_draw(ps_context* ctx, const psx_svg_render_list* render)
{
    if (!ctx || !render) {
        LOG_ERROR("Invalid arguements for svg render!\n");
        return false;
    }

    render_obj_base* head = ((svg_render_list_impl*)render)->head();
    while (head) {
        if (head->render_type() == RENDER_NORMAL) {
            head->render(ctx, NULL);
        }
        head = head->next();
    }
    return true;
}

#ifdef __cplusplus
}
#endif
