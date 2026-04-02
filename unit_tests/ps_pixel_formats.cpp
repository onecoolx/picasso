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

class PixelFormatTest : public ::testing::TestWithParam<ps_color_format>
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

    void SetUp() override
    {
        format = GetParam();
        canvas = NULL;
        ctx = NULL;
    }

    void TearDown() override
    {
        if (ctx) { ps_context_unref(ctx); }
        if (canvas) { ps_canvas_unref(canvas); }
    }

    ps_color_format format;
    ps_canvas* canvas;
    ps_context* ctx;
};

TEST_P(PixelFormatTest, CreateCanvas)
{
    canvas = ps_canvas_create(format, 100, 100);
    ASSERT_TRUE(canvas);
    EXPECT_EQ(format, ps_canvas_get_format(canvas));
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_P(PixelFormatTest, CreateContext)
{
    canvas = ps_canvas_create(format, 100, 100);
    ASSERT_TRUE(canvas);

    ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_P(PixelFormatTest, DrawRectangle)
{
    canvas = ps_canvas_create(format, 100, 100);
    ASSERT_TRUE(canvas);

    ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);

    ps_rect rc = {10, 10, 50, 50};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_P(PixelFormatTest, DrawEllipse)
{
    canvas = ps_canvas_create(format, 100, 100);
    ASSERT_TRUE(canvas);

    ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color color = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);

    ps_rect rc = {20, 20, 60, 60};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

INSTANTIATE_TEST_SUITE_P(
    AllFormats,
    PixelFormatTest,
    ::testing::Values(
        COLOR_FORMAT_RGBA,
        COLOR_FORMAT_ARGB,
        COLOR_FORMAT_ABGR,
        COLOR_FORMAT_BGRA,
        COLOR_FORMAT_RGB,
        COLOR_FORMAT_BGR,
        COLOR_FORMAT_RGB565,
        COLOR_FORMAT_RGB555,
        COLOR_FORMAT_A8
    )
);

class FormatConversionTest : public ::testing::Test
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

TEST_F(FormatConversionTest, RGBAToRGB565)
{
    ps_canvas* rgba = ps_canvas_create(COLOR_FORMAT_RGBA, 50, 50);
    ASSERT_TRUE(rgba);

    ps_context* ctx = ps_context_create(rgba, NULL);
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &red);
    ps_clear(ctx);

    ps_canvas* rgb565 = ps_canvas_create(COLOR_FORMAT_RGB565, 50, 50);
    ASSERT_TRUE(rgb565);

    ps_rect rc = {0, 0, 50, 50};
    ps_point pt = {0, 0};
    ps_canvas_bitblt(rgba, &rc, rgb565, &pt);

    ps_context_unref(ctx);
    ps_canvas_unref(rgba);
    ps_canvas_unref(rgb565);
}
