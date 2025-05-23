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

#include "psx_svg.h"

#include "picasso.h"
#include "psx_file.h"
#include "psx_image.h"
#include "psx_svg_node.h"
#include "psx_svg_render.h"

#ifdef __cplusplus
extern "C" {
#endif

psx_svg* PICAPI psx_svg_load(const char* name, psx_result* err_code)
{
    size_t size;
    ps_byte* file_data;
    pchar* file_name;

    if (!name) {
        if (err_code) {
            *err_code = S_BAD_PARAMS;
        }
        return NULL;
    }

    file_name = psx_path_create(name, NULL);
    if (!file_name) {
        if (err_code) {
            *err_code = S_FAILURE;
        }
        return NULL;
    }

    if (!psx_file_exists(file_name)) {
        if (err_code) {
            *err_code = S_BAD_PARAMS;
        }
        psx_path_destroy(file_name);
        return NULL;
    }

    size = psx_file_size(file_name);
    if (!size) {
        if (err_code) {
            *err_code = S_BAD_PARAMS;
        }
        psx_path_destroy(file_name);
        return NULL;
    }

    file_data = (ps_byte*)mem_malloc(size);
    if (!file_data) {
        if (err_code) {
            *err_code = S_OUT_OF_MEMORY;
        }
        psx_path_destroy(file_name);
        return NULL;
    }
    // read file data.
    if (!psx_file_read(file_name, file_data, size)) {
        free(file_data);
        if (err_code) {
            *err_code = S_FAILURE;
        }
        psx_path_destroy(file_name);
        return NULL;
    }

    psx_svg* svg = psx_svg_load_from_memory(file_data, size, err_code);
    psx_path_destroy(file_name);
    free(file_data);
    if (err_code) {
        *err_code = S_OK;
    }
    return svg;
}

psx_svg* PICAPI psx_svg_load_from_memory(const ps_byte* data, size_t length, psx_result* err_code)
{
    if (!data || !length) {
        if (err_code) {
            *err_code = S_BAD_PARAMS;
        }
        return NULL;
    }

    psx_svg_node* svg = psx_svg_load_data((const char*)data, (uint32_t)length);

    if (!svg) {
        if (err_code) {
            *err_code = S_FAILURE;
        }
        return NULL;
    }

    if (err_code) {
        *err_code = S_OK;
    }
    return (psx_svg*)svg;
}

void PICAPI psx_svg_destroy(psx_svg* doc)
{
    if (doc) {
        psx_svg_node_destroy((psx_svg_node*)doc);
    }
}

psx_svg_render* PICAPI psx_svg_render_create(const psx_svg* doc, psx_result* err_code)
{
    if (!doc) {
        if (err_code) {
            *err_code = S_BAD_PARAMS;
        }
        return NULL;
    }

    psx_svg_render_list* list = psx_svg_render_list_create((const psx_svg_node*)doc);
    if (!list) {
        if (err_code) {
            *err_code = S_FAILURE;
        }
        return NULL;
    }

    if (err_code) {
        *err_code = S_OK;
    }
    return (psx_svg_render*)list;
}

void PICAPI psx_svg_render_destroy(psx_svg_render* render)
{
    if (render) {
        psx_svg_render_list_destroy((psx_svg_render_list*)render);
    }
}

psx_result PICAPI psx_svg_render_draw(ps_context* ctx, const psx_svg_render* render)
{
    if (!ctx || !render) {
        return S_BAD_PARAMS;
    }

    if (!psx_svg_render_list_draw(ctx, (const psx_svg_render_list*)render)) {
        return S_FAILURE;
    }
    return S_OK;
}

psx_result PICAPI psx_svg_init(void)
{
    return (psx_result)psx_image_init();
}

void PICAPI psx_svg_shutdown(void)
{
    psx_image_shutdown();
}

#ifdef __cplusplus
}
#endif
