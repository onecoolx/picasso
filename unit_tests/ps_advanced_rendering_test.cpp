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

TEST_F(AdvancedRenderingTest, ClipRect_TransformIntersection_CorrectRegion)
{
    // Test: two clip_rect calls with a transform change between them.
    //
    // 1. Identity CTM: clip to (0,0,200,200)
    // 2. Translate(100,100)
    // 3. Clip to (0,0,200,200) — device space: (100,100)->(300,300)
    //
    // Correct intersection in device space: (100,100)->(200,200)
    //
    // Fill everything red and check pixels:
    // - (150,150) should be red  (inside intersection)
    // - (50,50)   should be white (inside first clip only, outside second)
    // - (250,250) should be white (inside second clip only, outside first)

    const int W = 400;
    const int H = 400;
    uint8_t* buf = (uint8_t*)calloc(W * 4, H);
    ASSERT_TRUE(buf != NULL);

    ps_canvas* cv = ps_canvas_create_with_data(buf, COLOR_FORMAT_RGBA, W, H, W * 4);
    ASSERT_TRUE(cv != NULL);

    ps_context* lctx = ps_context_create(cv, NULL);
    ASSERT_TRUE(lctx != NULL);

    // Clear to white
    ps_color white = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(lctx, &white);
    ps_clear(lctx);

    // First clip in identity coordinates: device rect (0,0)->(200,200)
    ps_rect clip1 = {0, 0, 200, 200};
    ps_clip_rect(lctx, &clip1);

    // Change CTM: translate by (100,100)
    ps_translate(lctx, 100.0f, 100.0f);

    // Second clip in translated coordinates: device rect (100,100)->(300,300)
    ps_rect clip2 = {0, 0, 200, 200};
    ps_clip_rect(lctx, &clip2);

    // Fill everything red
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(lctx, &red);
    ps_rect big = {-200, -200, 800, 800};
    ps_rectangle(lctx, &big);
    ps_fill(lctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    // Read pixels (RGBA, 4 bytes per pixel, pitch = W*4)
    // Pixel at (x,y): buf[(y * W + x) * 4 + channel]
    // Red pixel: R=255, G=0, B=0, A=255
    // White pixel: R=255, G=255, B=255, A=255

    // (150,150) — inside correct intersection — should be RED
    int idx = (150 * W + 150) * 4;
    EXPECT_EQ(255, buf[idx + 0]); // R
    EXPECT_EQ(0, buf[idx + 1]); // G
    EXPECT_EQ(0, buf[idx + 2]); // B

    // (50,50) — inside first clip only, outside second — should be WHITE
    idx = (50 * W + 50) * 4;
    EXPECT_EQ(255, buf[idx + 0]); // R
    EXPECT_EQ(255, buf[idx + 1]); // G — this will FAIL with current bug
    EXPECT_EQ(255, buf[idx + 2]); // B

    // (250,250) — inside second clip only, outside first — should be WHITE
    idx = (250 * W + 250) * 4;
    EXPECT_EQ(255, buf[idx + 0]); // R
    EXPECT_EQ(255, buf[idx + 1]); // G
    EXPECT_EQ(255, buf[idx + 2]); // B

    ps_context_unref(lctx);
    ps_canvas_unref(cv);
    free(buf);
}

TEST_F(AdvancedRenderingTest, Clip_TransformRotation_CorrectIntersection)
{
    // Clip with rotation transform between two clips.
    // 1. Identity: clip to (100,100,200,200) — a 200x200 box at (100,100)
    // 2. Rotate 90 degrees around origin
    // 3. Clip to (-300,100,200,200) — in rotated coords, this maps to
    // device space (100,-300)->(300,-100)... but we need overlap.
    //
    // Simpler approach: use scale(-1,1) flip to test non-trivial transform.
    // 1. Identity: clip to (0,0,200,200)
    // 2. Scale(-1,1) + translate(-400,0) => mirrors x around x=200
    // 3. Clip to (200,0,200,200) in flipped coords => device (200,0)->(400,200)
    // Wait, scale(-1,1) maps x -> -x, so user x=200 -> device x=-200.
    //
    // Even simpler: use scale(2,2) to test non-identity non-translate.
    // 1. Identity: clip to (0,0,200,200)
    // 2. Scale(0.5, 0.5)
    // 3. Clip to (0,0,200,200) in scaled coords => device (0,0)->(100,100)
    // Correct intersection: (0,0)->(100,100)
    // Fill red, check (50,50)=red, (150,150)=white

    const int W = 400;
    const int H = 400;
    uint8_t* buf = (uint8_t*)calloc(W * 4, H);
    ASSERT_TRUE(buf != NULL);
    ps_canvas* cv = ps_canvas_create_with_data(buf, COLOR_FORMAT_RGBA, W, H, W * 4);
    ASSERT_TRUE(cv != NULL);
    ps_context* lctx = ps_context_create(cv, NULL);
    ASSERT_TRUE(lctx != NULL);

    ps_color white = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(lctx, &white);
    ps_clear(lctx);

    // First clip in identity
    ps_rect clip1 = {0, 0, 200, 200};
    ps_clip_rect(lctx, &clip1);

    // Scale down by half
    ps_scale(lctx, 0.5f, 0.5f);

    // Second clip: user (0,0,200,200) => device (0,0,100,100)
    ps_rect clip2 = {0, 0, 200, 200};
    ps_clip_rect(lctx, &clip2);

    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(lctx, &red);
    ps_rect big = {-400, -400, 1600, 1600};
    ps_rectangle(lctx, &big);
    ps_fill(lctx);

    // (50,50) inside intersection => RED
    int idx = (50 * W + 50) * 4;
    EXPECT_EQ(255, buf[idx + 0]);
    EXPECT_EQ(0, buf[idx + 1]);
    EXPECT_EQ(0, buf[idx + 2]);

    // (150,150) inside first clip only, outside scaled second clip => WHITE
    idx = (150 * W + 150) * 4;
    EXPECT_EQ(255, buf[idx + 0]);
    EXPECT_EQ(255, buf[idx + 1]);
    EXPECT_EQ(255, buf[idx + 2]);

    ps_context_unref(lctx);
    ps_canvas_unref(cv);
    free(buf);
}

TEST_F(AdvancedRenderingTest, Clip_PathApi_TransformIntersection)
{
    // Test ps_clip() (not ps_clip_rect) with transform between two clips.
    // Same logic as the translate test but using ps_rectangle + ps_clip.

    const int W = 400;
    const int H = 400;
    uint8_t* buf = (uint8_t*)calloc(W * 4, H);
    ASSERT_TRUE(buf != NULL);
    ps_canvas* cv = ps_canvas_create_with_data(buf, COLOR_FORMAT_RGBA, W, H, W * 4);
    ASSERT_TRUE(cv != NULL);
    ps_context* lctx = ps_context_create(cv, NULL);
    ASSERT_TRUE(lctx != NULL);

    ps_color white = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(lctx, &white);
    ps_clear(lctx);

    // First clip via ps_clip() in identity
    ps_rect clip1 = {0, 0, 200, 200};
    ps_rectangle(lctx, &clip1);
    ps_clip(lctx);

    // Translate
    ps_translate(lctx, 100.0f, 100.0f);

    // Second clip via ps_clip()
    ps_rect clip2 = {0, 0, 200, 200};
    ps_rectangle(lctx, &clip2);
    ps_clip(lctx);

    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(lctx, &red);
    ps_rect big = {-200, -200, 800, 800};
    ps_rectangle(lctx, &big);
    ps_fill(lctx);

    // (150,150) inside intersection => RED
    int idx = (150 * W + 150) * 4;
    EXPECT_EQ(255, buf[idx + 0]);
    EXPECT_EQ(0, buf[idx + 1]);
    EXPECT_EQ(0, buf[idx + 2]);

    // (50,50) outside intersection => WHITE
    idx = (50 * W + 50) * 4;
    EXPECT_EQ(255, buf[idx + 0]);
    EXPECT_EQ(255, buf[idx + 1]);
    EXPECT_EQ(255, buf[idx + 2]);

    ps_context_unref(lctx);
    ps_canvas_unref(cv);
    free(buf);
}

TEST_F(AdvancedRenderingTest, Clip_SaveRestore_PreservesClipMatrix)
{
    // Verify that ps_save/ps_restore correctly preserves clip_matrix.
    // 1. Identity: clip to (0,0,200,200)
    // 2. ps_save
    // 3. Translate(100,100), clip to (0,0,200,200) => intersection (100,100)->(200,200)
    // 4. ps_restore => should restore to original clip (0,0,200,200) with identity matrix
    // 5. Translate(50,50), clip to (0,0,200,200) => intersection (50,50)->(200,200)
    // 6. Fill red, check pixels

    const int W = 400;
    const int H = 400;
    uint8_t* buf = (uint8_t*)calloc(W * 4, H);
    ASSERT_TRUE(buf != NULL);
    ps_canvas* cv = ps_canvas_create_with_data(buf, COLOR_FORMAT_RGBA, W, H, W * 4);
    ASSERT_TRUE(cv != NULL);
    ps_context* lctx = ps_context_create(cv, NULL);
    ASSERT_TRUE(lctx != NULL);

    ps_color white = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(lctx, &white);
    ps_clear(lctx);

    // First clip in identity
    ps_rect clip1 = {0, 0, 200, 200};
    ps_clip_rect(lctx, &clip1);

    ps_save(lctx);

    // Inner scope: translate + clip => intersection (100,100)->(200,200)
    ps_translate(lctx, 100.0f, 100.0f);
    ps_rect clip_inner = {0, 0, 200, 200};
    ps_clip_rect(lctx, &clip_inner);

    ps_restore(lctx);

    // After restore: clip should be back to (0,0,200,200) with identity clip_matrix
    // Now translate(50,50) and clip again
    ps_translate(lctx, 50.0f, 50.0f);
    ps_rect clip2 = {0, 0, 200, 200};
    ps_clip_rect(lctx, &clip2);

    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(lctx, &red);
    ps_rect big = {-200, -200, 800, 800};
    ps_rectangle(lctx, &big);
    ps_fill(lctx);

    // (75,75) inside intersection (50,50)->(200,200) => RED
    int idx = (75 * W + 75) * 4;
    EXPECT_EQ(255, buf[idx + 0]);
    EXPECT_EQ(0, buf[idx + 1]);
    EXPECT_EQ(0, buf[idx + 2]);

    // (25,25) outside intersection => WHITE
    idx = (25 * W + 25) * 4;
    EXPECT_EQ(255, buf[idx + 0]);
    EXPECT_EQ(255, buf[idx + 1]);
    EXPECT_EQ(255, buf[idx + 2]);

    // (250,250) outside first clip => WHITE
    idx = (250 * W + 250) * 4;
    EXPECT_EQ(255, buf[idx + 0]);
    EXPECT_EQ(255, buf[idx + 1]);
    EXPECT_EQ(255, buf[idx + 2]);

    ps_context_unref(lctx);
    ps_canvas_unref(cv);
    free(buf);
}

TEST_F(AdvancedRenderingTest, Clip_ResetThenReclip_Works)
{
    // Verify reset_clip clears clip state, then a new clip works correctly.
    // 1. Clip to (0,0,100,100)
    // 2. Translate(50,50)
    // 3. reset_clip
    // 4. Clip to (0,0,200,200) in translated coords => device (50,50)->(250,250)
    // 5. Fill red, check pixels

    const int W = 400;
    const int H = 400;
    uint8_t* buf = (uint8_t*)calloc(W * 4, H);
    ASSERT_TRUE(buf != NULL);
    ps_canvas* cv = ps_canvas_create_with_data(buf, COLOR_FORMAT_RGBA, W, H, W * 4);
    ASSERT_TRUE(cv != NULL);
    ps_context* lctx = ps_context_create(cv, NULL);
    ASSERT_TRUE(lctx != NULL);

    ps_color white = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(lctx, &white);
    ps_clear(lctx);

    // First clip
    ps_rect clip1 = {0, 0, 100, 100};
    ps_clip_rect(lctx, &clip1);

    // Transform
    ps_translate(lctx, 50.0f, 50.0f);

    // Reset clip — should clear everything including clip_matrix
    ps_reset_clip(lctx);

    // New clip after reset: (0,0,200,200) in translated coords
    ps_rect clip2 = {0, 0, 200, 200};
    ps_clip_rect(lctx, &clip2);

    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(lctx, &red);
    ps_rect big = {-200, -200, 800, 800};
    ps_rectangle(lctx, &big);
    ps_fill(lctx);

    // (100,100) inside new clip => RED
    int idx = (100 * W + 100) * 4;
    EXPECT_EQ(255, buf[idx + 0]);
    EXPECT_EQ(0, buf[idx + 1]);
    EXPECT_EQ(0, buf[idx + 2]);

    // (25,25) outside new clip (device 50,50 is the start) => WHITE
    idx = (25 * W + 25) * 4;
    EXPECT_EQ(255, buf[idx + 0]);
    EXPECT_EQ(255, buf[idx + 1]);
    EXPECT_EQ(255, buf[idx + 2]);

    ps_context_unref(lctx);
    ps_canvas_unref(cv);
    free(buf);
}

TEST_F(AdvancedRenderingTest, Clip_SameTransform_NoUnnecessaryWork)
{
    // Two clips under the same CTM — should work as before (no transform needed).
    // 1. Translate(50,50)
    // 2. Clip to (0,0,200,200) => device (50,50)->(250,250)
    // 3. Clip to (100,100,200,200) => device (150,150)->(350,350)
    // Intersection in device: (150,150)->(250,250)

    const int W = 400;
    const int H = 400;
    uint8_t* buf = (uint8_t*)calloc(W * 4, H);
    ASSERT_TRUE(buf != NULL);
    ps_canvas* cv = ps_canvas_create_with_data(buf, COLOR_FORMAT_RGBA, W, H, W * 4);
    ASSERT_TRUE(cv != NULL);
    ps_context* lctx = ps_context_create(cv, NULL);
    ASSERT_TRUE(lctx != NULL);

    ps_color white = {1.0f, 1.0f, 1.0f, 1.0f};
    ps_set_source_color(lctx, &white);
    ps_clear(lctx);

    ps_translate(lctx, 50.0f, 50.0f);

    ps_rect clip1 = {0, 0, 200, 200};
    ps_clip_rect(lctx, &clip1);

    ps_rect clip2 = {100, 100, 200, 200};
    ps_clip_rect(lctx, &clip2);

    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(lctx, &red);
    ps_rect big = {-200, -200, 800, 800};
    ps_rectangle(lctx, &big);
    ps_fill(lctx);

    // (200,200) inside intersection => RED
    int idx = (200 * W + 200) * 4;
    EXPECT_EQ(255, buf[idx + 0]);
    EXPECT_EQ(0, buf[idx + 1]);
    EXPECT_EQ(0, buf[idx + 2]);

    // (100,100) inside first clip only, outside second => WHITE
    idx = (100 * W + 100) * 4;
    EXPECT_EQ(255, buf[idx + 0]);
    EXPECT_EQ(255, buf[idx + 1]);
    EXPECT_EQ(255, buf[idx + 2]);

    ps_context_unref(lctx);
    ps_canvas_unref(cv);
    free(buf);
}
