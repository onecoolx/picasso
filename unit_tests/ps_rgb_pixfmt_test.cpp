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

class RGBPixelFormatTest : public ::testing::Test
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

TEST_F(RGBPixelFormatTest, RGB24Drawing)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGB, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fg = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {50, 50, 100, 100};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBPixelFormatTest, BGR24Drawing)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_BGR, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fg = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {25, 25, 150, 150};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBPixelFormatTest, RGB565Drawing)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGB565, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fg = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {40, 40, 120, 120};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBPixelFormatTest, RGB555Drawing)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGB555, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.2f, 0.2f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fg = {1.0f, 1.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {30, 30, 140, 140};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBPixelFormatTest, RGBGradient)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGB, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_point start = {0, 0};
    ps_point end = {200, 200};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_TRUE(gradient);

    ps_color c1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c2);

    ps_set_source_gradient(ctx, gradient);
    ps_rect rc = {0, 0, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_gradient_unref(gradient);
    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBPixelFormatTest, RGBAlphaBlending)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGB, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color layer1 = {1.0f, 0.0f, 0.0f, 0.5f};
    ps_set_source_color(ctx, &layer1);
    ps_rect rc1 = {30, 30, 100, 100};
    ps_rectangle(ctx, &rc1);
    ps_fill(ctx);

    ps_color layer2 = {0.0f, 0.0f, 1.0f, 0.5f};
    ps_set_source_color(ctx, &layer2);
    ps_rect rc2 = {70, 70, 100, 100};
    ps_rectangle(ctx, &rc2);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBPixelFormatTest, RGBStroke)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGB, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color stroke = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 5.0f);

    ps_rect rc = {50, 50, 100, 100};
    ps_rectangle(ctx, &rc);
    ps_stroke(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBPixelFormatTest, RGBComposite)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGB, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_MULTIPLY);

    ps_color fg = {0.8f, 0.2f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {50, 50, 100, 100};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBPixelFormatTest, RGBPattern)
{
    ps_canvas* pattern_canvas = ps_canvas_create(COLOR_FORMAT_RGB, 20, 20);
    ASSERT_TRUE(pattern_canvas);

    ps_context* pctx = ps_context_create(pattern_canvas, NULL);
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(pctx, &red);
    ps_rect rc1 = {0, 0, 10, 10};
    ps_rectangle(pctx, &rc1);
    ps_fill(pctx);

    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(pctx, &blue);
    ps_rect rc2 = {10, 10, 10, 10};
    ps_rectangle(pctx, &rc2);
    ps_fill(pctx);

    ps_image* img = ps_image_create_from_canvas(pattern_canvas, NULL);
    ASSERT_TRUE(img);

    ps_pattern* pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);
    ASSERT_TRUE(pattern);

    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGB, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_set_source_pattern(ctx, pattern);
    ps_rect rc3 = {0, 0, 200, 200};
    ps_rectangle(ctx, &rc3);
    ps_fill(ctx);

    ps_pattern_unref(pattern);
    ps_image_unref(img);
    ps_context_unref(pctx);
    ps_canvas_unref(pattern_canvas);
    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBPixelFormatTest, RGBClipping)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGB, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_rect clip_rc = {50, 50, 100, 100};
    ps_ellipse(ctx, &clip_rc);
    ps_clip(ctx);

    ps_color fg = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {0, 0, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}
