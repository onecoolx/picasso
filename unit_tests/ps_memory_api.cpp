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

#include "test.h"

class MemoryTest : public ::testing::Test
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

TEST_F(MemoryTest, CanvasRefCounting)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(canvas);

    ps_canvas_ref(canvas);
    ps_canvas_unref(canvas);
    ps_canvas_unref(canvas);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MemoryTest, ContextRefCounting)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_TRUE(canvas);

    ps_context* ctx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx);

    ps_context_ref(ctx);
    ps_context_unref(ctx);
    ps_context_unref(ctx);

    ps_canvas_unref(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MemoryTest, PathRefCounting)
{
    ps_path* path = ps_path_create();
    ASSERT_TRUE(path);

    ps_path_ref(path);
    ps_path_unref(path);
    ps_path_unref(path);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MemoryTest, PatternRefCounting)
{
    ps_image* img = ps_image_create_with_data(NULL, COLOR_FORMAT_RGBA, 50, 50, 50 * 4);
    if (img) {
        ps_pattern* pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);
        if (pattern) {
            ps_pattern_ref(pattern);
            ps_pattern_unref(pattern);
            ps_pattern_unref(pattern);
            EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
        }
        ps_image_unref(img);
    }
}

TEST_F(MemoryTest, GradientRefCounting)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_TRUE(gradient);

    ps_gradient_ref(gradient);
    ps_gradient_unref(gradient);
    ps_gradient_unref(gradient);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MemoryTest, FontRefCounting)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    if (font) {
        ps_font_ref(font);
        ps_font_unref(font);
        ps_font_unref(font);
        EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
    }
}

TEST_F(MemoryTest, ImageRefCounting)
{
    ps_image* img = ps_image_create_with_data(NULL, COLOR_FORMAT_RGBA, 10, 10, 10 * 4);
    if (img) {
        ps_image_ref(img);
        ps_image_unref(img);
        ps_image_unref(img);
        EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
    }
}

TEST_F(MemoryTest, MultipleObjectCreation)
{
    const int count = 100;

    for (int i = 0; i < count; i++) {
        ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 50, 50);
        ASSERT_TRUE(canvas);

        ps_context* ctx = ps_context_create(canvas, NULL);
        ASSERT_TRUE(ctx);

        ps_path* path = ps_path_create();
        ASSERT_TRUE(path);

        ps_path_unref(path);
        ps_context_unref(ctx);
        ps_canvas_unref(canvas);
    }

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(MemoryTest, NullPointerHandling)
{
    ps_canvas_ref(NULL);
    ps_canvas_unref(NULL);
    ps_context_ref(NULL);
    ps_context_unref(NULL);
    ps_path_ref(NULL);
    ps_path_unref(NULL);
    ps_pattern_ref(NULL);
    ps_pattern_unref(NULL);
    ps_gradient_ref(NULL);
    ps_gradient_unref(NULL);
    ps_font_ref(NULL);
    ps_font_unref(NULL);
    ps_image_ref(NULL);
    ps_image_unref(NULL);
}

TEST_F(MemoryTest, LargeCanvasCreation)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 2048, 2048);
    if (canvas) {
        ps_size size;
        ps_bool result = ps_canvas_get_size(canvas, &size);
        if (result) {
            EXPECT_EQ(2048, size.w);
            EXPECT_EQ(2048, size.h);
        }
        ps_canvas_unref(canvas);
    }
}

TEST_F(MemoryTest, ZeroSizeCanvas)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 0, 0);
    if (canvas) {
        ps_canvas_unref(canvas);
    }
}
