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

#ifndef _PSX_SVG_PLAYER_H_
#define _PSX_SVG_PLAYER_H_

#include "picasso_ext.h"
#include "psx_common.h"
#include "psx_svg_node.h"
#include "psx_svg_render.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psx_svg_player psx_svg_player;

// Player status
typedef enum {
    PSX_SVG_PLAYER_STOPPED = 0,
    PSX_SVG_PLAYER_PLAYING = 1,
    PSX_SVG_PLAYER_PAUSED = 2,
} psx_svg_player_state;

// Animation event callback (optional)
typedef enum {
    PSX_SVG_ANIM_EVENT_BEGIN = 0,
    PSX_SVG_ANIM_EVENT_END,
    PSX_SVG_ANIM_EVENT_REPEAT,
} psx_svg_anim_event_type;

typedef void (*psx_svg_anim_event_cb)(psx_svg_anim_event_type type, const char* anim_id, void* user_data);

// Create a player from an already-parsed SVG DOM root.
// The player internally builds a render list once and reuses it each frame.
psx_svg_player* psx_svg_player_create(const psx_svg_node* root, psx_result* err);
void psx_svg_player_destroy(psx_svg_player* p);

// Loop
void psx_svg_player_set_loop(psx_svg_player* p, bool loop);
bool psx_svg_player_get_loop(const psx_svg_player* p);

// Dpi
void psx_svg_player_set_dpi(psx_svg_player* p, int32_t dpi);
int32_t psx_svg_player_get_dpi(const psx_svg_player* p);

// Playback control
void psx_svg_player_play(psx_svg_player* p);
void psx_svg_player_pause(psx_svg_player* p);
void psx_svg_player_stop(psx_svg_player* p); // seek to 0 and stop
void psx_svg_player_seek(psx_svg_player* p, float seconds);
void psx_svg_player_tick(psx_svg_player* p, float delta_seconds);

// Time queries
float psx_svg_player_get_time(const psx_svg_player* p);
float psx_svg_player_get_duration(const psx_svg_player* p); // -1 for indefinite/unknown
psx_svg_player_state psx_svg_player_get_state(const psx_svg_player* p);

// Rendering
// Draws current frame into the provided Picasso context.
void psx_svg_player_draw(psx_svg_player* p, ps_context* ctx);

// Events (optional)
void psx_svg_player_set_event_callback(psx_svg_player* p, psx_svg_anim_event_cb cb, void* user);

// External triggers (reserved for begin="id.event" style timing).
// Minimal implementation may treat this as a simple start trigger by target id.
void psx_svg_player_trigger(psx_svg_player* p, const char* target_id, const char* event_name);

// DOM helpers
// Returns node by element id, or NULL if not found.
const psx_svg_node* psx_svg_player_get_node_by_id(const psx_svg_player* p, const char* id);

// Debug test hooks for property-based testing of motion path internals.
// These are NOT part of the public API — used only by unit tests.
bool psx_svg_player_debug_motion_path_parse(const char* path_str, uint32_t len,
                                            float** out_xs, float** out_ys, uint32_t* out_count);
char* psx_svg_player_debug_motion_path_format(const float* xs, const float* ys, uint32_t count);
void psx_svg_player_debug_motion_path_free(float* xs, float* ys);
void psx_svg_player_debug_motion_path_free_str(char* str);
bool psx_svg_player_debug_arc_length_position(const float* xs, const float* ys, uint32_t count,
                                              float t, float* out_x, float* out_y);

#ifdef __cplusplus
}
#endif

#endif /* _PSX_SVG_PLAYER_H_ */
