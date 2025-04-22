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

#ifndef _PSX_SVG_RENDER_H_
#define _PSX_SVG_RENDER_H_

#include "psx_common.h"
#include "psx_svg.h"

class psx_svg_render_obj
{
public:
    virtual ~psx_svg_render_obj() { }
    virtual psx_svg_tag type(void) const = 0;
    virtual void render(ps_context* ctx, const ps_matrix* matrix) = 0;
    virtual void get_bounding_rect(ps_rect* rc) const = 0;
    virtual void update(const psx_svg_node* node) = 0;
};

class psx_svg_render_list
{
public:
    virtual ~psx_svg_render_list() { }
    virtual uint32_t get_bytes_size(void) const = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

psx_svg_render_list* psx_svg_render_create(const psx_svg_node* doc);

void psx_svg_render_destroy(psx_svg_render_list* list);

bool psx_svg_draw(ps_context* ctx, const psx_svg_render_list* render);

#ifdef __cplusplus
}
#endif

#endif /*_PSX_SVG_RENDER_H_*/
