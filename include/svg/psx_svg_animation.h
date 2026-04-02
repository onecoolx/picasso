/**
 * \file psx_svg_animation.h
 * \author Zhang Ji Peng <onecoolx@gmail.com>
 * \date 2025/5/12
 *
 * This file includes all interfaces of picasso's SVG animation extension.
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
     * Player is stopped.
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
 *
 * \param type       The animation event type.
 * \param anim_id    The id of the animation element, can be NULL.
 * \param user_data  User data pointer passed to \a psx_svg_player_set_event_callback.
 */
typedef void (*psx_svg_anim_event_cb)(psx_svg_anim_event_type type, const char* anim_id, void* user_data);

/** @} end of extsvg svgtypes */

/**
 * \defgroup svgfunctions XSvg Functions
 * @{
 */

/**
 * \fn psx_svg_player* psx_svg_player_create(const psx_svg* root, psx_result* err)
 * \brief Create a new animation player from an already-parsed SVG document.
 *
 * The player internally builds a render list once and reuses it each frame.
 *
 * \param root  Pointer to an existing psx_svg object.
 * \param err   Pointer to a value to receiving the result code. Can be NULL.
 *
 * \return If success, the return value is the pointer to new psx_svg_player object.
 *         If fails, the return value is NULL, and result will be returned by \a err.
 *
 * \sa psx_svg_player_destroy
 */
PEXPORT psx_svg_player* PICAPI psx_svg_player_create(const psx_svg* root, psx_result* err);

/**
 * \fn void psx_svg_player_destroy(psx_svg_player* p)
 * \brief Destroy the psx_svg_player object and release resources.
 *
 * \param p  Pointer to an existing psx_svg_player object.
 *
 * \sa psx_svg_player_create
 */
PEXPORT void PICAPI psx_svg_player_destroy(psx_svg_player* p);

/**
 * \fn void psx_svg_player_set_loop(psx_svg_player* p, bool loop)
 * \brief Set whether the animation should loop when it reaches the end.
 *
 * \param p     Pointer to an existing psx_svg_player object.
 * \param loop  True to enable looping, False to disable.
 *
 * \sa psx_svg_player_get_loop
 */
PEXPORT void PICAPI psx_svg_player_set_loop(psx_svg_player* p, bool loop);

/**
 * \fn bool psx_svg_player_get_loop(const psx_svg_player* p)
 * \brief Return whether the animation looping is enabled.
 *
 * \param p  Pointer to an existing psx_svg_player object.
 *
 * \return True if looping is enabled, otherwise False.
 *
 * \sa psx_svg_player_set_loop
 */
PEXPORT bool PICAPI psx_svg_player_get_loop(const psx_svg_player* p);

/**
 * \fn void psx_svg_player_set_dpi(psx_svg_player* p, int32_t dpi)
 * \brief Set the DPI (dots per inch) used for unit conversion.
 *
 * \param p    Pointer to an existing psx_svg_player object.
 * \param dpi  The DPI value. Default is 96.
 *
 * \sa psx_svg_player_get_dpi
 */
PEXPORT void PICAPI psx_svg_player_set_dpi(psx_svg_player* p, int32_t dpi);

/**
 * \fn int32_t psx_svg_player_get_dpi(const psx_svg_player* p)
 * \brief Return the current DPI value.
 *
 * \param p  Pointer to an existing psx_svg_player object.
 *
 * \return The current DPI value.
 *
 * \sa psx_svg_player_set_dpi
 */
PEXPORT int32_t PICAPI psx_svg_player_get_dpi(const psx_svg_player* p);

/**
 * \fn void psx_svg_player_play(psx_svg_player* p)
 * \brief Start or resume playback of the animation.
 *
 * \param p  Pointer to an existing psx_svg_player object.
 *
 * \sa psx_svg_player_pause psx_svg_player_stop psx_svg_player_get_state
 */
PEXPORT void PICAPI psx_svg_player_play(psx_svg_player* p);

/**
 * \fn void psx_svg_player_pause(psx_svg_player* p)
 * \brief Pause the animation at the current time.
 *
 * \param p  Pointer to an existing psx_svg_player object.
 *
 * \sa psx_svg_player_play psx_svg_player_stop psx_svg_player_get_state
 */
PEXPORT void PICAPI psx_svg_player_pause(psx_svg_player* p);

/**
 * \fn void psx_svg_player_stop(psx_svg_player* p)
 * \brief Stop the animation and seek to time 0.
 *
 * \param p  Pointer to an existing psx_svg_player object.
 *
 * \sa psx_svg_player_play psx_svg_player_pause psx_svg_player_get_state
 */
PEXPORT void PICAPI psx_svg_player_stop(psx_svg_player* p);

/**
 * \fn void psx_svg_player_seek(psx_svg_player* p, float seconds)
 * \brief Seek the animation to an absolute time position.
 *
 * \param p        Pointer to an existing psx_svg_player object.
 * \param seconds  The target time in seconds.
 *
 * \sa psx_svg_player_tick psx_svg_player_get_time
 */
PEXPORT void PICAPI psx_svg_player_seek(psx_svg_player* p, float seconds);

/**
 * \fn void psx_svg_player_tick(psx_svg_player* p, float delta_seconds)
 * \brief Advance the animation by a relative time delta.
 *
 * \param p              Pointer to an existing psx_svg_player object.
 * \param delta_seconds  The time increment in seconds since the last tick.
 *
 * \sa psx_svg_player_seek psx_svg_player_get_time
 */
PEXPORT void PICAPI psx_svg_player_tick(psx_svg_player* p, float delta_seconds);

/**
 * \fn float psx_svg_player_get_time(const psx_svg_player* p)
 * \brief Return the current playback time in seconds.
 *
 * \param p  Pointer to an existing psx_svg_player object.
 *
 * \return The current time in seconds.
 *
 * \sa psx_svg_player_seek psx_svg_player_tick
 */
PEXPORT float PICAPI psx_svg_player_get_time(const psx_svg_player* p);

/**
 * \fn float psx_svg_player_get_duration(const psx_svg_player* p)
 * \brief Return the total duration of the animation.
 *
 * \param p  Pointer to an existing psx_svg_player object.
 *
 * \return The duration in seconds, or -1 for indefinite/unknown.
 *
 * \sa psx_svg_player_get_time
 */
PEXPORT float PICAPI psx_svg_player_get_duration(const psx_svg_player* p);

/**
 * \fn psx_svg_player_state psx_svg_player_get_state(const psx_svg_player* p)
 * \brief Return the current playback state of the player.
 *
 * \param p  Pointer to an existing psx_svg_player object.
 *
 * \return The current player state.
 *
 * \sa psx_svg_player_play psx_svg_player_pause psx_svg_player_stop
 */
PEXPORT psx_svg_player_state PICAPI psx_svg_player_get_state(const psx_svg_player* p);

/**
 * \fn void psx_svg_player_draw(psx_svg_player* p, ps_context* ctx)
 * \brief Draw the current animation frame into the provided Picasso context.
 *
 * \param p    Pointer to an existing psx_svg_player object.
 * \param ctx  Pointer to an existing ps_context object.
 *
 * \sa psx_svg_player_seek psx_svg_player_tick
 */
PEXPORT void PICAPI psx_svg_player_draw(psx_svg_player* p, ps_context* ctx);

/**
 * \fn void psx_svg_player_set_event_callback(psx_svg_player* p, psx_svg_anim_event_cb cb, void* user)
 * \brief Set a callback to receive animation events (begin, end, repeat).
 *
 * \param p     Pointer to an existing psx_svg_player object.
 * \param cb    The callback function, or NULL to clear.
 * \param user  User data pointer passed to the callback.
 *
 * \sa psx_svg_anim_event_cb psx_svg_anim_event_type
 */
PEXPORT void PICAPI psx_svg_player_set_event_callback(psx_svg_player* p, psx_svg_anim_event_cb cb, void* user);

/**
 * \fn void psx_svg_player_trigger(psx_svg_player* p, const char* target_id, const char* event_name)
 * \brief Send an external event trigger to the player.
 *
 * Used for animations with begin="id.event" style timing.
 *
 * \param p           Pointer to an existing psx_svg_player object.
 * \param target_id   The target element id, or NULL for untargeted events.
 * \param event_name  The event name (e.g. "click").
 *
 * \sa psx_svg_player_send_key
 */
PEXPORT void PICAPI psx_svg_player_trigger(psx_svg_player* p, const char* target_id, const char* event_name);

/**
 * \fn void psx_svg_player_send_key(psx_svg_player* p, char key)
 * \brief Forward a keyboard event to the player.
 *
 * Used for animations with begin="accessKey(x)" style timing.
 *
 * \param p    Pointer to an existing psx_svg_player object.
 * \param key  The character key pressed.
 *
 * \sa psx_svg_player_trigger
 */
PEXPORT void PICAPI psx_svg_player_send_key(psx_svg_player* p, char key);

/** @} end of extsvg svgfunctions */
/** @} end of extsvg */
/** @} end of extensions */

#ifdef __cplusplus
}
#endif

#endif /* _PSX_SVG_ANIMATION_H_ */
