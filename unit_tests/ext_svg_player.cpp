/*
    Copyright (c) 2025, Zhang Ji Peng
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

#include "psx_svg_player.h"

class SVGPlayerTest : public ::testing::Test
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

TEST_F(SVGPlayerTest, CreateFromData)
{
    const char* svg = "<svg width=100 height=100>"
                      "<rect x=10 y=10 width=80 height=80 fill=\"red\"/>"
                      "</svg>";

    psx_result r = S_OK;
    psx_svg_player_options opt;
    opt.take_ownership_of_root = True;
    opt.loop = False;
    opt.dpi = 96;

    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), &opt, &r);
    EXPECT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    EXPECT_EQ(PSX_SVG_PLAYER_STOPPED, psx_svg_player_get_state(p));
    EXPECT_FLOAT_EQ(0.0f, psx_svg_player_get_time(p));

    psx_svg_player_destroy(p);
}

TEST_F(SVGPlayerTest, BasicPlayPauseSeekTick)
{
    const char* svg = "<svg width=100 height=100>"
                      "<rect x=10 y=10 width=80 height=80 fill=\"red\"/>"
                      "</svg>";

    psx_result r = S_OK;
    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), NULL, &r);
    ASSERT_NE((psx_svg_player*)NULL, p);

    psx_svg_player_play(p);
    EXPECT_EQ(PSX_SVG_PLAYER_PLAYING, psx_svg_player_get_state(p));

    psx_svg_player_tick(p, 0.5f);
    EXPECT_NEAR(0.5f, psx_svg_player_get_time(p), 0.0001f);

    psx_svg_player_pause(p);
    EXPECT_EQ(PSX_SVG_PLAYER_PAUSED, psx_svg_player_get_state(p));

    psx_svg_player_tick(p, 1.0f);
    EXPECT_NEAR(0.5f, psx_svg_player_get_time(p), 0.0001f);

    psx_svg_player_seek(p, 2.0f);
    EXPECT_NEAR(2.0f, psx_svg_player_get_time(p), 0.0001f);

    psx_svg_player_stop(p);
    EXPECT_EQ(PSX_SVG_PLAYER_STOPPED, psx_svg_player_get_state(p));
    EXPECT_FLOAT_EQ(0.0f, psx_svg_player_get_time(p));

    // duration hint (no animation => unknown)
    EXPECT_NEAR(-1.0f, psx_svg_player_get_duration(p), 0.0001f);

    psx_svg_player_destroy(p);
}

TEST_F(SVGPlayerTest, DurationFromAnimate)
{
    const char* svg = "<svg width=100 height=100>"
                      "<rect x=10 y=10 width=80 height=80 fill=\"red\">"
                      "<animate attributeName=\"x\" from=\"10\" to=\"20\" dur=\"2s\"/>"
                      "</rect>"
                      "</svg>";

    psx_result r = S_OK;
    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), NULL, &r);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    EXPECT_NEAR(2.0f, psx_svg_player_get_duration(p), 0.0001f);

    psx_svg_player_destroy(p);
}
