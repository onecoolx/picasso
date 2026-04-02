/**
 * \file psx_svg_animation.h
 * \author Zhang Ji Peng <onecoolx@gmail.com>
 * \date 2025/5/12
 *
 * This file includes all interfaces of picasso's SVG extension function.
 \verbatim

    Copyright (C) 2008 ~ 2026  Zhang Ji Peng

    All rights reserved.

    Picasso is a vector graphic library.

 \endverbatim
 */

#ifndef _PSX_SVG_ANIMATION_H_
#define _PSX_SVG_ANIMATION_H_

#include "psx_svg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup extension Extensions
 * @{
 */
/**
 * \defgroup extsvg XSvg Extension
 * @{
 */
/**
 * \defgroup svgtypes XSvg Object Types
 * @{
 */

/**
 * \typedef psx_svg_player
 * \brief An opaque type represents an svg animation player.
 * \sa psx_svg_render, psx_svg
 */
typedef struct _psx_svg_player psx_svg_player;

/**
 * \brief SVG animation player status.
 */
typedef enum {
    /**
     * Player is stoped.
     */
    PSX_SVG_PLAYER_STOPPED = 0,
    /**
     * Player is playing.
     */
    PSX_SVG_PLAYER_PLAYING = 1,
    /**
     * Player is paused.
     */
    PSX_SVG_PLAYER_PAUSED = 2,
} psx_svg_player_state;

/**
 * \brief SVG animation event callback types.
 */
typedef enum {
    /**
     * SVG animation begin event.
     */
    PSX_SVG_ANIM_EVENT_BEGIN = 0,
    /**
     * SVG animation end event.
     */
    PSX_SVG_ANIM_EVENT_END,
    /**
     * SVG animation repeat event.
     */
    PSX_SVG_ANIM_EVENT_REPEAT,
} psx_svg_anim_event_type;

/**
 * \typedef psx_svg_anim_event_cb
 * \brief The callback for SVG animation event.
 */
typedef void (*psx_svg_anim_event_cb)(psx_svg_anim_event_type type, const char* anim_id, void* user_data);

/** @} end of extsvg svgtypes */

/**
 * \defgroup svgfunctions XSvg Functions
 * @{
 */

// Create a player from an already-parsed SVG DOM root.
// The player internally builds a render list once and reuses it each frame.
PEXPORT psx_svg_player* PICAPI psx_svg_player_create(const psx_svg* root, psx_result* err);
PEXPORT void PICAPI psx_svg_player_destroy(psx_svg_player* p);

// Loop
PEXPORT void PICAPI psx_svg_player_set_loop(psx_svg_player* p, bool loop);
PEXPORT bool PICAPI psx_svg_player_get_loop(const psx_svg_player* p);

// Dpi
PEXPORT void PICAPI psx_svg_player_set_dpi(psx_svg_player* p, int32_t dpi);
PEXPORT int32_t PICAPI psx_svg_player_get_dpi(const psx_svg_player* p);

// Playback control
PEXPORT void PICAPI psx_svg_player_play(psx_svg_player* p);
PEXPORT void PICAPI psx_svg_player_pause(psx_svg_player* p);
PEXPORT void PICAPI psx_svg_player_stop(psx_svg_player* p); // seek to 0 and stop
PEXPORT void PICAPI psx_svg_player_seek(psx_svg_player* p, float seconds);
PEXPORT void PICAPI psx_svg_player_tick(psx_svg_player* p, float delta_seconds);

// Time queries
PEXPORT float PICAPI psx_svg_player_get_time(const psx_svg_player* p);
PEXPORT float PICAPI psx_svg_player_get_duration(const psx_svg_player* p); // -1 for indefinite/unknown
PEXPORT psx_svg_player_state PICAPI psx_svg_player_get_state(const psx_svg_player* p);

// Rendering
// Draws current frame into the provided Picasso context.
PEXPORT void PICAPI psx_svg_player_draw(psx_svg_player* p, ps_context* ctx);

// Events (optional)
PEXPORT void PICAPI psx_svg_player_set_event_callback(psx_svg_player* p, psx_svg_anim_event_cb cb, void* user);

// External triggers (reserved for begin="id.event" style timing).
// Minimal implementation may treat this as a simple start trigger by target id.
PEXPORT void PICAPI psx_svg_player_trigger(psx_svg_player* p, const char* target_id, const char* event_name);

// Keyboard event forwarding for begin="accessKey(x)" timing.
PEXPORT void PICAPI psx_svg_player_send_key(psx_svg_player* p, char key);

/** @} end of extsvg svgfunctions */
/** @} end of extsvg */
/** @} end of extensions */

#ifdef __cplusplus
}
#endif

#endif /* _PSX_SVG_ANIMATION_H_ */
