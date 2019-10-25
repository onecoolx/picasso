/* Picasso - a vector graphics library
 *
 * Copyright (C) 2016 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "picasso_image.h"
#include "picasso_image_plugin.h"

#include "psx_image_io.h"
#include "psx_color_convert.h"
#if defined(WIN32) && defined(_MSC_VER)
#include <windows.h>
#endif
#include "webp/decode.h"
#include "webp/encode.h"

struct webp_image_ctx {
    // read
    WebPDecoderConfig dconfig;
    WebPDecBuffer* output_buffer;
    uint8_t* data;
    size_t   data_len;
    // write
    WebPConfig econfig;
    WebPPicture pic;
    image_writer_fn writer;
    void* writer_param;
};

static int read_webp_info(const ps_byte* data, size_t len, psx_image_header* header)
{
    WEBP_CSP_MODE color_mode;
    WebPBitstreamFeatures* bitstream = NULL;
    VP8StatusCode status = VP8_STATUS_OK;

    struct webp_image_ctx* ctx = (struct webp_image_ctx*)calloc(1, sizeof(struct webp_image_ctx));
    if (!ctx) {
        return -1; // out of memory.
    }
    
    if (!WebPInitDecoderConfig(&ctx->dconfig)) {
        free(ctx);
        return -1;
    }
    
    if (ctx->dconfig.input.has_animation) {
        free(ctx);// not support animation
        return -1;
    }

    bitstream = &ctx->dconfig.input;
    ctx->output_buffer = &ctx->dconfig.output;

    status = WebPGetFeatures(data, len, bitstream);
    if (status != VP8_STATUS_OK) {
        free(ctx);
        return -1;
    }
    
    if (bitstream->has_alpha) {
        color_mode = MODE_RGBA;
    } else {
        color_mode = MODE_RGB;
    }

    ctx->data = (uint8_t*)data;
    ctx->data_len = len;

    header->priv = ctx;
    header->width = bitstream->width;
    header->height = bitstream->height;
    header->pitch = header->width * (color_mode == MODE_RGBA ? 4 : 3);
    header->depth = color_mode == MODE_RGBA ? 32 : 24;
    header->bpp = color_mode == MODE_RGBA ? 4 : 3;
    header->format = 0;
    header->alpha = bitstream->has_alpha ? 1 : 0;
    header->frames = 1;
    return 0;
}

static int decode_webp_data(psx_image_header* header, const psx_image* image, psx_image_frame* frame, int idx, ps_byte* buffer, size_t buffer_len)
{
    int y, stride;
    VP8StatusCode status = VP8_STATUS_OK;

    struct webp_image_ctx* ctx = (struct webp_image_ctx*)header->priv;

    status = WebPDecode(ctx->data, ctx->data_len, &ctx->dconfig);
    if (status != VP8_STATUS_OK) {
        return -1;
    }
    
    stride = ctx->output_buffer->u.RGBA.stride;

    for (y = 0; y < ctx->output_buffer->height; y++) {
        ps_byte* row = buffer + stride * y;
        uint8_t* rgba = ctx->output_buffer->u.RGBA.rgba + stride * y;
        memcpy(row, rgba, stride);
    }

    return 0;
}

static int release_read_webp_info(psx_image_header* header)
{
    struct webp_image_ctx* ctx = (struct webp_image_ctx*)header->priv;
    WebPFreeDecBuffer(ctx->output_buffer);
    free(ctx);
    return 0;
}

static int get_bpp(ps_color_format fmt)
{
    switch (fmt) {
    case COLOR_FORMAT_RGBA:
    case COLOR_FORMAT_BGRA:
    case COLOR_FORMAT_ARGB:
    case COLOR_FORMAT_ABGR:
        return 4;
    case COLOR_FORMAT_RGB:
    case COLOR_FORMAT_BGR:
        return 3;
    case COLOR_FORMAT_RGB565:
    case COLOR_FORMAT_RGB555:
        return 2;
    default:
        return 4;
    }
}

static int get_depth(ps_color_format fmt)
{
    switch (fmt) {
    case COLOR_FORMAT_RGBA:
    case COLOR_FORMAT_BGRA:
    case COLOR_FORMAT_ARGB:
    case COLOR_FORMAT_ABGR:
        return 32;
    case COLOR_FORMAT_RGB:
    case COLOR_FORMAT_BGR:
        return 24;
    case COLOR_FORMAT_RGB565:
    case COLOR_FORMAT_RGB555:
        return 16;
    default:
        return 32;
    }
}

static int memory_write(const uint8_t* data, size_t data_size, const WebPPicture* picture)
{
    struct webp_image_ctx* ctx = (struct webp_image_ctx*)picture->custom_ptr;
    ctx->writer(ctx->writer_param, data, data_size);
    return 1;
}

static int write_webp_info(const psx_image* image, image_writer_fn func, void* param, float quality, psx_image_header* header)
{
    struct webp_image_ctx* ctx = (struct webp_image_ctx*)calloc(1, sizeof(struct webp_image_ctx));
    if (!ctx) {
        return -1; // out of memory.
    }

    ctx->writer = func;
    ctx->writer_param = param;

    if (!WebPConfigPreset(&ctx->econfig, WEBP_PRESET_DEFAULT, quality * 100)) {
        free(ctx);
        return -1;
    }

    if (!WebPPictureInit(&ctx->pic)) {
        free(ctx);
        return -1;
    }

    ctx->econfig.lossless = 0;
    ctx->pic.use_argb = 1;
    ctx->pic.width = image->width;
    ctx->pic.height = image->height;
    ctx->pic.writer = memory_write;
    ctx->pic.custom_ptr = ctx;

    header->priv = ctx;
    header->width = image->width;
    header->height = image->height;
    header->pitch = image->pitch;
    header->depth = get_depth(image->format);
    header->bpp = get_bpp(image->format);
    header->format = (int)image->format;
    header->alpha = (header->bpp == 4) ? 1 : 0;
    header->frames = 1;
    return 0;
}

static int webp_convert_32bit(psx_image_header* header, const ps_byte* buffer, size_t buffer_len, int is_argb)
{
    int x, y;
    int width = header->width;
    int height = header->height;

    struct webp_image_ctx* ctx = (struct webp_image_ctx*)header->priv;

    if (!WebPPictureAlloc(&ctx->pic))
        return 0;

    if (is_argb) {
        for (y = 0; y < height; ++y) {
            uint32_t* const dst = &ctx->pic.argb[y * ctx->pic.argb_stride];
            const int offset = y * header->pitch;
            for (x = 0; x < width; ++x) {
                uint8_t* argb = (uint8_t*)(dst + x);
                const uint8_t* pix = buffer + offset + x * header->bpp;
                argb[0] = pix[3];
                argb[1] = pix[2];
                argb[2] = pix[1];
                argb[3] = pix[0];
            }
        }
    } else { // abgr
        for (y = 0; y < height; ++y) {
            uint32_t* const dst = &ctx->pic.argb[y * ctx->pic.argb_stride];
            const int offset = y * header->pitch;
            for (x = 0; x < width; ++x) {
                uint8_t* argb = (uint8_t*)(dst + x);
                const uint8_t* pix = buffer + offset + x * header->bpp;
                argb[0] = pix[1];
                argb[1] = pix[2];
                argb[2] = pix[3];
                argb[3] = pix[0];
            }
        }
    }
    return 1;
}

static int webp_convert_16bit(psx_image_header* header, const ps_byte* buffer, size_t buffer_len, int is_rgb565)
{
    int x, y;
    int width = header->width;
    int height = header->height;

    struct webp_image_ctx* ctx = (struct webp_image_ctx*)header->priv;

    if (!WebPPictureAlloc(&ctx->pic))
        return 0;

    for (y = 0; y < height; ++y) {
        uint32_t* const dst = &ctx->pic.argb[y * ctx->pic.argb_stride];
        const int offset = y * header->pitch;
            for (x = 0; x < width; ++x) {
                uint8_t cbuf[4] = {0};
                uint8_t* argb = (uint8_t*)dst + x * 4;
                const uint8_t* pix = buffer + offset + x * header->bpp;
                if (is_rgb565) {
                    _rgb565_to_rgb(cbuf, pix, 1);
                } else {
                    _rgb555_to_rgb(cbuf, pix, 1);
                }
                argb[3] = 0xFF;
                argb[2] = cbuf[0];
                argb[1] = cbuf[1];
                argb[0] = cbuf[2];
            }
    }
    return 1;
}

static int encode_webp_data(psx_image_header* header, const psx_image* image, psx_image_frame* frame, int idx, const ps_byte* buffer, size_t buffer_len, int* ret)
{
    int ok;

    struct webp_image_ctx* ctx = (struct webp_image_ctx*)header->priv;

    switch (header->format) {
        case COLOR_FORMAT_RGBA:
            ok = WebPPictureImportRGBA(&ctx->pic, buffer, header->pitch);
            break;
        case COLOR_FORMAT_BGRA:
            ok = WebPPictureImportBGRA(&ctx->pic, buffer, header->pitch);
            break;
        case COLOR_FORMAT_ARGB:
            ok = webp_convert_32bit(header, buffer, buffer_len, 1); // 1 is argb
            break;
        case COLOR_FORMAT_ABGR:
            ok = webp_convert_32bit(header, buffer, buffer_len, 0); // 0 is abgr
            break;
        case COLOR_FORMAT_RGB:
            ok = WebPPictureImportRGB(&ctx->pic, buffer, header->pitch);
            break;
        case COLOR_FORMAT_BGR:
            ok = WebPPictureImportBGR(&ctx->pic, buffer, header->pitch);
            break;
        case COLOR_FORMAT_RGB565:
            ok = webp_convert_16bit(header, buffer, buffer_len, 1); // 1 is rgb565
            break;
        case COLOR_FORMAT_RGB555:
            ok = webp_convert_16bit(header, buffer, buffer_len, 0); // 1 is rgb555
            break;
        default:
            if (ret) *ret = S_NOT_SUPPORT;
            return -1;
    }

    ok = WebPEncode(&ctx->econfig, &ctx->pic);
    if (!ok) {
        if (ret) *ret = S_OUT_OF_MEMORY;
        return -1;
    }
    return 0;
}

static int release_write_webp_info(psx_image_header* header)
{
    struct webp_image_ctx* ctx = (struct webp_image_ctx*)header->priv;
    WebPPictureFree(&ctx->pic);
    free(ctx);
    return 0;
}

psx_image_operator * webp_coder = NULL;
static module_handle lib_image = INVALID_HANDLE;

typedef int (*register_func)(const char*, const ps_byte*, size_t, size_t, psx_priority_level, psx_image_operator*);
typedef int (*unregister_func)(psx_image_operator*);

#if defined(WIN32) && defined(_MSC_VER)
static wchar_t g_path[MAX_PATH];
static wchar_t* get_library_path(void)
{
    wchar_t *p = 0;
    memset(g_path, 0, sizeof(wchar_t) * MAX_PATH);
    GetModuleFileName(NULL, g_path, MAX_PATH);
    p = wcsrchr(g_path, '\\');
    p++; *p = 0;
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
    if (lib_image == INVALID_HANDLE)
        return;

    func = _module_get_symbol(lib_image, "psx_image_register_operator");
    if (!func)
        return;

    webp_coder = (psx_image_operator*)calloc(1, sizeof(psx_image_operator));
    if (!webp_coder)
        return;

    webp_coder->read_header_info = read_webp_info;
    webp_coder->decode_image_data = decode_webp_data;
    webp_coder->release_read_header_info = release_read_webp_info;

    webp_coder->write_header_info = write_webp_info;
    webp_coder->encode_image_data = encode_webp_data;
    webp_coder->release_write_header_info = release_write_webp_info;

    func("webp", (ps_byte*)"WEBPVP", 8, 6, PRIORITY_DEFAULT, webp_coder);
}

void psx_image_module_shutdown(void)
{
    unregister_func func = NULL;

    func = _module_get_symbol(lib_image, "psx_image_unregister_operator");
    if (func) {
        if (webp_coder) {
            func(webp_coder);
            free(webp_coder);
        }
    }

    if (lib_image != INVALID_HANDLE)
        _module_unload(lib_image);
}

const char* psx_image_module_get_string(int idx)
{
    switch (idx) {
        case MODULE_NAME:
            return "webp";
        default:
            return "unknow";
    }
}
