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

#ifndef _PSX_SVG_ANIM_STATE_H_
#define _PSX_SVG_ANIM_STATE_H_

#include "psx_svg_node.h"
#include "psx_svg_parser.h"
#include "picasso.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psx_svg_anim_state psx_svg_anim_state;

/* Returns true if a numeric (float) override exists for (target, attr).
 * Writes the value into *out_v. */
bool psx_svg_anim_get_float(const psx_svg_anim_state* s, const psx_svg_node* target,
                            psx_svg_attr_type attr, float* out_v);

/* Returns a pointer to an internal scratch ps_matrix if a transform override
 * exists for target, NULL otherwise.
 * The returned pointer is valid until the next call to psx_svg_anim_get_transform
 * or until the player is destroyed. Do NOT unref it. */
const ps_matrix* psx_svg_anim_get_transform(const psx_svg_anim_state* s,
                                            const psx_svg_node* target);

#ifdef __cplusplus
}
#endif

#endif /* _PSX_SVG_ANIM_STATE_H_ */
