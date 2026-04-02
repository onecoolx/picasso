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

class CompositePixelTest : public ::testing::Test
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

TEST_F(CompositePixelTest, OverlayMode)
{
    ps_color bg = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_OVERLAY);

    ps_color fg = {1.0f, 0.0f, 0.0f, 0.8f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {50, 50, 300, 300};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, ScreenMode)
{
    ps_color bg = {0.3f, 0.3f, 0.3f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_SCREEN);

    ps_color fg = {0.8f, 0.2f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, MultiplyMode)
{
    ps_color bg = {0.8f, 0.8f, 0.8f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_MULTIPLY);

    ps_color fg = {0.5f, 0.5f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {50, 50, 300, 300};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, DarkenMode)
{
    ps_color bg = {0.7f, 0.7f, 0.7f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_DARKEN);

    ps_color fg = {0.3f, 0.8f, 0.3f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {75, 75, 250, 250};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, LightenMode)
{
    ps_color bg = {0.3f, 0.3f, 0.3f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_LIGHTEN);

    ps_color fg = {0.6f, 0.2f, 0.8f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, PlusMode)
{
    ps_color bg = {0.4f, 0.4f, 0.4f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_PLUS);

    ps_color fg = {0.5f, 0.3f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {50, 50, 300, 300};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, MinusMode)
{
    ps_color bg = {0.8f, 0.8f, 0.8f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_MINUS);

    ps_color fg = {0.3f, 0.3f, 0.3f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, ExclusionMode)
{
    ps_color bg = {0.6f, 0.4f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_EXCLUSION);

    ps_color fg = {0.2f, 0.6f, 0.8f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {75, 75, 250, 250};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, DifferenceMode)
{
    ps_color bg = {0.7f, 0.3f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_DIFFERENCE);

    ps_color fg = {0.3f, 0.7f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {50, 50, 300, 300};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, SoftlightMode)
{
    ps_color bg = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_SOFTLIGHT);

    ps_color fg = {0.8f, 0.2f, 0.6f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, HardlightMode)
{
    ps_color bg = {0.4f, 0.6f, 0.8f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_HARDLIGHT);

    ps_color fg = {0.9f, 0.3f, 0.1f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {75, 75, 250, 250};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, BurnMode)
{
    ps_color bg = {0.7f, 0.7f, 0.7f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_BURN);

    ps_color fg = {0.3f, 0.5f, 0.7f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {50, 50, 300, 300};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, DodgeMode)
{
    ps_color bg = {0.3f, 0.3f, 0.3f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_DODGE);

    ps_color fg = {0.6f, 0.4f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, ContrastMode)
{
    ps_color bg = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_CONTRAST);

    ps_color fg = {0.8f, 0.8f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {75, 75, 250, 250};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, InvertMode)
{
    ps_color bg = {0.6f, 0.4f, 0.8f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_INVERT);

    ps_color fg = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {50, 50, 300, 300};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, InvertBlendMode)
{
    ps_color bg = {0.7f, 0.3f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_INVERT_BLEND);

    ps_color fg = {0.4f, 0.6f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, HueMode)
{
    ps_color bg = {0.8f, 0.2f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_HUE);

    ps_color fg = {0.2f, 0.8f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {75, 75, 250, 250};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, SaturationMode)
{
    ps_color bg = {0.6f, 0.6f, 0.6f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_SATURATION);

    ps_color fg = {1.0f, 0.0f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {50, 50, 300, 300};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, ColorMode)
{
    ps_color bg = {0.5f, 0.5f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_COLOR);

    ps_color fg = {0.2f, 0.6f, 0.9f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {100, 100, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CompositePixelTest, LuminosityMode)
{
    ps_color bg = {0.3f, 0.7f, 0.5f, 1.0f};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    ps_set_composite_operator(ctx, COMPOSITE_LUMINOSITY);

    ps_color fg = {0.8f, 0.4f, 0.2f, 1.0f};
    ps_set_source_color(ctx, &fg);
    ps_rect rc = {75, 75, 250, 250};
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}
