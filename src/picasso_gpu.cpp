/* Picasso - a vector graphics library
 *
 * Copyright (C) 2020 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "device.h"

#include "picasso_gpu.h"

#if ENABLE(EGL)
namespace picasso {

}

#ifdef __cplusplus
extern "C" {
#endif

ps_canvas* PICAPI ps_canvas_create_for_gpu_surface(ps_gr_surface surface, ps_gr_context context)
{
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif
