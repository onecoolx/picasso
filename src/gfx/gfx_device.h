/* Picasso - a vector graphics library
 *
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_DEVICE_H_
#define _GFX_DEVICE_H_

#include "common.h"
#include "device.h"
#include "interfaces.h"

namespace gfx {

class gfx_device : public device
{
public:
    static gfx_device* create(void);
    virtual ~gfx_device();

    virtual abstract_painter* create_painter(pix_fmt fmt);
    virtual void destroy_painter(abstract_painter* p);

    virtual abstract_raster_adapter* create_raster_adapter(void);
    virtual void destroy_raster_adapter(abstract_raster_adapter* d);

    virtual abstract_rendering_buffer* create_rendering_buffer(byte* buf,
                                                               uint32_t width, uint32_t height, int32_t stride);
    virtual void destroy_rendering_buffer(abstract_rendering_buffer* b);

    virtual abstract_mask_layer* create_mask_layer(byte* buf,
                                                   uint32_t width, uint32_t height, int32_t stride);
    virtual void destroy_mask_layer(abstract_mask_layer* m);

    virtual abstract_gradient_adapter* create_gradient_adapter(void);
    virtual void destroy_gradient_adapter(abstract_gradient_adapter* g);
protected:
    gfx_device();

};

}
#endif /*_GFX_DEVICE_H_*/
