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

TEST_F(SVGPlayerTest, AnimateRectX_FromTo)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"10\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"x\" from=\"10\" to=\"20\" dur=\"2s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), NULL, &r);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    // find target node
    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // t=0
    psx_svg_player_play(p);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 10.0f, 0.001f);
    }

    // t=1s in a 2s animation: x should be 15
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 15.0f, 0.001f);
    }

    // t=2s end instant: freeze should hold end value
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 20.0f, 0.001f);
    }

    // after end instant: freeze should still hold end value
    psx_svg_player_seek(p, 2.5f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 20.0f, 0.001f);
    }

    psx_svg_player_destroy(p);
}

TEST_F(SVGPlayerTest, SetRectX_To)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"10\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <set attributeName=\"x\" to=\"30\" begin=\"1s\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), NULL, &r);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // before begin
    psx_svg_player_seek(p, 0.5f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    // active interval
    psx_svg_player_seek(p, 1.5f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 30.0f, 0.001f);
    }

    // end instant for fill=remove: not active
    psx_svg_player_seek(p, 2.0f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    // after end with fill=remove
    psx_svg_player_seek(p, 2.5f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    psx_svg_player_destroy(p);
}

TEST_F(SVGPlayerTest, SetRectWidth_To)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"10\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <set attributeName=\"width\" to=\"30\" begin=\"1s\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), NULL, &r);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // before begin
    psx_svg_player_seek(p, 0.5f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_WIDTH, &v));
    }

    // active interval
    psx_svg_player_seek(p, 1.5f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_WIDTH, &v));
        EXPECT_NEAR(v, 30.0f, 0.001f);
    }

    // after end with fill=remove
    psx_svg_player_seek(p, 2.5f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_WIDTH, &v));
    }

    psx_svg_player_destroy(p);
}

TEST_F(SVGPlayerTest, AnimateRectOpacity_FromTo)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"opacity\" from=\"0\" to=\"1\" dur=\"2s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), NULL, &r);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // t=1s in a 2s animation: opacity should be 0.5
    psx_svg_player_seek(p, 1.0f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_OPACITY, &v));
        EXPECT_NEAR(v, 0.5f, 0.001f);
    }

    psx_svg_player_destroy(p);
}

TEST_F(SVGPlayerTest, SetRectRx_To)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" rx=\"1\" fill=\"#000\">"
        "    <set attributeName=\"rx\" to=\"5\" begin=\"1s\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), NULL, &r);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_seek(p, 0.5f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_RX, &v));
    }

    psx_svg_player_seek(p, 1.5f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_RX, &v));
        EXPECT_NEAR(v, 5.0f, 0.001f);
    }

    psx_svg_player_seek(p, 2.5f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_RX, &v));
    }

    psx_svg_player_destroy(p);
}

TEST_F(SVGPlayerTest, AnimateRectStrokeWidth_FromTo)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"none\" stroke=\"#000\" stroke-width=\"1\">"
        "    <animate attributeName=\"stroke-width\" from=\"1\" to=\"3\" dur=\"2s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), NULL, &r);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_seek(p, 1.0f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_STROKE_WIDTH, &v));
        EXPECT_NEAR(v, 2.0f, 0.001f);
    }

    psx_svg_player_destroy(p);
}

TEST_F(SVGPlayerTest, AnimateRepeatCountFractional_Ceil)
{
    // repeatCount allows floating; we currently ceil to avoid truncation.
    // repeatCount="1.2" => 2 repeats => total 2 * dur.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"x\" from=\"0\" to=\"10\" dur=\"1s\" repeatCount=\"1.2\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), NULL, &r);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    // duration should be 2s (2 repeats)
    EXPECT_NEAR(2.0f, psx_svg_player_get_duration(p), 0.0001f);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // during 2nd repeat (t=1.5s): active
    psx_svg_player_seek(p, 1.5f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 5.0f, 0.001f);
    }

    // after total (t=2.1s) with fill=remove: not active
    psx_svg_player_seek(p, 2.1f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    psx_svg_player_destroy(p);
}

TEST_F(SVGPlayerTest, BeginList_EarliestBeginUsed)
{
    // Minimal support: if begin has multiple offset times, we select the earliest.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <set attributeName=\"x\" to=\"20\" begin=\"1s;0s\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), NULL, &r);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // with earliest begin=0s, should be active at 0.5s
    psx_svg_player_seek(p, 0.5f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 20.0f, 0.001f);
    }

    // after 1s duration (t=1.1s), fill=remove: not active
    psx_svg_player_seek(p, 1.1f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    psx_svg_player_destroy(p);
}

TEST_F(SVGPlayerTest, AnimateRectFillOpacity_FromTo)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\" fill-opacity=\"0\">"
        "    <animate attributeName=\"fill-opacity\" from=\"0\" to=\"1\" dur=\"2s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), NULL, &r);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_seek(p, 1.0f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_FILL_OPACITY, &v));
        EXPECT_NEAR(v, 0.5f, 0.001f);
    }

    psx_svg_player_destroy(p);
}

TEST_F(SVGPlayerTest, AnimateGradientStopOpacity_FromTo)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <defs>"
        "    <linearGradient id=\"g\">"
        "      <stop id=\"s\" offset=\"0\" stop-color=\"#000\" stop-opacity=\"0\">"
        "        <animate attributeName=\"stop-opacity\" from=\"0\" to=\"1\" dur=\"2s\" fill=\"freeze\"/>"
        "      </stop>"
        "    </linearGradient>"
        "  </defs>"
        "  <rect x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"url(#g)\"/>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_player* p = psx_svg_player_create_from_data(svg, (uint32_t)strlen(svg), NULL, &r);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* stop = psx_svg_player_get_node_by_id(p, "s");
    ASSERT_TRUE(stop != NULL);

    psx_svg_player_seek(p, 1.0f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, stop, SVG_ATTR_GRADIENT_STOP_OPACITY, &v));
        EXPECT_NEAR(v, 0.5f, 0.001f);
    }

    psx_svg_player_destroy(p);
}
