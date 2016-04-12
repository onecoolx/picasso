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
#include <gif_lib.h>

#if GIFLIB_MAJOR > 5 || GIFLIB_MAJOR == 5 && GIFLIB_MINOR >= 1
#define GIF_CLOSE_DFILE(gif) DGifCloseFile(gif, NULL)
#define GIF_CLOSE_EFILE(gif) EGifCloseFile(gif, NULL)
#define QuantizeBuffer GifQuantizeBuffer
#define MakeMapObject GifMakeMapObject
#define FreeMapObject GifFreeMapObject
#ifndef TRUE
#define TRUE        1
#endif /* TRUE */
#ifndef FALSE
#define FALSE       0
#endif /* FALSE */
#else
#define GIF_CLOSE_DFILE(gif) DGifCloseFile(gif)
#define GIF_CLOSE_EFILE(gif) EGifCloseFile(gif)
/* extract bytes from an unsigned word */
#define LOBYTE(x)   ((x) & 0xff)
#define HIBYTE(x)   (((x) >> 8) & 0xff)
#endif

struct gif_image_ctx {
    GifFileType* gif;
    // read
    uint8_t *buf;
    uint32_t len;
    uint32_t pos;
    // write
    image_writer_fn writer;
    void* writer_param;
    GifByteType* red_buf;
    GifByteType* green_buf;
    GifByteType* blue_buf;
    GifByteType* output_buffer;
};

static int read_gif_from_memory(GifFileType *gif, GifByteType *buf, int len)
{
    struct gif_image_ctx *data = (struct gif_image_ctx *)gif->UserData;
    if ((data->pos + len) > data->len) {
        len = data->len - data->pos;
    }
    memcpy(buf, data->pos + data->buf, len);
    data->pos += len;
    return len;
}

static int read_gif_info(const ps_byte* data, size_t len, psx_image_header* header)
{
#if GIFLIB_MAJOR >= 5
    int errorcode = 0;
#endif

    struct gif_image_ctx* ctx = (struct gif_image_ctx*)calloc(1, sizeof(struct gif_image_ctx));
    if (!ctx) {
        return -1; // out of memory.
    }

    ctx->buf = (uint8_t*)data;
    ctx->len = len;
    ctx->pos = 0;

#if GIFLIB_MAJOR >= 5
    if ((ctx->gif = DGifOpen((void*)ctx, read_gif_from_memory, &errorcode)) == NULL) {
        free(ctx);
        return -1;
    }
#else
    if ((ctx->gif = DGifOpen((void*)ctx, read_gif_from_memory)) == NULL) {
        free(ctx);
        return -1;
    }
#endif

    if (GIF_OK != DGifSlurp(ctx->gif)) {
        GIF_CLOSE_DFILE(ctx->gif);
        free(ctx);
        return -1;
    }

    header->priv = ctx;
    header->width = ctx->gif->SWidth;
    header->height = ctx->gif->SHeight;
    header->pitch = ctx->gif->SWidth * 4;
    header->depth = 32;
    header->bpp = 4;
    header->format = 0;
    header->alpha = 1;
    header->frames = ctx->gif->ImageCount;
    return 0;
}

static int get_gif_transparent_color(GifFileType *gif, int frame)
{
    int x;
    ExtensionBlock *ext = gif->SavedImages[frame].ExtensionBlocks;
    int len = gif->SavedImages[frame].ExtensionBlockCount;
    for (x = 0; x < len; ++x, ++ext) {
        if ((ext->Function == GRAPHICS_EXT_FUNC_CODE) && (ext->Bytes[0] & 1)) {
            return ext->Bytes[3] == 0 ? 0 : (uint8_t) ext->Bytes[3];
        }
    }
    return -1;
}

static int get_gif_delay_time(GifFileType *gif, int frame)
{
    int x;
    ExtensionBlock *ext = gif->SavedImages[frame].ExtensionBlocks;
    int len = gif->SavedImages[frame].ExtensionBlockCount;
    for (x = 0; x < len; ++x, ++ext) {
        if ((ext->Function == GRAPHICS_EXT_FUNC_CODE)) {
            return ((ext->Bytes[2] << 8) | (ext->Bytes[1])) * 10; // ms
        }
    }
    return 0;
}

static int decode_gif_data(psx_image_header* header, psx_image_frame* frame, int idx, ps_byte* buffer, size_t buffer_len)
{
    int x, y, z;
    int bg_color = 0;
    int alpha_color = 0;
    struct GifImageDesc *img = NULL;
    struct ColorMapObject *colormap = NULL;
    uint8_t *src_data = NULL;
    uint32_t *dst_data = NULL;

    struct gif_image_ctx* ctx = (struct gif_image_ctx*)header->priv;

    img = &ctx->gif->SavedImages[idx].ImageDesc;
    // local colormap takes precedence over global
    colormap = img->ColorMap ? img->ColorMap : ctx->gif->SColorMap;
    alpha_color = get_gif_transparent_color(ctx->gif, idx);
    if (ctx->gif->SColorMap) {
        bg_color = (uint8_t)ctx->gif->SBackGroundColor;
    } else if (alpha_color >= 0) {
        bg_color = alpha_color;
    }

    frame->duration = get_gif_delay_time(ctx->gif, idx); 

    src_data = (uint8_t*)ctx->gif->SavedImages[idx].RasterBits;
    dst_data = (uint32_t*)buffer;

    if (!ctx->gif->Image.Interlace) {
        if (header->width == img->Width && header->height == img->Height) {
            for (y = 0; y < header->height; ++y) {
                for (x = 0; x < header->width; ++x) {
                    *dst_data = ((*src_data == alpha_color) ? 0 : 255) << 24
                        | colormap->Colors[*src_data].Blue << 16
                        | colormap->Colors[*src_data].Green << 8
                        | colormap->Colors[*src_data].Red;

                    dst_data++;
                    src_data++;
                }
            }
        } else {
            // Image does not take up whole "screen" so we need to fill-in the background
            int bottom = img->Top + img->Height;
            int right = img->Left + img->Width;

            for (y = 0; y < header->height; ++y) {
                for (x = 0; x < header->width; ++x) {
                    if (y < img->Top || y >= bottom || x < img->Left || x >= right) {
                        *dst_data = ((bg_color == alpha_color) ? 0 : 255) << 24
                            | colormap->Colors[bg_color].Blue << 16
                            | colormap->Colors[bg_color].Green << 8
                            | colormap->Colors[bg_color].Red;
                    } else {
                        *dst_data = ((*src_data == alpha_color) ? 0 : 255) << 24
                            | colormap->Colors[*src_data].Blue << 16
                            | colormap->Colors[*src_data].Green << 8
                            | colormap->Colors[*src_data].Red;
                    }

                    dst_data++;
                    src_data++;
                }
            }
        }
    } else {
        uint32_t *dst_ptr;
        uint8_t *src_ptr = src_data;
        // Image is interlaced so that it streams nice over 14.4k and 28.8k modems :)
        // We first load in 1/8 of the image, followed by another 1/8, followed by
        // 1/4 and finally the remaining 1/2.
        int ioffs[] = { 0, 4, 2, 1 };
        int ijumps[] = { 8, 8, 4, 2 };

        for (z = 0; z < 4; z++) {
            for (y = ioffs[z]; y < header->height; y += ijumps[z]) {
                dst_ptr = dst_data + header->width * y;
                for (x = 0; x < header->width; ++x) {
                    *dst_ptr = ((*src_ptr == alpha_color) ? 0 : 255) << 24
                        | (colormap->Colors[*src_ptr].Blue) << 16
                        | (colormap->Colors[*src_ptr].Green) << 8
                        | (colormap->Colors[*src_ptr].Red);

                    dst_ptr++;
                    src_ptr++;
                }
            }
        }
    }

    return 0;
}

static int release_read_gif_info(psx_image_header* header)
{
    struct gif_image_ctx* ctx = (struct gif_image_ctx*)header->priv;
    GIF_CLOSE_DFILE(ctx->gif);
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

static int write_gif_from_memory(GifFileType *gif, const GifByteType *buf, int len)
{
    struct gif_image_ctx *ctx = (struct gif_image_ctx *)gif->UserData;
    ctx->writer(ctx->writer_param, buf, len);
    return len;
}

static int write_gif_info(const psx_image* image, image_writer_fn func, void* param,
                                                     float quality, psx_image_header* header)
{
    size_t buf_size;
#if GIFLIB_MAJOR >= 5
    int errorcode = 0;
#endif

    struct gif_image_ctx* ctx = (struct gif_image_ctx*)calloc(1, sizeof(struct gif_image_ctx));
    if (!ctx) {
        return -1; // out of memory.
    }

    ctx->writer = func;
    ctx->writer_param = param;

#if GIFLIB_MAJOR >= 5
    if ((ctx->gif = EGifOpen((void*)ctx, write_gif_from_memory, &errorcode)) == NULL) {
        free(ctx);
        return -1;
    }

    if (image->num_frames > 1) {
        EGifSetGifVersion(ctx->gif, true);
    }
#else
    if ((ctx->gif = EGifOpen((void*)ctx, write_gif_from_memory)) == NULL) {
        free(ctx);
        return -1;
    }

    if (image->num_frames > 1) {
        EGifSetGifVersion("89a");
    } else {
        EGifSetGifVersion("87a");
    }
#endif

    if (EGifPutScreenDesc(ctx->gif, image->width, image->height, 8, 0, NULL) == GIF_ERROR) {
        GIF_CLOSE_EFILE(ctx->gif);
        free(ctx);
        return -1;
    }

    if (image->num_frames > 1) { // add netscape2.0 application extension to an animation gif.
#if GIFLIB_MAJOR >= 5
        EGifPutExtensionLeader(ctx->gif, APPLICATION_EXT_FUNC_CODE);
        EGifPutExtensionBlock(ctx->gif, 11, "NETSCAPE2.0");
        EGifPutExtensionBlock(ctx->gif, 3, "\x01\x00\x00");
        EGifPutExtensionTrailer(ctx->gif);
#else
        EGifPutExtensionFirst(ctx->gif, APPLICATION_EXT_FUNC_CODE, 11, "NETSCAPE2.0");
        EGifPutExtensionLast(ctx->gif, APPLICATION_EXT_FUNC_CODE, 3, "\x01\x00\x00");
#endif
    }

    buf_size = image->width * image->height * sizeof(GifByteType);

    ctx->red_buf = (GifByteType*)malloc(buf_size);
    ctx->green_buf = (GifByteType*)malloc(buf_size);
    ctx->blue_buf = (GifByteType*)malloc(buf_size);
    ctx->output_buffer = (GifByteType*)malloc(buf_size);

    if (!ctx->red_buf || !ctx->green_buf || !ctx->blue_buf || !ctx->output_buffer) {
        GIF_CLOSE_EFILE(ctx->gif);

        if (ctx->red_buf) 
            free(ctx->red_buf);
        if (ctx->green_buf)
            free(ctx->green_buf);
        if (ctx->blue_buf)
            free(ctx->blue_buf);
        if (ctx->output_buffer)
            free(ctx->output_buffer);

        free(ctx);
        return -1;
    }

    header->priv = ctx;
    header->width = image->width;
    header->height = image->height;
    header->pitch = image->pitch;
    header->depth = get_depth(image->format);
    header->bpp = get_bpp(image->format);
    header->format = (int)image->format;
    header->alpha = 1;
    header->frames = (int)image->num_frames;
    return 0;
}

static void gif_get_pixel_rgba_premultiply(int format, ps_byte* input_buffer, uint32_t idx, uint32_t rgba[])
{
    uint8_t color[4] = {0, 0, 0, 255}; // opaque black 

    switch (format) {
    case COLOR_FORMAT_RGBA:
        {
            color[0] = input_buffer[idx * 4];
            color[1] = input_buffer[idx * 4 + 1];
            color[2] = input_buffer[idx * 4 + 2];
            color[3] = input_buffer[idx * 4 + 3];
        }
        break;
    case COLOR_FORMAT_BGRA:
        _bgra_to_rgba(color, input_buffer + idx * 4, 1);
        break;
    case COLOR_FORMAT_ARGB:
        _argb_to_rgba(color, input_buffer + idx * 4, 1);
        break;
    case COLOR_FORMAT_ABGR:
        _abgr_to_rgba(color, input_buffer + idx * 4, 1);
        break;
    case COLOR_FORMAT_RGB:
        {
            color[0] = input_buffer[idx * 3];
            color[1] = input_buffer[idx * 3 + 1];
            color[2] = input_buffer[idx * 3 + 2];
        }
        break;
    case COLOR_FORMAT_BGR:
        _bgr_to_rgb(color, input_buffer + idx * 3, 1);
        break;
    case COLOR_FORMAT_RGB565:
        _rgb565_to_rgb(color, input_buffer + idx * 2, 1);
        break;
    case COLOR_FORMAT_RGB555:
        _rgb555_to_rgb(color, input_buffer + idx * 2, 1);
        break;
    default:
        // do nothing
        return;
    }

    rgba[0] = ((uint32_t)color[0] * color[3] + 255) >> 8;  
    rgba[1] = ((uint32_t)color[1] * color[3] + 255) >> 8;  
    rgba[2] = ((uint32_t)color[2] * color[3] + 255) >> 8;  
    rgba[3] = (uint32_t)color[3];
}

static int encode_gif_data(psx_image_header* header, psx_image_frame* frame, int idx, const ps_byte* buffer, size_t buffer_len, int* ret)
{
    int x, y;
    ColorMapObject *output_map = NULL;
    int map_size = 256;

    struct gif_image_ctx* ctx = (struct gif_image_ctx*)header->priv;

    if ((output_map = MakeMapObject(map_size, NULL)) == NULL) {
        return -1;
    }

    for (y = 0; y < header->height; y++) {
        ps_byte* row = (ps_byte*)(buffer + header->pitch * y);
        for (x = 0; x < header->width; x++) {
            uint32_t rgba[4] = {0}; // r, g, b, a 
            gif_get_pixel_rgba_premultiply(header->format, row, x, rgba);
            ctx->red_buf[header->width * y + x] = rgba[0];
            ctx->green_buf[header->width * y + x] = rgba[1];
            ctx->blue_buf[header->width * y + x] = rgba[2];
        }
    }
    
    if (QuantizeBuffer(header->width, header->height, &map_size,
		ctx->red_buf, ctx->green_buf, ctx->blue_buf, ctx->output_buffer, output_map->Colors) == GIF_ERROR) {
        FreeMapObject(output_map);
        return -1;
    }

    if (frame->duration > 0) {
        GifByteType extension[4];
#if GIFLIB_MAJOR >= 5
        GraphicsControlBlock gcb;
        gcb.DisposalMode = DISPOSAL_UNSPECIFIED; // FIXME: need specified ?
        gcb.UserInputFlag = false;
        gcb.DelayTime = frame->duration / 10;
        gcb.TransparentColor = -1; // FIXME: need specified ?

        EGifGCBToExtension(&gcb, extension);
#else
        int delay = frame->duration / 10;
        extension[0] = 0;
        extension[1] = LOBYTE(delay);
        extension[2] = HIBYTE(delay);
        extension[3] = (char)-1;
#endif
        if (EGifPutExtension(ctx->gif, GRAPHICS_EXT_FUNC_CODE, 4, extension) == GIF_ERROR) {
            FreeMapObject(output_map);
            return -1;
        }
    }

    if (EGifPutImageDesc(ctx->gif, 0, 0, header->width, header->height, FALSE, output_map) == GIF_ERROR) {
        FreeMapObject(output_map);
        return -1;
    }

    for (y = 0; y < header->height; y++) {
        EGifPutLine(ctx->gif, ctx->output_buffer + y * header->width, header->width);
    }
    FreeMapObject(output_map);
    return 0;
}

static int release_write_gif_info(psx_image_header* header)
{
    struct gif_image_ctx* ctx = (struct gif_image_ctx*)header->priv;

    GIF_CLOSE_EFILE(ctx->gif);

    if (ctx->red_buf) 
        free(ctx->red_buf);
    if (ctx->green_buf)
        free(ctx->green_buf);
    if (ctx->blue_buf)
        free(ctx->blue_buf);
    if (ctx->output_buffer)
        free(ctx->output_buffer);

    free(ctx);
    return 0;
}

psx_image_operator * gif_coder = NULL;
static module_handle lib_image = INVALID_HANDLE;

typedef int (*register_func)(const char*, const ps_byte*, size_t, psx_priority_level, psx_image_operator*);
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

    gif_coder = (psx_image_operator*)calloc(1, sizeof(psx_image_operator));
    if (!gif_coder)
        return;

    gif_coder->read_header_info = read_gif_info;
    gif_coder->decode_image_data = decode_gif_data;
    gif_coder->release_read_header_info = release_read_gif_info;

    gif_coder->write_header_info = write_gif_info;
    gif_coder->encode_image_data = encode_gif_data;
    gif_coder->release_write_header_info = release_write_gif_info;

    func("gif", (ps_byte*)"GIF", 3, PRIORITY_DEFAULT, gif_coder);
}

void psx_image_module_shutdown(void)
{
    unregister_func func = NULL;

    func = _module_get_symbol(lib_image, "psx_image_unregister_operator");
    if (func) {
        if (gif_coder) {
            func(gif_coder);
            free(gif_coder);
        }
    }

    if (lib_image != INVALID_HANDLE)
        _module_unload(lib_image);
}

const char* psx_image_module_get_string(int idx)
{
    switch (idx) {
        case MODULE_NAME:
            return "gif";
        default:
            return "unknow";
    }
}

