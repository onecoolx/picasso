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

#define SNAPSHOT_PATH "svg"
#include "test.h"

#include "svg/psx_svg.h"

class SvgAPITest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        PS_Init();
        psx_result ret = psx_svg_init();
        ASSERT_EQ(S_OK, ret);
    }

    static void TearDownTestSuite()
    {
        PS_Shutdown();
        psx_svg_shutdown();
    }

    void SetUp() override
    {
        clear_test_canvas();
    }
};

TEST_F(SvgAPITest, SvgDrawTiger)
{
    psx_result ret;
    psx_svg* svg = psx_svg_load("tiger.svg", &ret);
    EXPECT_EQ(S_OK, ret);
    ASSERT_NE(svg, nullptr);

    psx_svg_render* render = psx_svg_render_create(svg, &ret);
    EXPECT_EQ(S_OK, ret);
    ASSERT_NE(render, nullptr);

    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ASSERT_NE(ctx, nullptr);

    ps_identity(ctx);
    ps_scale(ctx, 0.3f, 0.3f);

    ret = psx_svg_render_draw(ctx, render);
    EXPECT_EQ(S_OK, ret);

    ps_translate(ctx, 600, 100);
    ps_scale(ctx, 0.5f, 0.5f);

    ret = psx_svg_render_draw(ctx, render);
    EXPECT_EQ(S_OK, ret);

    ps_translate(ctx, 600, 100);
    ps_scale(ctx, 0.5f, 0.5f);

    ret = psx_svg_render_draw(ctx, render);
    EXPECT_EQ(S_OK, ret);

    EXPECT_SNAPSHOT_EQ(svg_draw_tiger);

    ps_context_unref(ctx);
    psx_svg_render_destroy(render);
    psx_svg_destroy(svg);
}
