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

class ContextTest : public ::testing::Test
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
        canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 200);
        ctx = ps_context_create(canvas, NULL);
    }

    void TearDown() override
    {
        ps_context_unref(ctx);
        ps_canvas_unref(canvas);
    }

    ps_canvas* canvas;
    ps_context* ctx;
};

TEST_F(ContextTest, CreateAndRef)
{
    ps_context* ctx2 = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx2);
    ps_context_unref(ctx2);

    ps_context* ctx3 = ps_context_create(canvas, ctx);
    ASSERT_TRUE(ctx3);
    ps_context_unref(ctx3);

    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(ContextTest, CanvasAssociation)
{
    ps_canvas* newCanvas = ps_canvas_create(COLOR_FORMAT_ARGB, 400, 300);
    ps_context_set_canvas(ctx, newCanvas);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_canvas* canvas = ps_context_get_canvas(ctx);
    ASSERT_EQ(canvas, newCanvas);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_canvas_unref(newCanvas);

    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(ContextTest, AntialiasAndGamma)
{
    // gamma
    float g = 1.5;
    float old = ps_set_gamma(ctx, g);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    old = ps_set_gamma(ctx, old);
    EXPECT_EQ(g, old);

    ps_set_antialias(ctx, True);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    ps_set_antialias(ctx, False);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}
