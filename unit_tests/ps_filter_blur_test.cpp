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

class FilterBlurTest : public ::testing::Test
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
        clear_test_canvas();
        ctx = ps_context_create(get_test_canvas(), NULL);
        ASSERT_TRUE(ctx);
    }

    void TearDown() override
    {
        if (ctx) {
            ps_context_unref(ctx);
        }
    }

    ps_context* ctx;
};

TEST_F(FilterBlurTest, BasicBlur)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_blur(ctx, 0.5f);

    ps_color fg = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, LowBlur)
{
    ps_color bg = {0.8f, 0.8f, 0.8f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_blur(ctx, 0.1f);

    ps_color fg = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {150, 150, 100, 100};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, HighBlur)
{
    ps_color bg = {0.2f, 0.2f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_blur(ctx, 0.9f);

    ps_color fg = {1.0f, 1.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, BlurWithAlpha)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_blur(ctx, 0.4f);

    ps_color fg = {1.0f, 0.0f, 0.0f, 0.6f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, FilterNearest)
{
    ps_canvas* img_canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 50, 50);
    ASSERT_TRUE(img_canvas);

    ps_context* img_ctx = ps_context_create(img_canvas, NULL);
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(img_ctx, &red);
    ps_clear(img_ctx);

    ps_image* img = ps_image_create_from_canvas(img_canvas, NULL);
    ASSERT_TRUE(img);

    ps_set_filter(ctx, FILTER_NEAREST);
    ps_set_source_image(ctx, img);

    ps_save(ctx);
    ps_scale(ctx, 4.0f, 4.0f);
    ps_rect rc = {0, 0, 50, 50};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);
    ps_restore(ctx);

    ps_image_unref(img);
    ps_context_unref(img_ctx);
    ps_canvas_unref(img_canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, FilterBilinear)
{
    ps_canvas* img_canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 50, 50);
    ASSERT_TRUE(img_canvas);

    ps_context* img_ctx = ps_context_create(img_canvas, NULL);
    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(img_ctx, &blue);
    ps_clear(img_ctx);

    ps_image* img = ps_image_create_from_canvas(img_canvas, NULL);
    ASSERT_TRUE(img);

    ps_set_filter(ctx, FILTER_BILINEAR);
    ps_set_source_image(ctx, img);

    ps_save(ctx);
    ps_scale(ctx, 4.0f, 4.0f);
    ps_rect rc = {0, 0, 50, 50};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);
    ps_restore(ctx);

    ps_image_unref(img);
    ps_context_unref(img_ctx);
    ps_canvas_unref(img_canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, FilterGaussian)
{
    ps_canvas* img_canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 50, 50);
    ASSERT_TRUE(img_canvas);

    ps_context* img_ctx = ps_context_create(img_canvas, NULL);
    ps_color green = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_set_source_color(img_ctx, &green);
    ps_clear(img_ctx);

    ps_image* img = ps_image_create_from_canvas(img_canvas, NULL);
    ASSERT_TRUE(img);

    ps_set_filter(ctx, FILTER_GAUSSIAN);
    ps_set_source_image(ctx, img);

    ps_save(ctx);
    ps_scale(ctx, 4.0f, 4.0f);
    ps_rect rc = {0, 0, 50, 50};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);
    ps_restore(ctx);

    ps_image_unref(img);
    ps_context_unref(img_ctx);
    ps_canvas_unref(img_canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, FilterBicubic)
{
    ps_canvas* img_canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 50, 50);
    ASSERT_TRUE(img_canvas);

    ps_context* img_ctx = ps_context_create(img_canvas, NULL);
    ps_color yellow = {1.0f, 1.0f, 0.0f, 1.0f};
    ps_set_source_color(img_ctx, &yellow);
    ps_clear(img_ctx);

    ps_image* img = ps_image_create_from_canvas(img_canvas, NULL);
    ASSERT_TRUE(img);

    ps_set_filter(ctx, FILTER_BICUBIC);
    ps_set_source_image(ctx, img);

    ps_save(ctx);
    ps_scale(ctx, 4.0f, 4.0f);
    ps_rect rc = {0, 0, 50, 50};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);
    ps_restore(ctx);

    ps_image_unref(img);
    ps_context_unref(img_ctx);
    ps_canvas_unref(img_canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, GammaCorrection)
{
    ps_color bg = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_gamma(ctx, 2.2f);

    ps_color fg = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, LowGamma)
{
    ps_color bg = {0.8f, 0.8f, 0.8f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_gamma(ctx, 0.5f);

    ps_color fg = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {150, 150, 100, 100};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, HighGamma)
{
    ps_color bg = {0.2f, 0.2f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_gamma(ctx, 2.8f);

    ps_color fg = {1.0f, 1.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, AntialiasOn)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_antialias(ctx, True);

    ps_color stroke = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 2.0f);

    ps_point p1 = {50, 200};
    ps_move_to(ctx, &p1);
    ps_point p2 = {350, 200};
    ps_line_to(ctx, &p2);
    ps_stroke(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, AntialiasOff)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_antialias(ctx, False);

    ps_color stroke = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 2.0f);

    ps_point p1 = {50, 200};
    ps_move_to(ctx, &p1);
    ps_point p2 = {350, 200};
    ps_line_to(ctx, &p2);
    ps_stroke(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, AlphaLevel)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_alpha(ctx, 0.5f);

    ps_color fg = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FilterBlurTest, CombinedFilters)
{
    ps_color bg = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_blur(ctx, 0.3f);
    ps_set_gamma(ctx, 1.8f);
    ps_set_alpha(ctx, 0.8f);

    ps_color fg = {1.0f, 0.5f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}
