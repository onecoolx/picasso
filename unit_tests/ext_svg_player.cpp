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
#include "psx_svg_anim_state.h"

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

    // Helper: parse SVG data and create a player.
    // Caller must destroy both player and root (via destroy_player).
    static psx_svg_player* create_player(const char* svg,
                                         psx_result* r,
                                         psx_svg_node** out_root)
    {
        psx_svg_node* root = psx_svg_load_data(svg, (uint32_t)strlen(svg));
        if (!root) {
            if (r) { *r = S_FAILURE; }
            if (out_root) { *out_root = NULL; }
            return NULL;
        }
        psx_svg_player* p = psx_svg_player_create(root, r);
        if (!p) {
            psx_svg_node_destroy(root);
            if (out_root) { *out_root = NULL; }
            return NULL;
        }
        if (out_root) { *out_root = root; }
        return p;
    }

    static void destroy_player(psx_svg_player* p, psx_svg_node* root)
    {
        if (p) { psx_svg_player_destroy(p); }
        if (root) { psx_svg_node_destroy(root); }
    }
};

static inline bool psx_svg_player_debug_get_float_override(const psx_svg_player* p,
                                                           const psx_svg_node* target,
                                                           psx_svg_attr_type attr,
                                                           float* out_v)
{
    return psx_svg_anim_get_float(&p->anim_state, target, attr, out_v);
}

static bool psx_svg_player_debug_get_transform_override(const psx_svg_player* p,
                                                        const psx_svg_node* target,
                                                        float* a, float* b, float* c, float* d, float* e, float* f)
{
    if (!p || !target) {
        if (a) { *a = 1.0f; }
        if (b) { *b = 0.0f; }
        if (c) { *c = 0.0f; }
        if (d) { *d = 1.0f; }
        if (e) { *e = 0.0f; }
        if (f) { *f = 0.0f; }
        return false;
    }

    const psx_svg_anim_transform_item* it = psx_svg_anim_state_find_transform(&p->anim_state, target);
    if (!it) {
        if (a) { *a = 1.0f; }
        if (b) { *b = 0.0f; }
        if (c) { *c = 0.0f; }
        if (d) { *d = 1.0f; }
        if (e) { *e = 0.0f; }
        if (f) { *f = 0.0f; }
        return false;
    }

    if (a) { *a = it->a; }
    if (b) { *b = it->b; }
    if (c) { *c = it->c; }
    if (d) { *d = it->d; }
    if (e) { *e = it->e; }
    if (f) { *f = it->f; }
    return true;
}

TEST_F(SVGPlayerTest, CreateFromData)
{
    const char* svg = "<svg width=100 height=100>"
                      "<rect x=10 y=10 width=80 height=80 fill=\"red\"/>"
                      "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    EXPECT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    EXPECT_EQ(PSX_SVG_PLAYER_STOPPED, psx_svg_player_get_state(p));
    EXPECT_FLOAT_EQ(0.0f, psx_svg_player_get_time(p));

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, BasicPlayPauseSeekTick)
{
    const char* svg = "<svg width=100 height=100>"
                      "<rect x=10 y=10 width=80 height=80 fill=\"red\"/>"
                      "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
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

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, DurationFromAnimate)
{
    const char* svg = "<svg width=100 height=100>"
                      "<rect x=10 y=10 width=80 height=80 fill=\"red\">"
                      "<animate attributeName=\"x\" from=\"10\" to=\"20\" dur=\"2s\"/>"
                      "</rect>"
                      "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    EXPECT_NEAR(2.0f, psx_svg_player_get_duration(p), 0.0001f);

    destroy_player(p, root);
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
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
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

    destroy_player(p, root);
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
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
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

    destroy_player(p, root);
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
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
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

    destroy_player(p, root);
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
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
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

    destroy_player(p, root);
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
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
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

    destroy_player(p, root);
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
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
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

    destroy_player(p, root);
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
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
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

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, BeginList_EarliestBeginUsed)
{
    // Minimal support: if begin has multiple offset times, we select the earliest.
    // Use <animate> here because parser supports begin lists for non-<set> elements.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"x\" from=\"0\" to=\"10\" begin=\"1s;0s\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // with earliest begin=0s, should be active at 0.5s
    psx_svg_player_seek(p, 0.5f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 5.0f, 0.01f);
    }

    // At t=1.1s, latest begin<=t is 1.0s, so animation is active again.
    // local = 0.1s => x ~ 1.0
    psx_svg_player_seek(p, 1.1f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 1.0f, 0.05f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, BeginList_MultiTrigger)
{
    // Minimal support: begin list should re-trigger; at t=1.1s we are in the 2nd activation.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"x\" from=\"0\" to=\"10\" begin=\"0s;1s\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // During first activation (t=0.5): active
    psx_svg_player_seek(p, 0.5f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 5.0f, 0.01f);
    }

    // At t=1.1, 2nd activation started at 1.0s, so local=0.1s => x ~ 1.0
    psx_svg_player_seek(p, 1.1f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 1.0f, 0.05f);
    }

    // After 2nd activation ends (t=2.1): inactive
    psx_svg_player_seek(p, 2.1f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, BeginList_MoreThanFourEntries)
{
    // Begin list is dynamic; ensure a late begin triggers correctly.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"x\" from=\"0\" to=\"10\" begin=\"0s;1s;2s;3s;4s\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // At t=4.1s, latest begin<=t is 4.0s => local=0.1s => x ~ 1.0
    psx_svg_player_seek(p, 4.1f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 1.0f, 0.05f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, BeginList_UnorderedAndDuplicate_Normalized)
{
    // Unordered + duplicates: player should sort/dedupe so later triggers work.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"x\" from=\"0\" to=\"10\" begin=\"2s;0s;1s;1s\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // t=1.1s should pick begin=1.0s (even though list is unordered/has dup)
    psx_svg_player_seek(p, 1.1f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 1.0f, 0.05f);
    }

    // t=2.1s should pick begin=2.0s
    psx_svg_player_seek(p, 2.1f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 1.0f, 0.05f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, BeginList_SetElementLeakRepro)
{
    // Historically, <set begin="...;..."> triggered a leak under LSan.
    // This test is intended to keep that scenario covered.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <set attributeName=\"x\" to=\"10\" begin=\"1s;0s\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    // Force a couple seeks to exercise player evaluation.
    psx_svg_player_seek(p, 0.5f);
    psx_svg_player_seek(p, 1.1f);

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, Set_BeginList_MultiTrigger)
{
    // <set> with begin list should re-trigger and be active during each window.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <set attributeName=\"x\" to=\"10\" begin=\"0s;1s\" dur=\"0.5s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // First activation window: [0.0, 0.5)
    psx_svg_player_seek(p, 0.25f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    // After first ends (fill=remove): inactive
    psx_svg_player_seek(p, 0.75f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    // Second activation window: [1.0, 1.5)
    psx_svg_player_seek(p, 1.1f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    // After second ends: inactive
    psx_svg_player_seek(p, 1.6f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, Set_BeginList_FreezeHoldsAfterEachActivation)
{
    // With fill=freeze, the value should remain visible after the active window ends.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <set attributeName=\"x\" to=\"10\" begin=\"0s;1s\" dur=\"0.5s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // After first activation end (t=0.6): should still be held.
    psx_svg_player_seek(p, 0.6f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    // During second activation (t=1.1): active.
    psx_svg_player_seek(p, 1.1f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    // After second activation end (t=1.6): held.
    psx_svg_player_seek(p, 1.6f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, Set_EndList_RemoveCutsOffEachBeginActivation)
{
    // end-list should shorten the active interval per begin trigger.
    // begin: 0s;1s, end: 0.25s;1.25s, fill=remove => active only inside those windows.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <set attributeName=\"x\" to=\"10\" begin=\"0s;1s\" end=\"0.25s;1.25s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // During first activation.
    psx_svg_player_seek(p, 0.10f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    // After first end: removed.
    psx_svg_player_seek(p, 0.30f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    // During second activation.
    psx_svg_player_seek(p, 1.10f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    // After second end: removed.
    psx_svg_player_seek(p, 1.30f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, Set_EndList_FreezeHoldsAfterEndInstant)
{
    // With fill=freeze, value should hold after end.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <set attributeName=\"x\" to=\"10\" begin=\"0s\" end=\"0.25s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // After end.
    psx_svg_player_seek(p, 0.30f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, Set_BeginEvent_ExternalTriggerStarts)
{
    // Minimal Tiny 1.2 support: begin="click" is started via psx_svg_player_trigger().
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <set attributeName=\"x\" to=\"10\" begin=\"click\" dur=\"0.5s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    // Before trigger: inactive at t=0.
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    // Trigger at t=0.2s: should become active immediately.
    psx_svg_player_seek(p, 0.2f);
    psx_svg_player_trigger(p, NULL, "click");
    {
        float v = 0;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    // After dur expires with fill=remove: inactive.
    psx_svg_player_seek(p, 0.8f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, BeginList_RepeatDur_OverridesRepeatCount)
{
    // repeatDur should bound total active duration even if repeatCount is indefinite.
    // begin list triggers at 0s;1s, dur=0.5s, repeatDur=0.75s => total spans [begin, begin+0.75].
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"x\" from=\"0\" to=\"10\" begin=\"0s;1s\" dur=\"0.5s\" repeatCount=\"indefinite\" repeatDur=\"0.75s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // Trigger 0s: within repeatDur -> active
    psx_svg_player_seek(p, 0.60f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_GT(v, 0.0f);
        EXPECT_LT(v, 10.0f);
    }

    // After repeatDur end for first trigger (0.75s): remove => inactive
    psx_svg_player_seek(p, 0.80f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    // Trigger 1s: within its repeatDur window -> active
    psx_svg_player_seek(p, 1.60f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_GT(v, 0.0f);
        EXPECT_LT(v, 10.0f);
    }

    // After repeatDur end for second trigger (1.75s): inactive
    psx_svg_player_seek(p, 1.80f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, Set_BeginList_RepeatDur_RemoveCutsOff)
{
    // repeatDur should bound total active duration for <set> even if dur is longer.
    // begin list triggers at 0s;1s, dur=2s, repeatDur=0.25s, fill=remove => active only for 0.25s after each begin.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <set attributeName=\"x\" to=\"10\" begin=\"0s;1s\" dur=\"2s\" repeatDur=\"0.25s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // within first repeatDur window
    psx_svg_player_seek(p, 0.10f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    // after first repeatDur cutoff
    psx_svg_player_seek(p, 0.30f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    // within second window
    psx_svg_player_seek(p, 1.10f);
    {
        float v = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    // after second cutoff
    psx_svg_player_seek(p, 1.30f);
    {
        float v = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, Set_BeginList_RepeatDur_FreezeHolds)
{
    // With fill=freeze, value should hold after repeatDur cutoff.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <set attributeName=\"x\" to=\"10\" begin=\"0s\" dur=\"2s\" repeatDur=\"0.25s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    // after repeatDur end
    psx_svg_player_seek(p, 0.30f);
    {
        float v = 0;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    destroy_player(p, root);
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
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
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

    destroy_player(p, root);
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
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
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

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateValues_KeyTimes_Linear)
{
    // Tiny 1.2 common case: values + keyTimes, linear interpolation.
    // values="0;10;20" keyTimes="0;0.5;1" dur="1s"
    // t=0.25 => between 0 and 10 at local=0.25/0.5 => 5
    // t=0.75 => between 10 and 20 at local=(0.75-0.5)/0.5 => 15
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"x\" values=\"0;10;20\" keyTimes=\"0;0.5;1\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.25f);
    {
        float v = 0;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        EXPECT_NEAR(v, 5.0f, 0.05f);
    }

    psx_svg_player_seek(p, 0.75f);
    {
        float v = 0;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        EXPECT_NEAR(v, 15.0f, 0.05f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateValues_NoKeyTimes_LinearEvenSegments)
{
    // Tiny 1.2: values without keyTimes => evenly spaced segments.
    // values="0;10;20" dur="1s"
    // t=0.25 => between 0 and 10 at u=0.5 => 5
    // t=0.75 => between 10 and 20 at u=0.5 => 15
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"x\" values=\"0;10;20\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.25f);
    {
        float v = 0;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        EXPECT_NEAR(v, 5.0f, 0.05f);
    }

    psx_svg_player_seek(p, 0.75f);
    {
        float v = 0;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        EXPECT_NEAR(v, 15.0f, 0.05f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateValues_CalcModeDiscrete)
{
    // Tiny 1.2: calcMode="discrete" => no interpolation.
    // values="0;10;20" keyTimes="0;0.5;1" dur="1s"
    // t=0.25 => still first segment => 0
    // t=0.75 => second segment => 10
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"x\" values=\"0;10;20\" keyTimes=\"0;0.5;1\" calcMode=\"discrete\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.25f);
    {
        float v = 0;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        EXPECT_NEAR(v, 0.0f, 0.01f);
    }

    psx_svg_player_seek(p, 0.75f);
    {
        float v = 0;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        EXPECT_NEAR(v, 10.0f, 0.01f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateValues_CalcModeSpline_LinearEquivalent)
{
    // keySplines "0 0 1 1" is effectively linear; should match calcMode=linear.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\">"
        "    <animate attributeName=\"x\" dur=\"1s\" "
        "      values=\"0;10;20\" keyTimes=\"0;0.5;1\" "
        "      calcMode=\"spline\" keySplines=\"0 0 1 1; 0 0 1 1\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (p == NULL || r != S_OK) {
        EXPECT_NE(p, (psx_svg_player*)NULL);
        EXPECT_EQ(r, S_OK);
        if (p) {
            destroy_player(p, root);
        }
        return;
    }

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (n == NULL) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    // First segment (0..0.5): linear would yield 5 at t=0.25
    psx_svg_player_seek(p, 0.25f);
    float v = 0.0f;
    if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
        EXPECT_TRUE(false);
        destroy_player(p, root);
        return;
    }
    EXPECT_NEAR(v, 5.0f, 1e-3f);

    // Second segment (0.5..1): linear would yield 15 at t=0.75
    psx_svg_player_seek(p, 0.75f);
    if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
        EXPECT_TRUE(false);
        destroy_player(p, root);
        return;
    }
    EXPECT_NEAR(v, 15.0f, 1e-3f);

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateValues_CalcModeSpline_NonlinearEases)
{
    // Use an extreme cubic-bezier that should deviate from linear.
    // keySplines "0 0 1 0" biases y low, so early progress should be smaller than linear.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\">"
        "    <animate attributeName=\"x\" dur=\"1s\" "
        "      values=\"0;10\" keyTimes=\"0;1\" "
        "      calcMode=\"spline\" keySplines=\"0 0 1 0\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (p == NULL || r != S_OK) {
        EXPECT_NE(p, (psx_svg_player*)NULL);
        EXPECT_EQ(r, S_OK);
        if (p) {
            destroy_player(p, root);
        }
        return;
    }

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (n == NULL) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    // At t=0.25, linear would yield 2.5. With this spline it should be noticeably smaller.
    psx_svg_player_seek(p, 0.25f);
    float v = 0.0f;
    if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
        EXPECT_TRUE(false);
        destroy_player(p, root);
        return;
    }

    EXPECT_LT(v, 2.5f - 0.5f);
    EXPECT_GE(v, 0.0f);

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateValues_CalcModePaced_UniformMatchesLinear)
{
    // For uniformly spaced values, paced should match linear interpolation.
    // values="0;10;20" => equal segment lengths.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"x\" values=\"0;10;20\" calcMode=\"paced\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    // At t=0.25: in first half of animation; linear would be 5.
    psx_svg_player_seek(p, 0.25f);
    {
        float v = 0.0f;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        EXPECT_NEAR(v, 5.0f, 0.05f);
    }

    // At t=0.75: linear would be 15.
    psx_svg_player_seek(p, 0.75f);
    {
        float v = 0.0f;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        EXPECT_NEAR(v, 15.0f, 0.05f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateValues_CalcModePaced_NonUniformAllocatesTime)
{
    // Non-uniform spacing: values="0;1;11" => segment lengths: 1 and 10.
    // Paced should spend ~1/11 of the time on the first segment.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animate attributeName=\"x\" values=\"0;1;11\" calcMode=\"paced\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    // At t=0.5, we should be well into the long second segment.
    // Expected: after consuming first segment's 1/11 time, remaining progress is (0.5-1/11)/(10/11)=0.45
    // Value ~= 1 + 10*0.45 = 5.5
    psx_svg_player_seek(p, 0.5f);
    {
        float v = 0.0f;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        EXPECT_NEAR(v, 5.5f, 0.10f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateValues_CalcModePaced_AllEqualDegenerates)
{
    static const char* svg =
        "<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' width='100' height='100'>"
        "  <rect id='r' x='0' y='0' width='10' height='10'>"
        "    <animate attributeName='x' dur='1s' values='5;5;5' calcMode='paced' fill='freeze'/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    EXPECT_TRUE(p != NULL);
    if (!p) {
        return;
    }
    EXPECT_EQ(S_OK, r);

    psx_svg_player_play(p);
    psx_svg_player_tick(p, 0.0f);
    psx_svg_player_seek(p, 0.5f);
    psx_svg_player_tick(p, 0.0f);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    EXPECT_TRUE(n != NULL);
    if (n) {
        float v = -1.0f;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 5.0f, 0.01f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateValues_CalcModePaced_NegativeDeltas)
{
    static const char* svg =
        "<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' width='100' height='100'>"
        "  <rect id='r' x='0' y='0' width='10' height='10'>"
        "    <animate attributeName='x' dur='1s' values='10;0;-10' calcMode='paced' fill='freeze'/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    EXPECT_TRUE(p != NULL);
    if (!p) {
        return;
    }
    EXPECT_EQ(S_OK, r);

    // Distances: 10 and 10, so mid time should land mid of overall path.
    psx_svg_player_play(p);
    psx_svg_player_tick(p, 0.0f);
    psx_svg_player_seek(p, 0.5f);
    psx_svg_player_tick(p, 0.0f);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    EXPECT_TRUE(n != NULL);
    if (n) {
        float v = 999.0f;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 0.0f, 0.15f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateValues_CalcModePaced_KeyTimesTakesPrecedence)
{
    static const char* svg =
        "<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' width='100' height='100'>"
        "  <rect id='r' x='0' y='0' width='10' height='10'>"
        "    <animate attributeName='x' dur='1s' values='0;1;11' keyTimes='0;0.1;1' calcMode='paced' fill='freeze'/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    EXPECT_TRUE(p != NULL);
    if (!p) {
        return;
    }
    EXPECT_EQ(S_OK, r);

    // With keyTimes, at t=0.5 we are in segment [0.1,1] with local u ~= 0.444...
    // so v ~= 1 + (11-1)*0.444... ~= 5.44, not the paced mapping.
    psx_svg_player_play(p);
    psx_svg_player_tick(p, 0.0f);
    psx_svg_player_seek(p, 0.5f);
    psx_svg_player_tick(p, 0.0f);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    EXPECT_TRUE(n != NULL);
    if (n) {
        float v = -1.0f;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(v, 5.44f, 0.20f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateValues_CalcModePaced_Boundaries)
{
    static const char* svg =
        "<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' width='100' height='100'>"
        "  <rect id='r' x='0' y='0' width='10' height='10'>"
        "    <animate attributeName='x' dur='1s' values='0;1;11' calcMode='paced' fill='freeze'/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    EXPECT_TRUE(p != NULL);
    if (!p) {
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    EXPECT_TRUE(n != NULL);
    if (!n) {
        destroy_player(p, root);
        return;
    }

    psx_svg_player_play(p);
    psx_svg_player_tick(p, 0.0f);

    // boundary at end of first segment: total distances 1 and 10 => boundary time = 1/11
    psx_svg_player_seek(p, 1.0f / 11.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0.0f;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        // deterministic choice: boundary maps to start of second segment, value should be 1
        EXPECT_NEAR(v, 1.0f, 0.10f);
    }

    // t=1 should be last value
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0.0f;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
            destroy_player(p, root);
            return;
        }
        EXPECT_NEAR(v, 11.0f, 0.01f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateColorFill_FromTo_Discrete)
{
    // Minimal animateColor support in player: fill attribute, discrete mode.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"10\" height=\"10\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000000\">"
        "    <animateColor attributeName=\"fill\" from=\"#ff0000\" to=\"#0000ff\" dur=\"1s\" calcMode=\"discrete\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.25f);
    {
        float v = 0;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_FILL, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_FILL, &v));
            destroy_player(p, root);
            return;
        }
        union {
            uint32_t u;
            float f;
        } bits;
        bits.f = v;
        // internal packed format is RGB (no alpha)
        EXPECT_EQ((uint32_t)0x00FF0000u, bits.u);
    }

    psx_svg_player_seek(p, 0.75f);
    {
        float v = 0;
        if (!psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_FILL, &v)) {
            EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_FILL, &v));
            destroy_player(p, root);
            return;
        }
        union {
            uint32_t u;
            float f;
        } bits;
        bits.f = v;
        // internal packed format is RGB (no alpha)
        EXPECT_EQ((uint32_t)0x000000FFu, bits.u);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Translate_Discrete)
{
    // Step-3 (Tiny 1.2): minimal animateTransform playback, discrete mode.
    // NOTE: current player exposes only float overrides; for transform we will
    // add a debug getter returning a 2D matrix (a,b,c,d,e,f) in the next step.
    // This test is added first to drive implementation.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"10\" height=\"10\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"translate\" values=\"0 0; 10 20\" dur=\"1s\" calcMode=\"discrete\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    // t=0.25 => first value
    psx_svg_player_seek(p, 0.25f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0001f);
            EXPECT_NEAR(0.0f, b, 0.0001f);
            EXPECT_NEAR(0.0f, c, 0.0001f);
            EXPECT_NEAR(1.0f, d, 0.0001f);
            EXPECT_NEAR(0.0f, e, 0.0001f);
            EXPECT_NEAR(0.0f, f, 0.0001f);
        }
    }

    // t=0.75 => second value
    psx_svg_player_seek(p, 0.75f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0001f);
            EXPECT_NEAR(0.0f, b, 0.0001f);
            EXPECT_NEAR(0.0f, c, 0.0001f);
            EXPECT_NEAR(1.0f, d, 0.0001f);
            EXPECT_NEAR(10.0f, e, 0.0001f);
            EXPECT_NEAR(20.0f, f, 0.0001f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Translate_Linear)
{
    // Step-3 (Tiny 1.2): animateTransform translate, linear interpolation.
    // values="0 0; 10 20" dur="1s" => at t=0.25 => (2.5,5), t=0.75 => (7.5,15)
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"10\" height=\"10\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"translate\" values=\"0 0; 10 20\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.25f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0001f);
            EXPECT_NEAR(0.0f, b, 0.0001f);
            EXPECT_NEAR(0.0f, c, 0.0001f);
            EXPECT_NEAR(1.0f, d, 0.0001f);
            EXPECT_NEAR(2.5f, e, 0.0001f);
            EXPECT_NEAR(5.0f, f, 0.0001f);
        }
    }

    psx_svg_player_seek(p, 0.75f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0001f);
            EXPECT_NEAR(0.0f, b, 0.0001f);
            EXPECT_NEAR(0.0f, c, 0.0001f);
            EXPECT_NEAR(1.0f, d, 0.0001f);
            EXPECT_NEAR(7.5f, e, 0.0001f);
            EXPECT_NEAR(15.0f, f, 0.0001f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Translate_KeyTimes)
{
    // Step-3 continuation (Tiny 1.2): translate with values + keyTimes.
    // values has 3 key values, keyTimes are non-uniform:
    // (0,0) at 0.0
    // (10,0) at 0.2
    // (10,20) at 1.0
    // At doc t=0.10 => in seg [0.0,0.2], u=0.5 => (5,0)
    // At doc t=0.60 => in seg [0.2,1.0], u=(0.6-0.2)/0.8=0.5 => (10,10)
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"10\" height=\"10\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"translate\" values=\"0 0; 10 0; 10 20\" keyTimes=\"0; 0.2; 1\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.10f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0001f);
            EXPECT_NEAR(0.0f, b, 0.0001f);
            EXPECT_NEAR(0.0f, c, 0.0001f);
            EXPECT_NEAR(1.0f, d, 0.0001f);
            EXPECT_NEAR(5.0f, e, 0.0001f);
            EXPECT_NEAR(0.0f, f, 0.0001f);
        }
    }

    psx_svg_player_seek(p, 0.60f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0001f);
            EXPECT_NEAR(0.0f, b, 0.0001f);
            EXPECT_NEAR(0.0f, c, 0.0001f);
            EXPECT_NEAR(1.0f, d, 0.0001f);
            EXPECT_NEAR(10.0f, e, 0.0001f);
            EXPECT_NEAR(10.0f, f, 0.0001f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Translate_FromTo)
{
    // Step-3 continuation (Tiny 1.2): translate with from/to.
    // from="0 0" to="10 20" dur="1s" => at t=0.25 => (2.5,5), t=0.75 => (7.5,15)
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"10\" height=\"10\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"translate\" from=\"0 0\" to=\"10 20\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.25f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0001f);
            EXPECT_NEAR(0.0f, b, 0.0001f);
            EXPECT_NEAR(0.0f, c, 0.0001f);
            EXPECT_NEAR(1.0f, d, 0.0001f);
            EXPECT_NEAR(2.5f, e, 0.0001f);
            EXPECT_NEAR(5.0f, f, 0.0001f);
        }
    }

    psx_svg_player_seek(p, 0.75f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0001f);
            EXPECT_NEAR(0.0f, b, 0.0001f);
            EXPECT_NEAR(0.0f, c, 0.0001f);
            EXPECT_NEAR(1.0f, d, 0.0001f);
            EXPECT_NEAR(7.5f, e, 0.0001f);
            EXPECT_NEAR(15.0f, f, 0.0001f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Translate_FreezeHoldsAfterDur)
{
    // Step-3 continuation (Tiny 1.2): fill=freeze should hold the last transform.
    // values="0 0; 10 20" dur="1s" fill="freeze" => after 1s, still (10,20)
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"10\" height=\"10\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"translate\" values=\"0 0; 10 20\" dur=\"1s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    // Seek beyond dur.
    psx_svg_player_seek(p, 1.25f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0001f);
            EXPECT_NEAR(0.0f, b, 0.0001f);
            EXPECT_NEAR(0.0f, c, 0.0001f);
            EXPECT_NEAR(1.0f, d, 0.0001f);
            EXPECT_NEAR(10.0f, e, 0.0001f);
            EXPECT_NEAR(20.0f, f, 0.0001f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Translate_RepeatCount)
{
    // Step-3 continuation (Tiny 1.2): repeatCount for translate.
    // values="0 0; 10 0" dur="1s" repeatCount="2" fill="remove"
    // - At t=1.25: second iteration, local=0.25 => (2.5,0)
    // - At t=2.10: past total (2s), fill=remove => no override
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"10\" height=\"10\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"translate\" values=\"0 0; 10 0\" dur=\"1s\" repeatCount=\"2\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 1.25f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(2.5f, e, 0.0001f);
            EXPECT_NEAR(0.0f, f, 0.0001f);
        }
    }

    psx_svg_player_seek(p, 2.10f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Scale_Discrete)
{
    // Step-3 continuation (Tiny 1.2): animateTransform scale, discrete.
    // values="1 1; 2 3" dur="1s" calcMode="discrete"
    // Note: parser stores scale values in the same transform-values layout as translate:
    // - For scale, the numbers are placed in the first 2 slots of the transform data blob.
    // - The player encodes scale as matrix: a=sx, d=sy, e=f=0.
    // - At t=0.25: first value => scale(1,1)
    // - At t=0.75: second value => scale(2,3)
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"10\" height=\"10\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"scale\" values=\"1 1; 2 3\" dur=\"1s\" calcMode=\"discrete\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.25f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0001f);
            EXPECT_NEAR(0.0f, b, 0.0001f);
            EXPECT_NEAR(0.0f, c, 0.0001f);
            EXPECT_NEAR(1.0f, d, 0.0001f);
            EXPECT_NEAR(0.0f, e, 0.0001f);
            EXPECT_NEAR(0.0f, f, 0.0001f);
        }
    }

    psx_svg_player_seek(p, 0.75f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(2.0f, a, 0.0001f);
            EXPECT_NEAR(0.0f, b, 0.0001f);
            EXPECT_NEAR(0.0f, c, 0.0001f);
            EXPECT_NEAR(3.0f, d, 0.0001f);
            EXPECT_NEAR(0.0f, e, 0.0001f);
            EXPECT_NEAR(0.0f, f, 0.0001f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Rotate_Discrete)
{
    // Step-3 continuation (Tiny 1.2): animateTransform rotate, discrete.
    // values="0; 90" dur="1s" calcMode="discrete"
    // - At t=0.25: first value => rotate(0)
    // - At t=0.75: second value => rotate(90deg)
    // For now: only rotate(angle) around origin (no cx/cy).
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"10\" height=\"10\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"rotate\" values=\"0; 90\" dur=\"1s\" calcMode=\"discrete\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.25f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0001f);
            EXPECT_NEAR(0.0f, b, 0.0001f);
            EXPECT_NEAR(0.0f, c, 0.0001f);
            EXPECT_NEAR(1.0f, d, 0.0001f);
            EXPECT_NEAR(0.0f, e, 0.0001f);
            EXPECT_NEAR(0.0f, f, 0.0001f);
        }
    }

    psx_svg_player_seek(p, 0.75f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(0.0f, a, 0.0001f);
            EXPECT_NEAR(1.0f, b, 0.0001f);
            EXPECT_NEAR(-1.0f, c, 0.0001f);
            EXPECT_NEAR(0.0f, d, 0.0001f);
            EXPECT_NEAR(0.0f, e, 0.0001f);
            EXPECT_NEAR(0.0f, f, 0.0001f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Rotate_Linear)
{
    // Step-3 continuation (Tiny 1.2): animateTransform rotate, linear (default).
    // values="0; 90" dur="1s" => at t=0.5 => rotate(45deg)
    // For now: only rotate(angle) around origin (no cx/cy).
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"10\" height=\"10\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"rotate\" values=\"0; 90\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.50f);
    {
        // cos/sin(45deg)
        const float k = 0.70710678f;
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(k, a, 0.0002f);
            EXPECT_NEAR(k, b, 0.0002f);
            EXPECT_NEAR(-k, c, 0.0002f);
            EXPECT_NEAR(k, d, 0.0002f);
            EXPECT_NEAR(0.0f, e, 0.0001f);
            EXPECT_NEAR(0.0f, f, 0.0001f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Rotate_Linear_WithCenter)
{
    // rotate(angle cx cy) about a point: matrix is T(cx,cy)*R(angle)*T(-cx,-cy)
    // Use values="0 10 20; 90 10 20" so center is constant.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"rotate\" values=\"0 10 20; 90 10 20\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.5f);

    float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
    if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
        const float cs = 0.70710678f;
        const float sn = 0.70710678f;
        // For rotation about (10,20): e = cx - cx*cs + cy*sn, f = cy - cx*sn - cy*cs
        const float ex = 10.0f - 10.0f * cs + 20.0f * sn;
        const float fy = 20.0f - 10.0f * sn - 20.0f * cs;

        EXPECT_NEAR(a, cs, 0.0002f);
        EXPECT_NEAR(b, sn, 0.0002f);
        EXPECT_NEAR(c, -sn, 0.0002f);
        EXPECT_NEAR(d, cs, 0.0002f);
        EXPECT_NEAR(e, ex, 0.0002f);
        EXPECT_NEAR(f, fy, 0.0002f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Scale_Linear)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"scale\" values=\"1; 3\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.5f);

    float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    ps_bool ok = psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f);
    EXPECT_TRUE(ok);

    // At t=0.5, scale from 1 -> 3 should be 2.
    EXPECT_NEAR(a, 2.0f, 0.0002f);
    EXPECT_NEAR(b, 0.0f, 0.0002f);
    EXPECT_NEAR(c, 0.0f, 0.0002f);
    EXPECT_NEAR(d, 2.0f, 0.0002f);
    EXPECT_NEAR(e, 0.0f, 0.0002f);
    EXPECT_NEAR(f, 0.0f, 0.0002f);

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_SkewX_Linear_FromTo)
{
    // skewX(angle): matrix [ 1 tan(a) 0; 0 1 0; 0 0 1 ]
    // from=0 to=45 dur=1s => at t=0.5 => 22.5deg => tan ~= 0.41421356
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"skewX\" from=\"0\" to=\"45\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.5f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            const float tan225 = 0.41421356f;
            EXPECT_NEAR(1.0f, a, 0.0002f);
            EXPECT_NEAR(0.0f, b, 0.0002f);
            EXPECT_NEAR(tan225, c, 0.0003f);
            EXPECT_NEAR(1.0f, d, 0.0002f);
            EXPECT_NEAR(0.0f, e, 0.0002f);
            EXPECT_NEAR(0.0f, f, 0.0002f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_SkewX_Discrete_FromTo)
{
    // calcMode=discrete: use from for t in [0, 0.5), to for t in [0.5, 1].
    // skewX(angle): matrix [ 1 tan(a) 0; 0 1 0; 0 0 1 ]
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"skewX\" from=\"0\" to=\"45\" dur=\"1s\" calcMode=\"discrete\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    // Before mid: choose from (0deg => tan=0).
    psx_svg_player_seek(p, 0.25f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0002f);
            EXPECT_NEAR(0.0f, b, 0.0002f);
            EXPECT_NEAR(0.0f, c, 0.0002f);
            EXPECT_NEAR(1.0f, d, 0.0002f);
            EXPECT_NEAR(0.0f, e, 0.0002f);
            EXPECT_NEAR(0.0f, f, 0.0002f);
        }
    }

    // After mid: choose to (45deg => tan=1).
    psx_svg_player_seek(p, 0.75f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0002f);
            EXPECT_NEAR(0.0f, b, 0.0002f);
            EXPECT_NEAR(1.0f, c, 0.0003f);
            EXPECT_NEAR(1.0f, d, 0.0002f);
            EXPECT_NEAR(0.0f, e, 0.0002f);
            EXPECT_NEAR(0.0f, f, 0.0002f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_SkewX_Values_KeyTimes_Linear)
{
    // values + keyTimes (linear):
    // values="0;45;0" keyTimes="0;0.25;1" dur=1s
    // - at t=0.125: segment[0,0.25], u=0.5 => angle=22.5 => tan ~= 0.41421356
    // - at t=0.625: segment[0.25,1], u=(0.625-0.25)/0.75=0.5 => angle=22.5
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"skewX\" values=\"0;45;0\" keyTimes=\"0;0.25;1\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    const float tan225 = 0.41421356f;

    psx_svg_player_seek(p, 0.125f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0002f);
            EXPECT_NEAR(0.0f, b, 0.0002f);
            EXPECT_NEAR(tan225, c, 0.0003f);
            EXPECT_NEAR(1.0f, d, 0.0002f);
            EXPECT_NEAR(0.0f, e, 0.0002f);
            EXPECT_NEAR(0.0f, f, 0.0002f);
        }
    }

    psx_svg_player_seek(p, 0.625f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0002f);
            EXPECT_NEAR(0.0f, b, 0.0002f);
            EXPECT_NEAR(tan225, c, 0.0003f);
            EXPECT_NEAR(1.0f, d, 0.0002f);
            EXPECT_NEAR(0.0f, e, 0.0002f);
            EXPECT_NEAR(0.0f, f, 0.0002f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_SkewY_Linear_FromTo)
{
    // skewY(angle): matrix [ 1 0 0; tan(a) 1 0; 0 0 1 ]
    // from=0 to=45 dur=1s => at t=0.5 => 22.5deg => tan ~= 0.41421356
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"skewY\" from=\"0\" to=\"45\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    psx_svg_player_seek(p, 0.5f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            const float tan225 = 0.41421356f;
            EXPECT_NEAR(1.0f, a, 0.0002f);
            EXPECT_NEAR(tan225, b, 0.0003f);
            EXPECT_NEAR(0.0f, c, 0.0002f);
            EXPECT_NEAR(1.0f, d, 0.0002f);
            EXPECT_NEAR(0.0f, e, 0.0002f);
            EXPECT_NEAR(0.0f, f, 0.0002f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_SkewY_Discrete_FromTo)
{
    // calcMode=discrete: use from for t in [0, 0.5), to for t in [0.5, 1].
    // skewY(angle): matrix [ 1 0 0; tan(a) 1 0; 0 0 1 ]
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"skewY\" from=\"0\" to=\"45\" dur=\"1s\" calcMode=\"discrete\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    // Before mid: choose from (0deg => tan=0).
    psx_svg_player_seek(p, 0.25f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0002f);
            EXPECT_NEAR(0.0f, b, 0.0002f);
            EXPECT_NEAR(0.0f, c, 0.0002f);
            EXPECT_NEAR(1.0f, d, 0.0002f);
            EXPECT_NEAR(0.0f, e, 0.0002f);
            EXPECT_NEAR(0.0f, f, 0.0002f);
        }
    }

    // After mid: choose to (45deg => tan=1).
    psx_svg_player_seek(p, 0.75f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0002f);
            EXPECT_NEAR(1.0f, b, 0.0003f);
            EXPECT_NEAR(0.0f, c, 0.0002f);
            EXPECT_NEAR(1.0f, d, 0.0002f);
            EXPECT_NEAR(0.0f, e, 0.0002f);
            EXPECT_NEAR(0.0f, f, 0.0002f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_SkewY_Values_KeyTimes_Linear)
{
    // values + keyTimes (linear):
    // values="0;45;0" keyTimes="0;0.25;1" dur=1s
    // - at t=0.125: segment[0,0.25], u=0.5 => angle=22.5 => tan ~= 0.41421356
    // - at t=0.625: segment[0.25,1], u=(0.625-0.25)/0.75=0.5 => angle=22.5
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"skewY\" values=\"0;45;0\" keyTimes=\"0;0.25;1\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    if (!p) {
        EXPECT_NE((psx_svg_player*)NULL, p);
        return;
    }
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    if (!n) {
        EXPECT_TRUE(n != NULL);
        destroy_player(p, root);
        return;
    }

    const float tan225 = 0.41421356f;

    psx_svg_player_seek(p, 0.125f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0002f);
            EXPECT_NEAR(tan225, b, 0.0003f);
            EXPECT_NEAR(0.0f, c, 0.0002f);
            EXPECT_NEAR(1.0f, d, 0.0002f);
            EXPECT_NEAR(0.0f, e, 0.0002f);
            EXPECT_NEAR(0.0f, f, 0.0002f);
        }
    }

    psx_svg_player_seek(p, 0.625f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.0002f);
            EXPECT_NEAR(tan225, b, 0.0003f);
            EXPECT_NEAR(0.0f, c, 0.0002f);
            EXPECT_NEAR(1.0f, d, 0.0002f);
            EXPECT_NEAR(0.0f, e, 0.0002f);
            EXPECT_NEAR(0.0f, f, 0.0002f);
        }
    }

    destroy_player(p, root);
}

// ============================================================
// Priority 1: Render layer transform override integration tests
// ============================================================

TEST_F(SVGPlayerTest, AnimateTransform_Translate_TransformOverride_Exists)
{
    // Verify that after seek, the transform override is stored and retrievable
    // (i.e. the player correctly populates the anim_state.transforms array).
    // This is the prerequisite for the render layer to consume it.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"translate\" from=\"0 0\" to=\"20 40\" dur=\"1s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // At t=0.5s: translate(10, 20) => matrix(1,0,0,1,10,20)
    psx_svg_player_seek(p, 0.5f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(10.0f, e, 0.05f);
        EXPECT_NEAR(20.0f, f, 0.05f);
    }

    // At t=1s (freeze): translate(20, 40) => matrix(1,0,0,1,20,40)
    psx_svg_player_seek(p, 1.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(20.0f, e, 0.05f);
        EXPECT_NEAR(40.0f, f, 0.05f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Remove_TransformOverride_Cleared)
{
    // After animation ends with fill=remove, the transform override must be absent.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"translate\" from=\"0 0\" to=\"20 40\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // During animation: override present
    psx_svg_player_seek(p, 0.5f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
    }

    // After animation ends (fill=remove): override absent
    psx_svg_player_seek(p, 1.5f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
    }

    destroy_player(p, root);
}

// ============================================================
// Priority 3: animateTransform type="matrix"
// ============================================================

TEST_F(SVGPlayerTest, AnimateTransform_Matrix_Discrete_FromTo)
{
    // type="matrix" with from/to and calcMode="discrete":
    // from="1 0 0 1 0 0" to="2 0 0 2 10 20"
    // At t=0.25 (< 0.5): use from => identity
    // At t=0.75 (>= 0.5): use to => scale(2)+translate(10,20)
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"matrix\""
        "      from=\"1 0 0 1 0 0\" to=\"2 0 0 2 10 20\""
        "      dur=\"1s\" calcMode=\"discrete\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // t=0.25: discrete => from value (identity)
    psx_svg_player_seek(p, 0.25f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(0.0f, e, 0.001f);
        EXPECT_NEAR(0.0f, f, 0.001f);
    }

    // t=0.75: discrete => to value
    psx_svg_player_seek(p, 0.75f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(2.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(2.0f, d, 0.001f);
        EXPECT_NEAR(10.0f, e, 0.001f);
        EXPECT_NEAR(20.0f, f, 0.001f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Matrix_Linear_FromTo)
{
    // type="matrix" with from/to and linear interpolation (default):
    // from="1 0 0 1 0 0" to="1 0 0 1 10 0"
    // At t=0.5: lerp => matrix(1,0,0,1,5,0)
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"matrix\""
        "      from=\"1 0 0 1 0 0\" to=\"1 0 0 1 10 0\""
        "      dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // t=0.5: lerp => matrix(1,0,0,1,5,0)
    psx_svg_player_seek(p, 0.5f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(5.0f, e, 0.05f);
        EXPECT_NEAR(0.0f, f, 0.001f);
    }

    // t=1.0: at end => matrix(1,0,0,1,10,0)
    psx_svg_player_seek(p, 1.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        // fill=remove at exact end instant: may or may not be active depending on impl
        // Just check if active, the value should be near to
        if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
            EXPECT_NEAR(1.0f, a, 0.001f);
            EXPECT_NEAR(10.0f, e, 0.05f);
        }
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Matrix_Values_KeyTimes_Linear)
{
    // type="matrix" with values + keyTimes (linear):
    // values="1 0 0 1 0 0; 1 0 0 1 10 0; 1 0 0 1 10 20"
    // keyTimes="0; 0.5; 1" dur="1s"
    // At t=0.25 (in seg 0, u=0.5): lerp(0,10)*0.5 => e=5, f=0
    // At t=0.75 (in seg 1, u=0.5): lerp(10,10)*0.5=10, lerp(0,20)*0.5=10 => e=10, f=10
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateTransform attributeName=\"transform\" type=\"matrix\""
        "      values=\"1 0 0 1 0 0; 1 0 0 1 10 0; 1 0 0 1 10 20\""
        "      keyTimes=\"0; 0.5; 1\" dur=\"1s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    // t=0.25: seg 0, u=0.5 => e=5, f=0
    psx_svg_player_seek(p, 0.25f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(5.0f, e, 0.1f);
        EXPECT_NEAR(0.0f, f, 0.1f);
    }

    // t=0.75: seg 1, u=0.5 => e=10, f=10
    psx_svg_player_seek(p, 0.75f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        ASSERT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(10.0f, e, 0.1f);
        EXPECT_NEAR(10.0f, f, 0.1f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateCircleCx_FromTo)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"300\" height=\"100\">"
        "  <circle id=\"r\" cx=\"0\" cy=\"50\" r=\"15\" fill=\"blue\">"
        "    <animate attributeName=\"cx\" from=\"0\" to=\"300\" dur=\"5s\" fill=\"freeze\"/>"
        "  </circle>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);
    EXPECT_EQ(S_OK, r);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0: cx should be 0
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_CX, &v));
        EXPECT_NEAR(0.0f, v, 0.001f);
    }

    // t=2.5s midpoint: cx should be 150
    psx_svg_player_seek(p, 2.5f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_CX, &v));
        EXPECT_NEAR(150.0f, v, 0.1f);
    }

    // t=5s end: cx should be 300 (freeze)
    psx_svg_player_seek(p, 5.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_CX, &v));
        EXPECT_NEAR(300.0f, v, 0.001f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateCircleCy_FromTo)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"300\">"
        "  <circle id=\"r\" cx=\"50\" cy=\"0\" r=\"10\" fill=\"red\">"
        "    <animate attributeName=\"cy\" from=\"0\" to=\"200\" dur=\"4s\" fill=\"freeze\"/>"
        "  </circle>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=2s midpoint: cy should be 100
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_CY, &v));
        EXPECT_NEAR(100.0f, v, 0.1f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateCircleR_FromTo)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"200\" height=\"200\">"
        "  <circle id=\"r\" cx=\"100\" cy=\"100\" r=\"10\" fill=\"green\">"
        "    <animate attributeName=\"r\" from=\"10\" to=\"50\" dur=\"2s\" fill=\"freeze\"/>"
        "  </circle>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=1s midpoint: r should be 30
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_R, &v));
        EXPECT_NEAR(30.0f, v, 0.1f);
    }

    destroy_player(p, root);
}
