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

class AdvancedRenderingTest : public ::testing::Test
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

TEST_F(AdvancedRenderingTest, FillRuleWinding)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_fill_rule(ctx, FILL_RULE_WINDING);

    ps_rect outer = {50, 50, 300, 300};
    ps_rectangle(ctx, &outer);

    ps_rect inner = {100, 100, 200, 200};
    ps_rectangle(ctx, &inner);

    ps_color fill = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fill);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, FillRuleEvenOdd)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_fill_rule(ctx, FILL_RULE_EVEN_ODD);

    ps_rect outer = {50, 50, 300, 300};
    ps_rectangle(ctx, &outer);

    ps_rect inner = {100, 100, 200, 200};
    ps_rectangle(ctx, &inner);

    ps_color fill = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &fill);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, ComplexClipping)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_rect clip1 = {50, 50, 200, 200};
    ps_rectangle(ctx, &clip1);
    ps_clip(ctx);

    ps_rect clip2 = {150, 150, 200, 200};
    ps_rectangle(ctx, &clip2);
    ps_clip(ctx);

    ps_color fill = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fill);
    ps_rect rc = {0, 0, 400, 400};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, SaveRestoreState)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_save(ctx);
    ps_translate(ctx, 100.0f, 100.0f);
    ps_rotate(ctx, 0.785398f);
    ps_scale(ctx, 2.0f, 2.0f);

    ps_color fill1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fill1);
    ps_rect rc1 = {0, 0, 50, 50};
    ps_rectangle(ctx, &rc1);
    ps_fill(ctx);

    ps_restore(ctx);

    ps_color fill2 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &fill2);
    ps_rect rc2 = {200, 200, 50, 50};
    ps_rectangle(ctx, &rc2);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, NestedSaveRestore)
{
    ps_save(ctx);
    ps_translate(ctx, 50.0f, 50.0f);

    ps_save(ctx);
    ps_rotate(ctx, 0.5f);

    ps_save(ctx);
    ps_scale(ctx, 1.5f, 1.5f);

    ps_color fill = {1.0f, 0.5f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fill);
    ps_rect rc = {0, 0, 50, 50};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_restore(ctx);
    ps_restore(ctx);
    ps_restore(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, MiterLimit)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color stroke = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 20.0f);
    ps_set_line_join(ctx, LINE_JOIN_MITER);
    ps_set_miter_limit(ctx, 2.0f);

    ps_point p1 = {100, 200};
    ps_move_to(ctx, &p1);
    ps_point p2 = {200, 100};
    ps_line_to(ctx, &p2);
    ps_point p3 = {300, 200};
    ps_line_to(ctx, &p3);
    ps_stroke(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, DashPattern)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color stroke = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 3.0f);

    float dashes[] = {20.0f, 10.0f, 5.0f, 10.0f};
    ps_set_line_dash(ctx, 0.0f, dashes, 4);

    ps_point p1 = {50, 100};
    ps_move_to(ctx, &p1);
    ps_point p2 = {350, 100};
    ps_line_to(ctx, &p2);
    ps_stroke(ctx);

    ps_reset_line_dash(ctx);

    ps_point p3 = {50, 150};
    ps_move_to(ctx, &p3);
    ps_point p4 = {350, 150};
    ps_line_to(ctx, &p4);
    ps_stroke(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, RadialGradientSpread)
{
    ps_color bg = {0.2f, 0.2f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_point center = {200, 200};
    ps_point end = {200, 200};
    ps_gradient* gradient = ps_gradient_create_radial(GRADIENT_SPREAD_REFLECT, &center, 0.0f, &end, 80.0f);
    ASSERT_TRUE(gradient);

    ps_color c1 = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {0.0f, 0.5f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c2);

    ps_set_source_gradient(ctx, gradient);
    ps_rect rc = {0, 0, 400, 400};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_gradient_unref(gradient);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, LinearGradientRepeat)
{
    ps_color bg = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_point start = {100, 200};
    ps_point end = {200, 200};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_REPEAT, &start, &end);
    ASSERT_TRUE(gradient);

    ps_color c1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c2);

    ps_set_source_gradient(ctx, gradient);
    ps_rect rc = {0, 0, 400, 400};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_gradient_unref(gradient);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, GradientTransform)
{
    ps_point start = {0, 0};
    ps_point end = {100, 0};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_TRUE(gradient);

    ps_color c1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c2);

    ps_matrix* matrix = ps_matrix_create();
    ps_matrix_translate(matrix, 100.0f, 100.0f);
    ps_matrix_rotate(matrix, 0.785398f);
    ps_matrix_scale(matrix, 2.0f, 2.0f);

    ps_gradient_transform(gradient, matrix);

    ps_set_source_gradient(ctx, gradient);
    ps_rect rc = {0, 0, 400, 400};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_matrix_unref(matrix);
    ps_gradient_unref(gradient);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, PatternTransform)
{
    ps_canvas* pattern_canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 40, 40);
    ASSERT_TRUE(pattern_canvas);

    ps_context* pctx = ps_context_create(pattern_canvas, NULL);
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(pctx, &red);
    ps_rect rc1 = {0, 0, 20, 20};
    ps_rectangle(pctx, &rc1);
    ps_fill(pctx);

    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(pctx, &blue);
    ps_rect rc2 = {20, 20, 20, 20};
    ps_rectangle(pctx, &rc2);
    ps_fill(pctx);

    ps_image* img = ps_image_create_from_canvas(pattern_canvas, NULL);
    ASSERT_TRUE(img);

    ps_matrix* matrix = ps_matrix_create();
    ps_matrix_rotate(matrix, 0.785398f);

    ps_pattern* pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, matrix);
    ASSERT_TRUE(pattern);

    ps_set_source_pattern(ctx, pattern);
    ps_rect rc3 = {0, 0, 400, 400};
    ps_rectangle(ctx, &rc3);
    ps_fill(ctx);

    ps_pattern_unref(pattern);
    ps_matrix_unref(matrix);
    ps_image_unref(img);
    ps_context_unref(pctx);
    ps_canvas_unref(pattern_canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, ImageScaling)
{
    ps_canvas* img_canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(img_canvas);

    ps_context* img_ctx = ps_context_create(img_canvas, NULL);

    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);

    ps_color c1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c2);

    ps_set_source_gradient(img_ctx, gradient);
    ps_rect rc1 = {0, 0, 100, 100};
    ps_rectangle(img_ctx, &rc1);
    ps_fill(img_ctx);

    ps_image* img = ps_image_create_from_canvas(img_canvas, NULL);
    ASSERT_TRUE(img);

    ps_set_source_image(ctx, img);
    ps_save(ctx);
    ps_scale(ctx, 3.0f, 3.0f);
    ps_rect rc2 = {0, 0, 100, 100};
    ps_rectangle(ctx, &rc2);
    ps_fill(ctx);
    ps_restore(ctx);

    ps_image_unref(img);
    ps_gradient_unref(gradient);
    ps_context_unref(img_ctx);
    ps_canvas_unref(img_canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, CanvasBitBlt)
{
    ps_canvas* src_canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(src_canvas);

    ps_context* src_ctx = ps_context_create(src_canvas, NULL);
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(src_ctx, &red);
    ps_clear(src_ctx);

    ps_canvas* dst_canvas = ps_context_get_canvas(ctx);

    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_rect src_rect = {0, 0, 100, 100};
    ps_point dst_point = {50, 50};
    ps_canvas_bitblt(src_canvas, &src_rect, dst_canvas, &dst_point);

    ps_context_unref(src_ctx);
    ps_canvas_unref(src_canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, MultipleGradientStops)
{
    ps_point start = {0, 200};
    ps_point end = {400, 200};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_TRUE(gradient);

    ps_color c1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {1.0f, 1.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.25f, &c2);

    ps_color c3 = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.5f, &c3);

    ps_color c4 = {0.0f, 1.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.75f, &c4);

    ps_color c5 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c5);

    ps_set_source_gradient(ctx, gradient);
    ps_rect rc = {0, 0, 400, 400};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_gradient_unref(gradient);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(AdvancedRenderingTest, ClearGradientStops)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_TRUE(gradient);

    ps_color c1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c2);

    ps_gradient_clear_color_stops(gradient);

    ps_color c3 = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c3);

    ps_color c4 = {1.0f, 1.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c4);

    ps_set_source_gradient(ctx, gradient);
    ps_rect rc = {0, 0, 400, 400};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_gradient_unref(gradient);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}
