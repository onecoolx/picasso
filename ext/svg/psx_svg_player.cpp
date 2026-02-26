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

// Opaque animation override state passed to renderer.
// Renderer queries it via svg_anim_get_attr_override (declared in psx_svg_render.cpp).
struct psx_svg_anim_state {
    // Placeholder for future override tables.
    // For now, no overrides are applied.
    uint32_t reserved;
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
};

static psx_svg_player_options _default_options(void)
{
    psx_svg_player_options opt;
    opt.take_ownership_of_root = False;
    opt.loop = False;
    opt.dpi = 96;
    return opt;
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
        p->state = PSX_SVG_PLAYER_PLAYING;
    }

    void psx_svg_player_pause(psx_svg_player* p)
    {
        if (!p) {
            return;
        }
        if (p->state == PSX_SVG_PLAYER_PLAYING) {
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
    }

    void psx_svg_player_tick(psx_svg_player* p, float delta_seconds)
    {
        if (!p) {
            return;
        }
        if (p->state != PSX_SVG_PLAYER_PLAYING) {
            return;
        }

        if (delta_seconds <= 0.0f) {
            return;
        }

        p->time_sec += delta_seconds;

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

        // TODO: evaluate animations and update anim_state override tables.
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

} // extern "C"
