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

#ifndef _PSX_SVG_PARSER_H_
#define _PSX_SVG_PARSER_H_

#include "psx_common.h"
#include "psx_svg.h"
#include "psx_xml_token.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    SVG_PARSER_PROCESS = 0,
    SVG_PARSER_IGNORE,
};

typedef struct {
    uint32_t state;
    char* ignore_name;
    uint32_t ignore_len;
    int32_t dpi;
    psx_svg_node* doc_root;
    psx_svg_node* cur_node;
} psx_svg_parser;

void psx_svg_parser_init(psx_svg_parser* parser);
void psx_svg_parser_destroy(psx_svg_parser* parser);
bool psx_svg_parser_token(psx_svg_parser* parser, const psx_xml_token* token);
bool psx_svg_parser_is_finish(psx_svg_parser* parser);

#ifdef _DEBUG
void psx_svg_dump_tree(psx_svg_node* root, int depth);
#endif

#ifdef __cplusplus
}
#endif

#endif /*_PSX_SVG_PARSER_H_*/
