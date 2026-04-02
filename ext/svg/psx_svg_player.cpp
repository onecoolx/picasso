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

#include "psx_svg_animation.h"
#include "psx_svg_anim_state.h"

#include "psx_common.h"
#include "psx_svg_node.h"
#include "psx_svg_render.h"
#include "psx_svg_parser.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

static INLINE void _anim_state_reset(psx_svg_anim_state* s)
{
    if (!s) {
        return;
    }
    psx_array_clear(&s->overrides);
    psx_array_clear(&s->transforms);
    psx_array_clear(&s->motion_transforms);
    /* Keep dash buffers alive for reuse — just reset size, not free. */
    s->dash_overrides.size = 0;
    psx_array_clear(&s->color_overrides);
    s->active_target_count = 0;
}

/* --- O(1) bitmap attribute lookup (replaces long-chain conditionals) --- */

#define _ATTR_BITMAP_WORDS 3  /* uint32_t[3] = 96 bits, covers enum 0..95 */

/*
 * Bitmap values computed from psx_svg_attr_type enum.
 * Formula: bitmap[attr >> 5] |= (1u << (attr & 31))
 */
static const uint32_t _supported_anim_attr_bitmap[_ATTR_BITMAP_WORDS] = {
    0xF87FFF00u, /* bits 8-22,27-31: DISPLAY..Y2, FILL..STROKE_WIDTH */
    0x00075C7Fu, /* bits 32-38,42-44,46,48-50: LINECAP..OPACITY, STOP_OFFSET..STOP_OPACITY, FONT_STYLE, FONT_WEIGHT..TRANSFORM */
    0x00000000u
};

static const uint32_t _int32_anim_attr_bitmap[_ATTR_BITMAP_WORDS] = {
    0x10000300u, /* bits 8,9,28: DISPLAY, VISIBILITY, FILL_RULE */
    0x00014003u, /* bits 32,33,46,48: LINECAP, LINEJOIN, FONT_STYLE, FONT_WEIGHT */
    0x00000000u
};

static const uint32_t _color_anim_attr_bitmap[_ATTR_BITMAP_WORDS] = {
    0x48000000u, /* bits 27,30: FILL, STROKE */
    0x00000800u, /* bit 43: GRADIENT_STOP_COLOR */
    0x00000000u
};

static INLINE bool _is_supported_anim_attr(psx_svg_attr_type a)
{
    int i = (int)a;
    if (i < 0 || i >= _ATTR_BITMAP_WORDS * 32) {
        return false;
    }
    return (_supported_anim_attr_bitmap[(unsigned)i >> 5] & (1u << ((unsigned)i & 31))) != 0;
}

static INLINE bool _is_int32_anim_attr(psx_svg_attr_type a)
{
    int i = (int)a;
    if (i < 0 || i >= _ATTR_BITMAP_WORDS * 32) {
        return false;
    }
    return (_int32_anim_attr_bitmap[(unsigned)i >> 5] & (1u << ((unsigned)i & 31))) != 0;
}

static INLINE bool _is_color_anim_attr(psx_svg_attr_type a)
{
    int i = (int)a;
    if (i < 0 || i >= _ATTR_BITMAP_WORDS * 32) {
        return false;
    }
    return (_color_anim_attr_bitmap[(unsigned)i >> 5] & (1u << ((unsigned)i & 31))) != 0;
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

static INLINE void _anim_state_compose_transform(psx_svg_anim_state* s, const psx_svg_node* target,
                                                 float na, float nb, float nc, float nd, float ne, float nf)
{
    if (!s || !target) {
        return;
    }

    uint32_t n = psx_array_size(&s->transforms);
    for (uint32_t i = 0; i < n; i++) {
        psx_svg_anim_transform_item* it = psx_array_get(&s->transforms, i, psx_svg_anim_transform_item);
        if (it && it->target == target) {
            float oa = it->a, ob = it->b, oc = it->c, od = it->d, oe = it->e, of = it->f;
            it->a = oa * na + oc * nb;
            it->b = ob * na + od * nb;
            it->c = oa * nc + oc * nd;
            it->d = ob * nc + od * nd;
            it->e = oa * ne + oc * nf + oe;
            it->f = ob * ne + od * nf + of;
            return;
        }
    }

    _anim_state_set_transform(s, target, na, nb, nc, nd, ne, nf);
}

static INLINE const psx_svg_anim_transform_item* _anim_state_find_transform(const psx_svg_anim_state* s, const psx_svg_node* target)
{
    if (!s || !target) {
        return NULL;
    }
    uint32_t n = psx_array_size((psx_array*)&s->transforms);
    if (n == 0) {
        return NULL;
    }
    const psx_svg_anim_transform_item* arr = (const psx_svg_anim_transform_item*)s->transforms.data;
    uintptr_t key = (uintptr_t)target;
    uint32_t lo = 0;
    uint32_t hi = n;
    while (lo < hi) {
        uint32_t mid = lo + (hi - lo) / 2;
        uintptr_t mt = (uintptr_t)arr[mid].target;
        if (mt < key) {
            lo = mid + 1;
        } else if (mt > key) {
            hi = mid;
        } else {
            return &arr[mid];
        }
    }
    return NULL;
}

static INLINE void _anim_state_set_motion_transform(psx_svg_anim_state* s, const psx_svg_node* target,
                                                    float a, float b, float c, float d, float e, float f)
{
    if (!s || !target) {
        return;
    }

    uint32_t n = psx_array_size(&s->motion_transforms);
    for (uint32_t i = 0; i < n; i++) {
        psx_svg_anim_transform_item* it = psx_array_get(&s->motion_transforms, i, psx_svg_anim_transform_item);
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

    psx_array_append(&s->motion_transforms, NULL);
    psx_svg_anim_transform_item* dst = psx_array_get_last(&s->motion_transforms, psx_svg_anim_transform_item);
    dst->target = target;
    dst->a = a;
    dst->b = b;
    dst->c = c;
    dst->d = d;
    dst->e = e;
    dst->f = f;
}

static INLINE const psx_svg_anim_transform_item* _anim_state_find_motion_transform(const psx_svg_anim_state* s, const psx_svg_node* target)
{
    if (!s || !target) {
        return NULL;
    }
    uint32_t n = psx_array_size((psx_array*)&s->motion_transforms);
    if (n == 0) {
        return NULL;
    }
    const psx_svg_anim_transform_item* arr = (const psx_svg_anim_transform_item*)s->motion_transforms.data;
    uintptr_t key = (uintptr_t)target;
    uint32_t lo = 0;
    uint32_t hi = n;
    while (lo < hi) {
        uint32_t mid = lo + (hi - lo) / 2;
        uintptr_t mt = (uintptr_t)arr[mid].target;
        if (mt < key) {
            lo = mid + 1;
        } else if (mt > key) {
            hi = mid;
        } else {
            return &arr[mid];
        }
    }
    return NULL;
}

/* ── SBO (Small Buffer Optimization) for float lists ── */

static INLINE int _float_cmp_asc_fn(const void* a, const void* b)
{
    const float fa = *(const float*)a;
    const float fb = *(const float*)b;
    if (fa < fb) { return -1; }
    if (fa > fb) { return 1; }
    return 0;
}

/* _sbo_float_list typedef is in psx_svg_anim_state.h */

static INLINE void _sbo_init(_sbo_float_list* l)
{
    l->inline_val = 0.0f;
    l->heap_data = NULL;
    l->count = 0;
    l->heap_cap = 0;
}

static INLINE void _sbo_destroy(_sbo_float_list* l)
{
    if (l->heap_data) {
        mem_free(l->heap_data);
        l->heap_data = NULL;
    }
    l->count = 0;
    l->heap_cap = 0;
}

static INLINE void _sbo_push(_sbo_float_list* l, float val)
{
    if (l->count == 0) {
        l->inline_val = val;
        l->count = 1;
    } else if (l->count == 1) {
        uint32_t cap = 4;
        float* buf = (float*)mem_malloc(cap * sizeof(float));
        if (!buf) { return; }
        buf[0] = l->inline_val;
        buf[1] = val;
        l->heap_data = buf;
        l->heap_cap = cap;
        l->count = 2;
    } else {
        if (l->count >= l->heap_cap) {
            uint32_t new_cap = l->heap_cap * 2;
            float* buf = (float*)mem_realloc(l->heap_data, new_cap * sizeof(float));
            if (!buf) { return; }
            l->heap_data = buf;
            l->heap_cap = new_cap;
        }
        l->heap_data[l->count] = val;
        l->count++;
    }
}

static INLINE uint32_t _sbo_size(const _sbo_float_list* l)
{
    return l->count;
}

static INLINE float* _sbo_get(_sbo_float_list* l, uint32_t idx)
{
    if (l->count <= 1 && idx == 0) {
        return &l->inline_val;
    }
    if (l->count > 1 && idx < l->count) {
        return &l->heap_data[idx];
    }
    return NULL;
}

static INLINE const float* _sbo_get_const(const _sbo_float_list* l, uint32_t idx)
{
    if (l->count <= 1 && idx == 0) {
        return &l->inline_val;
    }
    if (l->count > 1 && idx < l->count) {
        return &l->heap_data[idx];
    }
    return NULL;
}

static INLINE void _sbo_clear(_sbo_float_list* l)
{
    l->count = 0;
}

static INLINE bool _sbo_empty(const _sbo_float_list* l)
{
    return l->count == 0;
}

static INLINE void _sbo_sort_dedup(_sbo_float_list* l)
{
    if (l->count <= 1) {
        return;
    }
    /* sort */
    qsort(l->heap_data, l->count, sizeof(float), _float_cmp_asc_fn);
    /* dedup */
    float* d = l->heap_data;
    uint32_t w = 1;
    for (uint32_t r = 1; r < l->count; r++) {
        if (d[r] != d[w - 1]) {
            d[w++] = d[r];
        }
    }
    l->count = w;
}

/* ── End / Begin list helpers (SBO-backed) ── */

static INLINE void _anim_item_end_list_init(psx_svg_anim_item* it)
{
    if (!it) {
        return;
    }
    _sbo_init(&it->ends_sec);
}

static INLINE void _anim_item_end_list_destroy(psx_svg_anim_item* it)
{
    if (!it) {
        return;
    }
    _sbo_destroy(&it->ends_sec);
}

static INLINE void _anim_item_end_list_normalize(psx_svg_anim_item* it)
{
    if (!it) {
        return;
    }
    _sbo_sort_dedup(&it->ends_sec);
}

static INLINE float _anim_item_end_for_begin(const psx_svg_anim_item* it, float begin_sec)
{
    if (!it) {
        return 0.0f;
    }
    uint32_t n = _sbo_size(&it->ends_sec);
    if (n == 0) {
        return 0.0f;
    }
    for (uint32_t i = 0; i < n; i++) {
        const float* ep = _sbo_get_const(&it->ends_sec, i);
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
    psx_array_append(&s->overrides, NULL);
    psx_svg_anim_override_item* dst = psx_array_get_last(&s->overrides, psx_svg_anim_override_item);
    dst->target = target;
    dst->attr = attr;
    dst->u.fval = v;
}

static INLINE void _anim_state_set_int32(psx_svg_anim_state* s, const psx_svg_node* target, psx_svg_attr_type attr, int32_t v)
{
    if (!s || !target || attr == SVG_ATTR_INVALID) {
        return;
    }
    psx_array_append(&s->overrides, NULL);
    psx_svg_anim_override_item* dst = psx_array_get_last(&s->overrides, psx_svg_anim_override_item);
    dst->target = target;
    dst->attr = attr;
    dst->u.ival = v;
}

static INLINE void _anim_state_set_color(psx_svg_anim_state* s, const psx_svg_node* target, psx_svg_attr_type attr, uint32_t color)
{
    if (!s || !target || attr == SVG_ATTR_INVALID) {
        return;
    }
    psx_array_append(&s->color_overrides, NULL);
    psx_svg_anim_color_item* dst = psx_array_get_last(&s->color_overrides, psx_svg_anim_color_item);
    dst->target = target;
    dst->attr = attr;
    dst->color = color;
}

static int _anim_override_cmp(const void* a, const void* b)
{
    const psx_svg_anim_override_item* aa = (const psx_svg_anim_override_item*)a;
    const psx_svg_anim_override_item* bb = (const psx_svg_anim_override_item*)b;
    uintptr_t ta = (uintptr_t)aa->target;
    uintptr_t tb = (uintptr_t)bb->target;
    if (ta < tb) {
        return -1;
    }
    if (ta > tb) {
        return 1;
    }
    if (aa->attr < bb->attr) {
        return -1;
    }
    if (aa->attr > bb->attr) {
        return 1;
    }
    return 0;
}

static int _anim_color_override_cmp(const void* a, const void* b)
{
    const psx_svg_anim_color_item* aa = (const psx_svg_anim_color_item*)a;
    const psx_svg_anim_color_item* bb = (const psx_svg_anim_color_item*)b;
    uintptr_t ta = (uintptr_t)aa->target;
    uintptr_t tb = (uintptr_t)bb->target;
    if (ta < tb) {
        return -1;
    }
    if (ta > tb) {
        return 1;
    }
    if (aa->attr < bb->attr) {
        return -1;
    }
    if (aa->attr > bb->attr) {
        return 1;
    }
    return 0;
}

#define INSERTION_SORT_THRESHOLD 16

/* ── Transform array sort (by target pointer) ── */

static int _anim_transform_cmp(const void* a, const void* b)
{
    const psx_svg_anim_transform_item* aa = (const psx_svg_anim_transform_item*)a;
    const psx_svg_anim_transform_item* bb = (const psx_svg_anim_transform_item*)b;
    uintptr_t ta = (uintptr_t)aa->target;
    uintptr_t tb = (uintptr_t)bb->target;
    if (ta < tb) { return -1; }
    if (ta > tb) { return 1; }
    return 0;
}

static INLINE void _anim_state_sort_transforms(psx_array* arr)
{
    uint32_t size = psx_array_size(arr);
    if (size < 2) {
        return;
    }
    if (size <= INSERTION_SORT_THRESHOLD) {
        psx_svg_anim_transform_item* items = (psx_svg_anim_transform_item*)arr->data;
        uint32_t i;
        for (i = 1; i < size; i++) {
            psx_svg_anim_transform_item key = items[i];
            uintptr_t kt = (uintptr_t)key.target;
            int32_t j = (int32_t)i - 1;
            while (j >= 0) {
                uintptr_t jt = (uintptr_t)items[j].target;
                if (jt > kt) {
                    items[j + 1] = items[j];
                    j--;
                } else {
                    break;
                }
            }
            items[j + 1] = key;
        }
    } else {
        qsort(arr->data, arr->size, arr->element_size, _anim_transform_cmp);
    }
}

static INLINE void _anim_state_sort_overrides(psx_svg_anim_state* s)
{
    if (!s) {
        return;
    }
    uint32_t size = psx_array_size(&s->overrides);
    if (size < 2) {
        return;
    }
    if (size <= INSERTION_SORT_THRESHOLD) {
        psx_svg_anim_override_item* arr = (psx_svg_anim_override_item*)s->overrides.data;
        uint32_t i;
        for (i = 1; i < size; i++) {
            psx_svg_anim_override_item key = arr[i];
            uintptr_t kt = (uintptr_t)key.target;
            int32_t ka = (int32_t)key.attr;
            int32_t j = (int32_t)i - 1;
            while (j >= 0) {
                uintptr_t jt = (uintptr_t)arr[j].target;
                if (jt > kt || (jt == kt && (int32_t)arr[j].attr > ka)) {
                    arr[j + 1] = arr[j];
                    j--;
                } else {
                    break;
                }
            }
            arr[j + 1] = key;
        }
    } else {
        qsort(s->overrides.data, s->overrides.size, s->overrides.element_size, _anim_override_cmp);
    }

    /* Post-sort dedup: keep last entry for each (target, attr) group */
    {
        psx_svg_anim_override_item* items = (psx_svg_anim_override_item*)s->overrides.data;
        uint32_t w = 0;
        uint32_t r;
        for (r = 1; r < s->overrides.size; r++) {
            if (items[r].target != items[w].target || items[r].attr != items[w].attr) {
                w++;
                if (w != r) {
                    items[w] = items[r];
                }
            } else {
                /* same key — overwrite w with r (last-writer-wins) */
                items[w] = items[r];
            }
        }
        s->overrides.size = w + 1;
    }
}

static INLINE void _anim_state_sort_color_overrides(psx_svg_anim_state* s)
{
    if (!s) {
        return;
    }
    uint32_t size = psx_array_size(&s->color_overrides);
    if (size < 2) {
        return;
    }
    if (size <= INSERTION_SORT_THRESHOLD) {
        psx_svg_anim_color_item* arr = (psx_svg_anim_color_item*)s->color_overrides.data;
        uint32_t i;
        for (i = 1; i < size; i++) {
            psx_svg_anim_color_item key = arr[i];
            uintptr_t kt = (uintptr_t)key.target;
            int32_t ka = (int32_t)key.attr;
            int32_t j = (int32_t)i - 1;
            while (j >= 0) {
                uintptr_t jt = (uintptr_t)arr[j].target;
                if (jt > kt || (jt == kt && (int32_t)arr[j].attr > ka)) {
                    arr[j + 1] = arr[j];
                    j--;
                } else {
                    break;
                }
            }
            arr[j + 1] = key;
        }
    } else {
        qsort(s->color_overrides.data, s->color_overrides.size, s->color_overrides.element_size, _anim_color_override_cmp);
    }

    /* Post-sort dedup: keep last entry for each (target, attr) group */
    {
        psx_svg_anim_color_item* items = (psx_svg_anim_color_item*)s->color_overrides.data;
        uint32_t w = 0;
        uint32_t r;
        for (r = 1; r < s->color_overrides.size; r++) {
            if (items[r].target != items[w].target || items[r].attr != items[w].attr) {
                w++;
                if (w != r) {
                    items[w] = items[r];
                }
            } else {
                /* same key — overwrite w with r (last-writer-wins) */
                items[w] = items[r];
            }
        }
        s->color_overrides.size = w + 1;
    }
}

static INLINE const psx_svg_anim_override_item* _anim_state_find(const psx_svg_anim_state* s, const psx_svg_node* target, psx_svg_attr_type attr)
{
    if (!s || !target || attr == SVG_ATTR_INVALID) {
        return NULL;
    }
    uint32_t n = psx_array_size((psx_array*)&s->overrides);
    if (n == 0) {
        return NULL;
    }

    uintptr_t key_target = (uintptr_t)target;
    int32_t key_attr = (int32_t)attr;
    uint32_t lo = 0;
    uint32_t hi = n;

    while (lo < hi) {
        uint32_t mid = lo + ((hi - lo) >> 1);
        const psx_svg_anim_override_item* it = psx_array_get((psx_array*)&s->overrides, mid, psx_svg_anim_override_item);
        uintptr_t t = (uintptr_t)it->target;
        if (t < key_target) {
            lo = mid + 1;
        } else if (t > key_target) {
            hi = mid;
        } else {
            int32_t a = (int32_t)it->attr;
            if (a < key_attr) {
                lo = mid + 1;
            } else if (a > key_attr) {
                hi = mid;
            } else {
                return it;
            }
        }
    }
    return NULL;
}

static INLINE const psx_svg_anim_color_item* _anim_state_find_color(const psx_svg_anim_state* s, const psx_svg_node* target, psx_svg_attr_type attr)
{
    if (!s || !target || attr == SVG_ATTR_INVALID) {
        return NULL;
    }
    uint32_t n = psx_array_size((psx_array*)&s->color_overrides);
    if (n == 0) {
        return NULL;
    }

    uintptr_t key_target = (uintptr_t)target;
    int32_t key_attr = (int32_t)attr;
    uint32_t lo = 0;
    uint32_t hi = n;

    while (lo < hi) {
        uint32_t mid = lo + ((hi - lo) >> 1);
        const psx_svg_anim_color_item* it = psx_array_get((psx_array*)&s->color_overrides, mid, psx_svg_anim_color_item);
        uintptr_t t = (uintptr_t)it->target;
        if (t < key_target) {
            lo = mid + 1;
        } else if (t > key_target) {
            hi = mid;
        } else {
            int32_t a = (int32_t)it->attr;
            if (a < key_attr) {
                lo = mid + 1;
            } else if (a > key_attr) {
                hi = mid;
            } else {
                return it;
            }
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
    return psx_svg_attr_values_get_transform_entry(list, idx, out_vals, out_len);
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
        return a->value.fval;
    }
    if (a->val_type == SVG_ATTR_VALUE_PTR && a->value.val) {
        const psx_svg_attr_values_list* list = (const psx_svg_attr_values_list*)a->value.val;
        if (list->length > 0) {
            const float* vals = (const float*)&list->data[0];
            return vals[0];
        }
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
    // Clock values stored as ms; convert to seconds.
    if (a->val_type == SVG_ATTR_VALUE_DATA) {
        if (a->value.fval <= 0.0f) {
            return 0.0f;
        }
        return a->value.fval * 0.001f;
    }

    // Fallback: list of clock values.
    if (a->val_type == SVG_ATTR_VALUE_PTR && a->value.val) {
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
    // Prefer earliest begin time from list.
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
    _sbo_init(&it->begins_sec);
}

static INLINE void _anim_item_begin_list_destroy(psx_svg_anim_item* it)
{
    if (!it) {
        return;
    }
    _sbo_destroy(&it->begins_sec);

    if (it->begin_event) {
        mem_free((void*)it->begin_event);
        it->begin_event = NULL;
    }

    if (it->begin_event_target_id) {
        mem_free((void*)it->begin_event_target_id);
        it->begin_event_target_id = NULL;
    }

    if (it->end_event) {
        mem_free((void*)it->end_event);
        it->end_event = NULL;
    }

    if (it->end_event_target_id) {
        mem_free((void*)it->end_event_target_id);
        it->end_event_target_id = NULL;
    }

    if (it->syncbase_ref_id) {
        mem_free((void*)it->syncbase_ref_id);
        it->syncbase_ref_id = NULL;
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

/* Compute the active duration for an anim item (used by restart logic). */
static INLINE float _anim_item_active_dur(const psx_svg_anim_item* it)
{
    if (!it || it->dur_sec <= 0.0f) {
        return 0.0f;
    }
    if (it->repeat_dur_sec > 0.0f) {
        return it->repeat_dur_sec;
    }
    if (it->repeat_count == 0) {
        return 1e30f; /* indefinite */
    }
    return it->dur_sec * (float)it->repeat_count;
}

static INLINE float _anim_item_begin_for_time(const psx_svg_anim_item* it, float doc_t)
{
    if (!it) {
        return 0.0f;
    }
    // Event-triggered animations: no instance times => not active.
    if (it->begin_event && _sbo_empty(&it->begins_sec)) {
        (void)doc_t;
        return 1e30f;
    }
    if (_sbo_empty(&it->begins_sec)) {
        return it->begin_sec;
    }

    const uint32_t n = _sbo_size(&it->begins_sec);

    // restart="never": only use the first (earliest) begin.
    if (it->restart_mode == SVG_ANIMATION_RESTART_NEVER) {
        const float* fp = _sbo_get_const(&it->begins_sec, 0);
        return fp ? *fp : 0.0f;
    }

    // restart="whenNotActive": iterate forwards, skip begins that fall
    // within a previous begin's active interval.
    if (it->restart_mode == SVG_ANIMATION_RESTART_WHEN_NOT_ACTIVE) {
        float ad = _anim_item_active_dur(it);
        // Build valid begins by forward scan: a begin is valid only if it
        // does not fall within the active interval of the current valid begin.
        float cur_valid = -1.0f;
        float best = -1.0f;
        for (uint32_t i = 0; i < n; i++) {
            const float* bp = _sbo_get_const(&it->begins_sec, i);
            float b = bp ? *bp : 0.0f;
            if (cur_valid < 0.0f || b >= cur_valid + ad) {
                // This begin is valid (not inside a previous active interval).
                cur_valid = b;
                if (b <= doc_t && b > best) {
                    best = b;
                }
            }
            // else: b falls within cur_valid's active interval => skip
        }
        if (best < 0.0f) {
            // all valid begins are in the future: return earliest valid
            cur_valid = -1.0f;
            for (uint32_t i = 0; i < n; i++) {
                const float* bp = _sbo_get_const(&it->begins_sec, i);
                float b = bp ? *bp : 0.0f;
                if (cur_valid < 0.0f || b >= cur_valid + ad) {
                    cur_valid = b;
                    return b;
                }
            }
            const float* fp = _sbo_get_const(&it->begins_sec, 0);
            return fp ? *fp : 0.0f;
        }
        return best;
    }

    // restart="always" (default): choose latest begin <= doc_t
    float best = -1.0f;
    for (uint32_t i = 0; i < n; i++) {
        const float* bp = _sbo_get_const(&it->begins_sec, i);
        float b = bp ? *bp : 0.0f;
        if (b <= doc_t && b > best) {
            best = b;
        }
    }
    if (best < 0.0f) {
        // all begins are in the future: keep earliest (for consistency)
        const float* ep0 = _sbo_get_const(&it->begins_sec, 0);
        float earliest = ep0 ? *ep0 : 0.0f;
        for (uint32_t i = 1; i < n; i++) {
            const float* ep = _sbo_get_const(&it->begins_sec, i);
            float e = ep ? *ep : 0.0f;
            if (e < earliest) {
                earliest = e;
            }
        }
        return earliest;
    }
    return best;
}

/*
 * Determine whether an animation is active at doc_t and compute its
 * current repeat iteration (0-based).  Returns true if active.
 */
static INLINE bool _anim_is_active(const psx_svg_anim_item* it, float doc_t, uint32_t* out_iteration)
{
    if (!it || it->dur_sec <= 0.0f) {
        if (out_iteration) { *out_iteration = 0; }
        return false;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        if (out_iteration) { *out_iteration = 0; }
        return false;
    }

    // Check explicit end (ends_sec list or end_sec scalar).
    {
        float end_s = 0.0f;
        if (_sbo_size(&it->ends_sec) > 0) {
            end_s = _anim_item_end_for_begin(it, begin_sec);
        } else if (it->end_sec > 0.0f) {
            end_s = it->end_sec;
        }
        if (end_s > 0.0f && end_s >= begin_sec && doc_t >= end_s) {
            if (out_iteration) { *out_iteration = 0; }
            return false;
        }
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

    /* Apply min/max constraints (mirror _anim_resolve_local_t). */
    if (has_total) {
        float min_s = it->min_sec;
        float max_s = it->max_sec;
        if (min_s > 0.0f && max_s > 0.0f && min_s > max_s) {
            /* ignore both */
        } else {
            if (max_s > 0.0f && total > max_s) { total = max_s; }
            if (min_s > 0.0f && total < min_s) { total = min_s; }
        }
    }

    if (has_total && local >= total) {
        /* Past active duration — not active for callback purposes. */
        if (out_iteration) { *out_iteration = 0; }
        return false;
    }

    /* Compute iteration (0-based). */
    uint32_t iter = (it->dur_sec > 0.0f) ? (uint32_t)(local / it->dur_sec) : 0;
    if (out_iteration) { *out_iteration = iter; }
    return true;
}

static INLINE void _anim_item_begin_list_normalize(psx_svg_anim_item* it)
{
    if (!it) {
        return;
    }
    _sbo_sort_dedup(&it->begins_sec);
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
 * If out_iteration is non-NULL, writes the 0-based repeat iteration index.
 */
static INLINE bool _anim_resolve_local_t(const psx_svg_anim_item* it, float doc_t, float* out_t,
                                         uint32_t* out_iteration)
{
    if (out_iteration) {
        *out_iteration = 0;
    }
    if (!it || !out_t || it->dur_sec <= 0.0f) {
        return false;
    }

    float begin_sec = _anim_item_begin_for_time(it, doc_t);
    if (doc_t < begin_sec) {
        return false;
    }

    // Check explicit end (ends_sec list or end_sec scalar).
    float end_sec = 0.0f;
    if (_sbo_size(&it->ends_sec) > 0) {
        end_sec = _anim_item_end_for_begin(it, begin_sec);
    } else if (it->end_sec > 0.0f) {
        end_sec = it->end_sec;
    }
    if (end_sec > 0.0f && end_sec < begin_sec) {
        end_sec = 0.0f;
    }
    if (end_sec > 0.0f && doc_t >= end_sec) {
        if (it->fill_mode == SVG_ANIMATION_FREEZE) {
            *out_t = 1.0f;
            if (out_iteration) {
                *out_iteration = (it->repeat_count > 1) ? (it->repeat_count - 1) : 0;
            }
            return true;
        }
        return false;
    }

    float local = doc_t - begin_sec;

    float total = 0.0f;
    bool has_total = false;
    float raw_total = 0.0f;
    if (it->repeat_dur_sec > 0.0f) {
        total = it->repeat_dur_sec;
        has_total = true;
    } else if (it->repeat_count == 0) {
        has_total = false;
    } else {
        total = it->dur_sec * (float)it->repeat_count;
        has_total = true;
    }
    raw_total = total;

    // Apply min/max active duration constraints (SVG Tiny 1.2 §16.3.3).
    // min > max => both ignored.
    if (has_total) {
        float min_s = it->min_sec;
        float max_s = it->max_sec;
        if (min_s > 0.0f && max_s > 0.0f && min_s > max_s) {
            // ignore both
        } else {
            if (max_s > 0.0f && total > max_s) {
                total = max_s;
            }
            if (min_s > 0.0f && total < min_s) {
                total = min_s;
            }
        }
    }

    // Compute iteration (0-based) before fmod.
    uint32_t iteration = (it->dur_sec > 0.0f) ? (uint32_t)(local / it->dur_sec) : 0;

    if (!has_total) {
        local = _anim_fmod(local, it->dur_sec);
    } else {
        if (local >= total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                // Frozen past end: iteration is the last completed one.
                if (it->repeat_count > 1) {
                    iteration = it->repeat_count - 1;
                }
                local = it->dur_sec;
            } else {
                return false;
            }
        } else if (raw_total > 0.0f && local >= raw_total) {
            // In min-extended region: past original duration but within min.
            // fill=freeze: hold final value; fill=remove: no override.
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                if (it->repeat_count > 1) {
                    iteration = it->repeat_count - 1;
                }
                local = it->dur_sec;
            } else {
                return false;
            }
        } else {
            local = _anim_fmod(local, it->dur_sec);
        }
    }

    if (out_iteration) {
        *out_iteration = iteration;
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

// Linearly interpolate two packed RGB uint32 colors, channel by channel.
// Returns the result as packed uint32.
static INLINE uint32_t _anim_lerp_color(uint32_t c0, uint32_t c1, float t)
{
    float r0 = (float)((c0 >> 16) & 0xFF);
    float g0 = (float)((c0 >> 8) & 0xFF);
    float b0 = (float)(c0 & 0xFF);
    float r1 = (float)((c1 >> 16) & 0xFF);
    float g1 = (float)((c1 >> 8) & 0xFF);
    float b1 = (float)(c1 & 0xFF);

    int ri = (int)(_anim_lerp(r0, r1, t) + 0.5f);
    int gi = (int)(_anim_lerp(g0, g1, t) + 0.5f);
    int bi = (int)(_anim_lerp(b0, b1, t) + 0.5f);
    if (ri < 0) { ri = 0; }
    if (ri > 255) { ri = 255; }
    if (gi < 0) { gi = 0; }
    if (gi > 255) { gi = 255; }
    if (bi < 0) { bi = 0; }
    if (bi > 255) { bi = 255; }

    return ((uint32_t)ri << 16) | ((uint32_t)gi << 8) | (uint32_t)bi;
}

/* ── stroke-dasharray animation helpers ── */

static INLINE void _anim_state_set_dash(psx_svg_anim_state* s, const psx_svg_node* target,
                                        const float* dashes, uint32_t count)
{
    if (!s || !target || !dashes || count == 0) {
        return;
    }

    /* Update existing entry if present. */
    uint32_t n = psx_array_size(&s->dash_overrides);
    for (uint32_t i = 0; i < n; i++) {
        psx_svg_anim_dash_item* di = psx_array_get(&s->dash_overrides, i, psx_svg_anim_dash_item);
        if (di && di->target == target) {
            if (di->count != count) {
                mem_free(di->dashes);
                di->dashes = (float*)mem_malloc(count * sizeof(float));
                di->count = count;
            }
            for (uint32_t j = 0; j < count; j++) {
                di->dashes[j] = dashes[j];
            }
            return;
        }
    }

    /* Append new entry — try to reuse ghost entry's dashes buffer. */
    {
        uint32_t idx = s->dash_overrides.size;
        psx_array_append(&s->dash_overrides, NULL);
        psx_svg_anim_dash_item* dst = psx_array_get_last(&s->dash_overrides, psx_svg_anim_dash_item);
        /* After psx_array_append with NULL, the slot at idx still contains
         * the ghost entry's old data (append just increments size). */
        float* old_dashes = dst->dashes;
        uint32_t old_count = dst->count;

        dst->target = target;
        dst->count = count;

        if (idx < s->dash_high_water && old_dashes) {
            /* Ghost entry exists with a buffer — check if reusable. */
            if (old_count == count) {
                /* Same size: reuse buffer, just overwrite values. */
                dst->dashes = old_dashes;
            } else {
                /* Size mismatch: free old, allocate new. */
                mem_free(old_dashes);
                dst->dashes = (float*)mem_malloc(count * sizeof(float));
            }
        } else {
            /* No ghost or ghost had NULL dashes — allocate fresh. */
            dst->dashes = (float*)mem_malloc(count * sizeof(float));
        }

        for (uint32_t j = 0; j < count; j++) {
            dst->dashes[j] = dashes[j];
        }

        if (s->dash_overrides.size > s->dash_high_water) {
            s->dash_high_water = s->dash_overrides.size;
        }
    }
}

/* Helper: read a float array from a psx_svg_attr that may be stored as
 * a values_list (PTR) or a single fval (DATA). Returns count of floats. */
static INLINE uint32_t _attr_as_float_array(const psx_svg_attr* a, const float** out_data)
{
    if (!a) {
        *out_data = NULL;
        return 0;
    }
    if (a->val_type == SVG_ATTR_VALUE_PTR && a->value.val) {
        const psx_svg_attr_values_list* list = (const psx_svg_attr_values_list*)a->value.val;
        *out_data = (const float*)&list->data[0];
        return list->length;
    }
    /* Single fval — caller must use address of fval field. */
    *out_data = &a->value.fval;
    return 1;
}

/* Compute GCD of two positive integers. */
static INLINE uint32_t _gcd(uint32_t a, uint32_t b)
{
    while (b) {
        uint32_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

/* Compute LCM of two positive integers. */
static INLINE uint32_t _lcm(uint32_t a, uint32_t b)
{
    if (a == 0 || b == 0) { return 0; }
    return (a / _gcd(a, b)) * b;
}

/*
 * Evaluate a stroke-dasharray animation at document time doc_t.
 * Writes the interpolated dash array into anim_state.
 * Handles length normalization (LCM) and element-wise lerp.
 * Clamps negative values to 0.
 */
static INLINE bool _anim_eval_dasharray(const psx_svg_anim_item* it, float doc_t,
                                        psx_svg_anim_state* s)
{
    if (!it || !s) {
        return false;
    }

    float t = 0.0f;
    uint32_t iteration = 0;
    if (!_anim_resolve_local_t(it, doc_t, &t, &iteration)) {
        return false;
    }

    const psx_svg_attr* afrom = it->cached_from;
    const psx_svg_attr* ato = it->cached_to;
    if (!afrom || !ato) {
        return false;
    }

    const float* from_data = NULL;
    const float* to_data = NULL;
    uint32_t from_len = _attr_as_float_array(afrom, &from_data);
    uint32_t to_len = _attr_as_float_array(ato, &to_data);

    if (from_len == 0 || to_len == 0 || !from_data || !to_data) {
        return false;
    }

    /* Normalize lengths: repeat each array to LCM length. */
    uint32_t norm_len = _lcm(from_len, to_len);
    if (norm_len == 0 || norm_len > 64) {
        norm_len = (from_len > to_len) ? from_len : to_len;
    }

    /* Stack buffer for interpolated result (max 64 elements). */
    float buf[64];
    for (uint32_t i = 0; i < norm_len; i++) {
        float fv = from_data[i % from_len];
        float tv = to_data[i % to_len];
        float v = _anim_lerp(fv, tv, t);
        /* Clamp negative to 0. */
        buf[i] = (v < 0.0f) ? 0.0f : v;
    }

    _anim_state_set_dash(s, it->target_node, buf, norm_len);
    return true;
}

/*
 * Compute the single-iteration final value for a numeric animation (t=1.0).
 * Used by accumulate="sum" to add iteration * final_value.
 */
static INLINE float _anim_eval_simple_final_value(const psx_svg_anim_item* it)
{
    const psx_svg_attr* avals = it->cached_values;
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length >= 1) {
            const float* vals = (const float*)&vlist->data[0];
            return vals[vlist->length - 1];
        }
    }
    const psx_svg_attr* ato = it->cached_to;
    if (ato) {
        return _attr_as_number(ato);
    }
    const psx_svg_attr* aby = it->cached_by;
    const psx_svg_attr* afrom = it->cached_from;
    if (aby) {
        float from_val = afrom ? _attr_as_number(afrom) : 0.0f;
        return from_val + _attr_as_number(aby);
    }
    return 0.0f;
}

/*
 * Read the static DOM base value for a numeric attribute on the target node.
 * Returns 0.0f if the attribute is not present.
 */
static INLINE float _anim_get_base_value(const psx_svg_anim_item* it)
{
    if (!it || !it->target_node) {
        return 0.0f;
    }
    const psx_svg_attr* a = _find_attr(it->target_node, it->target_attr);
    if (!a) {
        return 0.0f;
    }
    return _attr_as_number(a);
}

static INLINE bool _anim_eval_simple(const psx_svg_anim_item* it, float doc_t, float* out_v, bool* out_hold)
{
    if (!it || !out_v || !out_hold) {
        return false;
    }

    *out_hold = false;

    float t = 0.0f;
    uint32_t iteration = 0;
    if (!_anim_resolve_local_t(it, doc_t, &t, &iteration)) {
        return false;
    }
    if (t >= 1.0f && it->fill_mode == SVG_ANIMATION_FREEZE) {
        *out_hold = true;
    }

    const psx_svg_attr* avals = it->cached_values;

    // animateColor: handle before generic float path.
    if (it->tag == SVG_TAG_ANIMATE_COLOR
        && (it->target_attr == SVG_ATTR_FILL || it->target_attr == SVG_ATTR_STROKE
            || it->target_attr == SVG_ATTR_GRADIENT_STOP_COLOR)) {
        uint32_t calc_mode = SVG_ANIMATION_CALC_MODE_LINEAR;
        const psx_svg_attr* acm = it->cached_calc_mode;
        if (acm && acm->val_type == SVG_ATTR_VALUE_DATA) {
            calc_mode = (uint32_t)acm->value.ival;
        }

        if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
            const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
            const uint32_t* vals = (vlist->length > 0) ? (const uint32_t*)&vlist->data[0] : NULL;
            if (vals && vlist->length >= 1) {
                if (vlist->length == 1 || calc_mode == SVG_ANIMATION_CALC_MODE_DISCRETE) {
                    uint32_t idx = 0;
                    if (t >= 1.0f) {
                        idx = vlist->length - 1;
                    } else {
                        float step = 1.0f / (float)vlist->length;
                        idx = (uint32_t)(t / step);
                        if (idx >= vlist->length) {
                            idx = vlist->length - 1;
                        }
                    }
                    *out_v = _u32_to_f32_bits(vals[idx]);
                } else {
                    const psx_svg_attr* akt = it->cached_key_times;
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
                    *out_v = _u32_to_f32_bits(_anim_lerp_color(vals[seg], vals[seg + 1], u));
                }
                return true;
            }
        }

        const psx_svg_attr* afrom_c = it->cached_from;
        const psx_svg_attr* ato_c = it->cached_to;
        if (!afrom_c || !ato_c) {
            return false;
        }
        uint32_t c0 = (uint32_t)afrom_c->value.uval;
        uint32_t c1 = (uint32_t)ato_c->value.uval;
        if (calc_mode == SVG_ANIMATION_CALC_MODE_DISCRETE) {
            *out_v = _u32_to_f32_bits((t < 0.5f) ? c0 : c1);
        } else {
            *out_v = _u32_to_f32_bits(_anim_lerp_color(c0, c1, t));
        }
        return true;
    }

    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length >= 1) {
            const float* vals = (const float*)&vlist->data[0];
            if (vlist->length == 1) {
                *out_v = vals[0];
                if (it->accumulate_mode == SVG_ANIMATION_ACCUMULATE_SUM && iteration > 0) {
                    *out_v += (float)iteration * _anim_eval_simple_final_value(it);
                }
                return true;
            }

            uint32_t calc_mode = SVG_ANIMATION_CALC_MODE_LINEAR;
            const psx_svg_attr* acm = it->cached_calc_mode;
            if (acm && acm->val_type == SVG_ATTR_VALUE_DATA) {
                calc_mode = (uint32_t)acm->value.ival;
            }

            const psx_svg_attr* akt = it->cached_key_times;
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
                    const psx_svg_attr* aks = it->cached_key_splines;
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
            if (it->accumulate_mode == SVG_ANIMATION_ACCUMULATE_SUM && iteration > 0) {
                *out_v += (float)iteration * _anim_eval_simple_final_value(it);
            }
            return true;
        }
    }

    const psx_svg_attr* afrom = it->cached_from;
    const psx_svg_attr* ato = it->cached_to;
    if (afrom && ato) {
        float v0 = _attr_as_number(afrom);
        float v1 = _attr_as_number(ato);
        *out_v = _anim_lerp(v0, v1, t);
        if (it->accumulate_mode == SVG_ANIMATION_ACCUMULATE_SUM && iteration > 0) {
            *out_v += (float)iteration * _anim_eval_simple_final_value(it);
        }
        return true;
    }

    // by attribute fallback: by alone => from=0 to=by; from+by => to=from+by
    const psx_svg_attr* aby = it->cached_by;
    if (aby && !ato) {
        float by_val = _attr_as_number(aby);
        float from_val = afrom ? _attr_as_number(afrom) : 0.0f;
        *out_v = _anim_lerp(from_val, from_val + by_val, t);
        if (it->accumulate_mode == SVG_ANIMATION_ACCUMULATE_SUM && iteration > 0) {
            *out_v += (float)iteration * _anim_eval_simple_final_value(it);
        }
        return true;
    }

    return false;
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
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
        return false;
    }

    const psx_svg_attr* avals = it->cached_values;
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
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
        return false;
    }

    const psx_svg_attr* avals = it->cached_values;
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

        // keyTimes or evenly-spaced segment mapping.
        const psx_svg_attr* akt = it->cached_key_times;
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

    // from/to fallback.
    const psx_svg_attr* afrom = it->cached_from;
    const psx_svg_attr* ato = it->cached_to;
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
        float data[6];
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

    const psx_svg_attr* avals = it->cached_values;
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length >= 2) {
            const psx_svg_attr* akt = it->cached_key_times;
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

    const psx_svg_attr* afrom = it->cached_from;
    const psx_svg_attr* ato = it->cached_to;
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
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
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
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
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
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
        return false;
    }

    const psx_svg_attr* afrom = it->cached_from;
    const psx_svg_attr* ato = it->cached_to;
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
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
        return false;
    }

    const psx_svg_attr* afrom = it->cached_from;
    const psx_svg_attr* ato = it->cached_to;
    if (!afrom || !ato) {
        return false;
    }

    float ang = (t < 0.5f) ? _attr_as_number(afrom) : _attr_as_number(ato);
    *out_b = (float)tan(ang * 0.01745329252f);
    return true;
}

// matrix linear: lerp each of the 6 components independently.
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
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
        return false;
    }

    // values list path.
    const psx_svg_attr* avals = it->cached_values;
    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length >= 2) {
            const psx_svg_attr* akt = it->cached_key_times;
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

    // from/to fallback.
    const psx_svg_attr* afrom = it->cached_from;
    const psx_svg_attr* ato = it->cached_to;
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

// matrix discrete: snap to from or to based on t < 0.5.
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
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
        return false;
    }

    const psx_svg_attr* afrom = it->cached_from;
    const psx_svg_attr* ato = it->cached_to;
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
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
        return false;
    }

    const psx_svg_attr* avals = it->cached_values;
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
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
        return false;
    }

    const psx_svg_attr* avals = it->cached_values;
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

        const psx_svg_attr* akt = it->cached_key_times;
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

    // from/to fallback.
    const psx_svg_attr* afrom = it->cached_from;
    const psx_svg_attr* ato = it->cached_to;
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
        float data[6];
    };

    const struct _anim_transform_values_single* fromv = (const struct _anim_transform_values_single*)afrom->value.val;
    const struct _anim_transform_values_single* tov = (const struct _anim_transform_values_single*)ato->value.val;
    if (!fromv || !tov) {
        return false;
    }

    float sx0 = (fromv->length >= 1) ? fromv->data[0] : 1.0f;
    float sy0 = (fromv->length >= 2) ? fromv->data[1] : sx0;
    float sx1 = (tov->length >= 1) ? tov->data[0] : 1.0f;
    float sy1 = (tov->length >= 2) ? tov->data[1] : sx1;

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
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
        return false;
    }

    const psx_svg_attr* avals = it->cached_values;
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

    // First number is angle in degrees.
    float angle_deg = (vlen >= 1) ? base[0] : 0.0f;
    float angle_rad = (float)(angle_deg * (M_PI / 180.0f));
    float cs = (float)cosf(angle_rad);
    float sn = (float)sinf(angle_rad);

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
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
        return false;
    }

    const psx_svg_attr* avals = it->cached_values;
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

        const psx_svg_attr* akt = it->cached_key_times;
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

    // from/to fallback.
    {
        const psx_svg_attr* afrom = it->cached_from;
        const psx_svg_attr* ato = it->cached_to;
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
            float data[6];
        };

        const struct _anim_transform_values_single* fromv = (const struct _anim_transform_values_single*)afrom->value.val;
        const struct _anim_transform_values_single* tov = (const struct _anim_transform_values_single*)ato->value.val;
        if (!fromv || !tov) {
            return false;
        }

        float ang0 = (fromv->length >= 1) ? fromv->data[0] : 0.0f;
        float ang1 = (tov->length >= 1) ? tov->data[0] : ang0;
        float cx0 = (fromv->length >= 2) ? fromv->data[1] : 0.0f;
        float cy0 = (fromv->length >= 3) ? fromv->data[2] : 0.0f;
        float cx1 = (tov->length >= 2) ? tov->data[1] : cx0;
        float cy1 = (tov->length >= 3) ? tov->data[2] : cy0;

        float angle_deg = _anim_lerp(ang0, ang1, t);
        float cx = _anim_lerp(cx0, cx1, t);
        float cy = _anim_lerp(cy0, cy1, t);

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
}

/*
 * Compute the final-value (t=1.0) transform components for accumulate="sum".
 * For translate: returns (e_final, f_final) from the to/values-last entry.
 * For scale: returns (a_final-1, d_final-1) as additive offsets from identity.
 * For rotate: returns the final angle in degrees.
 * Writes into out_e/out_f for translate, out_a/out_d for scale.
 */
static INLINE void _anim_eval_transform_final_value(const psx_svg_anim_item* it, int32_t ttype,
                                                    float* out_a, float* out_b, float* out_c,
                                                    float* out_d, float* out_e, float* out_f)
{
    *out_a = 0.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 0.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    const psx_svg_attr* avals = it->cached_values;
    const float* final_vals = NULL;
    uint32_t final_len = 0;

    if (avals && avals->val_type == SVG_ATTR_VALUE_PTR && avals->value.val) {
        const psx_svg_attr_values_list* vlist = (const psx_svg_attr_values_list*)avals->value.val;
        if (vlist->length >= 1) {
            _anim_values_list_get_transform(vlist, vlist->length - 1, &final_vals, &final_len);
        }
    }
    if (!final_vals) {
        const psx_svg_attr* ato = it->cached_to;
        if (ato && ato->val_type == SVG_ATTR_VALUE_PTR && ato->value.val) {
            struct _tv { uint32_t length; float data[6]; };
            const struct _tv* tv = (const struct _tv*)ato->value.val;
            final_vals = tv->data;
            final_len = tv->length;
        }
    }
    if (!final_vals || final_len == 0) {
        return;
    }

    if (ttype == SVG_TRANSFORM_TYPE_TRANSLATE || ttype == 0) {
        *out_e = (final_len >= 1) ? final_vals[0] : 0.0f;
        *out_f = (final_len >= 2) ? final_vals[1] : 0.0f;
    } else if (ttype == SVG_TRANSFORM_TYPE_SCALE) {
        float sx = (final_len >= 1) ? final_vals[0] : 1.0f;
        float sy = (final_len >= 2) ? final_vals[1] : sx;
        /* For scale accumulation, the additive offset is (sx-1, sy-1) */
        *out_a = sx - 1.0f;
        *out_d = sy - 1.0f;
    } else if (ttype == SVG_TRANSFORM_TYPE_ROTATE) {
        /* Store final angle in out_a for rotate accumulation */
        *out_a = (final_len >= 1) ? final_vals[0] : 0.0f;
    }
}

/*
 * Dispatch animateTransform evaluation based on transform type and calcMode.
 * Returns true and writes the matrix components if the animation is active.
 */
static INLINE bool _anim_eval_transform_dispatch(const psx_svg_anim_item* it, float time_sec,
                                                 float* a, float* b, float* c,
                                                 float* d, float* e, float* f)
{
    const psx_svg_attr* acm = it->cached_calc_mode;
    bool is_discrete = (acm && acm->val_type == SVG_ATTR_VALUE_DATA && acm->value.ival == SVG_ANIMATION_CALC_MODE_DISCRETE) ? true : false;
    const psx_svg_attr* att = _find_attr(it->anim_node, SVG_ATTR_TRANSFORM_TYPE);
    int32_t ttype = (att && att->val_type == SVG_ATTR_VALUE_DATA) ? att->value.ival : 0;

    bool ok = false;
    if (is_discrete) {
        if (ttype == SVG_TRANSFORM_TYPE_SCALE) {
            ok = _anim_eval_transform_scale_discrete(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_ROTATE) {
            ok = _anim_eval_transform_rotate_discrete(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_X) {
            ok = _anim_eval_transform_skewx_discrete(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_Y) {
            ok = _anim_eval_transform_skewy_discrete(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_MATRIX) {
            ok = _anim_eval_transform_matrix_discrete(it, time_sec, a, b, c, d, e, f);
        } else {
            ok = _anim_eval_transform_translate_discrete(it, time_sec, a, b, c, d, e, f);
        }
    } else {
        if (ttype == SVG_TRANSFORM_TYPE_ROTATE) {
            ok = _anim_eval_transform_rotate_linear(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_SCALE) {
            ok = _anim_eval_transform_scale_linear(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_X) {
            ok = _anim_eval_transform_skewx_linear(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_SKEW_Y) {
            ok = _anim_eval_transform_skewy_linear(it, time_sec, a, b, c, d, e, f);
        } else if (ttype == SVG_TRANSFORM_TYPE_MATRIX) {
            ok = _anim_eval_transform_matrix_linear(it, time_sec, a, b, c, d, e, f);
        } else {
            ok = _anim_eval_transform_translate_linear(it, time_sec, a, b, c, d, e, f);
        }
    }

    /* Apply accumulate="sum" if active and iteration > 0. */
    if (ok && it->accumulate_mode == SVG_ANIMATION_ACCUMULATE_SUM) {
        uint32_t iteration = 0;
        float dummy_t = 0.0f;
        _anim_resolve_local_t(it, time_sec, &dummy_t, &iteration);
        if (iteration > 0) {
            float fa = 0, fb = 0, fc = 0, fd = 0, fe = 0, ff = 0;
            _anim_eval_transform_final_value(it, ttype, &fa, &fb, &fc, &fd, &fe, &ff);
            if (ttype == SVG_TRANSFORM_TYPE_TRANSLATE || ttype == 0) {
                *e += (float)iteration * fe;
                *f += (float)iteration * ff;
            } else if (ttype == SVG_TRANSFORM_TYPE_SCALE) {
                *a += (float)iteration * fa;
                *d += (float)iteration * fd;
            } else if (ttype == SVG_TRANSFORM_TYPE_ROTATE) {
                /* Rotate accumulation: add iteration * final_angle to the current angle,
                 * then recompute the rotation matrix. */
                float cur_angle_rad = (float)atan2(*b, *a);
                float cur_angle_deg = cur_angle_rad * (180.0f / 3.14159265358979323846f);
                float new_angle_deg = cur_angle_deg + (float)iteration * fa;
                float new_rad = new_angle_deg * (3.14159265358979323846f / 180.0f);
                float cs = (float)cos(new_rad);
                float sn = (float)sin(new_rad);
                *a = cs;
                *b = sn;
                *c = -sn;
                *d = cs;
            }
        }
    }

    return ok;
}

/* Motion path mini-parser */

typedef struct {
    float* xy; /* mem_malloc allocated interleaved coordinate array [x0,y0,x1,y1,...] */
    uint32_t count; /* current point count */
    uint32_t cap; /* array capacity (in points) */
} _motion_path_points;

static void _motion_path_destroy(_motion_path_points* pts)
{
    if (!pts) {
        return;
    }
    if (pts->xy) {
        mem_free(pts->xy);
        pts->xy = NULL;
    }
    pts->count = 0;
    pts->cap = 0;
}

static bool _motion_path_push(_motion_path_points* pts, float x, float y)
{
    if (!pts) {
        return false;
    }
    if (pts->count >= pts->cap) {
        uint32_t new_cap = pts->cap ? pts->cap * 2 : 8;
        float* nxy = (float*)mem_realloc(pts->xy, new_cap * 2 * sizeof(float));
        if (!nxy) {
            return false;
        }
        pts->xy = nxy;
        pts->cap = new_cap;
    }
    pts->xy[pts->count * 2] = x;
    pts->xy[pts->count * 2 + 1] = y;
    pts->count++;
    return true;
}

/* Skip whitespace and commas in path string. Returns new position. */
static uint32_t _motion_path_skip_ws(const char* s, uint32_t pos, uint32_t len)
{
    while (pos < len && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\r'
                         || s[pos] == '\n' || s[pos] == ',')) {
        pos++;
    }
    return pos;
}

/* Parse a float number from path string starting at pos.
 * Returns the position after the number, or the same pos if no number found.
 * Writes the parsed value into *out. */
static uint32_t _motion_path_parse_float(const char* s, uint32_t pos, uint32_t len, float* out)
{
    uint32_t start = pos;
    if (pos >= len) {
        return pos;
    }

    /* optional sign */
    if (s[pos] == '+' || s[pos] == '-') {
        pos++;
    }

    bool has_digits = false;

    /* integer part */
    while (pos < len && s[pos] >= '0' && s[pos] <= '9') {
        pos++;
        has_digits = true;
    }

    /* fractional part */
    if (pos < len && s[pos] == '.') {
        pos++;
        while (pos < len && s[pos] >= '0' && s[pos] <= '9') {
            pos++;
            has_digits = true;
        }
    }

    if (!has_digits) {
        return start;
    }

    /* optional exponent */
    if (pos < len && (s[pos] == 'e' || s[pos] == 'E')) {
        uint32_t epos = pos + 1;
        if (epos < len && (s[epos] == '+' || s[epos] == '-')) {
            epos++;
        }
        bool has_exp_digits = false;
        while (epos < len && s[epos] >= '0' && s[epos] <= '9') {
            epos++;
            has_exp_digits = true;
        }
        if (has_exp_digits) {
            pos = epos;
        }
    }

    /* Convert the substring to float. We need a null-terminated copy. */
    {
        char buf[64];
        uint32_t slen = pos - start;
        if (slen >= sizeof(buf)) {
            slen = sizeof(buf) - 1;
        }
        memcpy(buf, s + start, slen);
        buf[slen] = '\0';
        *out = (float)atof(buf);
    }
    return pos;
}

/* Curve flattening helpers */

#define FLATTEN_STEPS_QUAD 8
#define FLATTEN_STEPS_CUBIC 16
#define FLATTEN_STEPS_ARC 16

/* Flatten a quadratic Bezier curve into FLATTEN_STEPS_QUAD line segments.
 * B(t) = (1-t)^2 * P0 + 2(1-t)t * P1 + t^2 * P2
 * Pushes points for t = i/N, i = 1..N (start point already in array). */
static bool _motion_flatten_quadratic_bezier(
    _motion_path_points* pts,
    float x0, float y0,
    float cx, float cy,
    float ex, float ey)
{
    int i;
    for (i = 1; i <= FLATTEN_STEPS_QUAD; i++) {
        float t = (float)i / (float)FLATTEN_STEPS_QUAD;
        float u = 1.0f - t;
        float px = u * u * x0 + 2.0f * u * t * cx + t * t * ex;
        float py = u * u * y0 + 2.0f * u * t * cy + t * t * ey;
        if (!_motion_path_push(pts, px, py)) {
            return false;
        }
    }
    return true;
}

/* Flatten a cubic Bezier curve into FLATTEN_STEPS_CUBIC line segments.
 * B(t) = (1-t)^3 * P0 + 3(1-t)^2*t * P1 + 3(1-t)*t^2 * P2 + t^3 * P3
 * Pushes points for t = i/N, i = 1..N (start point already in array). */
static bool _motion_flatten_cubic_bezier(
    _motion_path_points* pts,
    float x0, float y0,
    float c1x, float c1y,
    float c2x, float c2y,
    float ex, float ey)
{
    int i;
    for (i = 1; i <= FLATTEN_STEPS_CUBIC; i++) {
        float t = (float)i / (float)FLATTEN_STEPS_CUBIC;
        float u = 1.0f - t;
        float u2 = u * u;
        float t2 = t * t;
        float px = u2 * u * x0 + 3.0f * u2 * t * c1x + 3.0f * u * t2 * c2x + t2 * t * ex;
        float py = u2 * u * y0 + 3.0f * u2 * t * c1y + 3.0f * u * t2 * c2y + t2 * t * ey;
        if (!_motion_path_push(pts, px, py)) {
            return false;
        }
    }
    return true;
}

/* Flatten an elliptical arc into FLATTEN_STEPS_ARC line segments.
 * Implements SVG spec F.6.5/F.6.6 endpoint-to-center parameterization.
 * Pushes points for angle = theta1 + dtheta*i/N, i = 1..N. */
static bool _motion_flatten_arc(
    _motion_path_points* pts,
    float x0, float y0,
    float rx, float ry,
    float x_rotation_deg,
    bool large_arc,
    bool sweep,
    float ex, float ey)
{
    /* Degenerate: start == end => skip */
    if (x0 == ex && y0 == ey) {
        return true;
    }

    /* Degenerate: zero radius => straight line */
    if (rx == 0.0f || ry == 0.0f) {
        return _motion_path_push(pts, ex, ey);
    }

    /* Ensure positive radii */
    if (rx < 0.0f) { rx = -rx; }
    if (ry < 0.0f) { ry = -ry; }

    /* F.6.5: compute (x1', y1') in rotated coordinate system */
    float phi = (float)(x_rotation_deg * (M_PI / 180.0));
    float cos_phi = cosf(phi);
    float sin_phi = sinf(phi);

    float dx2 = (x0 - ex) * 0.5f;
    float dy2 = (y0 - ey) * 0.5f;
    float x1p = cos_phi * dx2 + sin_phi * dy2;
    float y1p = -sin_phi * dx2 + cos_phi * dy2;

    /* F.6.6: check if radii need scaling */
    float x1p2 = x1p * x1p;
    float y1p2 = y1p * y1p;
    float rx2 = rx * rx;
    float ry2 = ry * ry;
    float lambda = x1p2 / rx2 + y1p2 / ry2;
    if (lambda > 1.0f) {
        float sl = sqrtf(lambda);
        rx *= sl;
        ry *= sl;
        rx2 = rx * rx;
        ry2 = ry * ry;
    }

    /* F.6.5: compute center point (cx', cy') in rotated system */
    float num = rx2 * ry2 - rx2 * y1p2 - ry2 * x1p2;
    float den = rx2 * y1p2 + ry2 * x1p2;
    float sq = 0.0f;
    if (den > 0.0f && num > 0.0f) {
        sq = sqrtf(num / den);
    }
    if (large_arc == sweep) {
        sq = -sq;
    }

    float cxp = sq * rx * y1p / ry;
    float cyp = -sq * ry * x1p / rx;

    /* Transform back to original coordinate system */
    float mx = (x0 + ex) * 0.5f;
    float my = (y0 + ey) * 0.5f;
    float cx = cos_phi * cxp - sin_phi * cyp + mx;
    float cy = sin_phi * cxp + cos_phi * cyp + my;

    /* Compute start angle theta1 and angle extent dtheta */
    float ux = (x1p - cxp) / rx;
    float uy = (y1p - cyp) / ry;
    float vx = (-x1p - cxp) / rx;
    float vy = (-y1p - cyp) / ry;

    /* angle of vector (ux, uy) relative to (1, 0) */
    float n_u = sqrtf(ux * ux + uy * uy);
    float theta1 = 0.0f;
    if (n_u > 0.0f) {
        float cos_t = ux / n_u;
        if (cos_t < -1.0f) { cos_t = -1.0f; }
        if (cos_t > 1.0f) { cos_t = 1.0f; }
        theta1 = acosf(cos_t);
        if (uy < 0.0f) { theta1 = -theta1; }
    }

    /* angle between (ux, uy) and (vx, vy) */
    float n_v = sqrtf(vx * vx + vy * vy);
    float dtheta = 0.0f;
    if (n_u > 0.0f && n_v > 0.0f) {
        float dot = ux * vx + uy * vy;
        float cos_d = dot / (n_u * n_v);
        if (cos_d < -1.0f) { cos_d = -1.0f; }
        if (cos_d > 1.0f) { cos_d = 1.0f; }
        dtheta = acosf(cos_d);
        /* sign: cross product */
        if (ux * vy - uy * vx < 0.0f) { dtheta = -dtheta; }
    }

    /* Adjust dtheta per sweep flag (SVG spec F.6.5 step 4) */
    if (sweep && dtheta < 0.0f) {
        dtheta += (float)(2.0 * M_PI);
    } else if (!sweep && dtheta > 0.0f) {
        dtheta -= (float)(2.0 * M_PI);
    }

    /* Sample FLATTEN_STEPS_ARC points uniformly in [theta1, theta1+dtheta] */
    int i;
    for (i = 1; i <= FLATTEN_STEPS_ARC; i++) {
        float angle = theta1 + dtheta * (float)i / (float)FLATTEN_STEPS_ARC;
        float ca = cosf(angle);
        float sa = sinf(angle);
        float px = cx + rx * ca * cos_phi - ry * sa * sin_phi;
        float py = cy + rx * ca * sin_phi + ry * sa * cos_phi;
        if (!_motion_path_push(pts, px, py)) {
            return false;
        }
    }
    return true;
}

/*
 * Parse SVG path string into point array.
 * Supports: M/m, L/l, H/h, V/v, Z/z, Q/q, T/t, C/c, S/s, A/a.
 * Curves are flattened to polyline segments.
 * Returns true if at least 1 point was parsed.
 */
static bool _motion_path_parse(const char* path_str, uint32_t len,
                               _motion_path_points* out)
{
    if (!out) {
        return false;
    }
    out->xy = NULL;
    out->count = 0;
    out->cap = 0;

    if (!path_str || len == 0) {
        return false;
    }

    float cur_x = 0.0f;
    float cur_y = 0.0f;
    float start_x = 0.0f;
    float start_y = 0.0f;
    float last_quad_cx = 0.0f;
    float last_quad_cy = 0.0f;
    bool has_last_quad = false;
    float last_cubic_c2x = 0.0f;
    float last_cubic_c2y = 0.0f;
    bool has_last_cubic = false;
    char cmd = 0;
    uint32_t pos = 0;

    while (pos < len) {
        pos = _motion_path_skip_ws(path_str, pos, len);
        if (pos >= len) {
            break;
        }

        char ch = path_str[pos];

        /* Check if this is a command letter */
        if (ch == 'M' || ch == 'm' || ch == 'L' || ch == 'l'
            || ch == 'H' || ch == 'h' || ch == 'V' || ch == 'v'
            || ch == 'Q' || ch == 'q' || ch == 'T' || ch == 't'
            || ch == 'C' || ch == 'c'
            || ch == 'S' || ch == 's'
            || ch == 'A' || ch == 'a') {
            cmd = ch;
            pos++;
        } else if (ch == 'Z' || ch == 'z') {
            /* closePath — add line segment back to most recent M point */
            pos++;
            if (cur_x != start_x || cur_y != start_y) {
                if (!_motion_path_push(out, start_x, start_y)) {
                    _motion_path_destroy(out);
                    return false;
                }
                cur_x = start_x;
                cur_y = start_y;
            }
            has_last_quad = false;
            has_last_cubic = false;
            cmd = 0;
            continue;
        } else if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
            /* Unrecognized command — skip it and continue */
            cmd = 0;
            pos++;
            continue;
        }
        /* else: implicit repeat of previous command (number follows directly) */

        if (cmd == 0) {
            /* No active command and not a number start — skip */
            if (!((ch >= '0' && ch <= '9') || ch == '-' || ch == '+' || ch == '.')) {
                pos++;
                continue;
            }
            /* Stray number with no command context — skip it */
            {
                float dummy = 0.0f;
                uint32_t npos = _motion_path_parse_float(path_str, pos, len, &dummy);
                if (npos == pos) {
                    pos++;
                } else {
                    pos = npos;
                }
            }
            continue;
        }

        /* H/h: horizontal lineTo — parse one float (x), keep current y */
        if (cmd == 'H' || cmd == 'h') {
            pos = _motion_path_skip_ws(path_str, pos, len);
            float x = 0.0f;
            uint32_t npos = _motion_path_parse_float(path_str, pos, len, &x);
            if (npos == pos) {
                cmd = 0;
                continue;
            }
            pos = npos;

            if (cmd == 'h') {
                x += cur_x;
            }

            if (!_motion_path_push(out, x, cur_y)) {
                _motion_path_destroy(out);
                return false;
            }
            cur_x = x;
            has_last_quad = false;
            has_last_cubic = false;
            continue;
        }

        /* V/v: vertical lineTo — parse one float (y), keep current x */
        if (cmd == 'V' || cmd == 'v') {
            pos = _motion_path_skip_ws(path_str, pos, len);
            float y = 0.0f;
            uint32_t npos = _motion_path_parse_float(path_str, pos, len, &y);
            if (npos == pos) {
                cmd = 0;
                continue;
            }
            pos = npos;

            if (cmd == 'v') {
                y += cur_y;
            }

            if (!_motion_path_push(out, cur_x, y)) {
                _motion_path_destroy(out);
                return false;
            }
            cur_y = y;
            has_last_quad = false;
            has_last_cubic = false;
            continue;
        }

        /* Q/q: quadratic Bezier — parse 4 floats (cx, cy, ex, ey) */
        if (cmd == 'Q' || cmd == 'q') {
            float cx = 0.0f, cy = 0.0f, ex = 0.0f, ey = 0.0f;
            pos = _motion_path_skip_ws(path_str, pos, len);
            uint32_t npos = _motion_path_parse_float(path_str, pos, len, &cx);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &cy);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &ex);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &ey);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            if (cmd == 'q') {
                cx += cur_x;
                cy += cur_y;
                ex += cur_x;
                ey += cur_y;
            }

            if (!_motion_flatten_quadratic_bezier(out, cur_x, cur_y, cx, cy, ex, ey)) {
                _motion_path_destroy(out);
                return false;
            }
            last_quad_cx = cx;
            last_quad_cy = cy;
            has_last_quad = true;
            has_last_cubic = false;
            cur_x = ex;
            cur_y = ey;
            continue;
        }

        /* T/t: smooth quadratic Bezier — parse 2 floats (ex, ey) */
        if (cmd == 'T' || cmd == 't') {
            float ex = 0.0f, ey = 0.0f;
            pos = _motion_path_skip_ws(path_str, pos, len);
            uint32_t npos = _motion_path_parse_float(path_str, pos, len, &ex);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &ey);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            if (cmd == 't') {
                ex += cur_x;
                ey += cur_y;
            }

            /* Compute reflected control point */
            float rcx, rcy;
            if (has_last_quad) {
                rcx = 2.0f * cur_x - last_quad_cx;
                rcy = 2.0f * cur_y - last_quad_cy;
            } else {
                rcx = cur_x;
                rcy = cur_y;
            }

            if (!_motion_flatten_quadratic_bezier(out, cur_x, cur_y, rcx, rcy, ex, ey)) {
                _motion_path_destroy(out);
                return false;
            }
            last_quad_cx = rcx;
            last_quad_cy = rcy;
            has_last_quad = true;
            has_last_cubic = false;
            cur_x = ex;
            cur_y = ey;
            continue;
        }

        /* C/c: cubic Bezier — parse 6 floats (c1x, c1y, c2x, c2y, ex, ey) */
        if (cmd == 'C' || cmd == 'c') {
            float c1x = 0.0f, c1y = 0.0f, c2x = 0.0f, c2y = 0.0f, ex = 0.0f, ey = 0.0f;
            pos = _motion_path_skip_ws(path_str, pos, len);
            uint32_t npos = _motion_path_parse_float(path_str, pos, len, &c1x);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &c1y);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &c2x);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &c2y);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &ex);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &ey);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            if (cmd == 'c') {
                c1x += cur_x;
                c1y += cur_y;
                c2x += cur_x;
                c2y += cur_y;
                ex += cur_x;
                ey += cur_y;
            }

            if (!_motion_flatten_cubic_bezier(out, cur_x, cur_y, c1x, c1y, c2x, c2y, ex, ey)) {
                _motion_path_destroy(out);
                return false;
            }
            last_cubic_c2x = c2x;
            last_cubic_c2y = c2y;
            has_last_cubic = true;
            has_last_quad = false;
            cur_x = ex;
            cur_y = ey;
            continue;
        }

        /* S/s: smooth cubic Bezier — parse 4 floats (c2x, c2y, ex, ey) */
        if (cmd == 'S' || cmd == 's') {
            float c2x = 0.0f, c2y = 0.0f, ex = 0.0f, ey = 0.0f;
            pos = _motion_path_skip_ws(path_str, pos, len);
            uint32_t npos = _motion_path_parse_float(path_str, pos, len, &c2x);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &c2y);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &ex);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &ey);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            if (cmd == 's') {
                c2x += cur_x;
                c2y += cur_y;
                ex += cur_x;
                ey += cur_y;
            }

            /* Compute reflected first control point */
            float rc1x, rc1y;
            if (has_last_cubic) {
                rc1x = 2.0f * cur_x - last_cubic_c2x;
                rc1y = 2.0f * cur_y - last_cubic_c2y;
            } else {
                rc1x = cur_x;
                rc1y = cur_y;
            }

            if (!_motion_flatten_cubic_bezier(out, cur_x, cur_y, rc1x, rc1y, c2x, c2y, ex, ey)) {
                _motion_path_destroy(out);
                return false;
            }
            last_cubic_c2x = c2x;
            last_cubic_c2y = c2y;
            has_last_cubic = true;
            has_last_quad = false;
            cur_x = ex;
            cur_y = ey;
            continue;
        }

        /* A/a: elliptical arc — parse 7 params (rx, ry, rotation, large_arc, sweep, ex, ey) */
        if (cmd == 'A' || cmd == 'a') {
            float arx = 0.0f, ary = 0.0f, xrot = 0.0f;
            float fla = 0.0f, fsw = 0.0f;
            float ex = 0.0f, ey = 0.0f;

            pos = _motion_path_skip_ws(path_str, pos, len);
            uint32_t npos = _motion_path_parse_float(path_str, pos, len, &arx);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &ary);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &xrot);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &fla);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &fsw);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &ex);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            pos = _motion_path_skip_ws(path_str, pos, len);
            npos = _motion_path_parse_float(path_str, pos, len, &ey);
            if (npos == pos) { cmd = 0; continue; }
            pos = npos;

            if (cmd == 'a') {
                ex += cur_x;
                ey += cur_y;
            }

            bool la = (fla != 0.0f);
            bool sw = (fsw != 0.0f);

            if (!_motion_flatten_arc(out, cur_x, cur_y, arx, ary, xrot, la, sw, ex, ey)) {
                _motion_path_destroy(out);
                return false;
            }
            has_last_quad = false;
            has_last_cubic = false;
            cur_x = ex;
            cur_y = ey;
            continue;
        }

        /* Parse coordinate pair for M/m/L/l */
        pos = _motion_path_skip_ws(path_str, pos, len);
        float x = 0.0f;
        float y = 0.0f;
        uint32_t npos = _motion_path_parse_float(path_str, pos, len, &x);
        if (npos == pos) {
            /* No number found — command has no more coordinates */
            cmd = 0;
            continue;
        }
        pos = npos;

        pos = _motion_path_skip_ws(path_str, pos, len);
        npos = _motion_path_parse_float(path_str, pos, len, &y);
        if (npos == pos) {
            /* Only one number — malformed, skip */
            cmd = 0;
            continue;
        }
        pos = npos;

        bool is_relative = (cmd == 'm' || cmd == 'l');
        if (is_relative) {
            x += cur_x;
            y += cur_y;
        }

        if (!_motion_path_push(out, x, y)) {
            _motion_path_destroy(out);
            return false;
        }

        cur_x = x;
        cur_y = y;
        has_last_quad = false;
        has_last_cubic = false;

        /* After M/m, record the start point for Z/z closePath */
        if (cmd == 'M' || cmd == 'm') {
            start_x = cur_x;
            start_y = cur_y;
        }

        /* After M/m, implicit subsequent coordinates are treated as L/l */
        if (cmd == 'M') {
            cmd = 'L';
        } else if (cmd == 'm') {
            cmd = 'l';
        }
    }

    return out->count > 0;
}

/* Arc-length parameterization */

typedef struct {
    float* cum_lengths; /* mem_malloc allocated cumulative arc length array */
    float total_length; /* total arc length */
} _motion_arc_table;

struct psx_svg_motion_cache {
    _motion_path_points pts;
    _motion_arc_table arc;
    bool ready;
};

static void _motion_arc_build(const _motion_path_points* pts, _motion_arc_table* out)
{
    out->cum_lengths = NULL;
    out->total_length = 0.0f;

    if (!pts || pts->count == 0) {
        return;
    }

    out->cum_lengths = (float*)mem_malloc(pts->count * sizeof(float));
    if (!out->cum_lengths) {
        return;
    }

    out->cum_lengths[0] = 0.0f;

    uint32_t i;
    for (i = 1; i < pts->count; i++) {
        float dx = pts->xy[i * 2] - pts->xy[(i - 1) * 2];
        float dy = pts->xy[i * 2 + 1] - pts->xy[(i - 1) * 2 + 1];
        float seg_len = sqrtf(dx * dx + dy * dy);
        out->cum_lengths[i] = out->cum_lengths[i - 1] + seg_len;
    }

    out->total_length = out->cum_lengths[pts->count - 1];
}

static void _motion_arc_destroy(_motion_arc_table* tbl)
{
    if (!tbl) {
        return;
    }
    if (tbl->cum_lengths) {
        mem_free(tbl->cum_lengths);
        tbl->cum_lengths = NULL;
    }
    tbl->total_length = 0.0f;
}

static psx_svg_motion_cache* _motion_cache_create(const char* path_str)
{
    if (!path_str) {
        return NULL;
    }

    psx_svg_motion_cache* cache = (psx_svg_motion_cache*)mem_malloc(sizeof(psx_svg_motion_cache));
    if (!cache) {
        return NULL;
    }
    memset(cache, 0, sizeof(psx_svg_motion_cache));

    uint32_t path_len = (uint32_t)strlen(path_str);
    if (!_motion_path_parse(path_str, path_len, &cache->pts)) {
        _motion_path_destroy(&cache->pts);
        mem_free(cache);
        return NULL;
    }

    _motion_arc_build(&cache->pts, &cache->arc);
    cache->ready = true;
    return cache;
}

static void _motion_cache_destroy(psx_svg_motion_cache* cache)
{
    if (!cache) {
        return;
    }
    _motion_arc_destroy(&cache->arc);
    _motion_path_destroy(&cache->pts);
    mem_free(cache);
}

static void _motion_arc_position(const _motion_path_points* pts,
                                 const _motion_arc_table* tbl,
                                 float t, float* out_x, float* out_y)
{
    if (!pts || pts->count == 0 || !tbl || !tbl->cum_lengths) {
        *out_x = 0.0f;
        *out_y = 0.0f;
        return;
    }

    /* Single point or zero total length → return first point */
    if (pts->count == 1 || tbl->total_length <= 0.0f) {
        *out_x = pts->xy[0];
        *out_y = pts->xy[1];
        return;
    }

    /* Clamp boundaries */
    if (t <= 0.0f) {
        *out_x = pts->xy[0];
        *out_y = pts->xy[1];
        return;
    }
    if (t >= 1.0f) {
        *out_x = pts->xy[(pts->count - 1) * 2];
        *out_y = pts->xy[(pts->count - 1) * 2 + 1];
        return;
    }

    float target_dist = t * tbl->total_length;

    /* Binary search for the segment containing target_dist */
    uint32_t lo = 0;
    uint32_t hi = pts->count - 1;
    while (lo + 1 < hi) {
        uint32_t mid = (lo + hi) / 2;
        if (tbl->cum_lengths[mid] <= target_dist) {
            lo = mid;
        } else {
            hi = mid;
        }
    }

    /* lo is the start index of the segment, hi = lo + 1 */
    float seg_start = tbl->cum_lengths[lo];
    float seg_end = tbl->cum_lengths[hi];
    float seg_len = seg_end - seg_start;

    float frac = 0.0f;
    if (seg_len > 0.0f) {
        frac = (target_dist - seg_start) / seg_len;
    }

    *out_x = pts->xy[lo * 2] + frac * (pts->xy[hi * 2] - pts->xy[lo * 2]);
    *out_y = pts->xy[lo * 2 + 1] + frac * (pts->xy[hi * 2 + 1] - pts->xy[lo * 2 + 1]);
}

/*
 * Compute the tangent angle (radians) of the motion path at normalized time t.
 * Returns atan2(dy, dx) of the segment containing the position at time t.
 * For single-point paths or zero-length paths, returns 0.
 */
static float _motion_arc_tangent_angle(const _motion_path_points* pts,
                                       const _motion_arc_table* tbl,
                                       float t)
{
    if (!pts || pts->count < 2 || !tbl || !tbl->cum_lengths || tbl->total_length <= 0.0f) {
        return 0.0f;
    }

    float ct = _anim_clampf(t, 0.0f, 1.0f);
    float target_dist = ct * tbl->total_length;

    /* Binary search for the segment containing target_dist */
    uint32_t lo = 0;
    uint32_t hi = pts->count - 1;
    while (lo + 1 < hi) {
        uint32_t mid = (lo + hi) / 2;
        if (tbl->cum_lengths[mid] <= target_dist) {
            lo = mid;
        } else {
            hi = mid;
        }
    }

    /* Compute direction of segment lo -> hi */
    float dx = pts->xy[hi * 2] - pts->xy[lo * 2];
    float dy = pts->xy[hi * 2 + 1] - pts->xy[lo * 2 + 1];

    return atan2f(dy, dx);
}

/*
 * Evaluate an <animateMotion> item at document time doc_t.
 * On success, writes a transform matrix into out_a..out_f.
 * Without rotate: pure translate (1,0,0,1,x,y).
 * With rotate: combined translate+rotate [cos(a),sin(a),-sin(a),cos(a),x,y].
 * Returns false if the animation is not active or has no valid path.
 */
static bool _anim_eval_motion(const psx_svg_anim_item* it, float doc_t,
                              float* out_a, float* out_b, float* out_c,
                              float* out_d, float* out_e, float* out_f)
{
    if (!it || !out_a || !out_b || !out_c || !out_d || !out_e || !out_f) {
        return false;
    }

    /* Defaults: identity */
    *out_a = 1.0f;
    *out_b = 0.0f;
    *out_c = 0.0f;
    *out_d = 1.0f;
    *out_e = 0.0f;
    *out_f = 0.0f;

    float t = 0.0f;
    if (!_anim_resolve_local_t(it, doc_t, &t, NULL)) {
        return false;
    }

    /* Use pre-cached path data if available (created during _collect_anims) */
    if (it->motion_cache && it->motion_cache->ready) {
        const _motion_path_points* pts = &it->motion_cache->pts;
        const _motion_arc_table* arc = &it->motion_cache->arc;

        float x = 0.0f;
        float y = 0.0f;
        _motion_arc_position(pts, arc, t, &x, &y);

        const psx_svg_attr* arot = _find_attr(it->anim_node, SVG_ATTR_ROTATE);
        if (arot) {
            float angle_rad = 0.0f;
            if (arot->class_type == SVG_ATTR_VALUE_INHERIT) {
                /* rotate="auto" (fval=0) or rotate="auto-reverse" (fval=180) */
                float tangent = _motion_arc_tangent_angle(pts, arc, t);
                float extra_deg = arot->value.fval; /* 0 for auto, 180 for auto-reverse */
                angle_rad = tangent + extra_deg * (float)M_PI / 180.0f;
            } else {
                /* Numeric rotate value in degrees */
                angle_rad = arot->value.fval * (float)M_PI / 180.0f;
            }
            float ca = cosf(angle_rad);
            float sa = sinf(angle_rad);
            *out_a = ca;
            *out_b = sa;
            *out_c = -sa;
            *out_d = ca;
        }

        *out_e = x;
        *out_f = y;
        return true;
    }

    /* from/to fallback when no motion_cache (no path attribute) */
    {
        const psx_svg_attr* afrom = it->cached_from;
        const psx_svg_attr* ato = it->cached_to;
        if (!afrom || !ato
            || afrom->val_type != SVG_ATTR_VALUE_PTR || !afrom->value.val
            || ato->val_type != SVG_ATTR_VALUE_PTR || !ato->value.val) {
            return false;
        }
        const psx_svg_attr_values_list* flist = (const psx_svg_attr_values_list*)afrom->value.val;
        const psx_svg_attr_values_list* tlist = (const psx_svg_attr_values_list*)ato->value.val;
        if (flist->length < 1 || tlist->length < 1) {
            return false;
        }
        const psx_svg_point* fp = (const psx_svg_point*)(&flist->data[0]);
        const psx_svg_point* tp = (const psx_svg_point*)(&tlist->data[0]);
        *out_e = fp->x + t * (tp->x - fp->x);
        *out_f = fp->y + t * (tp->y - fp->y);
        return true;
    }
}

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
    float end_sec = 0.0f;
    if (_sbo_size(&it->ends_sec) > 0) {
        end_sec = _anim_item_end_for_begin(it, begin_sec);
    } else if (it->end_sec > 0.0f) {
        end_sec = it->end_sec;
    }
    if (end_sec > 0.0f && end_sec < begin_sec) {
        end_sec = 0.0f;
    }

    if (end_sec > 0.0f && doc_t > end_sec) {
        if (it->fill_mode == SVG_ANIMATION_FREEZE) {
            *out_hold = true;
            // still return the frozen end value
            const psx_svg_attr* ato = it->cached_to;
            if (!ato) {
                return false;
            }
            *out_v = _attr_as_number(ato);
            return true;
        }
        return false;
    }

    if (it->dur_sec <= 0.0f) {
        const psx_svg_attr* ato = it->cached_to;
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

    // repeatDur bounds the active duration.
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
        if (active_dur <= 0.0f) {
            return false;
        }

        if (doc_t > end_sec) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = true;
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

        // Explicit end bounds the active interval.
        if (local > active_dur) {
            return false;
        }
    }

    if (it->repeat_count != 0) {
        float total = it->dur_sec * (float)it->repeat_count;
        if (local > total) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = true;
                local = it->dur_sec;
            } else {
                return false;
            }
        }
    }

    // No explicit end: check dur boundary.
    if (end_sec <= 0.0f) {
        if (local > it->dur_sec) {
            return false;
        }

        if (local == it->dur_sec) {
            if (it->fill_mode == SVG_ANIMATION_FREEZE) {
                *out_hold = true;
            } else if (it->fill_mode == SVG_ANIMATION_REMOVE) {
                return false;
            }
        }
    }

    const psx_svg_attr* ato = it->cached_to;
    if (!ato) {
        return false;
    }
    *out_v = _attr_as_number(ato);
    return true;
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
            if (an->val_type == SVG_ATTR_VALUE_DATA) {
                target_attr = (psx_svg_attr_type)an->value.ival;
            }
        }

        const psx_svg_node* target = node->parent();
        if (t == SVG_TAG_ANIMATE_MOTION) {
            target_attr = SVG_ATTR_TRANSFORM;
        }

        if (target && target_attr != SVG_ATTR_INVALID) {
            psx_svg_anim_item item;
            memset(&item, 0, sizeof(item));
            item.tag = t;
            item.anim_node = node;
            item.target_node = target;
            item.target_attr = target_attr;

            /* Pre-cache attribute pointers (Optimization 2) */
            item.cached_values = _find_attr(node, SVG_ATTR_VALUES);
            item.cached_from = _find_attr(node, SVG_ATTR_FROM);
            item.cached_to = _find_attr(node, SVG_ATTR_TO);
            item.cached_by = _find_attr(node, SVG_ATTR_BY);
            item.cached_key_times = _find_attr(node, SVG_ATTR_KEY_TIMES);
            item.cached_key_splines = _find_attr(node, SVG_ATTR_KEY_SPLINES);
            item.cached_calc_mode = _find_attr(node, SVG_ATTR_CALC_MODE);

            if (t == SVG_TAG_ANIMATE_MOTION) {
                const psx_svg_attr* apath = _find_attr(node, SVG_ATTR_PATH);
                if (apath && apath->val_type == SVG_ATTR_VALUE_PTR && apath->value.sval) {
                    item.motion_cache = _motion_cache_create(apath->value.sval);
                }
            }

            _anim_item_begin_list_init(&item);
            _anim_item_end_list_init(&item);

            const psx_svg_attr* abegin = _find_attr(node, SVG_ATTR_BEGIN);
            if (abegin && abegin->val_type == SVG_ATTR_VALUE_TIMING_LIST_PTR && abegin->value.val
                && (node->type() == SVG_TAG_SET || node->type() == SVG_TAG_ANIMATE || node->type() == SVG_TAG_ANIMATE_COLOR
                    || node->type() == SVG_TAG_ANIMATE_TRANSFORM || node->type() == SVG_TAG_ANIMATE_MOTION)
                && (abegin->attr_id == SVG_ATTR_BEGIN)) {
                const psx_svg_timing_list* tl = (const psx_svg_timing_list*)abegin->value.val;
                if (tl->syncbase_id) {
                    item.syncbase_ref_id = _dup_cstr(tl->syncbase_id);
                    item.syncbase_type = tl->syncbase_type;
                    item.syncbase_offset_sec = (tl->offsets_len > 0 && tl->offsets_ms)
                                               ? (tl->offsets_ms[0] * 0.001f) : 0.0f;
                    item.syncbase_resolved = false;
                    item.syncbase_circular = false;
                    _sbo_clear(&item.begins_sec);
                    item.begin_sec = 1e30f; // don't auto-activate until resolved
                } else if (tl->event_token) {
                    item.begin_event = _dup_cstr(tl->event_token);
                    item.begin_event_offset_sec = (tl->offsets_len > 0 && tl->offsets_ms)
                                                  ? (tl->offsets_ms[0] * 0.001f) : 0.0f;
                    item.begin_event_target_id = _dup_cstr(tl->event_target_id);
                    item.access_key = tl->access_key;
                    _sbo_clear(&item.begins_sec);
                    item.begin_sec = 1e30f; // don't auto-activate until triggered
                } else {
                    for (uint32_t i = 0; i < tl->offsets_len; i++) {
                        float ms = tl->offsets_ms ? tl->offsets_ms[i] : 0.0f;
                        float sec = (ms <= 0.0f) ? 0.0f : (ms * 0.001f);
                        _sbo_push(&item.begins_sec, sec);
                    }
                    _anim_item_begin_list_normalize(&item);
                    if (_sbo_size(&item.begins_sec) > 0) {
                        item.begin_sec = *_sbo_get_const(&item.begins_sec, 0);
                    }
                }
            } else if (abegin && abegin->val_type == SVG_ATTR_VALUE_PTR && abegin->value.val) {
                if (t == SVG_TAG_SET) {
                    const psx_svg_attr_values_list* list = (const psx_svg_attr_values_list*)abegin->value.val;
                    const float* vals = (list->length > 0) ? (const float*)&list->data[0] : NULL;
                    for (uint32_t i = 0; i < list->length; i++) {
                        float ms = vals[i];
                        float sec = (ms <= 0.0f) ? 0.0f : (ms * 0.001f);
                        _sbo_push(&item.begins_sec, sec);
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
                            _sbo_push(&item.begins_sec, sec);
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
                if (tl->event_token) {
                    item.end_event = _dup_cstr(tl->event_token);
                    item.end_event_target_id = _dup_cstr(tl->event_target_id);
                }
                for (uint32_t i = 0; i < tl->offsets_len; i++) {
                    float ms = tl->offsets_ms ? tl->offsets_ms[i] : 0.0f;
                    float sec = (ms <= 0.0f) ? 0.0f : (ms * 0.001f);
                    _sbo_push(&item.ends_sec, sec);
                }
                _anim_item_end_list_normalize(&item);
                item.end_sec = (_sbo_size(&item.ends_sec) > 0) ? *_sbo_get_const(&item.ends_sec, 0) : 0.0f;
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

            item.additive_mode = SVG_ANIMATION_ADDITIVE_REPLACE;
            const psx_svg_attr* additive_attr = _find_attr(node, SVG_ATTR_ADDITIVE);
            if (additive_attr && additive_attr->val_type == SVG_ATTR_VALUE_DATA) {
                item.additive_mode = (uint32_t)additive_attr->value.ival;
            }

            item.accumulate_mode = SVG_ANIMATION_ACCUMULATE_NONE;
            const psx_svg_attr* accum_attr = _find_attr(node, SVG_ATTR_ACCUMULATE);
            if (accum_attr && accum_attr->val_type == SVG_ATTR_VALUE_DATA) {
                item.accumulate_mode = (uint32_t)accum_attr->value.ival;
            }

            item.restart_mode = SVG_ANIMATION_RESTART_ALWAYS;
            const psx_svg_attr* arestart = _find_attr(node, SVG_ATTR_RESTART);
            if (arestart && arestart->val_type == SVG_ATTR_VALUE_DATA) {
                item.restart_mode = (uint32_t)arestart->value.ival;
            }

            item.min_sec = _attr_as_time_sec(_find_attr(node, SVG_ATTR_MIN));
            item.max_sec = _attr_as_time_sec(_find_attr(node, SVG_ATTR_MAX));

            /* Pre-cache DOM base value for additive="sum" animations */
            if (item.additive_mode == SVG_ANIMATION_ADDITIVE_SUM) {
                item.cached_base_value = _anim_get_base_value(&item);
                item.has_cached_base_value = true;
            }

            /* Pre-compute max active end time for fast culling */
            {
                /* If event-based begin, we can't predict — use safe fallback */
                if (item.begin_event || item.syncbase_ref_id) {
                    item.max_active_end_sec = 1e30f;
                } else {
                    /* Find the LAST (largest) begin time from begins_sec list */
                    float last_begin = item.begin_sec;
                    uint32_t bn = _sbo_size(&item.begins_sec);
                    for (uint32_t bi = 0; bi < bn; bi++) {
                        const float* bp = _sbo_get_const(&item.begins_sec, bi);
                        if (bp && *bp > last_begin) {
                            last_begin = *bp;
                        }
                    }

                    float active_dur = item.dur_sec;
                    if (item.repeat_count == 0) {
                        /* indefinite repeat */
                        active_dur = 1e30f;
                    } else if (item.repeat_count > 1) {
                        active_dur = item.dur_sec * (float)item.repeat_count;
                    }
                    if (item.repeat_dur_sec > 0.0f && item.repeat_dur_sec < active_dur) {
                        active_dur = item.repeat_dur_sec;
                    }
                    if (item.end_sec > 0.0f) {
                        float end_based = item.end_sec - last_begin;
                        if (end_based > 0.0f && end_based < active_dur) {
                            active_dur = end_based;
                        }
                    }
                    if (active_dur <= 0.0f || active_dur >= 1e30f) {
                        item.max_active_end_sec = 1e30f;
                    } else {
                        item.max_active_end_sec = last_begin + active_dur;
                    }
                }
            }

            psx_array_append(&p->anims, NULL);
            psx_svg_anim_item* dst = psx_array_get_last(&p->anims, psx_svg_anim_item);
            *dst = item;
        }
    }

    uint32_t c = node->child_count();
    for (uint32_t i = 0; i < c; i++) {
        _collect_anims(p, node->get_child(i));
    }
}

/*
 * Syncbase dependency resolution — uses sorted ID lookup table
 * for O(log n) binary search instead of O(n) strcmp scan.
 */
typedef struct {
    const char* id; /* anim_node->content(NULL) */
    uint32_t index; /* index in p->anims */
} _anim_id_entry;

static int _anim_id_entry_cmp(const void* a, const void* b)
{
    const _anim_id_entry* ea = (const _anim_id_entry*)a;
    const _anim_id_entry* eb = (const _anim_id_entry*)b;
    return strcmp(ea->id, eb->id);
}

/* Build a sorted id->index lookup table from p->anims.
 * Caller must mem_free the returned pointer.
 * *out_count receives the number of entries. */
static _anim_id_entry* _build_anim_id_table(psx_svg_player* p, uint32_t* out_count)
{
    uint32_t n = psx_array_size(&p->anims);
    *out_count = 0;
    if (n == 0) {
        return NULL;
    }

    /* First pass: count entries with non-NULL id */
    uint32_t id_count = 0;
    for (uint32_t i = 0; i < n; i++) {
        const psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
        if (it && it->anim_node) {
            const char* cid = it->anim_node->content(NULL);
            if (cid) {
                id_count++;
            }
        }
    }

    if (id_count == 0) {
        return NULL;
    }

    _anim_id_entry* table = (_anim_id_entry*)mem_malloc(id_count * sizeof(_anim_id_entry));
    if (!table) {
        return NULL;
    }

    /* Second pass: fill entries */
    uint32_t idx = 0;
    for (uint32_t i = 0; i < n; i++) {
        const psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
        if (it && it->anim_node) {
            const char* cid = it->anim_node->content(NULL);
            if (cid) {
                table[idx].id = cid;
                table[idx].index = i;
                idx++;
            }
        }
    }

    qsort(table, id_count, sizeof(_anim_id_entry), _anim_id_entry_cmp);
    *out_count = id_count;
    return table;
}

/* Binary search for an animation by id. Returns pointer into p->anims or NULL. */
static const psx_svg_anim_item* _find_anim_by_id(psx_svg_player* p,
                                                 const _anim_id_entry* table,
                                                 uint32_t table_count,
                                                 const char* id)
{
    if (!table || table_count == 0 || !id) {
        return NULL;
    }

    uint32_t lo = 0;
    uint32_t hi = table_count;
    while (lo < hi) {
        uint32_t mid = lo + (hi - lo) / 2;
        int cmp = strcmp(id, table[mid].id);
        if (cmp < 0) {
            hi = mid;
        } else if (cmp > 0) {
            lo = mid + 1;
        } else {
            return psx_array_get(&p->anims, table[mid].index, psx_svg_anim_item);
        }
    }
    return NULL;
}

/*
 * Resolve syncbase dependencies after all animations have been collected.
 * Uses iterative multi-pass resolution to handle chains (A->B->C).
 * Detects circular dependencies and missing references.
 */
static void _resolve_syncbase_deps(psx_svg_player* p)
{
    if (!p) {
        return;
    }

    uint32_t n = psx_array_size(&p->anims);
    if (n == 0) {
        return;
    }

    /* Build id lookup table for O(log n) search */
    uint32_t table_count = 0;
    _anim_id_entry* id_table = _build_anim_id_table(p, &table_count);

    // Multiple passes to resolve chains. Max passes = n.
    for (uint32_t pass = 0; pass < n; pass++) {
        bool any_resolved = false;

        for (uint32_t i = 0; i < n; i++) {
            psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
            if (!it || !it->syncbase_ref_id || it->syncbase_resolved || it->syncbase_circular) {
                continue;
            }

            // Find the referenced animation by id — O(log n) binary search
            const psx_svg_anim_item* ref = _find_anim_by_id(p, id_table, table_count, it->syncbase_ref_id);

            if (!ref) {
                // Missing reference: treat as indefinite
                it->syncbase_circular = true;
                continue;
            }

            // Check if ref itself has an unresolved syncbase dependency
            if (ref->syncbase_ref_id && !ref->syncbase_resolved && !ref->syncbase_circular) {
                // Ref not yet resolved — try again in next pass
                continue;
            }

            if (ref->syncbase_circular) {
                // Ref is circular — we are too
                it->syncbase_circular = true;
                continue;
            }

            // Ref is resolved (or has no syncbase dependency). Compute our begin time.
            float ref_time = 0.0f;
            if (it->syncbase_type == 1) {
                // end: ref.begin + ref.active_dur
                float active_dur = ref->dur_sec;
                if (ref->repeat_count > 1) {
                    active_dur = ref->dur_sec * (float)ref->repeat_count;
                }
                if (ref->repeat_dur_sec > 0.0f && ref->repeat_dur_sec < active_dur) {
                    active_dur = ref->repeat_dur_sec;
                }
                ref_time = ref->begin_sec + active_dur;
            } else {
                // begin: ref.begin
                ref_time = ref->begin_sec;
            }

            float begin_time = ref_time + it->syncbase_offset_sec;
            if (begin_time < 0.0f) {
                begin_time = 0.0f;
            }

            it->begin_sec = begin_time;
            _sbo_push(&it->begins_sec, begin_time);
            _anim_item_begin_list_normalize(it);
            it->syncbase_resolved = true;
            any_resolved = true;
        }

        if (!any_resolved) {
            break; // No progress — remaining are circular
        }
    }

    // Mark any still-unresolved syncbase items as circular
    for (uint32_t i = 0; i < n; i++) {
        psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
        if (it && it->syncbase_ref_id && !it->syncbase_resolved && !it->syncbase_circular) {
            it->syncbase_circular = true;
        }
    }

    /* Free the temporary lookup table */
    if (id_table) {
        mem_free(id_table);
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
    *out_v = it->u.fval;
    return true;
}

bool psx_svg_anim_get_int32(const psx_svg_anim_state* s,
                            const psx_svg_node* target,
                            psx_svg_attr_type attr,
                            int32_t* out_v)
{
    if (!out_v) {
        return false;
    }
    *out_v = 0;
    const psx_svg_anim_override_item* it = _anim_state_find(s, target, attr);
    if (!it) {
        return false;
    }
    *out_v = it->u.ival;
    return true;
}

bool psx_svg_anim_get_color(const psx_svg_anim_state* s,
                            const psx_svg_node* target,
                            psx_svg_attr_type attr,
                            uint32_t* out_color)
{
    if (!out_color) {
        return false;
    }
    *out_color = 0;
    const psx_svg_anim_color_item* it = _anim_state_find_color(s, target, attr);
    if (!it) {
        return false;
    }
    *out_color = it->color;
    return true;
}

// Compose motion_transform * animateTransform per SVG spec.
const ps_matrix* psx_svg_anim_get_transform(const psx_svg_anim_state* s,
                                            const psx_svg_node* target)
{
    const psx_svg_anim_transform_item* at = _anim_state_find_transform(s, target);
    const psx_svg_anim_transform_item* mt = _anim_state_find_motion_transform(s, target);

    if (!at && !mt) {
        return NULL;
    }

    if (mt && !at) {
        ps_matrix_init(s->scratch_matrix, mt->a, mt->b, mt->c, mt->d, mt->e, mt->f);
        return s->scratch_matrix;
    }

    if (at && !mt) {
        ps_matrix_init(s->scratch_matrix, at->a, at->b, at->c, at->d, at->e, at->f);
        return s->scratch_matrix;
    }

    // Both layers: result = motion * animateTransform
    float ma = mt->a, mb = mt->b, mc = mt->c, md = mt->d, me = mt->e, mf = mt->f;
    float aa = at->a, ab = at->b, ac = at->c, ad = at->d, ae = at->e, af = at->f;

    float ra = ma * aa + mc * ab;
    float rb = mb * aa + md * ab;
    float rc = ma * ac + mc * ad;
    float rd = mb * ac + md * ad;
    float re = ma * ae + mc * af + me;
    float rf = mb * ae + md * af + mf;

    ps_matrix_init(s->scratch_matrix, ra, rb, rc, rd, re, rf);
    return s->scratch_matrix;
}

const psx_svg_anim_transform_item* psx_svg_anim_state_find_transform(const psx_svg_anim_state* s, const psx_svg_node* target)
{
    return _anim_state_find_transform(s, target);
}

bool psx_svg_anim_get_dash(const psx_svg_anim_state* s, const psx_svg_node* target,
                           const float** out_dashes, uint32_t* out_count)
{
    if (!s || !target || !out_dashes || !out_count) {
        return false;
    }
    uint32_t n = psx_array_size((psx_array*)&s->dash_overrides);
    for (uint32_t i = 0; i < n; i++) {
        const psx_svg_anim_dash_item* di = psx_array_get((psx_array*)&s->dash_overrides, i, psx_svg_anim_dash_item);
        if (di && di->target == target) {
            *out_dashes = di->dashes;
            *out_count = di->count;
            return true;
        }
    }
    return false;
}

psx_svg_player* PICAPI psx_svg_player_create(const psx_svg* root, psx_result* out)
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

    p->root = (const psx_svg_node*)root;
    p->loop = false;
    p->dpi = 96; // default dpi

    p->render_list = psx_svg_render_list_create(p->root);
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

    psx_array_capacity_init(&p->anim_state.overrides, 2, sizeof(psx_svg_anim_override_item));
    psx_array_capacity_init(&p->anim_state.transforms, 2, sizeof(psx_svg_anim_transform_item));
    psx_array_capacity_init(&p->anim_state.motion_transforms, 2, sizeof(psx_svg_anim_transform_item));
    psx_array_capacity_init(&p->anim_state.dash_overrides, 2, sizeof(psx_svg_anim_dash_item));
    psx_array_capacity_init(&p->anim_state.color_overrides, 2, sizeof(psx_svg_anim_color_item));
    p->anim_state.scratch_matrix = ps_matrix_create();

    psx_svg_render_list_set_anim_state(p->render_list, &p->anim_state);

    psx_array_init(&p->anims, sizeof(psx_svg_anim_item));
    _collect_anims(p, p->root);
    _resolve_syncbase_deps(p);

    // compute duration hint
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

void PICAPI psx_svg_player_destroy(psx_svg_player* p)
{
    if (!p) {
        return;
    }

    // free per-item dynamic memory
    {
        uint32_t n = psx_array_size(&p->anims);
        for (uint32_t i = 0; i < n; i++) {
            psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
            if (it && it->motion_cache) {
                _motion_cache_destroy(it->motion_cache);
                it->motion_cache = NULL;
            }
            _anim_item_begin_list_destroy(it);
            _anim_item_end_list_destroy(it);
        }
    }

    psx_array_destroy(&p->anim_state.overrides);
    psx_array_destroy(&p->anim_state.transforms);
    psx_array_destroy(&p->anim_state.motion_transforms);

    /* Free dash override buffers up to high-water mark (includes ghost entries). */
    {
        uint32_t nd = p->anim_state.dash_high_water;
        psx_svg_anim_dash_item* arr = (psx_svg_anim_dash_item*)p->anim_state.dash_overrides.data;
        for (uint32_t i = 0; i < nd && arr; i++) {
            if (arr[i].dashes) {
                mem_free(arr[i].dashes);
            }
        }
        psx_array_destroy(&p->anim_state.dash_overrides);
    }

    psx_array_destroy(&p->anim_state.color_overrides);

    if (p->anim_state.active_targets) {
        mem_free(p->anim_state.active_targets);
        p->anim_state.active_targets = NULL;
    }

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

void PICAPI psx_svg_player_play(psx_svg_player* p)
{
    if (!p) {
        return;
    }
    if (p->state == PSX_SVG_PLAYER_STOPPED) {
        p->time_sec = 0.0f;
    }
    p->state = PSX_SVG_PLAYER_PLAYING;
}

void PICAPI psx_svg_player_pause(psx_svg_player* p)
{
    if (!p) {
        return;
    }
    if (p->state != PSX_SVG_PLAYER_STOPPED) {
        p->state = PSX_SVG_PLAYER_PAUSED;
    }
}

void PICAPI psx_svg_player_stop(psx_svg_player* p)
{
    if (!p) {
        return;
    }
    p->state = PSX_SVG_PLAYER_STOPPED;
    p->time_sec = 0.0f;
}

/*
 * Fire per-animation event callbacks (BEGIN / END / REPEAT) by comparing
 * each item's current active state and iteration with the previous frame.
 */
static void _anim_fire_callbacks(psx_svg_player* p)
{
    if (!p || !p->cb) {
        return;
    }

    uint32_t n = psx_array_size(&p->anims);
    for (uint32_t i = 0; i < n; i++) {
        psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
        if (!it) {
            continue;
        }

        /* Use cached eval-stage active state instead of recomputing. */
        bool active = it->eval_active;
        uint32_t cur_iter = it->eval_iteration;

        const char* anim_id = it->anim_node ? it->anim_node->content(NULL) : NULL;

        if (active && !it->was_active) {
            /* Transition inactive → active: fire BEGIN. */
            p->cb(PSX_SVG_ANIM_EVENT_BEGIN, anim_id, p->cb_user);
        }

        if (!active && it->was_active) {
            /* Transition active → inactive: fire END. */
            p->cb(PSX_SVG_ANIM_EVENT_END, anim_id, p->cb_user);
        }

        if (active && it->was_active && cur_iter > it->last_iteration) {
            /* Crossed one or more iteration boundaries: fire REPEAT for each. */
            uint32_t crosses = cur_iter - it->last_iteration;
            for (uint32_t k = 0; k < crosses; k++) {
                p->cb(PSX_SVG_ANIM_EVENT_REPEAT, anim_id, p->cb_user);
            }
        }

        it->was_active = active;
        it->last_iteration = cur_iter;
    }
}

static int _ptr_cmp(const void* a, const void* b)
{
    uintptr_t pa = *(const uintptr_t*)a;
    uintptr_t pb = *(const uintptr_t*)b;
    if (pa < pb) { return -1; }
    if (pa > pb) { return 1; }
    return 0;
}

static void _build_active_targets(psx_svg_anim_state* s)
{
    /* Count max possible targets (upper bound). */
    uint32_t max_count = psx_array_size(&s->overrides)
                         + psx_array_size(&s->color_overrides)
                         + psx_array_size(&s->transforms)
                         + psx_array_size(&s->motion_transforms)
                         + psx_array_size(&s->dash_overrides);
    if (max_count == 0) {
        s->active_target_count = 0;
        return;
    }

    /* Ensure capacity. */
    if (max_count > s->active_target_cap) {
        const psx_svg_node** new_arr = (const psx_svg_node**)mem_realloc(
                                           s->active_targets, max_count * sizeof(const psx_svg_node*));
        if (!new_arr) {
            s->active_target_count = 0;
            return;
        }
        s->active_targets = new_arr;
        s->active_target_cap = max_count;
    }

    uint32_t n = 0;
    uint32_t i, sz;

    /* Collect from overrides. */
    sz = psx_array_size(&s->overrides);
    for (i = 0; i < sz; i++) {
        const psx_svg_anim_override_item* it = psx_array_get(&s->overrides, i, psx_svg_anim_override_item);
        if (it) { s->active_targets[n++] = it->target; }
    }

    /* Collect from color_overrides. */
    sz = psx_array_size(&s->color_overrides);
    for (i = 0; i < sz; i++) {
        const psx_svg_anim_color_item* it = psx_array_get(&s->color_overrides, i, psx_svg_anim_color_item);
        if (it) { s->active_targets[n++] = it->target; }
    }

    /* Collect from transforms. */
    sz = psx_array_size(&s->transforms);
    for (i = 0; i < sz; i++) {
        const psx_svg_anim_transform_item* it = psx_array_get(&s->transforms, i, psx_svg_anim_transform_item);
        if (it) { s->active_targets[n++] = it->target; }
    }

    /* Collect from motion_transforms. */
    sz = psx_array_size(&s->motion_transforms);
    for (i = 0; i < sz; i++) {
        const psx_svg_anim_transform_item* it = psx_array_get(&s->motion_transforms, i, psx_svg_anim_transform_item);
        if (it) { s->active_targets[n++] = it->target; }
    }

    /* Collect from dash_overrides. */
    sz = psx_array_size(&s->dash_overrides);
    for (i = 0; i < sz; i++) {
        const psx_svg_anim_dash_item* it = psx_array_get(&s->dash_overrides, i, psx_svg_anim_dash_item);
        if (it) { s->active_targets[n++] = it->target; }
    }

    if (n == 0) {
        s->active_target_count = 0;
        return;
    }

    /* Sort by pointer value. */
    qsort(s->active_targets, n, sizeof(const psx_svg_node*), _ptr_cmp);

    /* Dedup. */
    uint32_t w = 1;
    for (i = 1; i < n; i++) {
        if (s->active_targets[i] != s->active_targets[w - 1]) {
            s->active_targets[w++] = s->active_targets[i];
        }
    }
    s->active_target_count = w;
}

static void _apply_animations_at_time(psx_svg_player* p)
{
    if (!p) {
        return;
    }

    _anim_state_reset(&p->anim_state);

    uint32_t n = psx_array_size(&p->anims);
    for (uint32_t i = 0; i < n; i++) {
        psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
        if (!it) {
            continue;
        }

        if (!(it->tag == SVG_TAG_ANIMATE || it->tag == SVG_TAG_SET || it->tag == SVG_TAG_ANIMATE_COLOR
              || it->tag == SVG_TAG_ANIMATE_TRANSFORM || it->tag == SVG_TAG_ANIMATE_MOTION)) {
            /* Unsupported tag — not evaluated. */
            it->eval_active = false;
            it->eval_iteration = 0;
            continue;
        }
        if (!_is_supported_anim_attr(it->target_attr)) {
            /* Unsupported attr — not evaluated. */
            it->eval_active = false;
            it->eval_iteration = 0;
            continue;
        }

        /* Fast cull — skip animations past their active window (fill=remove only) */
        if (it->fill_mode != SVG_ANIMATION_FREEZE && p->time_sec > it->max_active_end_sec) {
            it->eval_active = false;
            it->eval_iteration = 0;
            continue;
        }

        /* Record active state once for callback reuse. */
        {
            uint32_t ei = 0;
            it->eval_active = _anim_is_active(it, p->time_sec, &ei);
            it->eval_iteration = ei;
        }

        if (it->tag == SVG_TAG_ANIMATE_MOTION) {
            float a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;
            if (_anim_eval_motion(it, p->time_sec, &a, &b, &c, &d, &e, &f)) {
                _anim_state_set_motion_transform(&p->anim_state, it->target_node, a, b, c, d, e, f);
            }
            continue;
        }

        if (it->tag == SVG_TAG_ANIMATE_TRANSFORM && it->target_attr == SVG_ATTR_TRANSFORM) {
            float a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;
            if (_anim_eval_transform_dispatch(it, p->time_sec, &a, &b, &c, &d, &e, &f)) {
                if (it->additive_mode == SVG_ANIMATION_ADDITIVE_SUM) {
                    _anim_state_compose_transform(&p->anim_state, it->target_node, a, b, c, d, e, f);
                } else {
                    _anim_state_set_transform(&p->anim_state, it->target_node, a, b, c, d, e, f);
                }
            }
            continue;
        }

        /* stroke-dasharray: array interpolation. */
        if (it->target_attr == SVG_ATTR_STROKE_DASH_ARRAY && it->tag == SVG_TAG_ANIMATE) {
            _anim_eval_dasharray(it, p->time_sec, &p->anim_state);
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
            if (it->tag == SVG_TAG_ANIMATE_COLOR && _is_color_anim_attr(it->target_attr)) {
                uint32_t color = _f32_to_u32_bits(v);
                _anim_state_set_color(&p->anim_state, it->target_node, it->target_attr, color);
            } else if (_is_int32_anim_attr(it->target_attr)) {
                _anim_state_set_int32(&p->anim_state, it->target_node, it->target_attr, (int32_t)v);
            } else {
                // additive="sum": add DOM base value for numeric attributes.
                if (it->additive_mode == SVG_ANIMATION_ADDITIVE_SUM
                    && it->tag != SVG_TAG_ANIMATE_COLOR) {
                    if (it->has_cached_base_value) {
                        v += it->cached_base_value;
                    } else {
                        v += _anim_get_base_value(it);
                    }
                }
                _anim_state_set_float(&p->anim_state, it->target_node, it->target_attr, v);
            }
        }
    }

    _anim_state_sort_overrides(&p->anim_state);
    _anim_state_sort_color_overrides(&p->anim_state);
    _anim_state_sort_transforms(&p->anim_state.transforms);
    _anim_state_sort_transforms(&p->anim_state.motion_transforms);
    _build_active_targets(&p->anim_state);
}

void PICAPI psx_svg_player_seek(psx_svg_player* p, float seconds)
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

    _apply_animations_at_time(p);

    _anim_fire_callbacks(p);
}

void PICAPI psx_svg_player_tick(psx_svg_player* p, float delta_seconds)
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

    _apply_animations_at_time(p);

    _anim_fire_callbacks(p);
}

float PICAPI psx_svg_player_get_time(const psx_svg_player* p)
{
    return p ? p->time_sec : 0.0f;
}

float PICAPI psx_svg_player_get_duration(const psx_svg_player* p)
{
    return p ? p->duration_sec : -1.0f;
}

psx_svg_player_state PICAPI psx_svg_player_get_state(const psx_svg_player* p)
{
    return p ? p->state : PSX_SVG_PLAYER_STOPPED;
}

void PICAPI psx_svg_player_set_loop(psx_svg_player* p, bool loop)
{
    if (!p) {
        return;
    }
    p->loop = loop;
}

bool PICAPI psx_svg_player_get_loop(const psx_svg_player* p)
{
    return p ? p->loop : false;
}

void PICAPI psx_svg_player_set_dpi(psx_svg_player* p, int32_t dpi)
{
    if (!p) {
        return;
    }
    p->dpi = dpi;
}

int32_t PICAPI psx_svg_player_get_dpi(const psx_svg_player* p)
{
    return p ? p->dpi : 0;
}

void PICAPI psx_svg_player_draw(psx_svg_player* p, ps_context* ctx)
{
    if (!p || !ctx || !p->render_list) {
        return;
    }

    // Ensure animation state is applied.
    psx_svg_render_list_draw_anim(ctx, p->render_list, &p->anim_state);
}

void PICAPI psx_svg_player_set_event_callback(psx_svg_player* p, psx_svg_anim_event_cb cb, void* user)
{
    if (!p) {
        return;
    }
    p->cb = cb;
    p->cb_user = user;
}

void PICAPI psx_svg_player_trigger(psx_svg_player* p, const char* target_id, const char* event_name)
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

    // Pass 1: match begin events.
    for (uint32_t i = 0; i < n; i++) {
        psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
        if (!it || !it->begin_event) {
            continue;
        }
        if (strcmp(it->begin_event, event_name) != 0) {
            continue;
        }
        // id.event filtering: if animation specifies a target id, require match.
        if (it->begin_event_target_id) {
            if (!filter_id || strcmp(it->begin_event_target_id, filter_id) != 0) {
                continue;
            }
        } else if (filter_id) {
            // Backward compatible: no id prefix in begin token, but caller
            // supplied a target_id — match against target/anim node ids.
            const char* tid = it->target_node ? it->target_node->content(NULL) : NULL;
            const char* aid = it->anim_node ? it->anim_node->content(NULL) : NULL;
            if ((!tid || strcmp(tid, filter_id) != 0) && (!aid || strcmp(aid, filter_id) != 0)) {
                continue;
            }
        }

        float sec = p->time_sec + it->begin_event_offset_sec;

        // restart guard: check restart_mode before appending new begin.
        if (it->restart_mode == SVG_ANIMATION_RESTART_NEVER) {
            if (!_sbo_empty(&it->begins_sec)) {
                continue; // already activated once; ignore
            }
        } else if (it->restart_mode == SVG_ANIMATION_RESTART_WHEN_NOT_ACTIVE) {
            // Check if currently active: find current begin and see if sec < begin + active_dur.
            float ad = _anim_item_active_dur(it);
            float cur_begin = _anim_item_begin_for_time(it, sec);
            if (cur_begin < 1e29f && sec >= cur_begin && sec < cur_begin + ad) {
                continue; // currently active; ignore
            }
        }

        _sbo_push(&it->begins_sec, sec);
        _anim_item_begin_list_normalize(it);
        any = true;
    }

    // Pass 2: match end events — only if animation is currently active.
    for (uint32_t i = 0; i < n; i++) {
        psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
        if (!it || !it->end_event) {
            continue;
        }
        if (strcmp(it->end_event, event_name) != 0) {
            continue;
        }
        // id.event filtering for end events.
        if (it->end_event_target_id) {
            if (!filter_id || strcmp(it->end_event_target_id, filter_id) != 0) {
                continue;
            }
        }

        // Only end an animation that is currently active.
        float begin_sec = _anim_item_begin_for_time(it, p->time_sec);
        if (p->time_sec < begin_sec) {
            continue; // not yet started; ignore end event
        }

        float sec = p->time_sec;
        _sbo_push(&it->ends_sec, sec);
        _anim_item_end_list_normalize(it);
        any = true;
    }

    if (!any) {
        return;
    }

    _apply_animations_at_time(p);
}

void PICAPI psx_svg_player_send_key(psx_svg_player* p, char key)
{
    if (!p || !key) {
        return;
    }

    uint32_t n = psx_array_size(&p->anims);
    bool any = false;

    for (uint32_t i = 0; i < n; i++) {
        psx_svg_anim_item* it = psx_array_get(&p->anims, i, psx_svg_anim_item);
        if (!it || it->access_key == '\0') {
            continue;
        }
        if (it->access_key != key) {
            continue;
        }

        float sec = p->time_sec;

        /* restart guard (same as trigger) */
        if (it->restart_mode == SVG_ANIMATION_RESTART_NEVER) {
            if (!_sbo_empty(&it->begins_sec)) {
                continue;
            }
        } else if (it->restart_mode == SVG_ANIMATION_RESTART_WHEN_NOT_ACTIVE) {
            float ad = _anim_item_active_dur(it);
            float cur_begin = _anim_item_begin_for_time(it, sec);
            if (cur_begin < 1e29f && sec >= cur_begin && sec < cur_begin + ad) {
                continue;
            }
        }

        _sbo_push(&it->begins_sec, sec);
        _anim_item_begin_list_normalize(it);
        any = true;
    }

    if (!any) {
        return;
    }

    _apply_animations_at_time(p);
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
