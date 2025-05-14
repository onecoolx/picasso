/* Picasso - a vector graphics library
 *
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GRAPHIC_DEVICE_H_
#define _GRAPHIC_DEVICE_H_

#include "common.h"
#include "math_type.h"
#include "color_type.h"
#include "matrix.h"

namespace picasso {

class abstract_painter;
class abstract_raster_adapter;
class abstract_rendering_buffer;
class abstract_mask_layer;
class abstract_gradient_adapter;

class device
{
public:
    virtual ~device() { }

    // painter interface
    virtual abstract_painter* create_painter(pix_fmt fmt) = 0;
    virtual void destroy_painter(abstract_painter* p) = 0;

    // raster adapter
    virtual abstract_raster_adapter* create_raster_adapter(void) = 0;
    virtual void destroy_raster_adapter(abstract_raster_adapter* d) = 0;

    // redering buffer
    virtual abstract_rendering_buffer* create_rendering_buffer(byte* buf,
                                                               uint32_t width, uint32_t height, int32_t stride) = 0;
    virtual void destroy_rendering_buffer(abstract_rendering_buffer* b) = 0;

    // masking layer
    virtual abstract_mask_layer* create_mask_layer(byte* buf,
                                                   uint32_t width, uint32_t height, int32_t stride) = 0;
    virtual void destroy_mask_layer(abstract_mask_layer* m) = 0;

    // gradient adapter
    virtual abstract_gradient_adapter* create_gradient_adapter(void) = 0;
    virtual void destroy_gradient_adapter(abstract_gradient_adapter* g) = 0;
protected:
    device() { }
};

bool _init_system_device(void);
void _destroy_system_device(void);

bool is_valid_system_device(void);
device* get_system_device(void);
}
#endif/*_GRAPHIC_DEVICE_H_*/
