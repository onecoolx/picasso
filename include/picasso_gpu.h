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

#ifdef __cplusplus
}
#endif

#endif /*_PICASSO_GPU_ACCELERATION_H_*/

