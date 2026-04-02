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

class RGBAAdvancedTest : public ::testing::Test
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

TEST_F(RGBAAdvancedTest, ARGBFormat)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_ARGB, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.2f, 0.4f, 0.6f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fg = {1.0f, 0.5f, 0.0f, 0.7f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {50, 50, 100, 100};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBAAdvancedTest, ABGRFormat)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_ABGR, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.8f, 0.8f, 0.8f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fg = {0.2f, 0.8f, 0.2f, 0.6f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {40, 40, 120, 120};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBAAdvancedTest, BGRAFormat)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_BGRA, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.1f, 0.1f, 0.1f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fg = {0.9f, 0.1f, 0.5f, 0.8f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {30, 30, 140, 140};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBAAdvancedTest, MultiLayerAlpha)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    for (int i = 0; i < 5; i++) {
        ps_color layer = {1.0f - i * 0.2f, i * 0.2f, 0.5f, 0.3f};
        ps_set_source_color(ctx, &layer);
        ps_rect rc = {30.0f + i * 20.0f, 30.0f + i * 20.0f, 80.0f, 80.0f};
        ps_rectangle(ctx, &rc);
        ps_fill(ctx);
    }

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBAAdvancedTest, AlphaGradient)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_point start = {0, 0};
    ps_point end = {200, 200};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_TRUE(gradient);

    ps_color c1 = {1.0f, 0.0f, 0.0f, 0.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {1.0f, 0.0f, 0.0f, 1.0f};
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

TEST_F(RGBAAdvancedTest, AlphaComposite)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.0f, 0.0f, 0.0f, 0.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_SRC_OVER);

    ps_color fg1 = {1.0f, 0.0f, 0.0f, 0.5f};
    ps_set_source_color(ctx, &fg1);
    ps_rect rc1 = {30, 30, 100, 100};
    ps_ellipse(ctx, &rc1);
    ps_fill(ctx);

    ps_color fg2 = {0.0f, 0.0f, 1.0f, 0.5f};
    ps_set_source_color(ctx, &fg2);
    ps_rect rc2 = {70, 70, 100, 100};
    ps_ellipse(ctx, &rc2);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBAAdvancedTest, AlphaPattern)
{
    ps_canvas* pattern_canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 30, 30);
    ASSERT_TRUE(pattern_canvas);

    ps_context* pctx = ps_context_create(pattern_canvas, NULL);

    ps_color clear = {0.0f, 0.0f, 0.0f, 0.0f};
    ps_set_source_color(pctx, &clear);
    ps_clear(pctx);

    ps_color dot = {1.0f, 1.0f, 1.0f, 0.8f};
    ps_set_source_color(pctx, &dot);
    ps_rect rc_dot = {10, 10, 10, 10};
    ps_ellipse(pctx, &rc_dot);
    ps_fill(pctx);

    ps_image* img = ps_image_create_from_canvas(pattern_canvas, NULL);
    ASSERT_TRUE(img);

    ps_pattern* pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);
    ASSERT_TRUE(pattern);

    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.2f, 0.4f, 0.6f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_source_pattern(ctx, pattern);
    ps_rect rc = {0, 0, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_pattern_unref(pattern);
    ps_image_unref(img);
    ps_context_unref(pctx);
    ps_canvas_unref(pattern_canvas);
    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBAAdvancedTest, PremultipliedAlpha)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.0f, 0.0f, 0.0f, 0.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fg = {1.0f, 1.0f, 1.0f, 0.25f};
    ps_set_source_color(ctx, &fg);

    for (int i = 0; i < 4; i++) {
        ps_rect rc = {40.0f + i * 30.0f, 40.0f, 20.0f, 120.0f};
        ps_rectangle(ctx, &rc);
        ps_fill(ctx);
    }

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBAAdvancedTest, AlphaStroke)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color stroke = {0.0f, 0.0f, 0.0f, 0.5f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 10.0f);

    ps_rect rc = {50, 50, 100, 100};
    ps_rectangle(ctx, &rc);
    ps_stroke(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBAAdvancedTest, AlphaClipping)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.0f, 0.0f, 0.0f, 0.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_rect clip_rc = {50, 50, 100, 100};
    ps_ellipse(ctx, &clip_rc);
    ps_clip(ctx);

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

TEST_F(RGBAAdvancedTest, AlphaTransform)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fg = {1.0f, 0.0f, 0.0f, 0.6f};
    ps_set_source_color(ctx, &fg);

    for (int i = 0; i < 8; i++) {
        ps_save(ctx);
        ps_translate(ctx, 100.0f, 100.0f);
        ps_rotate(ctx, i * 3.14159f / 4.0f);
        ps_translate(ctx, -100.0f, -100.0f);

        ps_rect rc = {90, 50, 20, 50};
        ps_rectangle(ctx, &rc);
        ps_fill(ctx);

        ps_restore(ctx);
    }

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RGBAAdvancedTest, AlphaBlur)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_blur(ctx, 0.3f);

    ps_color fg = {0.0f, 0.0f, 0.0f, 0.8f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {70, 70, 60, 60};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}
