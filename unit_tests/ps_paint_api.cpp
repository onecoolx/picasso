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

class PaintTest : public ::testing::Test
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
        ctx = ps_context_create(get_test_canvas(), nullptr);
        path = ps_path_create();
        ASSERT_NE(ctx, nullptr);
        ASSERT_NE(path, nullptr);
    }

    void TearDown() override
    {
        if (path) { ps_path_unref(path); }
        if (ctx) { ps_context_unref(ctx); }
    }

    void createTestPath()
    {
        ps_point start = {50, 100};
        ps_point end = {200, 100};
        ps_path_move_to(path, &start);
        ps_path_line_to(path, &end);
    }

    void createComplexPath()
    {
        ps_point p1 = {100, 50};
        ps_point p2 = {150, 150};
        ps_point p3 = {200, 50};
        ps_path_move_to(path, &p1);
        ps_path_line_to(path, &p2);
        ps_path_line_to(path, &p3);
    }

    void createComplexPath2()
    {
        ps_point p1 = {100, 50};
        ps_point p2 = {200, 150};
        ps_point p3 = {300, 50};
        ps_path_move_to(path, &p1);
        ps_path_line_to(path, &p2);
        ps_path_line_to(path, &p3);
    }

    void createStarPath()
    {
        ps_point center = {200, 200};
        for (int i = 0; i < 10; i++) {
            float angle = i * 3.14159f / 5;
            float radius = (i % 2 == 0) ? 100.0f : 40.0f;
            ps_point p = {
                center.x + radius * cosf(angle),
                      center.y + radius * sinf(angle)
            };
            if (i == 0) {
                ps_path_move_to(path, &p);
            } else {
                ps_path_line_to(path, &p);
            }
        }
        ps_path_sub_close(path);

        for (int i = 0; i < 10; i++) {
            float angle = i * 3.14159f / 5;
            float radius = (i % 2 == 0) ? 50.0f : 20.0f;
            ps_point p = {
                center.x + radius * cosf(angle),
                      center.y + radius * sinf(angle)
            };
            if (i == 0) {
                ps_path_move_to(path, &p);
            } else {
                ps_path_line_to(path, &p);
            }
        }
        ps_path_sub_close(path);
    }

    ps_context* ctx;
    ps_path* path;
};

// Stroke Tests
TEST_F(PaintTest, StrokeLineCap)
{
    createTestPath();
    ps_set_line_width(ctx, 10.0f);

    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_line_cap(ctx, LINE_CAP_BUTT);
    ps_set_stroke_color(ctx, &color);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    ps_translate(ctx, 0, 50.0f);

    ps_set_line_cap(ctx, LINE_CAP_ROUND);
    ps_color color2 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_stroke_color(ctx, &color2);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    ps_translate(ctx, 0, 50.0f);

    ps_set_line_cap(ctx, LINE_CAP_SQUARE);
    ps_color color3 = {0.0f, 1.0f, 0.5f, 1.0f};
    ps_set_stroke_color(ctx, &color3);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    EXPECT_SNAPSHOT_EQ(stroke_line_cap);
}

TEST_F(PaintTest, StrokeLineJoinMiter)
{
    createComplexPath();
    ps_set_line_width(ctx, 15.0f);

    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &color);
    ps_set_line_join(ctx, LINE_JOIN_MITER);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    ps_translate(ctx, 0, 80.0f);

    ps_color color2 = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &color2);
    ps_set_line_join(ctx, LINE_JOIN_ROUND);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    ps_translate(ctx, 0, 80.0f);

    ps_color color3 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_stroke_color(ctx, &color3);
    ps_set_line_join(ctx, LINE_JOIN_BEVEL);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    EXPECT_SNAPSHOT_EQ(stroke_line_join);
}

TEST_F(PaintTest, StrokeLineInnerJoin)
{
    createComplexPath2();
    ps_set_line_width(ctx, 10.0f);

    ps_color color = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_stroke_color(ctx, &color);
    ps_set_line_inner_join(ctx, LINE_INNER_MITER);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    ps_translate(ctx, 0, 80.0f);

    ps_color color1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &color1);
    ps_set_line_inner_join(ctx, LINE_INNER_BEVEL);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    ps_translate(ctx, 0, 80.0f);

    ps_color color2 = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &color2);
    ps_set_line_inner_join(ctx, LINE_INNER_ROUND);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    ps_translate(ctx, 0, 80.0f);

    ps_color color3 = {1.0f, 0.0f, 1.0f, 1.0f};
    ps_set_stroke_color(ctx, &color3);
    ps_set_line_inner_join(ctx, LINE_INNER_JAG);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    EXPECT_SNAPSHOT_EQ(stroke_line_inner_join);
}

TEST_F(PaintTest, StrokeWidth)
{
    createTestPath();
    float old_width = ps_set_line_width(ctx, 5.0f);
    EXPECT_GT(old_width, 0.0f);
    ps_color color = {1.0f, 0.5f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &color);
    ps_set_path(ctx, path);
    ps_stroke(ctx);
    EXPECT_SNAPSHOT_EQ(stroke_width_5);
}

TEST_F(PaintTest, StrokeMiterLimit)
{
    createComplexPath();
    float old_limit = ps_set_miter_limit(ctx, 1.0f);
    EXPECT_GT(old_limit, 0.0f);
    ps_set_line_join(ctx, LINE_JOIN_MITER);
    ps_set_line_width(ctx, 20.0f);
    ps_color color = {0.5f, 0.0f, 1.0f, 1.0f};
    ps_set_stroke_color(ctx, &color);
    ps_set_path(ctx, path);
    ps_stroke(ctx);
    EXPECT_SNAPSHOT_EQ(stroke_miter_limit);
}

TEST_F(PaintTest, StrokeDashPattern)
{
    createTestPath();
    ps_color color = {1.0f, 0.0f, 1.0f, 1.0f};
    ps_set_stroke_color(ctx, &color);
    ps_set_line_width(ctx, 5.0f);

    float dashes[] = {10.0f, 5.0f, 2.0f, 5.0f};
    ps_set_line_dash(ctx, 0.0f, dashes, 4);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    ps_translate(ctx, 0, 50.0f);

    float dashes1[] = {10.0f, 5.0f};
    ps_set_line_dash(ctx, 5.0f, dashes1, 2);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    EXPECT_SNAPSHOT_EQ(stroke_dash);
}

TEST_F(PaintTest, StrokeResetDash)
{
    createTestPath();
    float dashes[] = {10.0f, 5.0f};
    ps_set_line_dash(ctx, 0.0f, dashes, 2);
    ps_set_line_width(ctx, 5.0f);
    ps_color color = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_stroke_color(ctx, &color);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    ps_translate(ctx, 0, 50.0f);
    ps_reset_line_dash(ctx);
    ps_set_path(ctx, path);
    ps_stroke(ctx);
    EXPECT_SNAPSHOT_EQ(stroke_reset_dash);
}

// Fill Tests
TEST_F(PaintTest, FillRule)
{
    createStarPath();
    ps_set_fill_rule(ctx, FILL_RULE_WINDING);
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_set_path(ctx, path);
    ps_fill(ctx);

    ps_translate(ctx, 250, 0.0f);

    ps_set_fill_rule(ctx, FILL_RULE_EVEN_ODD);
    ps_color color1 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &color1);
    ps_set_path(ctx, path);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(fill_rule_winding_evenodd);
}

TEST_F(PaintTest, FillAntialias)
{
    ps_rect rc = {100, 100, 150, 100};
    ps_path_add_ellipse(path, &rc);

    ps_set_antialias(ctx, True);
    ps_color color = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_set_path(ctx, path);
    ps_fill(ctx);

    ps_translate(ctx, 250, 0.0f);

    ps_set_antialias(ctx, False);
    ps_color color1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color1);
    ps_set_path(ctx, path);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(fill_whether_antialias);
}

TEST_F(PaintTest, BadCaseAntialiasNullContext)
{
    ps_set_antialias(nullptr, True);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

// Paint Tests (Combined Fill and Stroke)
TEST_F(PaintTest, PaintFillAndStroke)
{
    createStarPath();
    ps_set_fill_rule(ctx, FILL_RULE_EVEN_ODD);
    ps_set_line_join(ctx, LINE_JOIN_ROUND);
    ps_set_line_width(ctx, 3.0f);

    // Set fill color
    ps_color fill_color = {1.0f, 0.8f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fill_color);

    // Set stroke color
    ps_color stroke_color = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_stroke_color(ctx, &stroke_color);

    ps_set_path(ctx, path);
    ps_paint(ctx);

    EXPECT_SNAPSHOT_EQ(paint_fill_stroke);
}

// Gradient and Pattern Tests
TEST_F(PaintTest, StrokeWithLinearGradient)
{
    createTestPath();
    ps_point s = { 50.0f, 100.0f };
    ps_point e = { 200.0f, 100.0f };
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &s, &e);
    ASSERT_NE(gradient, nullptr);

    ps_color stop1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_color stop2 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &stop1);
    ps_gradient_add_color_stop(gradient, 1.0f, &stop2);

    ps_set_stroke_gradient(ctx, gradient);
    ps_set_line_width(ctx, 10.0f);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    ps_gradient_unref(gradient);
    EXPECT_SNAPSHOT_EQ(stroke_linear_gradient);
}

TEST_F(PaintTest, FillWithRadialGradient)
{
    ps_rect rc = {100, 100, 150, 100};
    ps_path_add_ellipse(path, &rc);

    ps_point s = { 120.0f, 120.0f };
    ps_point e = { 150.0f, 150.0f };
    ps_gradient* gradient = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &s, 0.0f, &e, 80.0f);
    ASSERT_NE(gradient, nullptr);

    ps_color stop1 = {1.0f, 1.0f, 0.0f, 1.0f};
    ps_color stop2 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &stop1);
    ps_gradient_add_color_stop(gradient, 1.0f, &stop2);

    ps_set_source_gradient(ctx, gradient);
    ps_set_path(ctx, path);
    ps_fill(ctx);

    ps_gradient_unref(gradient);
    EXPECT_SNAPSHOT_EQ(fill_radial_gradient);
}

// Bad Case Tests
TEST_F(PaintTest, BadCaseNullContext)
{
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(nullptr, &color);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_set_source_color(nullptr, &color);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_set_line_cap(nullptr, LINE_CAP_BUTT);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_set_line_join(nullptr, LINE_JOIN_MITER);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_stroke(nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_fill(nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_paint(nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

TEST_F(PaintTest, BadCaseNullColor)
{
    ps_set_stroke_color(ctx, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_set_source_color(ctx, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_set_shadow_color(ctx, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

TEST_F(PaintTest, BadCaseInvalidLineCap)
{
    // Test with invalid enum value (cast to valid type)
    ps_set_line_cap(ctx, static_cast<ps_line_cap>(999));
    EXPECT_EQ(ps_last_status(), STATUS_UNKNOWN_ERROR);
}

TEST_F(PaintTest, BadCaseInvalidLineJoin)
{
    ps_set_line_join(ctx, static_cast<ps_line_join>(999));
    EXPECT_EQ(ps_last_status(), STATUS_UNKNOWN_ERROR);
}

TEST_F(PaintTest, BadCaseInvalidFillRule)
{
    ps_fill_rule result = ps_set_fill_rule(ctx, static_cast<ps_fill_rule>(999));
    EXPECT_EQ(ps_last_status(), STATUS_UNKNOWN_ERROR);
    EXPECT_EQ(result, FILL_RULE_ERROR);
}

TEST_F(PaintTest, BadCaseZeroLineWidth)
{
    float old_width = ps_set_line_width(ctx, 0.0f);
    EXPECT_EQ(old_width, 1.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
}

TEST_F(PaintTest, BadCaseNegativeLineWidth)
{
    float old_width = ps_set_line_width(ctx, -5.0f);
    EXPECT_EQ(old_width, 1.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
}

TEST_F(PaintTest, BadCaseZeroMiterLimit)
{
    float old_limit = ps_set_miter_limit(ctx, 0.0f);
    EXPECT_EQ(old_limit, 4.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
}

TEST_F(PaintTest, BadCaseNegativeMiterLimit)
{
    float old_limit = ps_set_miter_limit(ctx, -1.0f);
    EXPECT_EQ(old_limit, 4.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
}

TEST_F(PaintTest, BadCaseNullGradient)
{
    ps_set_stroke_gradient(ctx, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_set_source_gradient(ctx, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

TEST_F(PaintTest, BadCaseNullImage)
{
    ps_set_stroke_image(ctx, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_set_source_image(ctx, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

TEST_F(PaintTest, BadCaseNullPattern)
{
    ps_set_stroke_pattern(ctx, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_set_source_pattern(ctx, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

TEST_F(PaintTest, BadCaseNullCanvas)
{
    ps_set_stroke_canvas(ctx, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_set_source_canvas(ctx, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

TEST_F(PaintTest, BadCaseEmptyDashArray)
{
    ps_set_line_dash(ctx, 0.0f, nullptr, 0);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

TEST_F(PaintTest, BadCaseStrokeEmptyPath)
{
    // Don't add any path commands
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &color);
    ps_set_line_width(ctx, 5.0f);
    ps_set_path(ctx, path);
    ps_stroke(ctx); // Should handle empty path gracefully
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
}

TEST_F(PaintTest, BadCaseFillEmptyPath)
{
    // Don't add any path commands
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_set_path(ctx, path);
    ps_fill(ctx); // Should handle empty path gracefully
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
}

// Shadow Tests
TEST_F(PaintTest, ShadowBasic)
{
    createTestPath();

    ps_set_shadow(ctx, 5.0f, 5.0f, 0.4f);
    ps_color shadow_color = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_shadow_color(ctx, &shadow_color);
    ps_set_line_width(ctx, 10.0f);
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &color);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    ps_translate(ctx, 0.0f, 50.0f);

    ps_reset_shadow(ctx);
    ps_color color1 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_stroke_color(ctx, &color1);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    EXPECT_SNAPSHOT_EQ(shadow_draw_basic);
}

TEST_F(PaintTest, BadCaseShadowNullContext)
{
    ps_set_shadow(nullptr, 5.0f, 5.0f, 2.0f);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_reset_shadow(nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_color color = {0.0f, 0.0f, 0.0f, 0.5f};
    ps_set_shadow_color(nullptr, &color);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

TEST_F(PaintTest, BadCaseShadowNullColor)
{
    ps_set_shadow_color(ctx, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

// Alpha Tests
TEST_F(PaintTest, AlphaValues)
{
    createTestPath();
    float old_alpha = ps_set_alpha(ctx, 0.5f);
    EXPECT_GT(old_alpha, 0.0f);
    ps_set_line_width(ctx, 10.0f);
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &color);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    EXPECT_SNAPSHOT_EQ(alpha_50_percent);
}

TEST_F(PaintTest, AlphaClamping)
{
    // Test alpha clamping to 0-1 range
    ps_set_alpha(ctx, -0.5f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    ps_set_alpha(ctx, 1.5f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Alpha should be clamped to valid range
    float current_alpha = ps_set_alpha(ctx, 0.8f);
    EXPECT_GE(current_alpha, 0.0f);
    EXPECT_LE(current_alpha, 1.0f);
}

TEST_F(PaintTest, BadCaseAlphaNullContext)
{
    float result = ps_set_alpha(nullptr, 0.5f);
    EXPECT_EQ(result, 0.0f);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

// Gamma Tests
TEST_F(PaintTest, GammaValues)
{
    ps_rect rc = {100, 100, 150, 100};
    ps_path_add_ellipse(path, &rc);

    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &color);
    ps_set_path(ctx, path);
    ps_stroke(ctx);

    ps_translate(ctx, 200.0f, 0.0f);

    float old_gamma = ps_set_gamma(ctx, 3.0f);
    EXPECT_GT(old_gamma, 0.0f);
    ps_set_path(ctx, path);
    ps_stroke(ctx);
    EXPECT_SNAPSHOT_EQ(gamma_value_1_5);
}

TEST_F(PaintTest, BadCaseGammaNullContext)
{
    float result = ps_set_gamma(nullptr, 1.0f);
    EXPECT_EQ(result, 0.0f);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

// Blur Tests
TEST_F(PaintTest, BlurValues)
{
    createComplexPath();
    float old_blur = ps_set_blur(ctx, 0.5f);
    EXPECT_GE(old_blur, 0.0f);
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_set_path(ctx, path);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(blur_draw_0_5);
}

TEST_F(PaintTest, BadCaseBlurNullContext)
{
    float result = ps_set_blur(nullptr, 0.5f);
    EXPECT_EQ(result, 0.0f);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

// Composite Operator Tests
TEST_F(PaintTest, CompositeOperators)
{
    createComplexPath();
    ps_composite old_op = ps_set_composite_operator(ctx, COMPOSITE_SRC_OVER);
    EXPECT_NE(old_op, COMPOSITE_ERROR);
    ps_color color = {1.0f, 0.0f, 0.0f, 0.5f};
    ps_set_source_color(ctx, &color);
    ps_set_path(ctx, path);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(composite_source_over);
}

TEST_F(PaintTest, BadCaseCompositeNullContext)
{
    ps_composite result = ps_set_composite_operator(nullptr, COMPOSITE_SRC_OVER);
    EXPECT_EQ(result, COMPOSITE_ERROR);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

TEST_F(PaintTest, BadCaseInvalidCompositeOperator)
{
    ps_composite result = ps_set_composite_operator(ctx, static_cast<ps_composite>(999));
    EXPECT_EQ(result, COMPOSITE_ERROR);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

#if 0
// Filter Tests
TEST_F(PaintTest, FilterTypes)
{
    createTestPath();
    ps_filter old_filter = ps_set_filter(ctx, FILTER_BILINEAR);
    EXPECT_NE(old_filter, FILTER_UNKNOWN);
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_fill(ctx);
    EXPECT_SNAPSHOT_EQ(filter_bilinear);
}
#endif

TEST_F(PaintTest, BadCaseFilterNullContext)
{
    ps_filter result = ps_set_filter(nullptr, FILTER_BILINEAR);
    EXPECT_EQ(result, FILTER_UNKNOWN);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

TEST_F(PaintTest, BadCaseInvalidFilter)
{
    ps_filter result = ps_set_filter(ctx, static_cast<ps_filter>(999));
    EXPECT_EQ(result, FILTER_UNKNOWN);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}
