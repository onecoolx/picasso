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

class PathCurveTest : public ::testing::Test
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

TEST_F(PathCurveTest, QuadraticBezier)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color stroke = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 2.0f);

    ps_point start = {50, 150};
    ps_move_to(ctx, &start);

    ps_point cp = {200, 50};
    ps_point end = {350, 150};
    ps_quad_to(ctx, &cp, &end);

    ps_stroke(ctx);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, CubicBezier)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color stroke = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 3.0f);

    ps_point start = {50, 200};
    ps_move_to(ctx, &start);

    ps_point cp1 = {150, 50};
    ps_point cp2 = {250, 350};
    ps_point end = {350, 200};
    ps_bezier_to(ctx, &cp1, &cp2, &end);

    ps_stroke(ctx);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, MultipleCurves)
{
    ps_color bg = {0.9f, 0.9f, 0.9f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fill = {1.0f, 0.5f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fill);

    ps_point p1 = {100, 200};
    ps_move_to(ctx, &p1);

    ps_point cp1 = {150, 100};
    ps_point p2 = {200, 200};
    ps_quad_to(ctx, &cp1, &p2);

    ps_point cp2 = {250, 300};
    ps_point p3 = {300, 200};
    ps_quad_to(ctx, &cp2, &p3);

    ps_point p4 = {300, 250};
    ps_line_to(ctx, &p4);

    ps_point p5 = {100, 250};
    ps_line_to(ctx, &p5);

    ps_close_path(ctx);
    ps_fill(ctx);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, SmoothCurve)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color stroke = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 2.0f);

    ps_point start = {50, 200};
    ps_move_to(ctx, &start);

    for (int i = 0; i < 5; i++) {
        ps_point cp = {100.0f + i * 60.0f, 100.0f + (i % 2) * 200.0f};
        ps_point end = {130.0f + i * 60.0f, 200.0f};
        ps_quad_to(ctx, &cp, &end);
    }

    ps_stroke(ctx);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, ArcPath)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color stroke = {0.0f, 0.5f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 3.0f);

    ps_point center = {200, 200};
    ps_arc(ctx, &center, 100.0f, 0.0f, 3.14159f, False);
    ps_stroke(ctx);

    ps_arc(ctx, &center, 80.0f, 0.0f, 6.28318f, False);
    ps_stroke(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, ComplexPath)
{
    ps_color bg = {0.2f, 0.2f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fill = {1.0f, 1.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fill);

    ps_point center = {200, 200};
    float radius = 100.0f;

    for (int i = 0; i < 5; i++) {
        float angle1 = i * 6.28318f / 5.0f;
        float angle2 = (i + 2) * 6.28318f / 5.0f;

        ps_point p1 = {center.x + radius * cos(angle1), center.y + radius * sin(angle1)};

        if (i == 0) {
            ps_move_to(ctx, &p1);
        }

        ps_point p2 = {center.x + radius * cos(angle2), center.y + radius * sin(angle2)};
        ps_line_to(ctx, &p2);
    }

    ps_close_path(ctx);
    ps_fill(ctx);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, RoundedRectangle)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fill = {0.2f, 0.6f, 0.8f, 1.0f};
    ps_set_source_color(ctx, &fill);

    ps_rect rc = {100, 100, 200, 150};
    ps_rounded_rect(ctx, &rc, 20.0f, 20.0f, 20.0f, 20.0f, 20.0f, 20.0f, 20.0f, 20.0f);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, EllipseArc)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color stroke = {0.8f, 0.2f, 0.2f, 1.0f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 4.0f);

    ps_rect rc = {100, 100, 200, 150};
    ps_ellipse(ctx, &rc);
    ps_stroke(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, PathTransform)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fill = {0.5f, 0.0f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &fill);

    ps_save(ctx);
    ps_translate(ctx, 200.0f, 200.0f);
    ps_rotate(ctx, 0.785398f);
    ps_scale(ctx, 1.5f, 1.0f);

    ps_rect rc = {-50, -50, 100, 100};
    ps_rounded_rect(ctx, &rc, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f);
    ps_fill(ctx);

    ps_restore(ctx);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, SubPath)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fill = {0.0f, 0.8f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fill);

    ps_rect rc1 = {50, 50, 100, 100};
    ps_rectangle(ctx, &rc1);

    ps_rect rc2 = {200, 200, 100, 100};
    ps_ellipse(ctx, &rc2);

    ps_fill(ctx);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, CurveWithGradient)
{
    ps_color bg = {0.1f, 0.1f, 0.1f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_point start = {50, 200};
    ps_move_to(ctx, &start);

    ps_point cp1 = {150, 50};
    ps_point cp2 = {250, 350};
    ps_point end = {350, 200};
    ps_bezier_to(ctx, &cp1, &cp2, &end);

    ps_point p2 = {350, 250};
    ps_line_to(ctx, &p2);

    ps_point cp3 = {250, 400};
    ps_point cp4 = {150, 100};
    ps_point p3 = {50, 250};
    ps_bezier_to(ctx, &cp3, &cp4, &p3);

    ps_close_path(ctx);

    ps_point gstart = {50, 200};
    ps_point gend = {350, 200};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &gstart, &gend);
    ASSERT_TRUE(gradient);

    ps_color c1 = {1.0f, 0.0f, 0.5f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {0.0f, 0.5f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c2);

    ps_set_source_gradient(ctx, gradient);
    ps_fill(ctx);

    ps_gradient_unref(gradient);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, TangentCurves)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color stroke = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 2.0f);

    ps_point p1 = {50, 200};
    ps_move_to(ctx, &p1);

    ps_point cp1 = {100, 100};
    ps_point cp2 = {150, 100};
    ps_point p2 = {200, 200};
    ps_bezier_to(ctx, &cp1, &cp2, &p2);

    ps_point cp3 = {250, 300};
    ps_point cp4 = {300, 300};
    ps_point p3 = {350, 200};
    ps_bezier_to(ctx, &cp3, &cp4, &p3);

    ps_stroke(ctx);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, ClosedCurvePath)
{
    ps_color bg = {0.9f, 0.9f, 0.9f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_color fill = {0.8f, 0.4f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &fill);

    ps_point p1 = {200, 100};
    ps_move_to(ctx, &p1);

    ps_point cp1 = {300, 150};
    ps_point p2 = {250, 250};
    ps_quad_to(ctx, &cp1, &p2);

    ps_point cp2 = {150, 300};
    ps_point p3 = {100, 200};
    ps_quad_to(ctx, &cp2, &p3);

    ps_point cp3 = {100, 100};
    ps_point p4 = {200, 100};
    ps_quad_to(ctx, &cp3, &p4);

    ps_close_path(ctx);
    ps_fill(ctx);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathCurveTest, CurveStrokeAndFill)
{
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_point p1 = {100, 200};
    ps_move_to(ctx, &p1);

    ps_point cp1 = {150, 100};
    ps_point cp2 = {250, 300};
    ps_point p2 = {300, 200};
    ps_bezier_to(ctx, &cp1, &cp2, &p2);

    ps_point p3 = {300, 250};
    ps_line_to(ctx, &p3);

    ps_point p4 = {100, 250};
    ps_line_to(ctx, &p4);

    ps_close_path(ctx);

    ps_color fill = {0.8f, 0.8f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &fill);
    ps_fill(ctx);

    ps_move_to(ctx, &p1);
    ps_bezier_to(ctx, &cp1, &cp2, &p2);
    ps_line_to(ctx, &p3);
    ps_line_to(ctx, &p4);
    ps_close_path(ctx);

    ps_color stroke = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &stroke);
    ps_set_line_width(ctx, 3.0f);
    ps_stroke(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}
