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
#include <vector>

class MaskLayerTest : public ::testing::Test
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

TEST_F(MaskLayerTest, BasicMaskCreation)
{
    ps_mask* mask = ps_mask_create(100, 100);
    ASSERT_TRUE(mask);
    ps_mask_unref(mask);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MaskLayerTest, MaskWithData)
{
    std::vector<ps_byte> data(100 * 100, 128);
    ps_mask* mask = ps_mask_create_with_data(data.data(), 100, 100);
    ASSERT_TRUE(mask);
    ps_mask_unref(mask);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MaskLayerTest, MaskWithGradient)
{
    std::vector<ps_byte> data(200 * 200);
    for (int y = 0; y < 200; y++) {
        for (int x = 0; x < 200; x++) {
            data[y * 200 + x] = (ps_byte)((x + y) * 255 / 400);
        }
    }

    ps_mask* mask = ps_mask_create_with_data(data.data(), 200, 200);
    ASSERT_TRUE(mask);

    ps_canvas* canvas = ps_context_get_canvas(ctx);
    ps_canvas_set_mask(canvas, mask);

    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_rect rc = {0, 0, 400, 400};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_canvas_reset_mask(canvas);
    ps_mask_unref(mask);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MaskLayerTest, MaskWithCircle)
{
    std::vector<ps_byte> data(200 * 200, 0);
    int cx = 100, cy = 100, radius = 80;

    for (int y = 0; y < 200; y++) {
        for (int x = 0; x < 200; x++) {
            int dx = x - cx;
            int dy = y - cy;
            int dist = (int)sqrt(dx * dx + dy * dy);
            if (dist < radius) {
                data[y * 200 + x] = 255;
            }
        }
    }

    ps_mask* mask = ps_mask_create_with_data(data.data(), 200, 200);
    ASSERT_TRUE(mask);

    ps_canvas* canvas = ps_context_get_canvas(ctx);
    ps_canvas_set_mask(canvas, mask);

    ps_color color = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_rect rc = {0, 0, 400, 400};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_canvas_reset_mask(canvas);
    ps_mask_unref(mask);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MaskLayerTest, MaskColorFilter)
{
    ps_mask* mask = ps_mask_create(100, 100);
    ASSERT_TRUE(mask);

    ps_color filter1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_mask_add_color_filter(mask, &filter1);

    ps_color filter2 = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_mask_add_color_filter(mask, &filter2);

    ps_mask_clear_color_filters(mask);

    ps_mask_unref(mask);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MaskLayerTest, MaskWithAlphaChannel)
{
    std::vector<ps_byte> data(100 * 100);
    for (int i = 0; i < 100 * 100; i++) {
        data[i] = (ps_byte)(i % 256);
    }

    ps_mask* mask = ps_mask_create_with_data(data.data(), 100, 100);
    ASSERT_TRUE(mask);

    ps_canvas* canvas = ps_context_get_canvas(ctx);
    ps_canvas_set_mask(canvas, mask);

    ps_color color = {0.5f, 0.5f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_rect rc = {50, 50, 300, 300};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    ps_canvas_reset_mask(canvas);
    ps_mask_unref(mask);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MaskLayerTest, MaskRefCounting)
{
    ps_mask* mask = ps_mask_create(50, 50);
    ASSERT_TRUE(mask);

    ps_mask* mask_ref = ps_mask_ref(mask);
    ASSERT_TRUE(mask_ref);
    ASSERT_EQ(mask, mask_ref);

    ps_mask_unref(mask_ref);
    ps_mask_unref(mask);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MaskLayerTest, MaskWithPattern)
{
    std::vector<ps_byte> data(50 * 50);
    for (int y = 0; y < 50; y++) {
        for (int x = 0; x < 50; x++) {
            data[y * 50 + x] = ((x / 10 + y / 10) % 2) ? 255 : 0;
        }
    }

    ps_mask* mask = ps_mask_create_with_data(data.data(), 50, 50);
    ASSERT_TRUE(mask);

    ps_canvas* canvas = ps_context_get_canvas(ctx);
    ps_canvas_set_mask(canvas, mask);

    ps_point start = {0, 0};
    ps_point end = {400, 400};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_TRUE(gradient);

    ps_color c1 = {1.0f, 1.0f, 0.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &c1);

    ps_color c2 = {1.0f, 0.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 1.0f, &c2);

    ps_set_source_gradient(ctx, gradient);
    ps_rect rc = {0, 0, 400, 400};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_gradient_unref(gradient);
    ps_canvas_reset_mask(canvas);
    ps_mask_unref(mask);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MaskLayerTest, MultipleMaskOperations)
{
    ps_mask* mask1 = ps_mask_create(100, 100);
    ASSERT_TRUE(mask1);

    ps_canvas* canvas = ps_context_get_canvas(ctx);
    ps_canvas_set_mask(canvas, mask1);

    ps_color color1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color1);
    ps_rect rc1 = {50, 50, 150, 150};
    ps_rectangle(ctx, &rc1);
    ps_fill(ctx);

    ps_canvas_reset_mask(canvas);

    ps_mask* mask2 = ps_mask_create(100, 100);
    ASSERT_TRUE(mask2);

    ps_canvas_set_mask(canvas, mask2);

    ps_color color2 = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &color2);
    ps_rect rc2 = {200, 200, 150, 150};
    ps_ellipse(ctx, &rc2);
    ps_fill(ctx);

    ps_canvas_reset_mask(canvas);

    ps_mask_unref(mask1);
    ps_mask_unref(mask2);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MaskLayerTest, MaskWithComposite)
{
    std::vector<ps_byte> data(150 * 150, 200);
    ps_mask* mask = ps_mask_create_with_data(data.data(), 150, 150);
    ASSERT_TRUE(mask);

    ps_canvas* canvas = ps_context_get_canvas(ctx);
    ps_canvas_set_mask(canvas, mask);

    ps_set_composite_operator(ctx, COMPOSITE_SCREEN);

    ps_color color = {0.8f, 0.2f, 0.6f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_rect rc = {100, 100, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_canvas_reset_mask(canvas);
    ps_mask_unref(mask);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}
