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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <psx_svg.h>

#include "psx_image.h"
#include "psx_image_plugin.h"
#include "psx_image_io.h"
#if defined(WIN32) && defined(_MSC_VER)
    #include <windows.h>
#endif

struct svg_image_ctx {
    psx_svg_render* svg_render;
};

static int read_svg_info(const ps_byte* data, size_t len, psx_image_header* header)
{
    struct svg_image_ctx* ctx = (struct svg_image_ctx*)calloc(1, sizeof(struct svg_image_ctx));
    if (!ctx) {
        return -1; // out of memory.
    }

    psx_result result;
    psx_svg* svg_doc = psx_svg_load_from_memory(data, len, &result);
    if (!svg_doc) {
        LOG_ERROR("SVG image read info failed! Reason: %s", psx_result_get_string(result));
        free(ctx);
        return -1;
    }

    ctx->svg_render = psx_svg_render_create(svg_doc, &result);
    if (!ctx->svg_render) {
        LOG_ERROR("SVG image parser doc failed! Reason: %s", psx_result_get_string(result));
        psx_svg_destroy(svg_doc);
        free(ctx);
        return -1;
    }

    ps_size rs = {0};
    ps_bool b = psx_svg_render_get_size(ctx->svg_render, &rs);
    if (!b) {
        LOG_ERROR("SVG image get size failed!");
        psx_svg_destroy(svg_doc);
        free(ctx);
        return -1;
    }

    int32_t width = (int32_t)rs.w;
    int32_t height = (int32_t)rs.h;

    header->priv = ctx;
    header->width = width;
    header->height = height;
    header->pitch = width * 4;
    header->depth = 32;
    header->bpp = 4;
    header->format = COLOR_FORMAT_RGBA;
    header->alpha = 1;
    header->frames = 1;

    psx_svg_destroy(svg_doc);
    return 0;
}

static int release_read_svg_info(psx_image_header* header)
{
    struct svg_image_ctx* ctx = (struct svg_image_ctx*)header->priv;
    psx_svg_render_destroy(ctx->svg_render);
    free(ctx);
    return 0;
}

static int decode_svg_data(psx_image_header* header, const psx_image* image, psx_image_frame* frame, int idx, ps_byte* buffer, size_t buffer_len)
{
    struct svg_image_ctx* ctx = (struct svg_image_ctx*)header->priv;

    ps_canvas* canvas = ps_canvas_create_with_data(buffer, header->format, header->width, header->height, header->pitch);
    ps_context* context = ps_context_create(canvas, NULL);

    psx_svg_render_draw(context, ctx->svg_render);

    ps_context_unref(context);
    ps_canvas_unref(canvas);
    return 0;
}

static psx_image_operator* svg_coder = NULL;
static module_handle lib_image = INVALID_HANDLE;

typedef int (*register_func)(const char*, const ps_byte*, size_t, size_t, psx_priority_level, psx_image_operator*);
typedef int (*unregister_func)(psx_image_operator*);

#if defined(WIN32) && defined(_MSC_VER)
static wchar_t g_path[MAX_PATH];
static wchar_t* get_library_path(void)
{
    wchar_t* p = 0;
    memset(g_path, 0, sizeof(wchar_t) * MAX_PATH);
    GetModuleFileName(NULL, g_path, MAX_PATH);
    p = wcsrchr(g_path, '\\');
    p++;
    *p = 0;
    lstrcat(g_path, L"psx_image.dll");
    return g_path;
}
#endif

void psx_image_module_init(void)
{
    register_func func = NULL;

#if defined(WIN32) && defined(_MSC_VER)
    lib_image = _module_load(get_library_path());
#else
    lib_image = _module_load("libpsx_image.so");
#endif
    if (lib_image == INVALID_HANDLE) {
        return;
    }

    func = _module_get_symbol(lib_image, "psx_image_register_operator");
    if (!func) {
        return;
    }

    svg_coder = (psx_image_operator*)calloc(1, sizeof(psx_image_operator));
    if (!svg_coder) {
        return;
    }

    svg_coder->read_header_info = read_svg_info;
    svg_coder->decode_image_data = decode_svg_data;
    svg_coder->release_read_header_info = release_read_svg_info;

    func("svg", (ps_byte*)"<svg", 0, 4, PRIORITY_DEFAULT, svg_coder);
    func("xml", (ps_byte*)"<?xml", 0, 5, PRIORITY_DEFAULT, svg_coder);
}

void psx_image_module_shutdown(void)
{
    unregister_func func = NULL;

    func = _module_get_symbol(lib_image, "psx_image_unregister_operator");
    if (func) {
        if (svg_coder) {
            func(svg_coder);
            func(svg_coder);
            free(svg_coder);
        }
    }

    if (lib_image != INVALID_HANDLE) {
        _module_unload(lib_image);
    }
}

const char* psx_image_module_get_string(int idx)
{
    switch (idx) {
        case MODULE_NAME:
            return "svg";
        default:
            return "unknown";
    }
}
