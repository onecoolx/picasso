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

class RenderingTest : public ::testing::Test
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

TEST_F(RenderingTest, ComplexGradientFill)
{
    ps_point start = {0, 0};
    ps_point end = {400, 400};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_TRUE(gradient);

    ps_color c1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {1.0f, 1.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.5f, &c2);

    ps_color c3 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c3);

    ps_set_source_gradient(ctx, gradient);

    ps_rect rc = {50, 50, 300, 300};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_gradient_unref(gradient);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RenderingTest, RadialGradientWithMultipleStops)
{
    ps_point center = {200, 200};
    ps_point end = {200, 200};
    ps_gradient* gradient = ps_gradient_create_radial(GRADIENT_SPREAD_REPEAT, &center, 0.0f, &end, 150.0f);
    ASSERT_TRUE(gradient);

    ps_color c1 = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.5f, &c2);

    ps_color c3 = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c3);

    ps_set_source_gradient(ctx, gradient);

    ps_rect rc = {50, 50, 300, 300};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    ps_gradient_unref(gradient);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(RenderingTest, PatternWithTransform)
{
    ps_canvas* pattern_canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 50, 50);
    ASSERT_TRUE(pattern_canvas);

    ps_context* pctx = ps_context_create(pattern_canvas, NULL);
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(pctx, &red);
    ps_rect rc1 = {0, 0, 25, 25};
    ps_rectangle(pctx, &rc1);
    ps_fill(pctx);

    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(pctx, &blue);
    ps_rect rc2 = {25, 25, 25, 25};
    ps_rectangle(pctx, &rc2);
    ps_fill(pctx);

    ps_image* img = ps_image_create_from_canvas(pattern_canvas, NULL);
    ASSERT_TRUE(img);

    ps_matrix* matrix = ps_matrix_create();
    ps_matrix_scale(matrix, 2.0f, 2.0f);

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
}

TEST_F(RenderingTest, ComplexPathWithGradient)
{
    ps_point start = {100, 100};
    ps_move_to(ctx, &start);

    ps_point p1 = {300, 100};
    ps_line_to(ctx, &p1);

    ps_point cp1 = {350, 150};
    ps_point cp2 = {350, 250};
    ps_point p2 = {300, 300};
    ps_bezier_to(ctx, &cp1, &cp2, &p2);

    ps_point p3 = {100, 300};
    ps_line_to(ctx, &p3);

    ps_close_path(ctx);

    ps_point gstart = {100, 100};
    ps_point gend = {300, 300};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &gstart, &gend);

    ps_color c1 = {1.0f, 0.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {0.0f, 1.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c2);

    ps_set_source_gradient(ctx, gradient);
    ps_fill(ctx);

    ps_gradient_unref(gradient);
}

TEST_F(RenderingTest, MultipleLayersWithAlpha)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color layer1 = {1.0f, 0.0f, 0.0f, 0.3f};
    ps_set_source_color(ctx, &layer1);
    ps_rect rc1 = {50, 50, 150, 150};
    ps_rectangle(ctx, &rc1);
    ps_fill(ctx);

    ps_color layer2 = {0.0f, 1.0f, 0.0f, 0.3f};
    ps_set_source_color(ctx, &layer2);
    ps_rect rc2 = {100, 100, 150, 150};
    ps_rectangle(ctx, &rc2);
    ps_fill(ctx);

    ps_color layer3 = {0.0f, 0.0f, 1.0f, 0.3f};
    ps_set_source_color(ctx, &layer3);
    ps_rect rc3 = {150, 150, 150, 150};
    ps_rectangle(ctx, &rc3);
    ps_fill(ctx);
}

TEST_F(RenderingTest, StrokeWithDashPattern)
{
    ps_color color = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_set_line_width(ctx, 3.0f);

    float dashes[] = {10.0f, 5.0f, 5.0f, 5.0f};
    ps_set_line_dash(ctx, 0.0f, dashes, 4);

    ps_point p1 = {50, 100};
    ps_move_to(ctx, &p1);

    ps_point p2 = {350, 100};
    ps_line_to(ctx, &p2);

    ps_point p3 = {350, 300};
    ps_line_to(ctx, &p3);

    ps_point p4 = {50, 300};
    ps_line_to(ctx, &p4);

    ps_close_path(ctx);
    ps_stroke(ctx);
}

TEST_F(RenderingTest, DifferentLineJoins)
{
    ps_color color = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_set_line_width(ctx, 10.0f);

    ps_set_line_join(ctx, LINE_JOIN_MITER);
    ps_point p1 = {50, 50};
    ps_move_to(ctx, &p1);
    ps_point p2 = {100, 100};
    ps_line_to(ctx, &p2);
    ps_point p3 = {150, 50};
    ps_line_to(ctx, &p3);
    ps_stroke(ctx);

    ps_set_line_join(ctx, LINE_JOIN_ROUND);
    ps_point p4 = {50, 150};
    ps_move_to(ctx, &p4);
    ps_point p5 = {100, 200};
    ps_line_to(ctx, &p5);
    ps_point p6 = {150, 150};
    ps_line_to(ctx, &p6);
    ps_stroke(ctx);

    ps_set_line_join(ctx, LINE_JOIN_BEVEL);
    ps_point p7 = {50, 250};
    ps_move_to(ctx, &p7);
    ps_point p8 = {100, 300};
    ps_line_to(ctx, &p8);
    ps_point p9 = {150, 250};
    ps_line_to(ctx, &p9);
    ps_stroke(ctx);
}

TEST_F(RenderingTest, DifferentLineCaps)
{
    ps_color color = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_set_line_width(ctx, 10.0f);

    ps_set_line_cap(ctx, LINE_CAP_BUTT);
    ps_point p1 = {50, 50};
    ps_move_to(ctx, &p1);
    ps_point p2 = {150, 50};
    ps_line_to(ctx, &p2);
    ps_stroke(ctx);

    ps_set_line_cap(ctx, LINE_CAP_ROUND);
    ps_point p3 = {50, 100};
    ps_move_to(ctx, &p3);
    ps_point p4 = {150, 100};
    ps_line_to(ctx, &p4);
    ps_stroke(ctx);

    ps_set_line_cap(ctx, LINE_CAP_SQUARE);
    ps_point p5 = {50, 150};
    ps_move_to(ctx, &p5);
    ps_point p6 = {150, 150};
    ps_line_to(ctx, &p6);
    ps_stroke(ctx);
}

TEST_F(RenderingTest, ClipWithComplexPath)
{
    ps_rect rc1 = {100, 100, 200, 200};
    ps_ellipse(ctx, &rc1);
    ps_clip(ctx);

    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);

    for (int i = 0; i < 10; i++) {
        ps_rect rc = {50.0f + i * 30.0f, 50.0f, 20.0f, 300.0f};
        ps_rectangle(ctx, &rc);
        ps_fill(ctx);
    }
}

TEST_F(RenderingTest, TransformWithRotation)
{
    ps_color color = {0.0f, 0.5f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &color);

    for (int i = 0; i < 12; i++) {
        ps_save(ctx);
        ps_translate(ctx, 200.0f, 200.0f);
        ps_rotate(ctx, i * 3.14159f / 6.0f);
        ps_translate(ctx, -200.0f, -200.0f);

        ps_rect rc = {190, 100, 20, 100};
        ps_rectangle(ctx, &rc);
        ps_fill(ctx);

        ps_restore(ctx);
    }
}

TEST_F(RenderingTest, MaskOperation)
{
    ps_mask* mask = ps_mask_create_with_data(NULL, 100, 100);
    if (mask) {
        ps_canvas* canvas = ps_context_get_canvas(ctx);

        ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
        ps_set_source_color(ctx, &color);

        ps_rect rc = {50, 50, 200, 200};
        ps_rectangle(ctx, &rc);

        ps_canvas_set_mask(canvas, mask);
        ps_fill(ctx);

        ps_canvas_reset_mask(canvas);
        ps_mask_unref(mask);
    }
}

TEST_F(RenderingTest, ShadowWithBlur)
{
    ps_set_shadow(ctx, 10.0f, 10.0f, 0.5f);
    ps_color shadow_color = {0.0f, 0.0f, 0.0f, 0.5f};
    ps_set_shadow_color(ctx, &shadow_color);

    ps_color fill_color = {1.0f, 0.5f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fill_color);

    ps_rect rc = {100, 100, 200, 150};
    ps_rounded_rect(ctx, &rc, 20.0f, 20.0f, 20.0f, 20.0f, 20.0f, 20.0f, 20.0f, 20.0f);
    ps_fill(ctx);
}

TEST_F(RenderingTest, TextWithGradient)
{
    ps_font* font = ps_font_create("Sans-Serif", CHARSET_ANSI, 48, FONT_WEIGHT_BOLD, False);
    if (font) {
        ps_set_font(ctx, font);

        ps_point start = {0, 0};
        ps_point end = {400, 0};
        ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);

        ps_color c1 = {1.0f, 0.0f, 0.0f, 1.0f};
        ps_gradient_add_color_stop(gradient, 0.0f, &c1);

        ps_color c2 = {0.0f, 0.0f, 1.0f, 1.0f};
        ps_gradient_add_color_stop(gradient, 1.0f, &c2);

        ps_set_source_gradient(ctx, gradient);
        ps_text_out_length(ctx, 50, 200, "Gradient Text", 13);

        ps_gradient_unref(gradient);
        ps_font_unref(font);
    }
}
