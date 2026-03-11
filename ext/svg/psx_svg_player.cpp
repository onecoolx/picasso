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
#include "psx_svg_anim_state.h"

#include "psx_svg.h"
#include "psx_svg_parser.h"

#include <string.h>
#include <math.h>

static INLINE void _anim_state_reset(psx_svg_anim_state* s)
{
    if (!s) {
        return;
    }
    psx_array_clear(&s->overrides);
    psx_array_clear(&s->transforms);
}

static INLINE bool _is_supported_anim_attr(psx_svg_attr_type a)
{
    return a == SVG_ATTR_X || a == SVG_ATTR_Y
           || a == SVG_ATTR_WIDTH || a == SVG_ATTR_HEIGHT
           || a == SVG_ATTR_OPACITY
           || a == SVG_ATTR_RX || a == SVG_ATTR_RY
           || a == SVG_ATTR_CX || a == SVG_ATTR_CY || a == SVG_ATTR_R
           || a == SVG_ATTR_STROKE_WIDTH
           || a == SVG_ATTR_FILL_OPACITY
           || a == SVG_ATTR_GRADIENT_STOP_OPACITY
           || a == SVG_ATTR_FILL
           || a == SVG_ATTR_TRANSFORM;
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

static INLINE bool _anim_values_list_get_transform(const psx_svg_attr_values_list* list, uint32_t idx,
                                                   const float** out_vals, uint32_t* out_len)
{
    if (!out_vals || !out_len) {
        return false;
    }
    *out_vals = NULL;
    *out_len = 0;

    if (!list || idx >= list->length) {
        return false;
    }

    // Must match psx_svg_parser.cpp::_transform_values_list
    struct _anim_transform_values_list {
        uint32_t length;
        float data[6];
    };

    const struct _anim_transform_values_list* base =
        (const struct _anim_transform_values_list*)(&list->data[0]);
    const struct _anim_transform_values_list* it = base + idx;
    *out_vals = &it->data[0];
    *out_len = it->length;
    return true;
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
    return fmodf(x, y);
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

static INLINE float _attr_as_float(const psx_svg_attr* a)
{
    return a ? a->value.fval : 0.0f;
}

static INLINE uint32_t _attr_as_u32(const psx_svg_attr* a, uint32_t defv)
{
    return a ? a->value.uval : defv;
}

/*
 * Resolve the local animation time for an item at document time doc_t.
 * Returns false if the animation is not active (before begin, or past end with fill=remove).
 * On true, writes the normalized [0,1] progress into *out_t.
 * If fill=freeze applies, *out_t is clamped to 1.0.
 */
static INLINE bool _anim_resolve_local_t(const psx_svg_anim_item* it, float doc_t, float* out_t)
{
    if (!it || !out_t || it->dur_sec <= 0.0f) {
        return false;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return false;
    }

    float local = doc_t - begin_sec;

    float total = 0.0f;
    bool has_total = false;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = true;
    } else if (it->repeat_count == 0) {
        has_total = false;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = true;
    }

    if (!has_total) {
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                local = it->dur_sec;
            } else {
                return false;
            }
        } else {
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    *out_t = _anim_clampf(local / it->dur_sec, 0.0f, 1.0f);
    return true;
}

/*
 * Find the interpolation segment for normalized time t in [0,1].
 * Uses keyTimes list (kts/kt_len) if provided and matching vlist->length,
 * otherwise falls back to evenly-spaced segments.
 * Writes segment index into *out_seg, and segment bounds into *out_t0 / *out_t1.
 */
static INLINE void _anim_find_segment(const psx_svg_attr_values_list* vlist,
                                      const float* kts, uint32_t kt_len,
                                      float t,
                                      uint32_t* out_seg, float* out_t0, float* out_t1)
{
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

    *out_seg = seg;
    *out_t0 = seg_t0;
    *out_t1 = seg_t1;
}

static INLINE bool _anim_eval_simple(const psx_svg_anim_item* it, float doc_t, float* out_v, bool* out_hold)
{
    if (!it || !out_v || !out_hold) {
        return false;
    }

    *out_hold = false;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }
    if (t >= 1.0f && it->fill_mode == SVG_ANIMATION_FREEZE) {
        *out_hold = true;
    }

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length >= 1) {
            const float* vals = (const float*)&vlist->data[0];
            if (vlist->length == 1) {
                *out_v = vals[0];
                return true;
            }

            uint32_t calc_mode = SVG_ANIMATION_CALC_MODE_LINEAR;
            const psx_svg_attr* acm = _find_attr(it->anim_node, SVG_ATTR_CALC_MODE);
            if (acm && acm->val_type == SVG_ATTR_VALUE_DATA) {
                calc_mode = (uint32_t)acm->value.ival;
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

            if (calc_mode == SVG_ANIMATION_CALC_MODE_PACED) {
                // Paced: allocate time proportionally to segment length in value space.
                float total_len = 0.0f;
                for (uint32_t i = 0; i + 1 < vlist->length; i++) {
                    float d = vals[i + 1] - vals[i];
                    if (d < 0.0f) { d = -d; }
                    total_len += d;
                }

                if (total_len <= 0.0f) {
                    seg = 0;
                    seg_t0 = 0.0f;
                    seg_t1 = 1.0f;
                } else {
                    float tt = _anim_clampf(t, 0.0f, 1.0f);
                    float acc = 0.0f;
                    for (uint32_t i = 0; i + 1 < vlist->length; i++) {
                        float d = vals[i + 1] - vals[i];
                        if (d < 0.0f) { d = -d; }
                        float w = d / total_len;
                        float a = acc;
                        float b = acc + w;
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
                _anim_find_segment(vlist, kts, kt_len, t, &seg, &seg_t0, &seg_t1);
            }

            float u = 0.0f;
            if (seg_t1 > seg_t0) {
                u = (t - seg_t0) / (seg_t1 - seg_t0);
            }
            u = _anim_clampf(u, 0.0f, 1.0f);

            if (calc_mode == SVG_ANIMATION_CALC_MODE_DISCRETE) {
                *out_v = vals[seg];
            } else {
                float eased_u = u;
                if (calc_mode == SVG_ANIMATION_CALC_MODE_SPLINE) {
                    const psx_svg_attr* aks = _find_attr(it->anim_node, SVG_ATTR_KEY_SPLINES);
                    if (aks && aks->val_type == SVG_ATTR_VALUE_PTR && aks->value.val) {
                        const psx_svg_attr_values_list* kslist = (const psx_svg_attr_values_list*)aks->value.val;
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
            return true;
        }
    }

    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return false;
    }

    // animateColor: packed uint32 in float bits, discrete only.
    if (it->tag == SVG_TAG_ANIMATE_COLOR && it->target_attr == SVG_ATTR_FILL) {
        const psx_svg_attr* av = _find_attr(it->anim_node, SVG_ATTR_VALUES);
        if (av && av->val_type == SVG_ATTR_VALUE_PTR && av->value.val) {
            const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)av->value.val;
            const uint32_t* vals = (vlist->length > 0) ? (const uint32_t*)&vlist->data[0] : NULL;
            if (vals && vlist->length >= 1) {
                uint32_t idx = 0;
                if (t >= 1.0f) {
                    idx = vlist->length - 1;
                } else if (vlist->length >= 2 && t >= 0.5f) {
                    idx = 1;
                }
                *out_v = _u32_to_f32_bits(vals[idx]);
                return true;
            }
        }

        uint32_t c0 = (uint32_t)afrom->value.uval;
        uint32_t c1 = (uint32_t)ato->value.uval;
        *out_v = _u32_to_f32_bits((t < 0.5f) ? c0 : c1);
        return true;
    }

    float v0 = _attr_as_number(afrom);
    float v1 = _attr_as_number(ato);
    *out_v = _anim_lerp(v0, v1, t);
    return true;
}

static INLINE bool _anim_eval_transform_translate_discrete(const psx_svg_anim_item* it, float doc_t,
                                                           float* out_a, float* out_b, float* out_c,
                                                           float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (!avals || avals->val_type != SVG_ATTR_VALUE_PTR || !avals->value.val) {
        return false;
    }

    const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
    if (vlist->length < 1) {
        return false;
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
        return false;
    }

    *out_e = (vlen >= 1) ? base[0] : 0.0f;
    *out_f = (vlen >= 2) ? base[1] : 0.0f;
    return true;
}

static INLINE bool _anim_eval_transform_translate_linear(const psx_svg_anim_item* it, float doc_t,
                                                         float* out_a, float* out_b, float* out_c,
                                                         float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length < 1) {
            return false;
        }
        if (vlist->length == 1) {
            const float* base0 = NULL;
            uint32_t vlen0 = 0;
            if (!_anim_values_list_get_transform(vlist, 0, &base0, &vlen0) || !base0) {
                return false;
            }
            *out_e = (vlen0 >= 1) ? base0[0] : 0.0f;
            *out_f = (vlen0 >= 2) ? base0[1] : 0.0f;
            return true;
        }

        // Segment mapping via keyTimes or evenly-spaced.
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
        _anim_find_segment(vlist, kts, kt_len, t, &seg, &seg_t0, &seg_t1);

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
            return false;
        }
        if (!_anim_values_list_get_transform(vlist, seg + 1, &base1, &vlen1) || !base1) {
            return false;
        }

        float tx0 = (vlen0 >= 1) ? base0[0] : 0.0f;
        float ty0 = (vlen0 >= 2) ? base0[1] : 0.0f;
        float tx1 = (vlen1 >= 1) ? base1[0] : tx0;
        float ty1 = (vlen1 >= 2) ? base1[1] : ty0;

        *out_e = _anim_lerp(tx0, tx1, u);
        *out_f = _anim_lerp(ty0, ty1, u);
        return true;
    }

    // Fallback: from/to interpolation.
    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return false;
    }
    if (afrom->val_type != SVG_ATTR_VALUE_PTR || !afrom->value.val) {
        return false;
    }
    if (ato->val_type != SVG_ATTR_VALUE_PTR || !ato->value.val) {
        return false;
    }

    // animateTransform from/to: single _transform_values_list { length; float data[4]; }
    struct _anim_transform_values_single {
        uint32_t length;
        float data[4];
    };

    const struct _anim_transform_values_single* fromv = (const struct _anim_transform_values_single*)afrom->value.val;
    const struct _anim_transform_values_single* tov = (const struct _anim_transform_values_single*)ato->value.val;
    if (!fromv || !tov) {
        return false;
    }

    float tx0 = (fromv->length >= 1) ? fromv->data[0] : 0.0f;
    float ty0 = (fromv->length >= 2) ? fromv->data[1] : 0.0f;
    float tx1 = (tov->length >= 1) ? tov->data[0] : tx0;
    float ty1 = (tov->length >= 2) ? tov->data[1] : ty0;

    *out_e = _anim_lerp(tx0, tx1, t);
    *out_f = _anim_lerp(ty0, ty1, t);
    return true;
}

/*
 * Interpolate a single angle (degrees) from a values list or from/to pair.
 * Used by skewX and skewY linear eval.
 */
static INLINE bool _anim_eval_angle_linear(const psx_svg_anim_item* it, float t, float* out_ang)
{
    if (!it || !out_ang) {
        return false;
    }

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length >= 2) {
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
            _anim_find_segment(vlist, kts, kt_len, t, &seg, &seg_t0, &seg_t1);

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
            uint32_t vlen0 = 0, vlen1 = 0;
            if (!_anim_values_list_get_transform(vlist, seg, &base0, &vlen0) || !base0) {
                return false;
            }
            if (!_anim_values_list_get_transform(vlist, seg + 1, &base1, &vlen1) || !base1) {
                return false;
            }
            float a0 = (vlen0 >= 1) ? base0[0] : 0.0f;
            float a1 = (vlen1 >= 1) ? base1[0] : a0;
            *out_ang = _anim_lerp(a0, a1, u);
            return true;
        }
        if (vlist->length == 1) {
            const float* base0 = NULL;
            uint32_t vlen0 = 0;
            if (!_anim_values_list_get_transform(vlist, 0, &base0, &vlen0) || !base0) {
                return false;
            }
            *out_ang = (vlen0 >= 1) ? base0[0] : 0.0f;
            return true;
        }
    }

    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return false;
    }
    *out_ang = _anim_lerp(_attr_as_number(afrom), _attr_as_number(ato), t);
    return true;
}

static INLINE bool _anim_eval_transform_skewx_linear(const psx_svg_anim_item* it, float doc_t,
                                                     float* out_a, float* out_b, float* out_c,
                                                     float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }

    float ang = 0.0f;
    if (!_anim_eval_angle_linear(it, t, &ang)) {
        return false;
    }

    // skewX(a): [ 1 tan(a) 0; 0 1 0; 0 0 1 ]
    *out_c = (float)tan(ang * 0.01745329252f);
    return true;
}

static INLINE bool _anim_eval_transform_skewy_linear(const psx_svg_anim_item* it, float doc_t,
                                                     float* out_a, float* out_b, float* out_c,
                                                     float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }

    float ang = 0.0f;
    if (!_anim_eval_angle_linear(it, t, &ang)) {
        return false;
    }

    // skewY(a): [ 1 0 0; tan(a) 1 0; 0 0 1 ]
    *out_b = (float)tan(ang * 0.01745329252f);
    return true;
}

static INLINE bool _anim_eval_transform_skewx_discrete(const psx_svg_anim_item* it, float doc_t,
                                                       float* out_a, float* out_b, float* out_c,
                                                       float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }

    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return false;
    }

    float ang = (t < 0.5f) ? _attr_as_number(afrom) : _attr_as_number(ato);
    *out_c = (float)tan(ang * 0.01745329252f);
    return true;
}

static INLINE bool _anim_eval_transform_skewy_discrete(const psx_svg_anim_item* it, float doc_t,
                                                       float* out_a, float* out_b, float* out_c,
                                                       float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }

    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return false;
    }

    float ang = (t < 0.5f) ? _attr_as_number(afrom) : _attr_as_number(ato);
    *out_b = (float)tan(ang * 0.01745329252f);
    return true;
}

// matrix(a,b,c,d,e,f) linear interpolation: each of the 6 components is lerped independently.
static INLINE bool _anim_eval_transform_matrix_linear(const psx_svg_anim_item* it, float doc_t,
                                                      float* out_a, float* out_b, float* out_c,
                                                      float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }

    // values list takes priority over from/to
    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length >= 2) {
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
            _anim_find_segment(vlist, kts, kt_len, t, &seg, &seg_t0, &seg_t1);

            float u = 0.0f;
            if (seg_t1 > seg_t0) {
                u = (t - seg_t0) / (seg_t1 - seg_t0);
            }
            u = _anim_clampf(u, 0.0f, 1.0f);

            const float* v0 = NULL;
            const float* v1 = NULL;
            uint32_t len0 = 0, len1 = 0;
            if (!_anim_values_list_get_transform(vlist, seg, &v0, &len0)) {
                return false;
            }
            if (!_anim_values_list_get_transform(vlist, seg + 1, &v1, &len1)) {
                return false;
            }
            if (len0 < 6 || len1 < 6) {
                return false;
            }
            *out_a = _anim_lerp(v0[0], v1[0], u);
            *out_b = _anim_lerp(v0[1], v1[1], u);
            *out_c = _anim_lerp(v0[2], v1[2], u);
            *out_d = _anim_lerp(v0[3], v1[3], u);
            *out_e = _anim_lerp(v0[4], v1[4], u);
            *out_f = _anim_lerp(v0[5], v1[5], u);
            return true;
        }
    }

    // from/to
    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return false;
    }
    if (afrom->val_type != SVG_ATTR_VALUE_PTR || !afrom->value.val) {
        return false;
    }
    if (ato->val_type != SVG_ATTR_VALUE_PTR || !ato->value.val) {
        return false;
    }

    const psx_svg_attr_values_list* fl = (const psx_svg_attr_values_list*)afrom->value.val;
    const psx_svg_attr_values_list* tl = (const psx_svg_attr_values_list*)ato->value.val;
    if (fl->length < 6 || tl->length < 6) {
        return false;
    }
    const float* fv = (const float*)&fl->data[0];
    const float* tv = (const float*)&tl->data[0];

    *out_a = _anim_lerp(fv[0], tv[0], t);
    *out_b = _anim_lerp(fv[1], tv[1], t);
    *out_c = _anim_lerp(fv[2], tv[2], t);
    *out_d = _anim_lerp(fv[3], tv[3], t);
    *out_e = _anim_lerp(fv[4], tv[4], t);
    *out_f = _anim_lerp(fv[5], tv[5], t);
    return true;
}

// matrix(a,b,c,d,e,f) discrete: snap to from or to based on t < 0.5.
static INLINE bool _anim_eval_transform_matrix_discrete(const psx_svg_anim_item* it, float doc_t,
                                                        float* out_a, float* out_b, float* out_c,
                                                        float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }

    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return false;
    }
    if (afrom->val_type != SVG_ATTR_VALUE_PTR || !afrom->value.val) {
        return false;
    }
    if (ato->val_type != SVG_ATTR_VALUE_PTR || !ato->value.val) {
        return false;
    }

    const psx_svg_attr_values_list* src = (t < 0.5f)
                                          ? (const psx_svg_attr_values_list*)afrom->value.val
                                          : (const psx_svg_attr_values_list*)ato->value.val;

    if (src->length < 6) {
        return false;
    }
    const float* sv = (const float*)&src->data[0];
    *out_a = sv[0];
    *out_b = sv[1];
    *out_c = sv[2];
    *out_d = sv[3];
    *out_e = sv[4];
    *out_f = sv[5];
    return true;
}

static INLINE bool _anim_eval_transform_scale_discrete(const psx_svg_anim_item* it, float doc_t,
                                                       float* out_a, float* out_b, float* out_c,
                                                       float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (!avals || avals->val_type != SVG_ATTR_VALUE_PTR || !avals->value.val) {
        return false;
    }

    const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
    if (vlist->length < 1) {
        return false;
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
        return false;
    }

    float sx = (vlen >= 1) ? base[0] : 1.0f;
    float sy = (vlen >= 2) ? base[1] : sx;
    *out_a = sx;
    *out_d = sy;
    return true;
}

static INLINE bool _anim_eval_transform_scale_linear(const psx_svg_anim_item* it, float doc_t,
                                                     float* out_a, float* out_b, float* out_c,
                                                     float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length < 1) {
            return false;
        }
        if (vlist->length == 1) {
            const float* base0 = NULL;
            uint32_t vlen0 = 0;
            if (!_anim_values_list_get_transform(vlist, 0, &base0, &vlen0) || !base0) {
                return false;
            }
            float sx = (vlen0 >= 1) ? base0[0] : 1.0f;
            float sy = (vlen0 >= 2) ? base0[1] : sx;
            *out_a = sx;
            *out_d = sy;
            return true;
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
        _anim_find_segment(vlist, kts, kt_len, t, &seg, &seg_t0, &seg_t1);

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
            return false;
        }
        if (!_anim_values_list_get_transform(vlist, seg + 1, &base1, &vlen1) || !base1) {
            return false;
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
        return true;
    }

    // Fallback: from/to interpolation if values is absent.
    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return false;
    }
    if (afrom->val_type != SVG_ATTR_VALUE_PTR || !afrom->value.val) {
        return false;
    }
    if (ato->val_type != SVG_ATTR_VALUE_PTR || !ato->value.val) {
        return false;
    }

    struct _anim_transform_values_single {
        uint32_t length;
        float data[4];
    };

    const struct _anim_transform_values_single* fromv = (const struct _anim_transform_values_single*)afrom->value.val;
    const struct _anim_transform_values_single* tov = (const struct _anim_transform_values_single*)ato->value.val;
    if (!fromv || !tov) {
        return false;
    }

    float sx0 = (fromv->length >= 1) ? fromv->data[0] : 1.0f;
    float sy0 = (fromv->length >= 2) ? fromv->data[1] : sx0;
    float sx1 = (tov->length >= 1) ? tov->data[0] : sx0;
    float sy1 = (tov->length >= 2) ? tov->data[1] : sy0;

    *out_a = _anim_lerp(sx0, sx1, t);
    *out_d = _anim_lerp(sy0, sy1, t);
    return true;
}

static INLINE bool _anim_eval_transform_rotate_discrete(const psx_svg_anim_item* it, float doc_t,
                                                        float* out_a, float* out_b, float* out_c,
                                                        float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (!avals || avals->val_type != SVG_ATTR_VALUE_PTR || !avals->value.val) {
        return false;
    }
    const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
    if (vlist->length < 1) {
        return false;
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
        return false;
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
    return true;
}

static INLINE bool _anim_eval_transform_rotate_linear(const psx_svg_anim_item* it, float doc_t,
                                                      float* out_a, float* out_b, float* out_c,
                                                      float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t)) {
        return false;
    }

    const psx_svg_attr* avals = _find_attr(it->anim_node, SVG_ATTR_VALUES);
    if (!avals || avals->val_type != SVG_ATTR_VALUE_PTR || !avals->value.val) {
        return false;
    }

    const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
    if (vlist->length < 1) {
        return false;
    }
    if (vlist->length == 1) {
        const float* base0 = NULL;
        uint32_t vlen0 = 0;
        if (!_anim_values_list_get_transform(vlist, 0, &base0, &vlen0) || !base0) {
            return false;
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
        return true;
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
    _anim_find_segment(vlist, kts, kt_len, t, &seg, &seg_t0, &seg_t1);

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
        return false;
    }
    if (!_anim_values_list_get_transform(vlist, seg + 1, &base1, &vlen1) || !base1) {
        return false;
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
    return true;
}

/*
 * Dispatch animateTransform evaluation based on transform type and calcMode.
 * Returns true and writes the matrix components if the animation is active.
 */
static INLINE bool _anim_eval_transform_dispatch(const psx_svg_anim_item* it, float time_sec,
                                                 float* a, float* b, float* c,
                                                 float* d, float* e, float* f)
{
    const psx_svg_attr* acm = _find_attr(it->anim_node, SVG_ATTR_CALC_MODE);
    bool is_discrete = (acm && acm->val_type == SVG_ATTR_VALUE_DATA && acm->value.ival == SVG_ANIMATION_CALC_MODE_DISCRETE) ? true : false;
    const psx_svg_attr* att = _find_attr(it->anim_node, SVG_ATTR_TRANSFORM_TYPE);
    int32_t ttype = (att && att->val_type == SVG_ATTR_VALUE_DATA) ? att->value.ival : 0;

    if (is_discrete) {
        if (ttype == SVG_TRANSFORM_TYPE_SCALE) {
            return _anim_eval_transform_scale_discrete(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_ROTATE) {
            return _anim_eval_transform_rotate_discrete(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_X) {
            return _anim_eval_transform_skewx_discrete(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_Y) {
            return _anim_eval_transform_skewy_discrete(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_MATRIX) {
            return _anim_eval_transform_matrix_discrete(it, time_sec, a, b, c, d, e, f);
        } else {
            return _anim_eval_transform_translate_discrete(it, time_sec, a, b, c, d, e, f);
        }
    } else {
        if (ttype == SVG_TRANSFORM_TYPE_ROTATE) {
            return _anim_eval_transform_rotate_linear(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_SCALE) {
            return _anim_eval_transform_scale_linear(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_X) {
            return _anim_eval_transform_skewx_linear(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_Y) {
            return _anim_eval_transform_skewy_linear(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_MATRIX) {
            return _anim_eval_transform_matrix_linear(it, time_sec, a, b, c, d, e, f);
        } else {
            return _anim_eval_transform_translate_linear(it, time_sec, a, b, c, d, e, f);
        }
    }
}

// Note: psx_svg_attr_values_list is a variable-sized blob (length + data[]).
// The element layout depends on which attribute it represents.

static INLINE bool _anim_eval_set(const psx_svg_anim_item* it, float doc_t, float* out_v, bool* out_hold)
{
    if (!it || !out_v || !out_hold) {
        return false;
    }
    *out_hold = false;

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return false;
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
            *out_hold = true;
            // still return the frozen end value
            const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
            if (!ato) {
                return false;
            }
            *out_v = _attr_as_number(ato);
            return true;
        }
        return false;
    }

    // If dur is missing/0, SVG <set> is treated as an instant change.
    if (it->dur_sec <= 0.0f) {
        const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
        if (!ato) {
            return false;
        }
        *out_v = _attr_as_number(ato);
        // end instant handling
        if (end_sec > 0.0f && doc_t == end_sec) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = true;
                return true;
            }
            if (it->fill_mode == SVG_ANIMATION_REMOVE) {
                return false;
            }
        }
        return true;
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
            return false;
        }

        // Outside active interval
        if (doc_t > end_sec) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = true;
                // treat as at end
                local = active_dur;
            } else {
                return false;
            }
        }

        // end instant
        if (doc_t == end_sec) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = true;
                local = active_dur;
            } else if (it->fill_mode == SVG_ANIMATION_REMOVE) {
                return false;
            }
        }

        // When explicit end is present, it defines the end of the active
        // interval (regardless of declared dur).
        if (local > active_dur) {
            return false;
        }
    }

    // For <set>, value is constant during the active interval.
    // Do NOT wrap with fmod: repeats don't change the value and wrapping breaks
    // fill=remove semantics at end-of-interval.
    if (it->repeat_count != 0) {
        float total = it->dur_sec * (float)it->repeat_count;
        if (local > total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = true;
                // Hold at end value.
                local = it->dur_sec;
            } else {
                return false;
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
            return false;
        }

        // If fill=freeze, keep value visible at the end instant.
        if (local == it->dur_sec) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = true;
            } else if (it->fill_mode == SVG_ANIMATION_REMOVE) {
                // end instant is not active for remove
                return false;
            }
        }
    }

    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!ato) {
        return false;
    }
    *out_v = _attr_as_number(ato);
    return true;
}

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
                    const psx_svg_attr_values_list* list = (const psx_svg_attr_values_list*)abegin->value.val;
                    const float* vals = (list->length > 0) ? (const float*)&list->data[0] : NULL;
                    for (uint32_t i = 0; i < list->length; i++) {
                        float ms = vals[i];
                        float sec = (ms <= 0.0f) ? 0.0f : (ms * 0.001f);
                        psx_array_append(&item.begins_sec, &sec);
                    }
                    _anim_item_begin_list_normalize(&item);
                    item.begin_sec = _attr_as_begin_time_sec(abegin);
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

#ifdef __cplusplus
extern "C" {
#endif

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

// Returns a pointer to the internal scratch ps_matrix if a transform override
// exists for target, NULL otherwise. The caller must NOT unref the returned pointer.
const ps_matrix* psx_svg_anim_get_transform(const psx_svg_anim_state* s,
                                            const psx_svg_node* target)
{
    const psx_svg_anim_transform_item* it = _anim_state_find_transform(s, target);
    if (!it) {
        return NULL;
    }
    // Reuse the scratch matrix: init in-place with the stored SVG matrix components.
    // ps_matrix_init(sx, shy, shx, sy, tx, ty) maps to SVG matrix(a,b,c,d,e,f).
    ps_matrix_init(s->scratch_matrix, it->a, it->b, it->c, it->d, it->e, it->f);
    return s->scratch_matrix;
}

const psx_svg_anim_transform_item* psx_svg_anim_state_find_transform(const psx_svg_anim_state* s, const psx_svg_node* target)
{
    return _anim_state_find_transform(s, target);
}

psx_svg_player* psx_svg_player_create(const psx_svg_node* root, psx_result* out)
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

    psx_svg_player* p = (psx_svg_player*)mem_malloc(sizeof(psx_svg_player));
    if (!p) {
        if (out) {
            *out = S_OUT_OF_MEMORY;
        }
        return NULL;
    }
    memset(p, 0, sizeof(psx_svg_player));

    p->root = root;
    p->loop = false;
    p->dpi = 96; // default dpi

    p->render_list = psx_svg_render_list_create(root);
    if (!p->render_list) {
        if (out) {
            *out = S_FAILURE;
        }
        mem_free(p);
        return NULL;
    }

    p->state = PSX_SVG_PLAYER_STOPPED;
    p->time_sec = 0.0f;
    p->duration_sec = -1.0f;

    psx_array_init(&p->anim_state.overrides, sizeof(psx_svg_anim_override_item));
    psx_array_init(&p->anim_state.transforms, sizeof(psx_svg_anim_transform_item));
    p->anim_state.scratch_matrix = ps_matrix_create();

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

    if (p->anim_state.scratch_matrix) {
        ps_matrix_unref(p->anim_state.scratch_matrix);
        p->anim_state.scratch_matrix = NULL;
    }

    psx_array_destroy(&p->anims);

    if (p->render_list) {
        psx_svg_render_list_destroy(p->render_list);
        p->render_list = NULL;
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
        if (!_is_supported_anim_attr(it->target_attr)) {
            continue;
        }

        if (it->tag == SVG_TAG_ANIMATE_TRANSFORM && it->target_attr == SVG_ATTR_TRANSFORM) {
            float a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;
            if (_anim_eval_transform_dispatch(it, p->time_sec, &a, &b, &c, &d, &e, &f)) {
                _anim_state_set_transform(&p->anim_state, it->target_node, a, b, c, d, e, f);
            }
            continue;
        }

        float v = 0.0f;
        bool hold = false;
        bool ok = false;
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
        if (!_is_supported_anim_attr(it->target_attr)) {
            continue;
        }

        if (it->tag == SVG_TAG_ANIMATE_TRANSFORM && it->target_attr == SVG_ATTR_TRANSFORM) {
            float a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;
            if (_anim_eval_transform_dispatch(it, p->time_sec, &a, &b, &c, &d, &e, &f)) {
                _anim_state_set_transform(&p->anim_state, it->target_node, a, b, c, d, e, f);
            }
            continue;
        }

        float v = 0.0f;
        bool hold = false;
        bool ok = false;
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

void psx_svg_player_set_loop(psx_svg_player* p, bool loop)
{
    if (!p) {
        return;
    }
    p->loop = loop;
}

bool psx_svg_player_get_loop(const psx_svg_player* p)
{
    return p ? p->loop : false;
}

void psx_svg_player_set_dpi(psx_svg_player* p, int32_t dpi)
{
    if (!p) {
        return;
    }
    p->dpi = dpi;
}

int32_t psx_svg_player_get_dpi(const psx_svg_player* p)
{
    return p ? p->dpi : 0;
}

void psx_svg_player_draw(psx_svg_player* p, ps_context* ctx)
{
    if (!p || !ctx || !p->render_list) {
        return;
    }

    // Ensure animation state is applied.
    psx_svg_render_list_draw_anim(ctx, p->render_list, &p->anim_state);
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
    bool any = false;
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
        any = true;
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
        if (!_is_supported_anim_attr(it->target_attr)) {
            continue;
        }

        if (it->tag == SVG_TAG_ANIMATE_TRANSFORM && it->target_attr == SVG_ATTR_TRANSFORM) {
            float a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;
            if (_anim_eval_transform_dispatch(it, p->time_sec, &a, &b, &c, &d, &e, &f)) {
                _anim_state_set_transform(&p->anim_state, it->target_node, a, b, c, d, e, f);
            }
            continue;
        }

        float v = 0.0f;
        bool hold = false;
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

struct _find_by_id_ctx {
    const char* id;
    const psx_svg_node* result;
};

static bool _find_by_id_cb(const psx_tree_node* node, void* ctx)
{
    _find_by_id_ctx* c = (_find_by_id_ctx*)ctx;
    const psx_svg_node* sn = (const psx_svg_node*)node;
    const char* cid = sn->content(NULL);
    if (cid && strcmp(cid, c->id) == 0) {
        c->result = sn;
        return false;
    }
    return true;
}

const psx_svg_node* psx_svg_player_get_node_by_id(const psx_svg_player* p, const char* id)
{
    if (!p || !p->root || !id) {
        return NULL;
    }

    _find_by_id_ctx ctx;
    ctx.id = id;
    ctx.result = NULL;
    psx_tree_traversal<PSX_TREE_WALK_PRE_ORDER>(p->root, &ctx, _find_by_id_cb);
    return ctx.result;
}

#ifdef __cplusplus
}
#endif
