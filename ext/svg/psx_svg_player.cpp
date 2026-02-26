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
#include <string.h>

// Opaque animation override state passed to renderer.
// Renderer queries it via svg_anim_get_attr_override (declared in psx_svg_render.cpp).
struct psx_svg_anim_state {
    psx_array overrides;
};

// NOTE: renderer currently does not query overrides. This will be wired when
// the animation override table is implemented and the renderer is updated.

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
    float begin_sec;
    float dur_sec;
    uint32_t repeat_count; // 0 => indefinite
    uint32_t fill_mode; // SVG_ANIMATION_*
} psx_svg_anim_item;

typedef struct {
    const psx_svg_node* target;
    psx_svg_attr_type attr;
    float fval;
} psx_svg_anim_override_item;

static INLINE void _anim_state_reset(psx_svg_anim_state* s)
{
    if (!s) {
        return;
    }
    psx_array_clear(&s->overrides);
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

    if (doc_t < it->begin_sec) {
        return False;
    }

    float local = doc_t - it->begin_sec;

    if (it->repeat_count == 0) {
        // indefinite
        local = _anim_fmod(local, it->dur_sec);
    } else {
        float total = it->dur_sec * (float)it->repeat_count;
        if (local > total) {
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

    const psx_svg_attr* afrom = _find_attr(it->anim_node, SVG_ATTR_FROM);
    const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
    if (!afrom || !ato) {
        return False;
    }

    float v0 = _attr_as_number(afrom);
    float v1 = _attr_as_number(ato);
    *out_v = _anim_lerp(v0, v1, t);
    return True;
}

static INLINE ps_bool _anim_eval_set(const psx_svg_anim_item* it, float doc_t, float* out_v, ps_bool* out_hold)
{
    if (!it || !out_v || !out_hold) {
        return False;
    }
    *out_hold = False;

    if (doc_t < it->begin_sec) {
        return False;
    }

    // If dur is missing/0, SVG <set> is treated as an instant change.
    if (it->dur_sec <= 0.0f) {
        const psx_svg_attr* ato = _find_attr(it->anim_node, SVG_ATTR_TO);
        if (!ato) {
            return False;
        }
        *out_v = _attr_as_number(ato);
        return True;
    }

    float local = doc_t - it->begin_sec;

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

    // Active during [0, dur].
    if (local > it->dur_sec) {
        return False;
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

            item.begin_sec = _attr_as_time_sec(_find_attr(node, SVG_ATTR_BEGIN));
            item.dur_sec = _attr_as_time_sec(_find_attr(node, SVG_ATTR_DUR));
            // parser uses 0 for indefinite
            item.repeat_count = _attr_as_u32(_find_attr(node, SVG_ATTR_REPEAT_COUNT), 1);
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

        psx_array_destroy(&p->anim_state.overrides);

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

            if (!(it->tag == SVG_TAG_ANIMATE || it->tag == SVG_TAG_SET)) {
                continue;
            }
            if (!(it->target_attr == SVG_ATTR_X || it->target_attr == SVG_ATTR_Y || it->target_attr == SVG_ATTR_WIDTH || it->target_attr == SVG_ATTR_HEIGHT || it->target_attr == SVG_ATTR_OPACITY)) {
                continue;
            }

            float v = 0.0f;
            ps_bool hold = False;
            ps_bool ok = False;
            if (it->tag == SVG_TAG_ANIMATE) {
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

            if (!(it->tag == SVG_TAG_ANIMATE || it->tag == SVG_TAG_SET)) {
                continue;
            }
            // start minimal: numeric attributes only
            if (!(it->target_attr == SVG_ATTR_X || it->target_attr == SVG_ATTR_Y || it->target_attr == SVG_ATTR_WIDTH || it->target_attr == SVG_ATTR_HEIGHT || it->target_attr == SVG_ATTR_OPACITY)) {
                continue;
            }

            float v = 0.0f;
            ps_bool hold = False;
            ps_bool ok = False;
            if (it->tag == SVG_TAG_ANIMATE) {
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
        (void)target_id;
        (void)event_name;
        if (!p) {
            return;
        }
        // TODO: implement event-based begin timing.
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

} // extern "C"
