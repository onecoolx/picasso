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

#ifndef _PSX_SVG_ANIM_STATE_H_
#define _PSX_SVG_ANIM_STATE_H_

#include "psx_svg_node.h"
#include "psx_svg_player.h"
#include "psx_svg_parser.h"
#include "picasso.h"

#include <math.h>

#ifndef M_PI
    #define M_PI 3.1415926f
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psx_svg_motion_cache psx_svg_motion_cache;

typedef struct {
    const psx_svg_node* target;
    float* dashes;
    uint32_t count;
} psx_svg_anim_dash_item;

// opaque animation override state passed to renderer.
struct psx_svg_anim_state {
    psx_array overrides;
    psx_array transforms; // animateTransform overrides
    psx_array motion_transforms; // animateMotion overrides (independent layer)
    psx_array dash_overrides; // stroke-dasharray overrides (psx_svg_anim_dash_item[])
    ps_matrix* scratch_matrix; // reused each frame; owned by this struct
};

struct psx_svg_player {
    const psx_svg_node* root;
    psx_svg_render_list* render_list;

    psx_svg_anim_state anim_state;

    psx_svg_player_state state;
    bool loop;

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
    psx_svg_motion_cache* motion_cache; // animateMotion path cache (owned by player)
    float begin_sec; // kept for duration hint compatibility
    float dur_sec;
    float end_sec; // optional explicit end, 0 => unspecified
    uint32_t repeat_count; // 0 => indefinite
    float repeat_dur_sec; // optional explicit repeat duration, 0 => unspecified
    uint32_t fill_mode; // SVG_ANIMATION_*
    uint32_t additive_mode; // SVG_ANIMATION_ADDITIVE_REPLACE or SVG_ANIMATION_ADDITIVE_SUM
    uint32_t restart_mode; // SVG_ANIMATION_RESTART_ALWAYS / WHEN_NOT_ACTIVE / NEVER
    float min_sec; // minimum active duration (0 = unspecified)
    float max_sec; // maximum active duration (0 = indefinite/unspecified)

    // Minimal Tiny 1.2 external event trigger support.
    // If begin is specified as a non-numeric token (e.g. begin="click"), we
    // store the trigger name and allow external callers to start the animation
    // via psx_svg_player_trigger(). Owned by the player.
    const char* begin_event;
    float begin_event_offset_sec; // event+offset: delay in seconds after trigger
    const char* begin_event_target_id; // id.event: "btn" from "btn.click", NULL if none

    // End event trigger support (e.g. end="click").
    // When matched, current time is appended to ends_sec. Owned by the player.
    const char* end_event;
    const char* end_event_target_id; // id.event: target id for end event, NULL if none

    // Begin list support: store begin times (sec) and choose the latest begin <= doc_t.
    psx_array begins_sec;

    // End list support: store end times (sec) and choose the earliest end >= begin (per trigger).
    psx_array ends_sec;

    uint32_t accumulate_mode; // SVG_ANIMATION_ACCUMULATE_NONE or _SUM

    bool was_active;
    uint32_t last_iteration;

    char access_key;

    const char* syncbase_ref_id;
    uint32_t syncbase_type;
    float syncbase_offset_sec; // offset in seconds (e.g., +1s)
    bool syncbase_resolved; // true once begin time has been resolved
    bool syncbase_circular; // true if circular dependency detected
} psx_svg_anim_item;

typedef struct {
    const psx_svg_node* target;
    psx_svg_attr_type attr;
    union {
        float fval;
        int32_t ival;
    } u;
} psx_svg_anim_override_item;

typedef struct {
    const psx_svg_node* target;
    float a, b;
    float c, d;
    float e, f;
} psx_svg_anim_transform_item;

/* returns true if a numeric (float) override exists for (target, attr).
 * writes the value into *out_v. */
bool psx_svg_anim_get_float(const psx_svg_anim_state* s, const psx_svg_node* target,
                            psx_svg_attr_type attr, float* out_v);

/* returns true if an integer override exists for (target, attr).
 * writes the value into *out_v. */
bool psx_svg_anim_get_int32(const psx_svg_anim_state* s, const psx_svg_node* target,
                            psx_svg_attr_type attr, int32_t* out_v);

/* returns a pointer to an internal scratch ps_matrix if a transform override
 * exists for target, NULL otherwise.
 * the returned pointer is valid until the next call to psx_svg_anim_get_transform
 * or until the player is destroyed.*/
const ps_matrix* psx_svg_anim_get_transform(const psx_svg_anim_state* s,
                                            const psx_svg_node* target);

/* returns a pointer to an internal transform item.
 * exists for target, NULL otherwise.
 * the returned pointer is valid until the next call to psx_svg_anim_state_find_transform
 * or until the player is destroyed.*/
const psx_svg_anim_transform_item* psx_svg_anim_state_find_transform(const psx_svg_anim_state* s,
                                                                     const psx_svg_node* target);

/* returns true if a dasharray override exists for target.
 * writes the dash array pointer and count into *out_dashes / *out_count. */
bool psx_svg_anim_get_dash(const psx_svg_anim_state* s, const psx_svg_node* target,
                           const float** out_dashes, uint32_t* out_count);

#ifdef __cplusplus
}
#endif

#endif /* _PSX_SVG_ANIM_STATE_H_ */
