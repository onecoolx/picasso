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

#define SNAPSHOT_PATH "draw"
#include "test.h"

class PixelFormatOperationsTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        PS_Init();
    }

    static void TearDownTestSuite()
    {
        PS_Shutdown();
    }
};

TEST_F(PixelFormatOperationsTest, RGBAToARGB)
{
    ps_canvas* rgba = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(rgba);

    ps_context* ctx = ps_context_create(rgba, NULL);
    ps_color red = {1.0f, 0.0f, 0.0f, 0.8f};
    ps_set_source_color(ctx, &red);
    ps_clear(ctx);

    ps_canvas* argb = ps_canvas_create(COLOR_FORMAT_ARGB, 100, 100);
    ASSERT_TRUE(argb);

    ps_rect rc = {0, 0, 100, 100};
    ps_point pt = {0, 0};
    ps_canvas_bitblt(rgba, &rc, argb, &pt);

    ps_context_unref(ctx);
    ps_canvas_unref(rgba);
    ps_canvas_unref(argb);
}

TEST_F(PixelFormatOperationsTest, RGBAToABGR)
{
    ps_canvas* rgba = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(rgba);

    ps_context* ctx = ps_context_create(rgba, NULL);
    ps_color green = {0.0f, 1.0f, 0.0f, 0.9f};
    ps_set_source_color(ctx, &green);
    ps_clear(ctx);

    ps_canvas* abgr = ps_canvas_create(COLOR_FORMAT_ABGR, 100, 100);
    ASSERT_TRUE(abgr);

    ps_rect rc = {0, 0, 100, 100};
    ps_point pt = {0, 0};
    ps_canvas_bitblt(rgba, &rc, abgr, &pt);

    ps_context_unref(ctx);
    ps_canvas_unref(rgba);
    ps_canvas_unref(abgr);
}

TEST_F(PixelFormatOperationsTest, RGBAToBGRA)
{
    ps_canvas* rgba = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(rgba);

    ps_context* ctx = ps_context_create(rgba, NULL);
    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &blue);
    ps_clear(ctx);

    ps_canvas* bgra = ps_canvas_create(COLOR_FORMAT_BGRA, 100, 100);
    ASSERT_TRUE(bgra);

    ps_rect rc = {0, 0, 100, 100};
    ps_point pt = {0, 0};
    ps_canvas_bitblt(rgba, &rc, bgra, &pt);

    ps_context_unref(ctx);
    ps_canvas_unref(rgba);
    ps_canvas_unref(bgra);
}

TEST_F(PixelFormatOperationsTest, RGBAToRGB)
{
    ps_canvas* rgba = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(rgba);

    ps_context* ctx = ps_context_create(rgba, NULL);
    ps_color yellow = {1.0f, 1.0f, 0.0f, 0.5f};
    ps_set_source_color(ctx, &yellow);
    ps_clear(ctx);

    ps_canvas* rgb = ps_canvas_create(COLOR_FORMAT_RGB, 100, 100);
    ASSERT_TRUE(rgb);

    ps_rect rc = {0, 0, 100, 100};
    ps_point pt = {0, 0};
    ps_canvas_bitblt(rgba, &rc, rgb, &pt);

    ps_context_unref(ctx);
    ps_canvas_unref(rgba);
    ps_canvas_unref(rgb);
}

TEST_F(PixelFormatOperationsTest, RGBAToBGR)
{
    ps_canvas* rgba = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(rgba);

    ps_context* ctx = ps_context_create(rgba, NULL);
    ps_color cyan = {0.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &cyan);
    ps_clear(ctx);

    ps_canvas* bgr = ps_canvas_create(COLOR_FORMAT_BGR, 100, 100);
    ASSERT_TRUE(bgr);

    ps_rect rc = {0, 0, 100, 100};
    ps_point pt = {0, 0};
    ps_canvas_bitblt(rgba, &rc, bgr, &pt);

    ps_context_unref(ctx);
    ps_canvas_unref(rgba);
    ps_canvas_unref(bgr);
}

TEST_F(PixelFormatOperationsTest, RGBAToRGB555)
{
    ps_canvas* rgba = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(rgba);

    ps_context* ctx = ps_context_create(rgba, NULL);
    ps_color magenta = {1.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &magenta);
    ps_clear(ctx);

    ps_canvas* rgb555 = ps_canvas_create(COLOR_FORMAT_RGB555, 100, 100);
    ASSERT_TRUE(rgb555);

    ps_rect rc = {0, 0, 100, 100};
    ps_point pt = {0, 0};
    ps_canvas_bitblt(rgba, &rc, rgb555, &pt);

    ps_context_unref(ctx);
    ps_canvas_unref(rgba);
    ps_canvas_unref(rgb555);
}

TEST_F(PixelFormatOperationsTest, RGBAToA8)
{
    ps_canvas* rgba = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(rgba);

    ps_context* ctx = ps_context_create(rgba, NULL);
    ps_color gray = {0.5f, 0.5f, 0.5f, 0.7f};
    ps_set_source_color(ctx, &gray);
    ps_clear(ctx);

    ps_canvas* a8 = ps_canvas_create(COLOR_FORMAT_A8, 100, 100);
    ASSERT_TRUE(a8);

    ps_rect rc = {0, 0, 100, 100};
    ps_point pt = {0, 0};
    ps_canvas_bitblt(rgba, &rc, a8, &pt);

    ps_context_unref(ctx);
    ps_canvas_unref(rgba);
    ps_canvas_unref(a8);
}

TEST_F(PixelFormatOperationsTest, RGB565ToRGBA)
{
    ps_canvas* rgb565 = ps_canvas_create(COLOR_FORMAT_RGB565, 100, 100);
    ASSERT_TRUE(rgb565);

    ps_context* ctx = ps_context_create(rgb565, NULL);
    ps_color orange = {1.0f, 0.5f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &orange);
    ps_clear(ctx);

    ps_canvas* rgba = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(rgba);

    ps_rect rc = {0, 0, 100, 100};
    ps_point pt = {0, 0};
    ps_canvas_bitblt(rgb565, &rc, rgba, &pt);

    ps_context_unref(ctx);
    ps_canvas_unref(rgb565);
    ps_canvas_unref(rgba);
}

TEST_F(PixelFormatOperationsTest, PartialBitBlt)
{
    ps_canvas* src = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
    ASSERT_TRUE(src);

    ps_context* ctx = ps_context_create(src, NULL);
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &red);
    ps_clear(ctx);

    ps_canvas* dst = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
    ASSERT_TRUE(dst);

    ps_context* ctx2 = ps_context_create(dst, NULL);
    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx2, &blue);
    ps_clear(ctx2);

    ps_rect rc = {50, 50, 100, 100};
    ps_point pt = {75, 75};
    ps_canvas_bitblt(src, &rc, dst, &pt);

    ps_context_unref(ctx);
    ps_context_unref(ctx2);
    ps_canvas_unref(src);
    ps_canvas_unref(dst);
}

TEST_F(PixelFormatOperationsTest, AlphaBlending)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);

    ps_color white = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &white);
    ps_clear(ctx);

    ps_color red_alpha = {1.0f, 0.0f, 0.0f, 0.5f};
    ps_set_source_color(ctx, &red_alpha);
    ps_rect rc1 = {50, 50, 100, 100};
    ps_rectangle(ctx, &rc1);
    ps_fill(ctx);

    ps_color blue_alpha = {0.0f, 0.0f, 1.0f, 0.5f};
    ps_set_source_color(ctx, &blue_alpha);
    ps_rect rc2 = {100, 100, 100, 100};
    ps_rectangle(ctx, &rc2);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
}

TEST_F(PixelFormatOperationsTest, GrayscaleOperations)
{
    ps_canvas* rgba = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(rgba);

    ps_context* ctx = ps_context_create(rgba, NULL);

    for (int i = 0; i < 10; i++) {
        float gray = i / 10.0f;
        ps_color c = {gray, gray, gray, 1.0f};
        ps_set_source_color(ctx, &c);
        ps_rect rc = {i * 10.0f, 0.0f, 10.0f, 100.0f};
        ps_rectangle(ctx, &rc);
        ps_fill(ctx);
    }

    ps_context_unref(ctx);
    ps_canvas_unref(rgba);
}

TEST_F(PixelFormatOperationsTest, ColorChannelTest)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 300, 100);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);

    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &red);
    ps_rect rc1 = {0, 0, 100, 100};
    ps_rectangle(ctx, &rc1);
    ps_fill(ctx);

    ps_color green = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &green);
    ps_rect rc2 = {100, 0, 100, 100};
    ps_rectangle(ctx, &rc2);
    ps_fill(ctx);

    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &blue);
    ps_rect rc3 = {200, 0, 100, 100};
    ps_rectangle(ctx, &rc3);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
}
