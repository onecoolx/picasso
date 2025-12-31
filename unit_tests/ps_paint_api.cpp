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
            float radius = (i % 2 == 0) ? 100 : 40;
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
            float radius = (i % 2 == 0) ? 50 : 20;
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
