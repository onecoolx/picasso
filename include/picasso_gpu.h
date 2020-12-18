/**
 * \file picasso_gpu.h
 * \author Zhang Ji Peng <onecoolx@gmail.com>
 * \date 2020/12/16
 *
 * This file includes all interfaces of picasso gpu acceleration.
 \verbatim

    Copyright (C) 2008 ~ 2020  Zhang Ji Peng

    All rights reserved.

    Picasso is a vector graphic library.

 \endverbatim
 */

#ifndef _PICASSO_GPU_ACCELERATION_H_
#define _PICASSO_GPU_ACCELERATION_H_

#include "picasso.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ENABLE_EGL) && (ENABLE_EGL == 1)
#include <EGL/egl.h>
typedef EGLSurface ps_gr_surface;
typedef EGLContext ps_gr_context;
#else
typedef void* ps_gr_surface;
typedef void* ps_gr_context;
#endif

/**
 * \defgroup canvas Canvas
 * @{
 */

/**
 * \fn ps_canvas* PICAPI ps_canvas_create_for_gpu_surface(ps_gr_surface surface, ps_gr_context context)
 * \brief Create a new canvas with a GPU rendering surface for hardware acceleration.
 *
 * \param surface Handle for gpu rendering surface.
 * \param context Handle for gpu rendering context.
 *
 * \return If the function succeeds, the return value is the pointer to a new canvas object.
 *         If the function fails, the return value is NULL.
 *
 * \note To get extended error information, call \a ps_last_status.
 *
 * \sa ps_canvas_create, ps_canvas_create_with_data, ps_canvas_create_compatible,
 *     ps_canvas_create_from_image, ps_canvas_ref, ps_canvas_unref
 */
PEXPORT ps_canvas* PICAPI ps_canvas_create_for_gpu_surface(ps_gr_surface surface, ps_gr_context context);

/** @} end of canvas functions*/

#ifdef __cplusplus
}
#endif

#endif /*_PICASSO_GPU_ACCELERATION_H_*/

