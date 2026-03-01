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

#include "psx_svg_player.h"

#include "psx_svg.h"
#include "psx_svg_parser.h"

#include <string.h>
#include <math.h>

// Opaque animation override state passed to renderer.
struct psx_svg_anim_state {
    psx_array overrides;
    psx_array transforms;
};

struct psx_svg_player {
    psx_svg_node* root;
    psx_svg_render_list* render_list;

    psx_svg_anim_state anim_state;

    psx_svg_player_state state;
    ps_bool loop;
    ps_bool own_root;

    float time_sec;
    float duration_sec; // -1 for indefinite/unknown

    int32_t dpi;

    psx_svg_anim_event_cb cb;
    void* cb_user;
    psx_array anims;
};

typedef struct {
    psx_svg_tag tag;
    const psx_svg_node* anim_node;
    const psx_svg_node* target_node;
    psx_svg_attr_type target_attr;
    float begin_sec; // kept for duration hint compatibility
    float dur_sec;
    float end_sec; // optional explicit end, 0 => unspecified
    uint32_t repeat_count; // 0 => indefinite
    float repeat_dur_sec; // optional explicit repeat duration, 0 => unspecified
    uint32_t fill_mode; // SVG_ANIMATION_*

    // Minimal Tiny 1.2 external event trigger support.
    // If begin is specified as a non-numeric token (e.g. begin="click"), we
    // store the trigger name and allow external callers to start the animation
    // via psx_svg_player_trigger(). Owned by the player.
    const char* begin_event;

    // Begin list support: store begin times (sec) and choose the latest begin <= doc_t.
    psx_array begins_sec;

    // End list support: store end times (sec) and choose the earliest end >= begin (per trigger).
    psx_array ends_sec;
} psx_svg_anim_item;

typedef struct {
    const psx_svg_node* target;
    psx_svg_attr_type attr;
    float fval;
} psx_svg_anim_override_item;

typedef struct {
    const psx_svg_node* target;
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
} psx_svg_anim_transform_item;

static INLINE void _anim_state_reset(psx_svg_anim_state* s)
{
    if (!s) {
        return;
    }
    psx_array_clear(&s->overrides);
    psx_array_clear(&s->transforms);
}

static INLINE void _anim_state_set_transform(psx_svg_anim_state* s, const psx_svg_node* target,
                                             float a, float b, float c, float d, float e, float f)
{
    if (!s || !target) {
        return;
    }

    uint32_t n = psx_array_size(&s->transforms);
    for (uint32_t i = 0; i < n; i++) {
        psx_svg_anim_transform_item* it = psx_array_get(&s->transforms, i, psx_svg_anim_transform_item);
        if (it && it->target == target) {
            it->a = a;
            it->b = b;
            it->c = c;
            it->d = d;
            it->e = e;
            it->f = f;
            return;
        }
    }

    psx_array_append(&s->transforms, NULL);
    psx_svg_anim_transform_item* dst = psx_array_get_last(&s->transforms, psx_svg_anim_transform_item);
    dst->target = target;
    dst->a = a;
    dst->b = b;
    dst->c = c;
    dst->d = d;
    dst->e = e;
    dst->f = f;
}

static INLINE const psx_svg_anim_transform_item* _anim_state_find_transform(const psx_svg_anim_state* s, const psx_svg_node* target)
{
    if (!s || !target) {
        return NULL;
    }
    uint32_t n = psx_array_size((psx_array*)&s->transforms);
    for (uint32_t i = 0; i < n; i++) {
        const psx_svg_anim_transform_item* it = psx_array_get((psx_array*)&s->transforms, i, psx_svg_anim_transform_item);
        if (it && it->target == target) {
            return it;
        }
    }
    return NULL;
}

static INLINE void _anim_item_end_list_init(psx_svg_anim_item* it)
{
    if (!it) {
        return;
    }
    psx_array_init_type(&it->ends_sec, float);
}

static INLINE void _anim_item_end_list_destroy(psx_svg_anim_item* it)
{
    if (!it) {
        return;
    }
    psx_array_destroy(&it->ends_sec);
}

static INLINE void _anim_item_end_list_normalize(psx_svg_anim_item* it)
{
    if (!it) {
        return;
    }
    uint32_t n = psx_array_size(&it->ends_sec);
    if (n <= 1) {
        return;
    }
    // Sort ascending for stable selection logic.
    // Comparator is defined later in this file; use a local one to avoid
    // forward-decl complexity.
    struct _cmp {
        static int asc(const void* a, const void* b)
        {
            const float fa = *(const float*)a;
            const float fb = *(const float*)b;
            if (fa < fb) {
                return -1;
            }
            if (fa > fb) {
                return 1;
            }
            return 0;
        }
    };
    qsort(it->ends_sec.data, n, sizeof(float), _cmp::asc);

    // in-place dedupe exact duplicates
    float* d = (float*)it->ends_sec.data;
    uint32_t w = 1;
    for (uint32_t r = 1; r < n; r++) {
        if (d[r] != d[w - 1]) {
            d[w++] = d[r];
        }
    }
    it->ends_sec.size = w;
}

static INLINE float _anim_item_end_for_begin(const psx_svg_anim_item* it, float begin_sec)
{
    if (!it) {
        return 0.0f;
    }
    uint32_t n = psx_array_size((psx_array*)&it->ends_sec);
    if (n == 0) {
        return 0.0f;
    }
    // ends_sec is sorted ascending
    for (uint32_t i = 0; i < n; i++) {
        const float* ep = psx_array_get((psx_array*)&it->ends_sec, i, float);
        float e = ep ? *ep : 0.0f;
        if (e >= begin_sec) {
            return e;
        }
    }
    return 0.0f;
}

static INLINE void _anim_state_set_float(psx_svg_anim_state* s, const psx_svg_node* target, psx_svg_attr_type attr, float v)
{
    if (!s || !target || attr == SVG_ATTR_INVALID) {
        return;
    }
    uint32_t n = psx_array_size(&s->overrides);
    for (uint32_t i = 0; i < n; i++) {
        psx_svg_anim_override_item* it = psx_array_get(&s->overrides, i, psx_svg_anim_override_item);
        if (it->target == target && it->attr == attr) {
            it->fval = v;
            return;
        }
    }
    psx_array_append(&s->overrides, NULL);
    psx_svg_anim_override_item* dst = psx_array_get_last(&s->overrides, psx_svg_anim_override_item);
    dst->target = target;
    dst->attr = attr;
    dst->fval = v;
}

static INLINE const psx_svg_anim_override_item* _anim_state_find(const psx_svg_anim_state* s, const psx_svg_node* target, psx_svg_attr_type attr)
{
    if (!s || !target || attr == SVG_ATTR_INVALID) {
        return NULL;
    }
    uint32_t n = psx_array_size((psx_array*)&s->overrides);
    for (uint32_t i = 0; i < n; i++) {
        const psx_svg_anim_override_item* it = psx_array_get((psx_array*)&s->overrides, i, psx_svg_anim_override_item);
        if (it->target == target && it->attr == attr) {
            return it;
        }
    }
    return NULL;
}

static INLINE float _anim_lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

static INLINE ps_bool _anim_values_list_get_transform(const psx_svg_attr_values_list* list, uint32_t idx,
                                                      const float** out_vals, uint32_t* out_len)
{
    if (!out_vals || !out_len) {
        return False;
    }
    *out_vals = NULL;
    *out_len = 0;

    if (!list || idx >= list->length) {
        return False;
    }

    // Must match psx_svg_parser.cpp::_transform_values_list
    struct _anim_transform_values_list {
        uint32_t length;
        float data[4];
    };

    const struct _anim_transform_values_list* base =
        (const struct _anim_transform_values_list*)(&list->data[0]);
    const struct _anim_transform_values_list* it = base + idx;
    *out_vals = &it->data[0];
    *out_len = it->length;
    return True;
}

static INLINE float _anim_clampf(float v, float lo, float hi)
{
    if (v < lo) {
        return lo;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

static INLINE float _anim_fmod(float x, float y)
{
    if (y <= 0.0f) {
        return 0.0f;
    }
    // fmodf may not be available in all C++98 libm combinations.
    return (float)fmod((double)x, (double)y);
}

static INLINE float _u32_to_f32_bits(uint32_t u)
{
    union {
        uint32_t u;
        float f;
    } v;
    v.u = u;
    return v.f;
}

static INLINE uint32_t _f32_to_u32_bits(float f)
{
    union {
        uint32_t u;
        float f;
    } v;
    v.f = f;
    return v.u;
}

// Cubic-bezier helpers for calcMode="spline".
// Control points are (x1,y1,x2,y2) with implicit endpoints (0,0) and (1,1).
static INLINE float _anim_cubic_bezier_sample(float a1, float a2, float u)
{
    float inv = 1.0f - u;
    return 3.0f * inv * inv * u * a1 + 3.0f * inv * u * u * a2 + u * u * u;
}

static INLINE float _anim_cubic_bezier_sample_derivative(float a1, float a2, float u)
{
    float inv = 1.0f - u;
    return 3.0f * inv * inv * a1 + 6.0f * inv * u * (a2 - a1) + 3.0f * u * u * (1.0f - a2);
}

static INLINE float _anim_cubic_bezier_y_for_x(float x1, float y1, float x2, float y2, float x)
{
    x = _anim_clampf(x, 0.0f, 1.0f);

    // Solve _anim_cubic_bezier_sample(x1,x2,u) == x for u.
    float u = x;
    float lo = 0.0f;
    float hi = 1.0f;
    for (int i = 0; i < 8; i++) {
        float xu = _anim_cubic_bezier_sample(x1, x2, u);
        float dx = xu - x;
        if (dx < 0.0f) {
            dx = -dx;
        }
        if (dx < 1e-6f) {
            break;
        }

        float d = _anim_cubic_bezier_sample_derivative(x1, x2, u);
        if (d > 1e-6f) {
            // Use signed delta for Newton step.
            float nu = u - (xu - x) / d;
            if (nu >= lo && nu <= hi) {
                u = nu;
            } else {
                if (xu > x) {
                    hi = u;
                } else {
                    lo = u;
                }
                u = 0.5f * (lo + hi);
            }
        } else {
            if (xu > x) {
                hi = u;
            } else {
                lo = u;
            }
            u = 0.5f * (lo + hi);
        }
    }

    float y = _anim_cubic_bezier_sample(y1, y2, u);
    return _anim_clampf(y, 0.0f, 1.0f);
}

static INLINE ps_bool _is_anim_color_value(const psx_svg_attr* a)
{
    if (!a) {
        return False;
    }
    // Heuristic: animateColor parser stores from/to/by as DATA(uval) and values
    // as PTR(list of uint32). Numeric animate uses DATA(fval) / PTR(list of float).
    if (a->val_type == SVG_ATTR_VALUE_DATA) {
        return True;
    }
    if (a->val_type == SVG_ATTR_VALUE_PTR && a->value.val) {
        return True;
    }
    return False;
}

static INLINE float _attr_as_number(const psx_svg_attr* a)
{
    if (!a) {
        return 0.0f;
    }
    if (a->val_type == SVG_ATTR_VALUE_DATA) {
        // Parser stores different kinds of data in union:
        // - clock times in value.fval (ms)
        // - attributeName/fill/etc in value.ival
        // For numeric animation values (from/to/by), prefer fval.
        return a->value.fval;
    }
    if (a->val_type == SVG_ATTR_VALUE_PTR && a->value.val) {
        const psx_svg_attr_values_list* list = (const psx_svg_attr_values_list*)a->value.val;
        if (list->length > 0) {
            const float* vals = (const float*)&list->data[0];
            return vals[0];
        }
        // Fallback: some animation attributes may be stored as a string pointer.
        const char* s = (const char*)a->value.val;
        const char c0 = s[0];
        if ((c0 >= '0' && c0 <= '9') || c0 == '-' || c0 == '+' || c0 == '.') {
            return (float)atof(s);
        }
    }
    return a->value.fval;
}

static psx_svg_player_options _default_options(void)
{
    psx_svg_player_options opt;
    opt.take_ownership_of_root = False;
    opt.loop = False;
    opt.dpi = 96;
    return opt;
}

static INLINE const psx_svg_attr* _find_attr(const psx_svg_node* node, psx_svg_attr_type type)
{
    if (!node) {
        return NULL;
    }
    uint32_t n = node->attr_count();
    for (uint32_t i = 0; i < n; i++) {
        const psx_svg_attr* a = node->attr_at(i);
        if (a && (psx_svg_attr_type)a->attr_id == type) {
            return a;
        }
    }
    return NULL;
}

static INLINE float _attr_as_time_sec(const psx_svg_attr* a)
{
    if (!a) {
        return 0.0f;
    }
    // Clock time attributes (begin/dur/min/max/repeatDur) are stored as DATA fval (ms)
    // in parser (_process_clock_time). Convert to seconds here.
    if (a->val_type == SVG_ATTR_VALUE_DATA) {
        if (a->value.fval <= 0.0f) {
            return 0.0f;
        }
        return a->value.fval * 0.001f;
    }

    // Fallback for any legacy representation.
    if (a->val_type == SVG_ATTR_VALUE_PTR && a->value.val) {
        // Legacy representation: list of clock values (seconds).
        const psx_svg_attr_values_list* list = (const psx_svg_attr_values_list*)a->value.val;
        if (list->length > 0) {
            const float* vals = (const float*)&list->data[0];
            return vals[0];
        }
    }
    return a->value.fval;
}

static INLINE float _attr_as_begin_time_sec(const psx_svg_attr* a)
{
    if (!a) {
        return 0.0f;
    }
    // If begin is stored as a list (VALUE_PTR), prefer the earliest begin time.
    // Note: for non-<set>, begin/end are parsed into a list of ms (see _animation_begin_end_cb).
    if (a->val_type == SVG_ATTR_VALUE_PTR && a->value.val) {
        const psx_svg_attr_values_list* list = (const psx_svg_attr_values_list*)a->value.val;
        if (list->length > 0) {
            const float* vals = (const float*)&list->data[0];
            float minv = vals[0];
            for (uint32_t i = 1; i < list->length; i++) {
                if (vals[i] < minv) {
                    minv = vals[i];
                }
            }
            // values are ms
            return (minv <= 0.0f) ? 0.0f : (minv * 0.001f);
        }
        return 0.0f;
    }
    return _attr_as_time_sec(a);
}

static INLINE void _anim_item_begin_list_init(psx_svg_anim_item* it)
{
    if (!it) {
        return;
    }
    psx_array_init_type(&it->begins_sec, float);
}

static INLINE void _anim_item_begin_list_destroy(psx_svg_anim_item* it)
{
    if (!it) {
        return;
    }
    psx_array_destroy(&it->begins_sec);

    if (it->begin_event) {
        mem_free((void*)it->begin_event);
        it->begin_event = NULL;
    }
}

static INLINE const char* _dup_cstr(const char* s)
{
    if (!s) {
        return NULL;
    }
    size_t n = strlen(s);
    char* d = (char*)mem_malloc((uint32_t)n + 1);
    if (!d) {
        return NULL;
    }
    mem_copy(d, s, (uint32_t)n);
    d[n] = '\0';
    return d;
}

static INLINE ps_bool _is_clock_value_token(const char* s)
{
    if (!s) {
        return False;
    }
    while (*s && isspace(*s)) {
        s++;
    }
    if (!*s) {
        return False;
    }
    // Clock-values start with digit, sign or '.'
    return (strchr("0123456789+-.", *s) != NULL) ? True : False;
}

static INLINE const char* _find_attr_string_raw(const psx_svg_node* node, psx_svg_attr_type type)
{
    if (!node) {
        return NULL;
    }
    uint32_t n = node->attr_count();
    for (uint32_t i = 0; i < n; i++) {
        const psx_svg_attr* a = node->attr_at(i);
        if (!a) {
            continue;
        }
        if ((psx_svg_attr_type)a->attr_id != type) {
            continue;
        }
        // begin/end are stored as timing list; no raw string is available here.
        return NULL;
    }
    return NULL;
}

static INLINE float _anim_item_begin_for_time(const psx_svg_anim_item* it, float doc_t)
{
    if (!it) {
        return 0.0f;
    }
    // Event-triggered animations: no instance times => not active.
    if (it->begin_event && psx_array_empty((psx_array*)&it->begins_sec)) {
        (void)doc_t;
        return 1e30f;
    }
    if (psx_array_empty((psx_array*)&it->begins_sec)) {
        return it->begin_sec;
    }
    // choose latest begin <= doc_t
    float best = -1.0f;
    const uint32_t n = psx_array_size((psx_array*)&it->begins_sec);
    for (uint32_t i = 0; i < n; i++) {
        const float* bp = psx_array_get((psx_array*)&it->begins_sec, i, float);
        float b = bp ? *bp : 0.0f;
        if (b <= doc_t && b > best) {
            best = b;
        }
    }
    if (best < 0.0f) {
        // all begins are in the future: keep earliest (for consistency)
        const float* ep0 = psx_array_get((psx_array*)&it->begins_sec, 0, float);
        float earliest = ep0 ? *ep0 : 0.0f;
        for (uint32_t i = 1; i < n; i++) {
            const float* ep = psx_array_get((psx_array*)&it->begins_sec, i, float);
            float e = ep ? *ep : 0.0f;
            if (e < earliest) {
                earliest = e;
            }
        }
        return earliest;
    }
    return best;
}

static INLINE int _float_cmp_asc(const void* a, const void* b)
{
    const float fa = *(const float*)a;
    const float fb = *(const float*)b;
    if (fa < fb) {
        return -1;
    }
    if (fa > fb) {
        return 1;
    }
    return 0;
}

static INLINE void _anim_item_begin_list_normalize(psx_svg_anim_item* it)
{
    if (!it) {
        return;
    }
    uint32_t n = psx_array_size(&it->begins_sec);
    if (n <= 1) {
        return;
    }

    // Sort ascending for stable selection logic and future extensions.
    qsort(it->begins_sec.data, n, sizeof(float), _float_cmp_asc);

    // Dedupe exact-equal values in-place.
    float* v = (float*)it->begins_sec.data;
    uint32_t w = 1;
    for (uint32_t i = 1; i < n; i++) {
        if (v[i] != v[w - 1]) {
            v[w++] = v[i];
        }
    }
    it->begins_sec.size = w;
}

typedef struct {
    float begin_sec;
    ps_bool valid;
} psx_svg_begin_list;

static INLINE void _begin_list_init(psx_svg_begin_list* bl)
{
    if (!bl) {
        return;
    }
    bl->begin_sec = 0.0f;
    bl->valid = False;
}

static INLINE void _begin_list_from_attr(psx_svg_begin_list* bl, const psx_svg_attr* a)
{
    _begin_list_init(bl);
    if (!bl) {
        return;
    }
    if (!a) {
        bl->valid = True;
        bl->begin_sec = 0.0f;
        return;
    }

    if (a->val_type == SVG_ATTR_VALUE_PTR && a->value.val) {
        // For non-<set>, begin can be a value list of clock-time values (ms).
        const psx_svg_attr_values_list* list = (const psx_svg_attr_values_list*)a->value.val;
        if (list->length > 0) {
            const float* vals = (const float*)&list->data[0];
            // pick earliest
            float minv = vals[0];
            for (uint32_t i = 1; i < list->length; i++) {
                if (vals[i] < minv) {
                    minv = vals[i];
                }
            }
            bl->begin_sec = (minv <= 0.0f) ? 0.0f : (minv * 0.001f);
            bl->valid = True;
            return;
        }
    }

    bl->begin_sec = _attr_as_time_sec(a);
    bl->valid = True;
}

static INLINE float _begin_list_current_begin(const psx_svg_begin_list* bl, float doc_t)
{
    // Legacy helper retained for older code paths.
    // Current begin-list semantics are implemented in psx_svg_anim_item
    // via _anim_item_begin_for_time().
    (void)doc_t;
    if (!bl || !bl->valid) {
        return 0.0f;
    }
    return bl->begin_sec;
}

static INLINE float _attr_as_float(const psx_svg_attr* a)
{
    return a ? a->value.fval : 0.0f;
}

static INLINE uint32_t _attr_as_u32(const psx_svg_attr* a, uint32_t defv)
{
    return a ? a->value.uval : defv;
}

static INLINE ps_bool _anim_eval_simple(const psx_svg_anim_item* it, float doc_t, float* out_v, ps_bool* out_hold)
{
    if (!it || !out_v || !out_hold) {
        return False;
    }

    *out_hold = False;

    if (it->dur_sec <= 0.0f) {
        return False;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return False;
    }

    float local = doc_t - begin_sec;

    // Compute total active duration from repeatCount/repeatDur.
    float total = 0.0f;
    ps_bool has_total = False;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = True;
    } else if (it->repeat_count == 0) {
        // indefinite
        has_total = False;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = True;
    }

    if (!has_total) {
        // indefinite
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = True;
                local = it->dur_sec;
            } else {
                return False;
            }
        } else {
            // within total repeats
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    float t = _anim_clampf(local / it->dur_sec, 0.0f, 1.0f);

    // (calcMode="spline") cubic-bezier helper: map x in [0,1] to y.
    // Defined as static functions (no C++ lambda) for broader compiler compatibility.

    // Step-2 (Tiny 1.2 subset): values + optional keyTimes (linear).
    // If values is present, it overrides from/to interpolation.
    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length >= 1) {
            const float* vals = (const float*)&vlist->data[0];
            if (vlist->length == 1) {
                *out_v = vals[0];
                return True;
            }

            // calcMode handling (Tiny 1.2 subset): linear(default) / discrete.
            uint32_t calc_mode = SVG_ANIMATION_CALC_MODE_LINEAR;
            const psx_svg_attr* acm = _find_attr(it->anim_node, SVG_ATTR_CALC_MODE);
            if (acm && acm->val_type == SVG_ATTR_VALUE_DATA) {
                calc_mode = (uint32_t)acm->value.ival;
            }

            // Determine segment via keyTimes if provided.
            const psx_svg_attr* akt = _find_attr(it->anim_node, SVG_ATTR_KEY_TIMES);
            const float* kts = NULL;
            uint32_t kt_len = 0;
            if (akt && akt->val_type == SVG_ATTR_VALUE_PTR && akt->value.val) {
                const psx_svg_attr_values_list* ktlist = (const psx_svg_attr_values_list*)akt->value.val;
                kt_len = ktlist->length;
                if (kt_len >= 2) {
                    kts = (const float*)&ktlist->data[0];
                }
            }

            uint32_t seg = 0;
            float seg_t0 = 0.0f;
            float seg_t1 = 1.0f;
            if (kts && kt_len == vlist->length) {
                // Find i s.t. t in [kts[i], kts[i+1]]. Clamp to last segment.
                for (uint32_t i = 0; i + 1 < kt_len; i++) {
                    float a = _anim_clampf(kts[i], 0.0f, 1.0f);
                    float b = _anim_clampf(kts[i + 1], 0.0f, 1.0f);
                    if (t >= a && (t <= b || i + 2 == kt_len)) {
                        seg = i;
                        seg_t0 = a;
                        seg_t1 = b;
                        break;
                    }
                }
                if (seg_t1 <= seg_t0) {
                    seg_t0 = 0.0f;
                    seg_t1 = 1.0f;
                }
            } else if (calc_mode == SVG_ANIMATION_CALC_MODE_PACED) {
                // Paced: allocate time proportionally to segment length in value space.
                // Only for numeric values. If all deltas are zero, fall back to equal spacing.
                float total_len = 0.0f;
                for (uint32_t i = 0; i + 1 < vlist->length; i++) {
                    float d = vals[i + 1] - vals[i];
                    if (d < 0.0f) {
                        d = -d;
                    }
                    total_len += d;
                }

                if (total_len <= 0.0f) {
                    // Degenerate: all values equal.
                    seg = 0;
                    seg_t0 = 0.0f;
                    seg_t1 = 1.0f;
                } else {
                    // Clamp t to avoid float error causing "no segment selected" at the end.
                    float tt = _anim_clampf(t, 0.0f, 1.0f);
                    float acc = 0.0f;
                    for (uint32_t i = 0; i + 1 < vlist->length; i++) {
                        float d = vals[i + 1] - vals[i];
                        if (d < 0.0f) {
                            d = -d;
                        }
                        float w = d / total_len;
                        float a = acc;
                        float b = acc + w;
                        // Use half-open intervals [a,b) except for the last segment [a,1].
                        if ((tt >= a && tt < b) || (i + 2 == vlist->length && tt >= a && tt <= 1.0f)) {
                            seg = i;
                            seg_t0 = a;
                            seg_t1 = b;
                            break;
                        }
                        acc = b;
                    }
                    if (seg_t1 <= seg_t0) {
                        seg = vlist->length - 2;
                        seg_t0 = 0.0f;
                        seg_t1 = 1.0f;
                    }
                }
            } else {
                // Evenly spaced segments.
                float step = 1.0f / (float)(vlist->length - 1);
                seg = (uint32_t)(t / step);
                if (seg >= vlist->length - 1) {
                    seg = vlist->length - 2;
                }
                seg_t0 = step * (float)seg;
                seg_t1 = step * (float)(seg + 1);
            }

            float u = 0.0f;
            if (seg_t1 > seg_t0) {
                u = (t - seg_t0) / (seg_t1 - seg_t0);
            }
            u = _anim_clampf(u, 0.0f, 1.0f);

            if (calc_mode == SVG_ANIMATION_CALC_MODE_DISCRETE) {
                // Discrete: hold the segment's start value.
                *out_v = vals[seg];
            } else {
                float eased_u = u;
                if (calc_mode == SVG_ANIMATION_CALC_MODE_SPLINE) {
                    // keySplines provides one cubic-bezier per segment.
                    const psx_svg_attr* aks = _find_attr(it->anim_node, SVG_ATTR_KEY_SPLINES);
                    if (aks && aks->val_type == SVG_ATTR_VALUE_PTR && aks->value.val) {
                        const psx_svg_attr_values_list* kslist = (const psx_svg_attr_values_list*)aks->value.val;
                        // Parser stores keySplines as a list of psx_svg_point, 2 points per segment.
                        uint32_t need = (vlist->length - 1) * 2;
                        if (kslist->length == need && seg < (vlist->length - 1)) {
                            const psx_svg_point* pts = (const psx_svg_point*)&kslist->data[0];
                            uint32_t base = seg * 2;
                            float x1 = _anim_clampf(pts[base + 0].x, 0.0f, 1.0f);
                            float y1 = _anim_clampf(pts[base + 0].y, 0.0f, 1.0f);
                            float x2 = _anim_clampf(pts[base + 1].x, 0.0f, 1.0f);
                            float y2 = _anim_clampf(pts[base + 1].y, 0.0f, 1.0f);
                            eased_u = _anim_cubic_bezier_y_for_x(x1, y1, x2, y2, u);
                        }
                    }
                }
                *out_v = _anim_lerp(vals[seg], vals[seg + 1], eased_u);
            }
            return True;
        }
    }

    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return False;
    }

    // animateColor is stored as packed uint32 in attr union. For discrete mode
    // (and for now only discrete is enabled for colors), preserve exact bits.
    if (it->tag == SVG_TAG_ANIMATE_COLOR && it->target_attr == SVG_ATTR_FILL) {
        const psx_svg_attr* av = _find_attr(it->anim_node, SVG_ATTR_VALUES);
        if (av && av->val_type == SVG_ATTR_VALUE_PTR && av->value.val) {
            const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)av->value.val;
            const uint32_t* vals = (vlist->length > 0) ? (const uint32_t*)&vlist->data[0] : NULL;
            if (vals && vlist->length >= 1) {
                // Discrete selection uses time segments, pick index 0/last.
                uint32_t idx = 0;
                const psx_svg_attr* acm = _find_attr(it->anim_node, SVG_ATTR_CALC_MODE);
                ps_bool is_discrete = (acm && acm->val_type == SVG_ATTR_VALUE_DATA && acm->value.ival == SVG_ANIMATION_CALC_MODE_DISCRETE) ? True : False;
                if (!is_discrete) {
                    // For now, only discrete is supported for colors.
                    is_discrete = True;
                }
                if (is_discrete) {
                    if (t >= 1.0f) {
                        idx = vlist->length - 1;
                    } else {
                        // 2-value list: [0,0.5) => 0; [0.5,1) => 1
                        if (vlist->length >= 2 && t >= 0.5f) {
                            idx = 1;
                        }
                    }
                    *out_v = _u32_to_f32_bits(vals[idx]);
                    return True;
                }
            }
        }

        const psx_svg_attr* ato2 = _find_attr(it->anim_node, SVG_ATTR_TO);
        const psx_svg_attr* afr2 = _find_attr(it->anim_node, SVG_ATTR_FROM);
        uint32_t c0 = afr2 ? (uint32_t)afr2->value.uval : 0;
        uint32_t c1 = ato2 ? (uint32_t)ato2->value.uval : c0;
        uint32_t c = (t < 0.5f) ? c0 : c1;
        *out_v = _u32_to_f32_bits(c);
        return True;
    }

    float v0 = _attr_as_number(afrom);
    float v1 = _attr_as_number(ato);
    *out_v = _anim_lerp(v0, v1, t);
    return True;
}

static INLINE ps_bool _anim_eval_transform_translate_discrete(const psx_svg_anim_item* it, float doc_t,
                                                              float* out_a, float* out_b, float* out_c,
                                                              float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return False;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    if (it->dur_sec <= 0.0f) {
        return False;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return False;
    }

    float local = doc_t - begin_sec;

    // Compute total active duration from repeatCount/repeatDur.
    float total = 0.0f;
    ps_bool has_total = False;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = True;
    } else if (it->repeat_count == 0) {
        // indefinite
        has_total = False;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = True;
    }

    if (!has_total) {
        // indefinite
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                // Freeze at end of simple duration.
                local = it->dur_sec;
            } else {
                return False;
            }
        } else {
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    float t = _anim_clampf(local / it->dur_sec, 0.0f, 1.0f);

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (!avals || avals->val_type != SVG_ATTR_VALUE_PTR || !avals->value.val) {
        return False;
    }

    const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
    if (vlist->length < 1) {
        return False;
    }

    // Parser stores animateTransform values as `_transform_values_list` entries.
    // See psx_svg_parser.cpp::_transform_values_list.
    // For freeze beyond dur, clamp to last value.
    uint32_t idx = 0;
    if (t >= 1.0f) {
        idx = vlist->length - 1;
    } else if (vlist->length >= 2 && t >= 0.5f) {
        idx = 1;
    }

    const float* base = NULL;
    uint32_t vlen = 0;
    if (!_anim_values_list_get_transform(vlist, idx, &base, &vlen) || !base) {
        return False;
    }

    float tx = 0.0f;
    float ty = 0.0f;
    if (vlen >= 1) {
        tx = base[0];
    }
    if (vlen >= 2) {
        ty = base[1];
    }

    *out_e = tx;
    *out_f = ty;
    return True;
}

static INLINE ps_bool _anim_eval_transform_translate_linear(const psx_svg_anim_item* it, float doc_t,
                                                            float* out_a, float* out_b, float* out_c,
                                                            float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return False;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    if (it->dur_sec <= 0.0f) {
        return False;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return False;
    }

    float local = doc_t - begin_sec;

    // Compute total active duration from repeatCount/repeatDur.
    float total = 0.0f;
    ps_bool has_total = False;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = True;
    } else if (it->repeat_count == 0) {
        // indefinite
        has_total = False;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = True;
    }

    if (!has_total) {
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                local = it->dur_sec;
            } else {
                return False;
            }
        } else {
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    float t = _anim_clampf(local / it->dur_sec, 0.0f, 1.0f);

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length < 1) {
            return False;
        }
        if (vlist->length == 1) {
            const float* base0 = NULL;
            uint32_t vlen0 = 0;
            if (!_anim_values_list_get_transform(vlist, 0, &base0, &vlen0) || !base0) {
                return False;
            }
            *out_e = (vlen0 >= 1) ? base0[0] : 0.0f;
            *out_f = (vlen0 >= 2) ? base0[1] : 0.0f;
            return True;
        }

        // Segment mapping mirrors numeric values+keyTimes logic in _anim_eval_simple.
        const psx_svg_attr* akt = _find_attr(it->anim_node, SVG_ATTR_KEY_TIMES);
        const float* kts = NULL;
        uint32_t kt_len = 0;
        if (akt && akt->val_type == SVG_ATTR_VALUE_PTR && akt->value.val) {
            const psx_svg_attr_values_list* ktlist = (const psx_svg_attr_values_list*)akt->value.val;
            kt_len = ktlist->length;
            if (kt_len >= 2) {
                kts = (const float*)&ktlist->data[0];
            }
        }

        uint32_t seg = 0;
        float seg_t0 = 0.0f;
        float seg_t1 = 1.0f;
        if (kts && kt_len == vlist->length) {
            for (uint32_t i = 0; i + 1 < kt_len; i++) {
                float a = _anim_clampf(kts[i], 0.0f, 1.0f);
                float b = _anim_clampf(kts[i + 1], 0.0f, 1.0f);
                if (t >= a && (t <= b || i + 2 == kt_len)) {
                    seg = i;
                    seg_t0 = a;
                    seg_t1 = b;
                    break;
                }
            }
            if (seg_t1 <= seg_t0) {
                seg_t0 = 0.0f;
                seg_t1 = 1.0f;
            }
        } else {
            float step = 1.0f / (float)(vlist->length - 1);
            seg = (uint32_t)(t / step);
            if (seg >= vlist->length - 1) {
                seg = vlist->length - 2;
            }
            seg_t0 = step * (float)seg;
            seg_t1 = step * (float)(seg + 1);
        }

        float u = 0.0f;
        if (seg_t1 > seg_t0) {
            u = (t - seg_t0) / (seg_t1 - seg_t0);
        }
        u = _anim_clampf(u, 0.0f, 1.0f);

        if (seg >= vlist->length - 1) {
            seg = vlist->length - 2;
        }

        const float* base0 = NULL;
        const float* base1 = NULL;
        uint32_t vlen0 = 0;
        uint32_t vlen1 = 0;
        if (!_anim_values_list_get_transform(vlist, seg, &base0, &vlen0) || !base0) {
            return False;
        }
        if (!_anim_values_list_get_transform(vlist, seg + 1, &base1, &vlen1) || !base1) {
            return False;
        }

        float tx0 = (vlen0 >= 1) ? base0[0] : 0.0f;
        float ty0 = (vlen0 >= 2) ? base0[1] : 0.0f;
        float tx1 = (vlen1 >= 1) ? base1[0] : tx0;
        float ty1 = (vlen1 >= 2) ? base1[1] : ty0;

        *out_e = _anim_lerp(tx0, tx1, u);
        *out_f = _anim_lerp(ty0, ty1, u);
        return True;
    }

    // (moved) _anim_eval_transform_scale_discrete

    // Fallback: from/to interpolation if values is absent.
    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return False;
    }

    // Parser stores animateTransform from/to in the attr union (not as values-list blob).
    // It is a single `_transform_values_list { length; float data[4]; }`.
    if (afrom->val_type != SVG_ATTR_VALUE_PTR || !afrom->value.val) {
        return False;
    }
    if (ato->val_type != SVG_ATTR_VALUE_PTR || !ato->value.val) {
        return False;
    }

    struct _anim_transform_values_single {
        uint32_t length;
        float data[4];
    };

    const struct _anim_transform_values_single* fromv = (const struct _anim_transform_values_single*)afrom->value.val;
    const struct _anim_transform_values_single* tov = (const struct _anim_transform_values_single*)ato->value.val;
    if (!fromv || !tov) {
        return False;
    }

    float tx0 = (fromv->length >= 1) ? fromv->data[0] : 0.0f;
    float ty0 = (fromv->length >= 2) ? fromv->data[1] : 0.0f;
    float tx1 = (tov->length >= 1) ? tov->data[0] : tx0;
    float ty1 = (tov->length >= 2) ? tov->data[1] : ty0;

    *out_e = _anim_lerp(tx0, tx1, t);
    *out_f = _anim_lerp(ty0, ty1, t);
    return True;
}

static INLINE ps_bool _anim_eval_transform_skewx_linear(const psx_svg_anim_item* it, float doc_t,
                                                        float* out_a, float* out_b, float* out_c,
                                                        float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return False;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    if (it->dur_sec <= 0.0f) {
        return False;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return False;
    }

    float local = doc_t - begin_sec;

    // Compute total active duration from repeatCount/repeatDur.
    float total = 0.0f;
    ps_bool has_total = False;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = True;
    } else if (it->repeat_count == 0) {
        // indefinite
        has_total = False;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = True;
    }

    if (!has_total) {
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                local = it->dur_sec;
            } else {
                return False;
            }
        } else {
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    float t01 = _anim_clampf(local / it->dur_sec, 0.0f, 1.0f);
    float ang = 0.0f;
    ps_bool ok_ang = False;

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length < 1) {
            return False;
        }

        if (vlist->length == 1) {
            const float* base0 = NULL;
            uint32_t vlen0 = 0;
            if (!_anim_values_list_get_transform(vlist, 0, &base0, &vlen0) || !base0) {
                return False;
            }
            ang = (vlen0 >= 1) ? base0[0] : 0.0f;
            ok_ang = True;
        } else {
            const psx_svg_attr* akt = _find_attr(it->anim_node, SVG_ATTR_KEY_TIMES);
            const float* kts = NULL;
            uint32_t kt_len = 0;
            if (akt && akt->val_type == SVG_ATTR_VALUE_PTR && akt->value.val) {
                const psx_svg_attr_values_list* ktlist = (const psx_svg_attr_values_list*)akt->value.val;
                kt_len = ktlist->length;
                if (kt_len >= 2) {
                    kts = (const float*)&ktlist->data[0];
                }
            }

            uint32_t seg = 0;
            float seg_t0 = 0.0f;
            float seg_t1 = 1.0f;
            if (kts && kt_len == vlist->length) {
                for (uint32_t i = 0; i + 1 < kt_len; i++) {
                    float a = _anim_clampf(kts[i], 0.0f, 1.0f);
                    float b = _anim_clampf(kts[i + 1], 0.0f, 1.0f);
                    if (t01 >= a && (t01 <= b || i + 2 == kt_len)) {
                        seg = i;
                        seg_t0 = a;
                        seg_t1 = b;
                        break;
                    }
                }
                if (seg_t1 <= seg_t0) {
                    seg_t0 = 0.0f;
                    seg_t1 = 1.0f;
                }
            } else {
                float step = 1.0f / (float)(vlist->length - 1);
                seg = (uint32_t)(t01 / step);
                if (seg >= vlist->length - 1) {
                    seg = vlist->length - 2;
                }
                seg_t0 = step * (float)seg;
                seg_t1 = step * (float)(seg + 1);
            }

            float u = 0.0f;
            if (seg_t1 > seg_t0) {
                u = (t01 - seg_t0) / (seg_t1 - seg_t0);
            }
            u = _anim_clampf(u, 0.0f, 1.0f);

            if (seg >= vlist->length - 1) {
                seg = vlist->length - 2;
            }

            const float* base0 = NULL;
            const float* base1 = NULL;
            uint32_t vlen0 = 0;
            uint32_t vlen1 = 0;
            if (!_anim_values_list_get_transform(vlist, seg, &base0, &vlen0) || !base0) {
                return False;
            }
            if (!_anim_values_list_get_transform(vlist, seg + 1, &base1, &vlen1) || !base1) {
                return False;
            }

            float a0 = (vlen0 >= 1) ? base0[0] : 0.0f;
            float a1 = (vlen1 >= 1) ? base1[0] : a0;
            ang = _anim_lerp(a0, a1, u);
            ok_ang = True;
        }
    }

    if (!ok_ang) {
        const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
        const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
        if (!afrom || !ato) {
            return False;
        }
        float from_a = _attr_as_number(afrom);
        float to_a = _attr_as_number(ato);
        ang = _anim_lerp(from_a, to_a, t01);
    }

    const float rad = ang * 0.01745329252f;
    const float tan_a = (float)tan(rad);

    // skewX(a): [ 1 tan(a) 0; 0 1 0; 0 0 1 ]
    *out_c = tan_a;
    return True;
}

static INLINE ps_bool _anim_eval_transform_skewy_linear(const psx_svg_anim_item* it, float doc_t,
                                                        float* out_a, float* out_b, float* out_c,
                                                        float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return False;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    if (it->dur_sec <= 0.0f) {
        return False;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return False;
    }

    float local = doc_t - begin_sec;

    // Compute total active duration from repeatCount/repeatDur.
    float total = 0.0f;
    ps_bool has_total = False;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = True;
    } else if (it->repeat_count == 0) {
        // indefinite
        has_total = False;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = True;
    }

    if (!has_total) {
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                local = it->dur_sec;
            } else {
                return False;
            }
        } else {
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    float t01 = _anim_clampf(local / it->dur_sec, 0.0f, 1.0f);
    float ang = 0.0f;
    ps_bool ok_ang = False;

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length < 1) {
            return False;
        }

        if (vlist->length == 1) {
            const float* base0 = NULL;
            uint32_t vlen0 = 0;
            if (!_anim_values_list_get_transform(vlist, 0, &base0, &vlen0) || !base0) {
                return False;
            }
            ang = (vlen0 >= 1) ? base0[0] : 0.0f;
            ok_ang = True;
        } else {
            const psx_svg_attr* akt = _find_attr(it->anim_node, SVG_ATTR_KEY_TIMES);
            const float* kts = NULL;
            uint32_t kt_len = 0;
            if (akt && akt->val_type == SVG_ATTR_VALUE_PTR && akt->value.val) {
                const psx_svg_attr_values_list* ktlist = (const psx_svg_attr_values_list*)akt->value.val;
                kt_len = ktlist->length;
                if (kt_len >= 2) {
                    kts = (const float*)&ktlist->data[0];
                }
            }

            uint32_t seg = 0;
            float seg_t0 = 0.0f;
            float seg_t1 = 1.0f;
            if (kts && kt_len == vlist->length) {
                for (uint32_t i = 0; i + 1 < kt_len; i++) {
                    float a = _anim_clampf(kts[i], 0.0f, 1.0f);
                    float b = _anim_clampf(kts[i + 1], 0.0f, 1.0f);
                    if (t01 >= a && (t01 <= b || i + 2 == kt_len)) {
                        seg = i;
                        seg_t0 = a;
                        seg_t1 = b;
                        break;
                    }
                }
                if (seg_t1 <= seg_t0) {
                    seg_t0 = 0.0f;
                    seg_t1 = 1.0f;
                }
            } else {
                float step = 1.0f / (float)(vlist->length - 1);
                seg = (uint32_t)(t01 / step);
                if (seg >= vlist->length - 1) {
                    seg = vlist->length - 2;
                }
                seg_t0 = step * (float)seg;
                seg_t1 = step * (float)(seg + 1);
            }

            float u = 0.0f;
            if (seg_t1 > seg_t0) {
                u = (t01 - seg_t0) / (seg_t1 - seg_t0);
            }
            u = _anim_clampf(u, 0.0f, 1.0f);

            if (seg >= vlist->length - 1) {
                seg = vlist->length - 2;
            }

            const float* base0 = NULL;
            const float* base1 = NULL;
            uint32_t vlen0 = 0;
            uint32_t vlen1 = 0;
            if (!_anim_values_list_get_transform(vlist, seg, &base0, &vlen0) || !base0) {
                return False;
            }
            if (!_anim_values_list_get_transform(vlist, seg + 1, &base1, &vlen1) || !base1) {
                return False;
            }

            float a0 = (vlen0 >= 1) ? base0[0] : 0.0f;
            float a1 = (vlen1 >= 1) ? base1[0] : a0;
            ang = _anim_lerp(a0, a1, u);
            ok_ang = True;
        }
    }

    if (!ok_ang) {
        const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
        const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
        if (!afrom || !ato) {
            return False;
        }

        float from_a = _attr_as_number(afrom);
        float to_a = _attr_as_number(ato);
        ang = _anim_lerp(from_a, to_a, t01);
    }

    const float rad = ang * 0.01745329252f;
    const float tan_a = (float)tan(rad);

    // skewY(a): [ 1 0 0; tan(a) 1 0; 0 0 1 ]
    *out_b = tan_a;
    return True;
}

static INLINE ps_bool _anim_eval_transform_skewx_discrete(const psx_svg_anim_item* it, float doc_t,
                                                          float* out_a, float* out_b, float* out_c,
                                                          float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return False;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    if (it->dur_sec <= 0.0f) {
        return False;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return False;
    }

    float local = doc_t - begin_sec;

    float total = 0.0f;
    ps_bool has_total = False;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = True;
    } else if (it->repeat_count == 0) {
        has_total = False;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = True;
    }

    if (!has_total) {
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                local = it->dur_sec;
            } else {
                return False;
            }
        } else {
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    float t = _anim_clampf(local / it->dur_sec, 0.0f, 1.0f);

    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return False;
    }

    float ang = (t < 0.5f) ? _attr_as_number(afrom) : _attr_as_number(ato);
    float rad = ang * 3.14159265f / 180.0f;
    *out_c = (float)tan(rad);
    return True;
}

static INLINE ps_bool _anim_eval_transform_skewy_discrete(const psx_svg_anim_item* it, float doc_t,
                                                          float* out_a, float* out_b, float* out_c,
                                                          float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return False;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    if (it->dur_sec <= 0.0f) {
        return False;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return False;
    }

    float local = doc_t - begin_sec;

    float total = 0.0f;
    ps_bool has_total = False;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = True;
    } else if (it->repeat_count == 0) {
        has_total = False;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = True;
    }

    if (!has_total) {
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                local = it->dur_sec;
            } else {
                return False;
            }
        } else {
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    float t = _anim_clampf(local / it->dur_sec, 0.0f, 1.0f);

    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return False;
    }

    float ang = (t < 0.5f) ? _attr_as_number(afrom) : _attr_as_number(ato);
    float rad = ang * 3.14159265f / 180.0f;
    *out_b = (float)tan(rad);
    return True;
}

static INLINE ps_bool _anim_eval_transform_scale_discrete(const psx_svg_anim_item* it, float doc_t,
                                                          float* out_a, float* out_b, float* out_c,
                                                          float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return False;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    if (it->dur_sec <= 0.0f) {
        return False;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return False;
    }

    float local = doc_t - begin_sec;

    // repeatCount/repeatDur + fill time handling (mirrors _anim_eval_simple).
    float total = 0.0f;
    ps_bool has_total = False;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = True;
    } else if (it->repeat_count == 0) {
        has_total = False;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = True;
    }

    if (!has_total) {
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                local = it->dur_sec;
            } else {
                return False;
            }
        } else {
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    float t = _anim_clampf(local / it->dur_sec, 0.0f, 1.0f);

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (!avals || avals->val_type != SVG_ATTR_VALUE_PTR || !avals->value.val) {
        return False;
    }

    const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
    if (vlist->length < 1) {
        return False;
    }

    uint32_t idx = 0;
    if (t >= 1.0f) {
        idx = vlist->length - 1;
    } else if (vlist->length >= 2 && t >= 0.5f) {
        idx = 1;
    }

    const float* base = NULL;
    uint32_t vlen = 0;
    if (!_anim_values_list_get_transform(vlist, idx, &base, &vlen) || !base) {
        return False;
    }

    float sx = 1.0f;
    float sy = 1.0f;
    if (vlen >= 1) {
        sx = base[0];
    }
    if (vlen >= 2) {
        sy = base[1];
    } else {
        sy = sx;
    }

    *out_a = sx;
    *out_d = sy;
    return True;
}

static INLINE ps_bool _anim_eval_transform_scale_linear(const psx_svg_anim_item* it, float doc_t,
                                                        float* out_a, float* out_b, float* out_c,
                                                        float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return False;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    if (it->dur_sec <= 0.0f) {
        return False;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return False;
    }

    float local = doc_t - begin_sec;

    // Compute total active duration from repeatCount/repeatDur.
    float total = 0.0f;
    ps_bool has_total = False;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = True;
    } else if (it->repeat_count == 0) {
        // indefinite
        has_total = False;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = True;
    }

    if (!has_total) {
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                local = it->dur_sec;
            } else {
                return False;
            }
        } else {
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    float t = _anim_clampf(local / it->dur_sec, 0.0f, 1.0f);

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length < 1) {
            return False;
        }
        if (vlist->length == 1) {
            const float* base0 = NULL;
            uint32_t vlen0 = 0;
            if (!_anim_values_list_get_transform(vlist, 0, &base0, &vlen0) || !base0) {
                return False;
            }
            float sx = (vlen0 >= 1) ? base0[0] : 1.0f;
            float sy = (vlen0 >= 2) ? base0[1] : sx;
            *out_a = sx;
            *out_d = sy;
            return True;
        }

        const psx_svg_attr* akt = _find_attr(it->anim_node, SVG_ATTR_KEY_TIMES);
        const float* kts = NULL;
        uint32_t kt_len = 0;
        if (akt && akt->val_type == SVG_ATTR_VALUE_PTR && akt->value.val) {
            const psx_svg_attr_values_list* ktlist = (const psx_svg_attr_values_list*)akt->value.val;
            kt_len = ktlist->length;
            if (kt_len >= 2) {
                kts = (const float*)&ktlist->data[0];
            }
        }

        uint32_t seg = 0;
        float seg_t0 = 0.0f;
        float seg_t1 = 1.0f;
        if (kts && kt_len == vlist->length) {
            for (uint32_t i = 0; i + 1 < kt_len; i++) {
                float a = _anim_clampf(kts[i], 0.0f, 1.0f);
                float b = _anim_clampf(kts[i + 1], 0.0f, 1.0f);
                if (t >= a && (t <= b || i + 2 == kt_len)) {
                    seg = i;
                    seg_t0 = a;
                    seg_t1 = b;
                    break;
                }
            }
            if (seg_t1 <= seg_t0) {
                seg_t0 = 0.0f;
                seg_t1 = 1.0f;
            }
        } else {
            float step = 1.0f / (float)(vlist->length - 1);
            seg = (uint32_t)(t / step);
            if (seg >= vlist->length - 1) {
                seg = vlist->length - 2;
            }
            seg_t0 = step * (float)seg;
            seg_t1 = step * (float)(seg + 1);
        }

        float u = 0.0f;
        if (seg_t1 > seg_t0) {
            u = (t - seg_t0) / (seg_t1 - seg_t0);
        }
        u = _anim_clampf(u, 0.0f, 1.0f);

        if (seg >= vlist->length - 1) {
            seg = vlist->length - 2;
        }

        const float* base0 = NULL;
        const float* base1 = NULL;
        uint32_t vlen0 = 0;
        uint32_t vlen1 = 0;
        if (!_anim_values_list_get_transform(vlist, seg, &base0, &vlen0) || !base0) {
            return False;
        }
        if (!_anim_values_list_get_transform(vlist, seg + 1, &base1, &vlen1) || !base1) {
            return False;
        }

        float sx0 = (vlen0 >= 1) ? base0[0] : 1.0f;
        float sy0 = (vlen0 >= 2) ? base0[1] : sx0;
        float sx1 = (vlen1 >= 1) ? base1[0] : sx0;
        float sy1 = (vlen1 >= 2) ? base1[1] : sy0;

        // If a scale entry only provides one number, it implies uniform scale.
        if (vlen0 < 2) {
            sy0 = sx0;
        }
        if (vlen1 < 2) {
            sy1 = sx1;
        }

        *out_a = _anim_lerp(sx0, sx1, u);
        *out_d = _anim_lerp(sy0, sy1, u);
        return True;
    }

    // Fallback: from/to interpolation if values is absent.
    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return False;
    }
    if (afrom->val_type != SVG_ATTR_VALUE_PTR || !afrom->value.val) {
        return False;
    }
    if (ato->val_type != SVG_ATTR_VALUE_PTR || !ato->value.val) {
        return False;
    }

    struct _anim_transform_values_single {
        uint32_t length;
        float data[4];
    };

    const struct _anim_transform_values_single* fromv = (const struct _anim_transform_values_single*)afrom->value.val;
    const struct _anim_transform_values_single* tov = (const struct _anim_transform_values_single*)ato->value.val;
    if (!fromv || !tov) {
        return False;
    }

    float sx0 = (fromv->length >= 1) ? fromv->data[0] : 1.0f;
    float sy0 = (fromv->length >= 2) ? fromv->data[1] : sx0;
    float sx1 = (tov->length >= 1) ? tov->data[0] : sx0;
    float sy1 = (tov->length >= 2) ? tov->data[1] : sy0;

    *out_a = _anim_lerp(sx0, sx1, t);
    *out_d = _anim_lerp(sy0, sy1, t);
    return True;
}

static INLINE ps_bool _anim_eval_transform_rotate_discrete(const psx_svg_anim_item* it, float doc_t,
                                                           float* out_a, float* out_b, float* out_c,
                                                           float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return False;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    if (it->dur_sec <= 0.0f) {
        return False;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return False;
    }

    float local = doc_t - begin_sec;

    // repeatCount/repeatDur + fill time handling (mirrors _anim_eval_simple).
    float total = 0.0f;
    ps_bool has_total = False;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = True;
    } else if (it->repeat_count == 0) {
        has_total = False;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = True;
    }

    if (!has_total) {
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                local = it->dur_sec;
            } else {
                return False;
            }
        } else {
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    float t = _anim_clampf(local / it->dur_sec, 0.0f, 1.0f);

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (!avals || avals->val_type != SVG_ATTR_VALUE_PTR || !avals->value.val) {
        return False;
    }
    const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
    if (vlist->length < 1) {
        return False;
    }

    uint32_t idx = 0;
    if (t >= 1.0f) {
        idx = vlist->length - 1;
    } else if (vlist->length >= 2 && t >= 0.5f) {
        idx = 1;
    }

    const float* base = NULL;
    uint32_t vlen = 0;
    if (!_anim_values_list_get_transform(vlist, idx, &base, &vlen) || !base) {
        return False;
    }

    // Rotate values are stored in the transform-values blob; first number is angle in degrees.
    float angle_deg = (vlen >= 1) ? base[0] : 0.0f;
    const float pi = 3.14159265358979323846f;
    float angle_rad = angle_deg * (pi / 180.0f);
    float cs = (float)cos(angle_rad);
    float sn = (float)sin(angle_rad);

    // Matrix for rotation about origin.
    *out_a = cs;
    *out_b = sn;
    *out_c = -sn;
    *out_d = cs;
    return True;
}

static INLINE ps_bool _anim_eval_transform_rotate_linear(const psx_svg_anim_item* it, float doc_t,
                                                         float* out_a, float* out_b, float* out_c,
                                                         float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return False;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    if (it->dur_sec <= 0.0f) {
        return False;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return False;
    }

    float local = doc_t - begin_sec;

    // Compute total active duration from repeatCount/repeatDur.
    float total = 0.0f;
    ps_bool has_total = False;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = True;
    } else if (it->repeat_count == 0) {
        // indefinite
        has_total = False;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = True;
    }

    if (!has_total) {
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                local = it->dur_sec;
            } else {
                return False;
            }
        } else {
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    float t = _anim_clampf(local / it->dur_sec, 0.0f, 1.0f);

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (!avals || avals->val_type != SVG_ATTR_VALUE_PTR || !avals->value.val) {
        return False;
    }

    const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
    if (vlist->length < 1) {
        return False;
    }
    if (vlist->length == 1) {
        const float* base0 = NULL;
        uint32_t vlen0 = 0;
        if (!_anim_values_list_get_transform(vlist, 0, &base0, &vlen0) || !base0) {
            return False;
        }
        float angle_deg = (vlen0 >= 1) ? base0[0] : 0.0f;
        float cx = (vlen0 >= 2) ? base0[1] : 0.0f;
        float cy = (vlen0 >= 3) ? base0[2] : 0.0f;
        const float pi = 3.14159265358979323846f;
        float angle_rad = angle_deg * (pi / 180.0f);
        float cs = (float)cos(angle_rad);
        float sn = (float)sin(angle_rad);
        *out_a = cs;
        *out_b = sn;
        *out_c = -sn;
        *out_d = cs;
        // rotate about (cx,cy): T(cx,cy)*R*T(-cx,-cy)
        *out_e = cx - cx * cs + cy * sn;
        *out_f = cy - cx * sn - cy * cs;
        return True;
    }

    const psx_svg_attr* akt = _find_attr(it->anim_node, SVG_ATTR_KEY_TIMES);
    const float* kts = NULL;
    uint32_t kt_len = 0;
    if (akt && akt->val_type == SVG_ATTR_VALUE_PTR && akt->value.val) {
        const psx_svg_attr_values_list* ktlist = (const psx_svg_attr_values_list*)akt->value.val;
        kt_len = ktlist->length;
        if (kt_len >= 2) {
            kts = (const float*)&ktlist->data[0];
        }
    }

    uint32_t seg = 0;
    float seg_t0 = 0.0f;
    float seg_t1 = 1.0f;
    if (kts && kt_len == vlist->length) {
        for (uint32_t i = 0; i + 1 < kt_len; i++) {
            float a = _anim_clampf(kts[i], 0.0f, 1.0f);
            float b = _anim_clampf(kts[i + 1], 0.0f, 1.0f);
            if (t >= a && (t <= b || i + 2 == kt_len)) {
                seg = i;
                seg_t0 = a;
                seg_t1 = b;
                break;
            }
        }
        if (seg_t1 <= seg_t0) {
            seg_t0 = 0.0f;
            seg_t1 = 1.0f;
        }
    } else {
        float step = 1.0f / (float)(vlist->length - 1);
        seg = (uint32_t)(t / step);
        if (seg >= vlist->length - 1) {
            seg = vlist->length - 2;
        }
        seg_t0 = step * (float)seg;
        seg_t1 = step * (float)(seg + 1);
    }

    float u = 0.0f;
    if (seg_t1 > seg_t0) {
        u = (t - seg_t0) / (seg_t1 - seg_t0);
    }
    u = _anim_clampf(u, 0.0f, 1.0f);

    if (seg >= vlist->length - 1) {
        seg = vlist->length - 2;
    }

    const float* base0 = NULL;
    const float* base1 = NULL;
    uint32_t vlen0 = 0;
    uint32_t vlen1 = 0;
    if (!_anim_values_list_get_transform(vlist, seg, &base0, &vlen0) || !base0) {
        return False;
    }
    if (!_anim_values_list_get_transform(vlist, seg + 1, &base1, &vlen1) || !base1) {
        return False;
    }

    float ang0 = (vlen0 >= 1) ? base0[0] : 0.0f;
    float ang1 = (vlen1 >= 1) ? base1[0] : ang0;

    // rotate values can be: angle [cx cy]
    float cx0 = (vlen0 >= 2) ? base0[1] : 0.0f;
    float cy0 = (vlen0 >= 3) ? base0[2] : 0.0f;
    float cx1 = (vlen1 >= 2) ? base1[1] : cx0;
    float cy1 = (vlen1 >= 3) ? base1[2] : cy0;

    // If a rotate entry provides cx but not cy, treat cy as 0.
    if (vlen0 == 2) {
        cy0 = 0.0f;
    }
    if (vlen1 == 2) {
        cy1 = 0.0f;
    }

    float angle_deg = _anim_lerp(ang0, ang1, u);
    float cx = _anim_lerp(cx0, cx1, u);
    float cy = _anim_lerp(cy0, cy1, u);

    const float pi = 3.14159265358979323846f;
    float angle_rad = angle_deg * (pi / 180.0f);
    float cs = (float)cos(angle_rad);
    float sn = (float)sin(angle_rad);
    *out_a = cs;
    *out_b = sn;
    *out_c = -sn;
    *out_d = cs;
    *out_e = cx - cx * cs + cy * sn;
    *out_f = cy - cx * sn - cy * cs;
    return True;
}

// Note: psx_svg_attr_values_list is a variable-sized blob (length + data[]).
// The element layout depends on which attribute it represents.

static INLINE ps_bool _anim_eval_set(const psx_svg_anim_item* it, float doc_t, float* out_v, ps_bool* out_hold)
{
    if (!it || !out_v || !out_hold) {
        return False;
    }
    *out_hold = False;

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return False;
    }

    // If an explicit end (or end-list) is present, it can shorten the active interval.
    // Effective end is the earliest end time >= current begin.
    float end_sec = 0.0f;
    if (psx_array_size((psx_array*)&it->ends_sec) > 0) {
        end_sec = _anim_item_end_for_begin(it, begin_sec);
    } else if (it->end_sec > 0.0f) {
        end_sec = it->end_sec;
    }
    if (end_sec > 0.0f && end_sec < begin_sec) {
        // invalid; ignore
        end_sec = 0.0f;
    }

    // If explicit end is present and we've passed it, the interval is inactive
    // for fill=remove, or held for fill=freeze.
    if (end_sec > 0.0f && doc_t > end_sec) {
        if (it->fill_mode == SVG_ANIMATION_FREEZE) {
            *out_hold = True;
            // still return the frozen end value
            const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
            if (!ato) {
                return False;
            }
            *out_v = _attr_as_number(ato);
            return True;
        }
        return False;
    }

    // If dur is missing/0, SVG <set> is treated as an instant change.
    if (it->dur_sec <= 0.0f) {
        const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
        if (!ato) {
            return False;
        }
        *out_v = _attr_as_number(ato);
        // end instant handling
        if (end_sec > 0.0f && doc_t == end_sec) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = True;
                return True;
            }
            if (it->fill_mode == SVG_ANIMATION_REMOVE) {
                return False;
            }
        }
        return True;
    }

    float local = doc_t - begin_sec;

    // If repeatDur is specified, it bounds the active duration (unless explicit end is earlier).
    float repeat_end_sec = 0.0f;
    if (it->repeat_dur_sec > 0.0f) {
        repeat_end_sec = begin_sec + it->repeat_dur_sec;
        if (repeat_end_sec < begin_sec) {
            repeat_end_sec = 0.0f;
        }
    }

    if (repeat_end_sec > 0.0f) {
        if (end_sec <= 0.0f || repeat_end_sec < end_sec) {
            end_sec = repeat_end_sec;
        }
    }

    if (end_sec > 0.0f) {
        float active_dur = end_sec - begin_sec;
        // Per SMIL, if end <= begin then interval is empty. Treat as no-op.
        if (active_dur <= 0.0f) {
            return False;
        }

        // Outside active interval
        if (doc_t > end_sec) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = True;
                // treat as at end
                local = active_dur;
            } else {
                return False;
            }
        }

        // end instant
        if (doc_t == end_sec) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = True;
                local = active_dur;
            } else if (it->fill_mode == SVG_ANIMATION_REMOVE) {
                return False;
            }
        }

        // When explicit end is present, it defines the end of the active
        // interval (regardless of declared dur).
        if (local > active_dur) {
            return False;
        }
    }

    // For <set>, value is constant during the active interval.
    // Do NOT wrap with fmod: repeats don't change the value and wrapping breaks
    // fill=remove semantics at end-of-interval.
    if (it->repeat_count != 0) {
        float total = it->dur_sec * (float)it->repeat_count;
        if (local > total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = True;
                // Hold at end value.
                local = it->dur_sec;
            } else {
                return False;
            }
        }
    }

    // Active interval end:
    // - If explicit end is present: [begin, end]
    // - Else: [begin, begin+dur]
    // NOTE: the end_sec path (explicit end or repeatDur mapped to end) already
    // handles doc_t > end_sec and doc_t == end_sec above. Do not reject again
    // here, otherwise fill=freeze at end instant/after end can be lost.
    if (end_sec <= 0.0f) {
        // Active during [0, dur].
        if (local > it->dur_sec) {
            return False;
        }

        // If fill=freeze, keep value visible at the end instant.
        if (local == it->dur_sec) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = True;
            } else if (it->fill_mode == SVG_ANIMATION_REMOVE) {
                // end instant is not active for remove
                return False;
            }
        }
    }

    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!ato) {
        return False;
    }
    *out_v = _attr_as_number(ato);
    return True;
}

extern "C" {

    bool psx_svg_anim_get_float(const psx_svg_anim_state* s,
                                const psx_svg_node* target,
                                psx_svg_attr_type attr,
                                float* out_v)
    {
        if (!out_v) {
            return false;
        }
        *out_v = 0.0f;
        const psx_svg_anim_override_item* it = _anim_state_find(s, target, attr);
        if (!it) {
            return false;
        }
        *out_v = it->fval;
        return true;
    }

} // extern "C"

static const psx_svg_node* _find_child_mpath(const psx_svg_node* n)
{
    if (!n) {
        return NULL;
    }
    uint32_t c = n->child_count();
    for (uint32_t i = 0; i < c; i++) {
        const psx_svg_node* ch = n->get_child(i);
        if (ch && ch->type() == SVG_TAG_MPATH) {
            return ch;
        }
    }
    return NULL;
}

static void _collect_anims(psx_svg_player* p, const psx_svg_node* node)
{
    if (!p || !node) {
        return;
    }

    psx_svg_tag t = node->type();
    if (t == SVG_TAG_ANIMATE || t == SVG_TAG_SET || t == SVG_TAG_ANIMATE_COLOR || t == SVG_TAG_ANIMATE_TRANSFORM || t == SVG_TAG_ANIMATE_MOTION) {
        const psx_svg_attr* an = _find_attr(node, SVG_ATTR_ATTRIBUTE_NAME);
        psx_svg_attr_type target_attr = SVG_ATTR_INVALID;
        if (an) {
            // attributeName is stored as DATA ival (enum psx_svg_attr_type) in parser.
            if (an->val_type == SVG_ATTR_VALUE_DATA) {
                target_attr = (psx_svg_attr_type)an->value.ival;
            }
        }

        const psx_svg_node* target = node->parent();
        if (t == SVG_TAG_ANIMATE_MOTION) {
            // for animateMotion, allow mpath child; real resolve later
            (void)_find_child_mpath(node);
        }

        if (target && target_attr != SVG_ATTR_INVALID) {
            psx_svg_anim_item item;
            memset(&item, 0, sizeof(item));
            item.tag = t;
            item.anim_node = node;
            item.target_node = target;
            item.target_attr = target_attr;

            _anim_item_begin_list_init(&item);
            _anim_item_end_list_init(&item);

            const psx_svg_attr* abegin = _find_attr(node, SVG_ATTR_BEGIN);
            if (abegin && abegin->val_type == SVG_ATTR_VALUE_TIMING_LIST_PTR && abegin->value.val
                && (node->type() == SVG_TAG_SET || node->type() == SVG_TAG_ANIMATE || node->type() == SVG_TAG_ANIMATE_COLOR
                    || node->type() == SVG_TAG_ANIMATE_TRANSFORM || node->type() == SVG_TAG_ANIMATE_MOTION)
                && (abegin->attr_id == SVG_ATTR_BEGIN)) {
                const psx_svg_timing_list* tl = (const psx_svg_timing_list*)abegin->value.val;
                if (t == SVG_TAG_SET && tl->event_token) {
                    item.begin_event = _dup_cstr(tl->event_token);
                    psx_array_clear(&item.begins_sec);
                    item.begin_sec = 1e30f; // don't auto-activate until triggered
                }
                for (uint32_t i = 0; i < tl->offsets_len; i++) {
                    float ms = tl->offsets_ms ? tl->offsets_ms[i] : 0.0f;
                    float sec = (ms <= 0.0f) ? 0.0f : (ms * 0.001f);
                    psx_array_append(&item.begins_sec, &sec);
                }
                _anim_item_begin_list_normalize(&item);
                if (psx_array_size(&item.begins_sec) > 0) {
                    item.begin_sec = *(psx_array_get(&item.begins_sec, 0, float));
                }
            } else if (abegin && abegin->val_type == SVG_ATTR_VALUE_PTR && abegin->value.val) {
                if (t == SVG_TAG_SET) {
                    // Parser currently parses begin/end lists as clock offsets only.
                    const psx_svg_attr_values_list* list = (const psx_svg_attr_values_list*)abegin->value.val;
                    // Detect event tokens by looking at the original string.
                    // Current parser callback parses only clock offsets; a non-numeric token
                    // gets converted to 0ms ("indefinite" is also 0ms). We disambiguate by
                    // checking the raw string: if it's non-numeric, treat it as event.
                    const char* raw = _find_attr_string_raw(node, SVG_ATTR_BEGIN);
                    if (raw && !_is_clock_value_token(raw)) {
                        item.begin_event = _dup_cstr(raw);
                        psx_array_clear(&item.begins_sec);
                        item.begin_sec = 1e30f;
                    } else {
                        // Treat as a clock-value list (ms) parsed by parser.
                        const float* vals = (list->length > 0) ? (const float*)&list->data[0] : NULL;
                        for (uint32_t i = 0; i < list->length; i++) {
                            float ms = vals[i];
                            float sec = (ms <= 0.0f) ? 0.0f : (ms * 0.001f);
                            psx_array_append(&item.begins_sec, &sec);
                        }
                        _anim_item_begin_list_normalize(&item);
                        item.begin_sec = _attr_as_begin_time_sec(abegin);
                    }
                } else {
                    const psx_svg_attr_values_list* list = (const psx_svg_attr_values_list*)abegin->value.val;
                    if (list->length > 0) {
                        const float* vals = (const float*)&list->data[0];
                        for (uint32_t i = 0; i < list->length; i++) {
                            float ms = vals[i];
                            float sec = (ms <= 0.0f) ? 0.0f : (ms * 0.001f);
                            psx_array_append(&item.begins_sec, &sec);
                        }

                        _anim_item_begin_list_normalize(&item);

                        // begin_sec used for duration hint; keep earliest.
                        item.begin_sec = _attr_as_begin_time_sec(abegin);
                    } else {
                        item.begin_sec = 0.0f;
                    }
                }
            } else {
                item.begin_sec = _attr_as_begin_time_sec(abegin);
            }

            const psx_svg_attr* aend = _find_attr(node, SVG_ATTR_END);
            if (aend && aend->val_type == SVG_ATTR_VALUE_TIMING_LIST_PTR && aend->value.val && aend->attr_id == SVG_ATTR_END) {
                const psx_svg_timing_list* tl = (const psx_svg_timing_list*)aend->value.val;
                for (uint32_t i = 0; i < tl->offsets_len; i++) {
                    float ms = tl->offsets_ms ? tl->offsets_ms[i] : 0.0f;
                    float sec = (ms <= 0.0f) ? 0.0f : (ms * 0.001f);
                    psx_array_append(&item.ends_sec, &sec);
                }
                _anim_item_end_list_normalize(&item);
                item.end_sec = (psx_array_size(&item.ends_sec) > 0) ? *(psx_array_get(&item.ends_sec, 0, float)) : 0.0f;
            } else {
                item.end_sec = _attr_as_time_sec(aend);
            }

            item.dur_sec = _attr_as_time_sec(_find_attr(node, SVG_ATTR_DUR));
            // parser uses 0 for indefinite
            item.repeat_count = _attr_as_u32(_find_attr(node, SVG_ATTR_REPEAT_COUNT), 1);
            item.repeat_dur_sec = _attr_as_time_sec(_find_attr(node, SVG_ATTR_REPEAT_DUR));
            item.fill_mode = SVG_ANIMATION_REMOVE;
            // fill="freeze|remove" is parsed by paint parser for animation nodes.
            const psx_svg_attr* fill = _find_attr(node, SVG_ATTR_FILL);
            if (fill && fill->val_type == SVG_ATTR_VALUE_DATA) {
                if (fill->value.ival == SVG_ANIMATION_FREEZE || fill->value.ival == SVG_ANIMATION_REMOVE) {
                    item.fill_mode = (uint32_t)fill->value.ival;
                }
            }

            psx_array_append(&p->anims, NULL);
            psx_svg_anim_item* dst = psx_array_get_last(&p->anims, psx_svg_anim_item);
            *dst = item;

            // No extra normalization needed: parser stores <set> begin/dur as clock-time DATA(fval ms).
        }
    }

    uint32_t c = node->child_count();
    for (uint32_t i = 0; i < c; i++) {
        _collect_anims(p, node->get_child(i));
    }
}

extern "C" {

    static psx_svg_player* _psx_svg_player_create_impl(psx_svg_node* root,
                                                       const psx_svg_player_options* opt_in,
                                                       psx_result* out)
    {
        if (out) {
            *out = S_OK;
        }
        if (!root) {
            if (out) {
                *out = S_BAD_PARAMS;
            }
            return NULL;
        }

        psx_svg_player_options opt = opt_in ? *opt_in : _default_options();

        psx_svg_player* p = (psx_svg_player*)mem_malloc(sizeof(psx_svg_player));
        if (!p) {
            if (out) {
                *out = S_OUT_OF_MEMORY;
            }
            return NULL;
        }
        memset(p, 0, sizeof(psx_svg_player));

        p->root = root;
        p->own_root = opt.take_ownership_of_root;
        p->loop = opt.loop;
        p->dpi = (opt.dpi > 0) ? opt.dpi : 96;

        p->render_list = psx_svg_render_list_create(root);
        if (!p->render_list) {
            if (out) {
                *out = S_FAILURE;
            }
            if (p->own_root) {
                psx_svg_node_destroy(p->root);
            }
            mem_free(p);
            return NULL;
        }

        p->state = PSX_SVG_PLAYER_STOPPED;
        p->time_sec = 0.0f;
        p->duration_sec = -1.0f;

        psx_array_init(&p->anim_state.overrides, sizeof(psx_svg_anim_override_item));
        psx_array_init(&p->anim_state.transforms, sizeof(psx_svg_anim_transform_item));

        psx_array_init(&p->anims, sizeof(psx_svg_anim_item));
        _collect_anims(p, p->root);

        // compute a simple duration hint
        // If there is no animation, keep duration as unknown (-1). This avoids
        // treating a static document as having duration 0 and immediately
        // stopping on the first tick.
        if (psx_array_size(&p->anims) == 0) {
            p->duration_sec = -1.0f;
        } else {
            float end_max = 0.0f;
            p->duration_sec = 0.0f;
            uint32_t n = psx_array_size(&p->anims);
            for (uint32_t i = 0; i < n; i++) {
                psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
                float endt = it->begin_sec + it->dur_sec;
                if (it->repeat_count > 1) {
                    endt = it->begin_sec + it->dur_sec * (float)it->repeat_count;
                } else if (it->repeat_count == 0) {
                    p->duration_sec = -1.0f;
                    end_max = 0.0f;
                    break;
                }
                if (endt > end_max) {
                    end_max = endt;
                }
            }
            if (p->duration_sec != -1.0f) {
                p->duration_sec = end_max;
            }
        }

        return p;
    }

    psx_svg_player* psx_svg_player_create(const psx_svg_node* root,
                                          const psx_svg_player_options* opt_in,
                                          psx_result* out)
    {
        // We may need non-const access internally for indices/caches, but we do not
        // mutate the DOM in this minimal implementation.
        return _psx_svg_player_create_impl((psx_svg_node*)root, opt_in, out);
    }

    psx_svg_player* psx_svg_player_create_from_data(const char* svg_data,
                                                    uint32_t len,
                                                    const psx_svg_player_options* opt,
                                                    psx_result* out)
    {
        if (out) {
            *out = S_OK;
        }
        if (!svg_data || !len) {
            if (out) {
                *out = S_BAD_PARAMS;
            }
            return NULL;
        }

        psx_svg_node* root = psx_svg_load_data(svg_data, len);
        if (!root) {
            if (out) {
                *out = S_FAILURE;
            }
            return NULL;
        }

        psx_svg_player_options o = opt ? *opt : _default_options();
        // We created the node here; player should own it unless caller explicitly overrides.
        o.take_ownership_of_root = True;

        psx_svg_player* p = _psx_svg_player_create_impl(root, &o, out);
        if (!p) {
            psx_svg_node_destroy(root);
            return NULL;
        }

        return p;
    }

    void psx_svg_player_destroy(psx_svg_player* p)
    {
        if (!p) {
            return;
        }

        // Per-item dynamic memory
        {
            uint32_t n = psx_array_size(&p->anims);
            for (uint32_t i = 0; i < n; i++) {
                psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
                _anim_item_begin_list_destroy(it);
                _anim_item_end_list_destroy(it);
            }
        }

        psx_array_destroy(&p->anim_state.overrides);
        psx_array_destroy(&p->anim_state.transforms);

        psx_array_destroy(&p->anims);

        if (p->render_list) {
            psx_svg_render_list_destroy(p->render_list);
            p->render_list = NULL;
        }

        if (p->own_root && p->root) {
            psx_svg_node_destroy(p->root);
            p->root = NULL;
        }

        mem_free(p);
    }

    void psx_svg_player_play(psx_svg_player* p)
    {
        if (!p) {
            return;
        }
        if (p->state == PSX_SVG_PLAYER_STOPPED) {
            p->time_sec = 0.0f;
        }
        p->state = PSX_SVG_PLAYER_PLAYING;
    }

    void psx_svg_player_pause(psx_svg_player* p)
    {
        if (!p) {
            return;
        }
        if (p->state != PSX_SVG_PLAYER_STOPPED) {
            p->state = PSX_SVG_PLAYER_PAUSED;
        }
    }

    void psx_svg_player_stop(psx_svg_player* p)
    {
        if (!p) {
            return;
        }
        p->state = PSX_SVG_PLAYER_STOPPED;
        p->time_sec = 0.0f;
    }

    void psx_svg_player_seek(psx_svg_player* p, float seconds)
    {
        if (!p) {
            return;
        }
        if (seconds < 0.0f) {
            seconds = 0.0f;
        }
        p->time_sec = seconds;

        if (p->state == PSX_SVG_PLAYER_STOPPED) {
            p->state = PSX_SVG_PLAYER_PAUSED;
        }

        // Rebuild overrides at the new time so callers can query immediately
        // without needing a positive tick.
        _anim_state_reset(&p->anim_state);
        uint32_t n = psx_array_size(&p->anims);
        for (uint32_t i = 0; i < n; i++) {
            const psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);

            if (!(it->tag == SVG_TAG_ANIMATE || it->tag == SVG_TAG_SET || it->tag == SVG_TAG_ANIMATE_COLOR || it->tag == SVG_TAG_ANIMATE_TRANSFORM)) {
                continue;
            }
            if (!(it->target_attr == SVG_ATTR_X || it->target_attr == SVG_ATTR_Y || it->target_attr == SVG_ATTR_WIDTH || it->target_attr == SVG_ATTR_HEIGHT || it->target_attr == SVG_ATTR_OPACITY || it->target_attr == SVG_ATTR_RX || it->target_attr == SVG_ATTR_RY || it->target_attr == SVG_ATTR_STROKE_WIDTH || it->target_attr == SVG_ATTR_FILL_OPACITY || it->target_attr == SVG_ATTR_GRADIENT_STOP_OPACITY || it->target_attr == SVG_ATTR_FILL || it->target_attr == SVG_ATTR_TRANSFORM)) {
                continue;
            }

            if (it->tag == SVG_TAG_ANIMATE_TRANSFORM && it->target_attr == SVG_ATTR_TRANSFORM) {
                float a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;
                const psx_svg_attr* acm = _find_attr(it->anim_node, SVG_ATTR_CALC_MODE);
                ps_bool is_discrete = (acm && acm->val_type == SVG_ATTR_VALUE_DATA && acm->value.ival == SVG_ANIMATION_CALC_MODE_DISCRETE) ? True : False;
                const psx_svg_attr* att = _find_attr(it->anim_node, SVG_ATTR_TRANSFORM_TYPE);
                int32_t ttype = (att && att->val_type == SVG_ATTR_VALUE_DATA) ? att->value.ival : 0;
                ps_bool ok2 = False;
                if (is_discrete) {
                    if (ttype == SVG_TRANSFORM_TYPE_SCALE) {
                        ok2 = _anim_eval_transform_scale_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_ROTATE) {
                        ok2 = _anim_eval_transform_rotate_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_X) {
                        ok2 = _anim_eval_transform_skewx_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_Y) {
                        ok2 = _anim_eval_transform_skewy_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else {
                        // Default/legacy: translate.
                        ok2 = _anim_eval_transform_translate_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    }
                } else {
                    if (ttype == SVG_TRANSFORM_TYPE_ROTATE) {
                        ok2 = _anim_eval_transform_rotate_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SCALE) {
                        ok2 = _anim_eval_transform_scale_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_X) {
                        ok2 = _anim_eval_transform_skewx_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_Y) {
                        ok2 = _anim_eval_transform_skewy_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else {
                        // For now, only translate supports non-discrete modes.
                        ok2 = _anim_eval_transform_translate_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    }
                }
                if (ok2) {
                    _anim_state_set_transform(&p->anim_state, it->target_node, a, b, c, d, e, f);
                }
                continue;
            }

            float v = 0.0f;
            ps_bool hold = False;
            ps_bool ok = False;
            if (it->tag == SVG_TAG_ANIMATE || it->tag == SVG_TAG_ANIMATE_COLOR) {
                ok = _anim_eval_simple(it, p->time_sec, &v, &hold);
            } else {
                ok = _anim_eval_set(it, p->time_sec, &v, &hold);
            }
            if (ok) {
                (void)hold;
                // For animateColor(fill), v stores packed uint32 in float bits.
                _anim_state_set_float(&p->anim_state, it->target_node, it->target_attr, v);
            }
        }
    }

    void psx_svg_player_tick(psx_svg_player* p, float delta_seconds)
    {
        if (!p) {
            return;
        }
        if (p->state == PSX_SVG_PLAYER_PLAYING) {
            if (delta_seconds > 0.0f) {
                p->time_sec += delta_seconds;
            }
        }

        if (p->duration_sec >= 0.0f && p->time_sec > p->duration_sec) {
            if (p->loop) {
                p->time_sec = 0.0f;
                if (p->cb) {
                    p->cb(PSX_SVG_ANIM_EVENT_REPEAT, NULL, p->cb_user);
                }
            } else {
                p->time_sec = p->duration_sec;
                p->state = PSX_SVG_PLAYER_STOPPED;
                if (p->cb) {
                    p->cb(PSX_SVG_ANIM_EVENT_END, NULL, p->cb_user);
                }
            }
        }

        // Evaluate animations and update anim_state override tables.
        _anim_state_reset(&p->anim_state);

        uint32_t n = psx_array_size(&p->anims);
        for (uint32_t i = 0; i < n; i++) {
            const psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);

            if (!(it->tag == SVG_TAG_ANIMATE || it->tag == SVG_TAG_SET || it->tag == SVG_TAG_ANIMATE_COLOR || it->tag == SVG_TAG_ANIMATE_TRANSFORM)) {
                continue;
            }
            // Tiny 1.2 minimal player: numeric attributes + animateColor(fill).
            if (!(it->target_attr == SVG_ATTR_X || it->target_attr == SVG_ATTR_Y || it->target_attr == SVG_ATTR_WIDTH || it->target_attr == SVG_ATTR_HEIGHT || it->target_attr == SVG_ATTR_OPACITY || it->target_attr == SVG_ATTR_RX || it->target_attr == SVG_ATTR_RY || it->target_attr == SVG_ATTR_STROKE_WIDTH || it->target_attr == SVG_ATTR_FILL_OPACITY || it->target_attr == SVG_ATTR_GRADIENT_STOP_OPACITY || it->target_attr == SVG_ATTR_FILL || it->target_attr == SVG_ATTR_TRANSFORM)) {
                continue;
            }

            if (it->tag == SVG_TAG_ANIMATE_TRANSFORM && it->target_attr == SVG_ATTR_TRANSFORM) {
                float a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;
                const psx_svg_attr* acm = _find_attr(it->anim_node, SVG_ATTR_CALC_MODE);
                ps_bool is_discrete = (acm && acm->val_type == SVG_ATTR_VALUE_DATA && acm->value.ival == SVG_ANIMATION_CALC_MODE_DISCRETE) ? True : False;
                const psx_svg_attr* att = _find_attr(it->anim_node, SVG_ATTR_TRANSFORM_TYPE);
                int32_t ttype = (att && att->val_type == SVG_ATTR_VALUE_DATA) ? att->value.ival : 0;
                ps_bool ok2 = False;
                if (is_discrete) {
                    if (ttype == SVG_TRANSFORM_TYPE_SCALE) {
                        ok2 = _anim_eval_transform_scale_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_ROTATE) {
                        ok2 = _anim_eval_transform_rotate_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_X) {
                        ok2 = _anim_eval_transform_skewx_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_Y) {
                        ok2 = _anim_eval_transform_skewy_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else {
                        ok2 = _anim_eval_transform_translate_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    }
                } else {
                    if (ttype == SVG_TRANSFORM_TYPE_ROTATE) {
                        ok2 = _anim_eval_transform_rotate_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SCALE) {
                        ok2 = _anim_eval_transform_scale_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_X) {
                        ok2 = _anim_eval_transform_skewx_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_Y) {
                        ok2 = _anim_eval_transform_skewy_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else {
                        ok2 = _anim_eval_transform_translate_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    }
                }
                if (ok2) {
                    _anim_state_set_transform(&p->anim_state, it->target_node, a, b, c, d, e, f);
                }
                continue;
            }

            float v = 0.0f;
            ps_bool hold = False;
            ps_bool ok = False;
            if (it->tag == SVG_TAG_ANIMATE || it->tag == SVG_TAG_ANIMATE_COLOR) {
                ok = _anim_eval_simple(it, p->time_sec, &v, &hold);
            } else {
                ok = _anim_eval_set(it, p->time_sec, &v, &hold);
            }
            if (ok) {
                (void)hold;
                _anim_state_set_float(&p->anim_state, it->target_node, it->target_attr, v);
            }
        }
    }

    float psx_svg_player_get_time(const psx_svg_player* p)
    {
        return p ? p->time_sec : 0.0f;
    }

    float psx_svg_player_get_duration(const psx_svg_player* p)
    {
        return p ? p->duration_sec : -1.0f;
    }

    psx_svg_player_state psx_svg_player_get_state(const psx_svg_player* p)
    {
        return p ? p->state : PSX_SVG_PLAYER_STOPPED;
    }

    void psx_svg_player_set_loop(psx_svg_player* p, ps_bool loop)
    {
        if (!p) {
            return;
        }
        p->loop = loop;
    }

    ps_bool psx_svg_player_get_loop(const psx_svg_player* p)
    {
        return p ? p->loop : False;
    }

    void psx_svg_player_draw(psx_svg_player* p, ps_context* ctx)
    {
        if (!p || !ctx || !p->render_list) {
            return;
        }

        // Ensure animation state is applied.
        psx_svg_render_list_draw_anim(ctx, p->render_list, &p->anim_state);
    }

    const psx_svg_render_list* psx_svg_player_get_render_list(const psx_svg_player* p)
    {
        return p ? p->render_list : NULL;
    }

    void psx_svg_player_set_event_callback(psx_svg_player* p, psx_svg_anim_event_cb cb, void* user)
    {
        if (!p) {
            return;
        }
        p->cb = cb;
        p->cb_user = user;
    }

    void psx_svg_player_trigger(psx_svg_player* p, const char* target_id, const char* event_name)
    {
        if (!p) {
            return;
        }
        if (!event_name || !*event_name) {
            return;
        }

        const char* filter_id = (target_id && *target_id) ? target_id : NULL;

        uint32_t n = psx_array_size(&p->anims);
        ps_bool any = False;
        for (uint32_t i = 0; i < n; i++) {
            psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
            if (!it || !it->begin_event) {
                continue;
            }
            if (strcmp(it->begin_event, event_name) != 0) {
                continue;
            }
            if (filter_id) {
                const char* tid = it->target_node ? it->target_node->content(NULL) : NULL;
                const char* aid = it->anim_node ? it->anim_node->content(NULL) : NULL;
                if ((!tid || strcmp(tid, filter_id) != 0) && (!aid || strcmp(aid, filter_id) != 0)) {
                    continue;
                }
            }

            float sec = p->time_sec;
            psx_array_append(&it->begins_sec, &sec);
            _anim_item_begin_list_normalize(it);
            any = True;
        }

        if (!any) {
            return;
        }

        // Rebuild overrides at the current time so callers can query immediately.
        _anim_state_reset(&p->anim_state);
        n = psx_array_size(&p->anims);
        for (uint32_t i = 0; i < n; i++) {
            const psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
            if (!it) {
                continue;
            }
            if (!(it->tag == SVG_TAG_ANIMATE || it->tag == SVG_TAG_SET || it->tag == SVG_TAG_ANIMATE_COLOR || it->tag == SVG_TAG_ANIMATE_TRANSFORM)) {
                continue;
            }
            if (!(it->target_attr == SVG_ATTR_X || it->target_attr == SVG_ATTR_Y || it->target_attr == SVG_ATTR_WIDTH || it->target_attr == SVG_ATTR_HEIGHT || it->target_attr == SVG_ATTR_OPACITY || it->target_attr == SVG_ATTR_RX || it->target_attr == SVG_ATTR_RY || it->target_attr == SVG_ATTR_STROKE_WIDTH || it->target_attr == SVG_ATTR_FILL_OPACITY || it->target_attr == SVG_ATTR_GRADIENT_STOP_OPACITY || it->target_attr == SVG_ATTR_FILL || it->target_attr == SVG_ATTR_TRANSFORM)) {
                continue;
            }

            if (it->tag == SVG_TAG_ANIMATE_TRANSFORM && it->target_attr == SVG_ATTR_TRANSFORM) {
                float a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;
                const psx_svg_attr* acm = _find_attr(it->anim_node, SVG_ATTR_CALC_MODE);
                ps_bool is_discrete = (acm && acm->val_type == SVG_ATTR_VALUE_DATA && acm->value.ival == SVG_ANIMATION_CALC_MODE_DISCRETE) ? True : False;
                const psx_svg_attr* att = _find_attr(it->anim_node, SVG_ATTR_TRANSFORM_TYPE);
                int32_t ttype = (att && att->val_type == SVG_ATTR_VALUE_DATA) ? att->value.ival : 0;
                ps_bool ok2 = False;
                if (is_discrete) {
                    if (ttype == SVG_TRANSFORM_TYPE_SCALE) {
                        ok2 = _anim_eval_transform_scale_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_ROTATE) {
                        ok2 = _anim_eval_transform_rotate_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_X) {
                        ok2 = _anim_eval_transform_skewx_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_Y) {
                        ok2 = _anim_eval_transform_skewy_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else {
                        ok2 = _anim_eval_transform_translate_discrete(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    }
                } else {
                    if (ttype == SVG_TRANSFORM_TYPE_ROTATE) {
                        ok2 = _anim_eval_transform_rotate_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SCALE) {
                        ok2 = _anim_eval_transform_scale_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_X) {
                        ok2 = _anim_eval_transform_skewx_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_Y) {
                        ok2 = _anim_eval_transform_skewy_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    } else {
                        ok2 = _anim_eval_transform_translate_linear(it, p->time_sec, &a, &b, &c, &d, &e, &f);
                    }
                }
                if (ok2) {
                    _anim_state_set_transform(&p->anim_state, it->target_node, a, b, c, d, e, f);
                }
                continue;
            }

            float v = 0.0f;
            ps_bool hold = False;
            if (it->tag == SVG_TAG_SET) {
                if (_anim_eval_set(it, p->time_sec, &v, &hold)) {
                    _anim_state_set_float(&p->anim_state, it->target_node, it->target_attr, v);
                }
            } else {
                if (_anim_eval_simple(it, p->time_sec, &v, &hold)) {
                    _anim_state_set_float(&p->anim_state, it->target_node, it->target_attr, v);
                }
            }
        }
    }

    const psx_svg_node* psx_svg_player_get_node_by_id(const psx_svg_player* p, const char* id)
    {
        if (!p || !p->root || !id) {
            return NULL;
        }

        // Parser stores element id into node->content(). We do a simple DFS.
        psx_svg_node* stack[64];
        uint32_t sp = 0;
        stack[sp++] = p->root;

        while (sp) {
            psx_svg_node* n = stack[--sp];
            const char* cid = n->content(NULL);
            if (cid && strcmp(cid, id) == 0) {
                return n;
            }

            uint32_t child_count = n->child_count();
            for (uint32_t i = 0; i < child_count; i++) {
                psx_svg_node* c = n->get_child(i);
                if (c && sp < (sizeof(stack) / sizeof(stack[0]))) {
                    stack[sp++] = c;
                }
            }
        }

        return NULL;
    }

    ps_bool psx_svg_player_debug_get_float_override(const psx_svg_player* p,
                                                    const psx_svg_node* target,
                                                    psx_svg_attr_type attr,
                                                    float* out_v)
    {
        if (!p) {
            return False;
        }
        if (psx_svg_anim_get_float(&p->anim_state, target, attr, out_v)) {
            return True;
        }
        return False;
    }

    // Debug API for transform overrides.
    // Tiny 1.2 step-3: animateTransform will drive this.
    ps_bool psx_svg_player_debug_get_transform_override(const psx_svg_player* p,
                                                        const psx_svg_node* target,
                                                        float* a, float* b, float* c, float* d, float* e, float* f)
    {
        if (!p || !target) {
            if (a) { *a = 1.0f; }
            if (b) { *b = 0.0f; }
            if (c) { *c = 0.0f; }
            if (d) { *d = 1.0f; }
            if (e) { *e = 0.0f; }
            if (f) { *f = 0.0f; }
            return False;
        }

        const psx_svg_anim_transform_item* it = _anim_state_find_transform(&p->anim_state, target);
        if (!it) {
            if (a) { *a = 1.0f; }
            if (b) { *b = 0.0f; }
            if (c) { *c = 0.0f; }
            if (d) { *d = 1.0f; }
            if (e) { *e = 0.0f; }
            if (f) { *f = 0.0f; }
            return False;
        }

        if (a) { *a = it->a; }
        if (b) { *b = it->b; }
        if (c) { *c = it->c; }
        if (d) { *d = it->d; }
        if (e) { *e = it->e; }
        if (f) { *f = it->f; }
        return True;
    }

} // extern "C"
