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

#ifndef _PSX_SVG_H_
#define _PSX_SVG_H_

#include "psx_common.h"
#include "psx_tree.h"
#include "psx_linear_allocator.h"

typedef enum {
    SVG_TAG_INVALID = -1,
    SVG_TAG_CONTENT,
    SVG_TAG_ENTITY,
    SVG_TAG_SVG,
    SVG_TAG_USE,
    SVG_TAG_A,
    SVG_TAG_G,
    SVG_TAG_PATH,
    SVG_TAG_RECT,
    SVG_TAG_CIRCLE,
    SVG_TAG_ELLIPSE,
    SVG_TAG_LINE,
    SVG_TAG_POLYLINE,
    SVG_TAG_POLYGON,
    SVG_TAG_SOLID_COLOR,
    SVG_TAG_LINEAR_GRADIENT,
    SVG_TAG_RADIAL_GRADIENT,
    SVG_TAG_STOP,
    SVG_TAG_DEFS,
    SVG_TAG_IMAGE,
    SVG_TAG_MPATH,
    SVG_TAG_SET,
    SVG_TAG_ANIMATE,
    SVG_TAG_ANIMATE_COLOR,
    SVG_TAG_ANIMATE_TRANSFORM,
    SVG_TAG_ANIMATE_MOTION,
    SVG_TAG_ANIMATION,
    SVG_TAG_TEXT,
    SVG_TAG_TSPAN,
    SVG_TAG_TEXT_AREA,
    SVG_TAG_TBREAK,
} psx_svg_tag;

// svg dom node
class psx_svg_node : public psx_tree_node
{
public:
    psx_svg_node(psx_svg_node* parent);
    virtual ~psx_svg_node();

    NON_COPYABLE_CLASS(psx_svg_node);
private:
};

// svg document
class psx_svg_doc
{
public:
    ~psx_svg_doc();

    NON_COPYABLE_CLASS(psx_svg_doc);
private:
    psx_linear_allocator* m_allocator;
    psx_svg_node* m_doc;
};

psx_svg_doc* psx_svg_load(const char* svg_data, uint32_t len);
void psx_svg_destroy(psx_svg_doc* doc);

psx_svg_node* psx_svg_node_create(psx_svg_node* parent);
void psx_svg_node_destroy(psx_svg_node* node);

#endif /* _PSX_SVG_H_ */
