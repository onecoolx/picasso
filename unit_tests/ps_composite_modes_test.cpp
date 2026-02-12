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

class CompositeModeTest : public ::testing::TestWithParam<ps_composite>
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

    void DrawTestPattern()
    {
        ps_color red = {1.0f, 0.0f, 0.0f, 0.8f};
        ps_set_source_color(ctx, &red);
        ps_rect rc1 = {100, 100, 150, 150};
        ps_ellipse(ctx, &rc1);
        ps_fill(ctx);

        ps_color blue = {0.0f, 0.0f, 1.0f, 0.8f};
        ps_set_source_color(ctx, &blue);
        ps_rect rc2 = {150, 150, 150, 150};
        ps_rectangle(ctx, &rc2);
        ps_fill(ctx);
    }

    ps_context* ctx;
};

TEST_P(CompositeModeTest, CompositeMode)
{
    ps_composite mode = GetParam();

    ps_composite old_mode = ps_set_composite_operator(ctx, mode);
    EXPECT_NE(COMPOSITE_ERROR, old_mode);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    DrawTestPattern();
}

INSTANTIATE_TEST_SUITE_P(
    AllCompositeModes,
    CompositeModeTest,
    ::testing::Values(
        COMPOSITE_CLEAR,
        COMPOSITE_SRC,
        COMPOSITE_DST,
        COMPOSITE_SRC_OVER,
        COMPOSITE_DST_OVER,
        COMPOSITE_SRC_IN,
        COMPOSITE_DST_IN,
        COMPOSITE_SRC_OUT,
        COMPOSITE_DST_OUT,
        COMPOSITE_SRC_ATOP,
        COMPOSITE_DST_ATOP,
        COMPOSITE_XOR,
        COMPOSITE_DARKEN,
        COMPOSITE_LIGHTEN
    )
);

TEST_F(CompositeModeTest, InvalidCompositeMode)
{
    ps_composite result = ps_set_composite_operator(ctx, static_cast<ps_composite>(999));
    EXPECT_EQ(COMPOSITE_ERROR, result);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(CompositeModeTest, NullContext)
{
    ps_composite result = ps_set_composite_operator(NULL, COMPOSITE_SRC_OVER);
    EXPECT_EQ(COMPOSITE_ERROR, result);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(CompositeModeTest, StateSaveRestore)
{
    ps_set_composite_operator(ctx, COMPOSITE_SRC_OVER);
    ps_save(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_XOR);
    DrawTestPattern();

    ps_restore(ctx);
    DrawTestPattern();
}
