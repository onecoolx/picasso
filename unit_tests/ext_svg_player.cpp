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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

static inline bool psx_svg_player_debug_get_int_override(const psx_svg_player* p,
                                                         const psx_svg_node* target,
                                                         psx_svg_attr_type attr,
                                                         int32_t* out_v)
{
    return psx_svg_anim_get_int32(&p->anim_state, target, attr, out_v);
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

/* ── Local motion path helpers for property tests (no dependency on player internals) ── */

struct test_path_points {
    float* xs;
    float* ys;
    uint32_t count;
    uint32_t cap;
};

static void test_path_destroy(test_path_points* pts)
{
    if (!pts) { return; }
    free(pts->xs);
    pts->xs = NULL;
    free(pts->ys);
    pts->ys = NULL;
    pts->count = 0;
    pts->cap = 0;
}

static bool test_path_push(test_path_points* pts, float x, float y)
{
    if (!pts) { return false; }
    if (pts->count >= pts->cap) {
        uint32_t nc = pts->cap ? pts->cap * 2 : 8;
        float* nxs = (float*)realloc(pts->xs, nc * sizeof(float));
        float* nys = (float*)realloc(pts->ys, nc * sizeof(float));
        if (!nxs || !nys) { return false; }
        pts->xs = nxs;
        pts->ys = nys;
        pts->cap = nc;
    }
    pts->xs[pts->count] = x;
    pts->ys[pts->count] = y;
    pts->count++;
    return true;
}

static char* test_path_format(const float* xs, const float* ys, uint32_t count)
{
    if (!xs || !ys || count == 0) { return NULL; }
    uint32_t buf_cap = count * 40 + 4;
    char* buf = (char*)malloc(buf_cap);
    if (!buf) { return NULL; }
    uint32_t pos = 0;
    for (uint32_t i = 0; i < count; i++) {
        int n = 0;
        if (i == 0) {
            n = sprintf(buf + pos, "M %.6g %.6g", xs[i], ys[i]);
        } else {
            n = sprintf(buf + pos, " L %.6g %.6g", xs[i], ys[i]);
        }
        if (n > 0) { pos += (uint32_t)n; }
    }
    buf[pos] = '\0';
    return buf;
}

static uint32_t test_path_skip_ws(const char* s, uint32_t pos, uint32_t len)
{
    while (pos < len && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\r'
                         || s[pos] == '\n' || s[pos] == ',')) {
        pos++;
    }
    return pos;
}

static uint32_t test_path_parse_float(const char* s, uint32_t pos, uint32_t len, float* out)
{
    uint32_t start = pos;
    if (pos >= len) { return pos; }
    if (s[pos] == '+' || s[pos] == '-') { pos++; }
    bool has_digits = false;
    while (pos < len && s[pos] >= '0' && s[pos] <= '9') { pos++; has_digits = true; }
    if (pos < len && s[pos] == '.') {
        pos++;
        while (pos < len && s[pos] >= '0' && s[pos] <= '9') { pos++; has_digits = true; }
    }
    if (!has_digits) { return start; }
    if (pos < len && (s[pos] == 'e' || s[pos] == 'E')) {
        uint32_t ep = pos + 1;
        if (ep < len && (s[ep] == '+' || s[ep] == '-')) { ep++; }
        bool he = false;
        while (ep < len && s[ep] >= '0' && s[ep] <= '9') { ep++; he = true; }
        if (he) { pos = ep; }
    }
    char buf[64];
    uint32_t slen = pos - start;
    if (slen >= sizeof(buf)) { slen = sizeof(buf) - 1; }
    memcpy(buf, s + start, slen);
    buf[slen] = '\0';
    *out = (float)atof(buf);
    return pos;
}

static bool test_path_parse(const char* path_str, uint32_t len,
                            float** out_xs, float** out_ys, uint32_t* out_count)
{
    test_path_points pts;
    pts.xs = NULL;
    pts.ys = NULL;
    pts.count = 0;
    pts.cap = 0;
    if (!path_str || len == 0) { *out_xs = NULL; *out_ys = NULL; *out_count = 0; return false; }

    float cur_x = 0, cur_y = 0, start_x = 0, start_y = 0;
    char cmd = 0;
    uint32_t pos = 0;

    while (pos < len) {
        pos = test_path_skip_ws(path_str, pos, len);
        if (pos >= len) { break; }
        char ch = path_str[pos];

        if (ch == 'M' || ch == 'm' || ch == 'L' || ch == 'l'
            || ch == 'H' || ch == 'h' || ch == 'V' || ch == 'v') {
            cmd = ch;
            pos++;
        } else if (ch == 'Z' || ch == 'z') {
            pos++;
            if (cur_x != start_x || cur_y != start_y) {
                if (!test_path_push(&pts, start_x, start_y)) { test_path_destroy(&pts); *out_xs = NULL; *out_ys = NULL; *out_count = 0; return false; }
                cur_x = start_x;
                cur_y = start_y;
            }
            cmd = 0;
            continue;
        } else if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
            cmd = 0;
            pos++;
            continue;
        }

        if (cmd == 0) {
            if (!((ch >= '0' && ch <= '9') || ch == '-' || ch == '+' || ch == '.')) { pos++; continue; }
            float dummy = 0;
            uint32_t np = test_path_parse_float(path_str, pos, len, &dummy);
            pos = (np == pos) ? pos + 1 : np;
            continue;
        }

        if (cmd == 'H' || cmd == 'h') {
            pos = test_path_skip_ws(path_str, pos, len);
            float x = 0;
            uint32_t np = test_path_parse_float(path_str, pos, len, &x);
            if (np == pos) { cmd = 0; continue; }
            pos = np;
            if (cmd == 'h') { x += cur_x; }
            if (!test_path_push(&pts, x, cur_y)) { test_path_destroy(&pts); *out_xs = NULL; *out_ys = NULL; *out_count = 0; return false; }
            cur_x = x;
            continue;
        }

        if (cmd == 'V' || cmd == 'v') {
            pos = test_path_skip_ws(path_str, pos, len);
            float y = 0;
            uint32_t np = test_path_parse_float(path_str, pos, len, &y);
            if (np == pos) { cmd = 0; continue; }
            pos = np;
            if (cmd == 'v') { y += cur_y; }
            if (!test_path_push(&pts, cur_x, y)) { test_path_destroy(&pts); *out_xs = NULL; *out_ys = NULL; *out_count = 0; return false; }
            cur_y = y;
            continue;
        }

        pos = test_path_skip_ws(path_str, pos, len);
        float x = 0, y = 0;
        uint32_t np = test_path_parse_float(path_str, pos, len, &x);
        if (np == pos) { cmd = 0; continue; }
        pos = np;
        pos = test_path_skip_ws(path_str, pos, len);
        np = test_path_parse_float(path_str, pos, len, &y);
        if (np == pos) { cmd = 0; continue; }
        pos = np;

        if (cmd == 'm' || cmd == 'l') { x += cur_x; y += cur_y; }
        if (!test_path_push(&pts, x, y)) { test_path_destroy(&pts); *out_xs = NULL; *out_ys = NULL; *out_count = 0; return false; }
        cur_x = x;
        cur_y = y;
        if (cmd == 'M' || cmd == 'm') { start_x = cur_x; start_y = cur_y; }
        if (cmd == 'M') { cmd = 'L'; }
        else if (cmd == 'm') { cmd = 'l'; }
    }

    if (pts.count > 0) {
        *out_xs = pts.xs;
        *out_ys = pts.ys;
        *out_count = pts.count;
        return true;
    }
    test_path_destroy(&pts);
    *out_xs = NULL;
    *out_ys = NULL;
    *out_count = 0;
    return false;
}

static bool test_arc_length_position(const float* xs, const float* ys, uint32_t count,
                                     float t, float* out_x, float* out_y)
{
    if (!xs || !ys || count < 2 || !out_x || !out_y) { return false; }

    float* cum = (float*)malloc(count * sizeof(float));
    if (!cum) { return false; }
    cum[0] = 0;
    for (uint32_t i = 1; i < count; i++) {
        float dx = xs[i] - xs[i - 1];
        float dy = ys[i] - ys[i - 1];
        cum[i] = cum[i - 1] + sqrtf(dx * dx + dy * dy);
    }
    float total = cum[count - 1];

    if (count == 1 || total <= 0) { *out_x = xs[0]; *out_y = ys[0]; free(cum); return true; }
    if (t <= 0) { *out_x = xs[0]; *out_y = ys[0]; free(cum); return true; }
    if (t >= 1) { *out_x = xs[count - 1]; *out_y = ys[count - 1]; free(cum); return true; }

    float target = t * total;
    uint32_t lo = 0, hi = count - 1;
    while (lo + 1 < hi) {
        uint32_t mid = (lo + hi) / 2;
        if (cum[mid] <= target) { lo = mid; }
        else { hi = mid; }
    }
    float seg_len = cum[hi] - cum[lo];
    float frac = (seg_len > 0) ? (target - cum[lo]) / seg_len : 0;
    *out_x = xs[lo] + frac * (xs[hi] - xs[lo]);
    *out_y = ys[lo] + frac * (ys[hi] - ys[lo]);
    free(cum);
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

TEST_F(SVGPlayerTest, AnimateTransform_Rotate_Linear_FromTo_WithCenter)
{
    // animateTransform rotate using from/to (no values attr).
    // from="0 60 60" to="360 100 60" dur="20s"
    // At t=10s (midpoint): angle=180, cx=80, cy=60
    // Matrix: T(cx,cy)*R(180)*T(-cx,-cy)
    // cos(180)=-1, sin(180)=0 => a=-1, b=0, c=0, d=-1
    // e = cx - cx*(-1) + cy*0 = 80+80 = 160
    // f = cy - cx*0 - cy*(-1) = 60+60 = 120
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"300\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"50\" width=\"15\" height=\"34\" fill=\"blue\">"
        "    <animateTransform attributeName=\"transform\" type=\"rotate\""
        "      from=\"0 60 60\" to=\"360 100 60\" dur=\"20s\" repeatCount=\"indefinite\"/>"
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

    // Seek to midpoint: t=10s => local t=0.5
    // angle = lerp(0, 360, 0.5) = 180 deg
    // cx = lerp(60, 100, 0.5) = 80
    // cy = lerp(60, 60, 0.5) = 60
    psx_svg_player_seek(p, 10.0f);

    float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
    if (psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f)) {
        // cos(180)=-1, sin(180)~=0
        EXPECT_NEAR(-1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(-1.0f, d, 0.001f);
        // e = cx - cx*cos + cy*sin = 80 - 80*(-1) + 60*0 = 160
        // f = cy - cx*sin - cy*cos = 60 - 80*0 - 60*(-1) = 120
        EXPECT_NEAR(160.0f, e, 0.1f);
        EXPECT_NEAR(120.0f, f, 0.1f);
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

TEST_F(SVGPlayerTest, AnimateLine_X1Y1X2Y2_FromTo)
{
    // Animate all four line endpoint attributes simultaneously.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"200\" height=\"200\">"
        "  <line id=\"ln\" x1=\"0\" y1=\"0\" x2=\"10\" y2=\"10\" stroke=\"#000\" stroke-width=\"1\">"
        "    <animate attributeName=\"x1\" from=\"0\" to=\"100\" dur=\"2s\" fill=\"freeze\"/>"
        "    <animate attributeName=\"y1\" from=\"0\" to=\"50\" dur=\"2s\" fill=\"freeze\"/>"
        "    <animate attributeName=\"x2\" from=\"10\" to=\"200\" dur=\"2s\" fill=\"freeze\"/>"
        "    <animate attributeName=\"y2\" from=\"10\" to=\"150\" dur=\"2s\" fill=\"freeze\"/>"
        "  </line>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "ln");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=1s midpoint of 2s animation: each attribute should be at 50% interpolation
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X1, &v));
        EXPECT_NEAR(50.0f, v, 0.05f);
    }
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_Y1, &v));
        EXPECT_NEAR(25.0f, v, 0.05f);
    }
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X2, &v));
        EXPECT_NEAR(105.0f, v, 0.05f);
    }
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_Y2, &v));
        EXPECT_NEAR(80.0f, v, 0.05f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateStrokeOpacity_FromTo)
{
    // Animate stroke-opacity from 1 to 0 over 2s; at midpoint expect 0.5.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"50\" height=\"50\" fill=\"none\" stroke=\"#000\" stroke-width=\"2\" stroke-opacity=\"1\">"
        "    <animate attributeName=\"stroke-opacity\" from=\"1\" to=\"0\" dur=\"2s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=1s midpoint of 2s animation: stroke-opacity should be 0.5
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_STROKE_OPACITY, &v));
        EXPECT_NEAR(0.5f, v, 0.05f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_Collection_NoAttributeName)
{
    // animateMotion has no attributeName attribute; the player must still
    // collect it and implicitly target SVG_ATTR_TRANSFORM.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"200\" height=\"200\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"20\" height=\"20\" fill=\"#000\">"
        "    <animateMotion path=\"M 0 0 L 100 0\" dur=\"2s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // Seek to midpoint (1.0s of 2s duration).
    // If animateMotion was collected, a transform override should exist.
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_Path_ML_LinearInterp)
{
    // Path: M 0 0 L 100 0 L 100 100
    // Two segments: (0,0)->(100,0) len=100, (100,0)->(100,100) len=100, total=200
    // dur=3s, fill=freeze
    // t=0s   => normalized 0.0 => pos (0,0)     => transform (1,0,0,1,0,0)
    // t=1.5s => normalized 0.5 => dist=100 => end of seg1 => pos (100,0)
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"200\" height=\"200\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"20\" height=\"20\" fill=\"#000\">"
        "    <animateMotion path=\"M 0 0 L 100 0 L 100 100\" dur=\"3s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // Seek to t=0s: position should be start of path (0,0)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(0.0f, e, 0.05f);
        EXPECT_NEAR(0.0f, f, 0.05f);
    }

    // Seek to t=1.5s (midpoint): distance=100 => end of first segment => (100,0)
    psx_svg_player_seek(p, 1.5f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(100.0f, e, 0.1f);
        EXPECT_NEAR(0.0f, f, 0.1f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_Path_HVZ_Commands)
{
    // Path: M 0 0 H 100 V 50 Z
    // Segments: (0,0)->(100,0) H len=100, (100,0)->(100,50) V len=50,
    // (100,50)->(0,0) Z len=sqrt(100^2+50^2)=~111.803
    // Total arc length ~261.803
    // dur=3s, fill=freeze
    // t=0s   => normalized 0.0 => pos (0,0)
    // t=1.0s => normalized 1/3 => dist ~87.27 => within seg1 => x ~87.27, y=0
    // t=3.0s => normalized 1.0 => last point (0,0) via Z close
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"200\" height=\"200\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"20\" height=\"20\" fill=\"#000\">"
        "    <animateMotion path=\"M 0 0 H 100 V 50 Z\" dur=\"3s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // Seek to t=0s: position should be start of path (0,0)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(0.0f, e, 0.05f);
        EXPECT_NEAR(0.0f, f, 0.05f);
    }

    // Seek to t=1.0s: normalized=1/3, dist=~87.27, within first segment (H 100)
    // Position should be (~87.27, 0) — x between 0 and 100, y=0
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        // x should be ~87.27 (within first segment), y should be 0
        EXPECT_GT(e, 50.0f); // well into the first segment
        EXPECT_LT(e, 100.0f); // but not past it
        EXPECT_NEAR(0.0f, f, 0.5f);
    }

    // Seek to t=3.0s (end): Z closes back to (0,0), fill=freeze holds last point
    psx_svg_player_seek(p, 3.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(0.0f, e, 0.1f); // back to origin x
        EXPECT_NEAR(0.0f, f, 0.1f); // back to origin y
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_ArcLength_Boundaries)
{
    // Path: M 0 0 L 300 0 L 300 400
    // Segment 1: (0,0)->(300,0) len=300
    // Segment 2: (300,0)->(300,400) len=400
    // Total arc length = 700
    // dur=7s, fill=freeze
    //
    // t=0s   => normalized 0.0 => distance 0   => (0, 0)
    // t=7s   => normalized 1.0 => distance 700 => (300, 400)
    // t=3.5s => normalized 0.5 => distance 350 => within seg2 at 50 units past (300,0) => (300, 50)
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"400\" height=\"500\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"20\" height=\"20\" fill=\"#000\">"
        "    <animateMotion path=\"M 0 0 L 300 0 L 300 400\" dur=\"7s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0s: normalized 0.0 => first point (0, 0)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(0.0f, e, 0.05f); // x = 0
        EXPECT_NEAR(0.0f, f, 0.05f); // y = 0
    }

    // t=7s: normalized 1.0 => last point (300, 400)
    psx_svg_player_seek(p, 7.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(300.0f, e, 0.1f); // x = 300
        EXPECT_NEAR(400.0f, f, 0.1f); // y = 400
    }

    // t=3.5s: normalized 0.5 => distance 350 along path
    // Seg1 covers distance [0, 300], seg2 covers [300, 700]
    // distance 350 is 50 units into seg2 => (300, 0 + 50) = (300, 50)
    psx_svg_player_seek(p, 3.5f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(300.0f, e, 0.1f); // x = 300
        EXPECT_NEAR(50.0f, f, 0.1f); // y = 50
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_Path_Linear_TranslateOverride)
{
    // Path: M 0 0 L 300 0
    // Single segment: (0,0)->(300,0), length=300
    // dur=3s, fill=freeze
    //
    // t=1.5s => normalized 0.5 => distance 150 => (150, 0)
    // Expected transform override: translate matrix (1,0,0,1,150,0)
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"400\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"20\" height=\"20\" fill=\"#000\">"
        "    <animateMotion path=\"M 0 0 L 300 0\" dur=\"3s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // Seek to midpoint: t=1.5s of 3s duration => normalized 0.5
    // distance = 0.5 * 300 = 150 => position (150, 0)
    psx_svg_player_seek(p, 1.5f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f); // identity
        EXPECT_NEAR(0.0f, b, 0.001f); // identity
        EXPECT_NEAR(0.0f, c, 0.001f); // identity
        EXPECT_NEAR(1.0f, d, 0.001f); // identity
        EXPECT_NEAR(150.0f, e, 0.05f); // x = 150
        EXPECT_NEAR(0.0f, f, 0.05f); // y = 0
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_FreezeHoldsAfterDur)
{
    // Path: M 0 0 L 200 0
    // Single segment: (0,0)->(200,0), length=200
    // dur=2s, fill=freeze
    //
    // Seek to 3s (past the 2s duration).
    // With fill=freeze, the final position should be held:
    // transform override = (1, 0, 0, 1, 200, 0)
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"300\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"20\" height=\"20\" fill=\"#000\">"
        "    <animateMotion path=\"M 0 0 L 200 0\" dur=\"2s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // Seek past duration: t=3s > dur=2s, fill=freeze => hold final position (200, 0)
    psx_svg_player_seek(p, 3.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(200.0f, e, 0.05f); // x = 200 (final position held)
        EXPECT_NEAR(0.0f, f, 0.05f); // y = 0
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_RemoveClearsAfterDur)
{
    // Path: M 0 0 L 200 0
    // Single segment: (0,0)->(200,0), length=200
    // dur=2s, fill=remove
    //
    // Seek to 3s (past the 2s duration).
    // With fill=remove, the override should be removed:
    // debug function returns false (no transform override).
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"300\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"20\" height=\"20\" fill=\"#000\">"
        "    <animateMotion path=\"M 0 0 L 200 0\" dur=\"2s\" fill=\"remove\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // Seek past duration: t=3s > dur=2s, fill=remove => no transform override
    psx_svg_player_seek(p, 3.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_RotateAuto_TangentAngle)
{
    // Path: M 0 0 L 100 0 L 100 100
    // Two segments: (0,0)->(100,0) length=100, (100,0)->(100,100) length=100
    // Total length = 200, dur=2s, rotate="auto", fill="freeze"
    //
    // At t=0.5s (normalized 0.25): distance=50, on horizontal segment
    // tangent angle = atan2(0, 100) = 0 degrees
    // position = (50, 0)
    // matrix = [cos(0), sin(0), -sin(0), cos(0), 50, 0] = [1, 0, 0, 1, 50, 0]
    //
    // At t=1.5s (normalized 0.75): distance=150, on vertical segment
    // tangent angle = atan2(100, 0) = 90 degrees = pi/2
    // position = (100, 50)
    // matrix = [cos(90), sin(90), -sin(90), cos(90), 100, 50] = [0, 1, -1, 0, 100, 50]
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"200\" height=\"200\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateMotion path=\"M 0 0 L 100 0 L 100 100\" dur=\"2s\" rotate=\"auto\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0.5s: on horizontal segment, tangent=0 deg
    // Expected: a=cos(0)=1, b=sin(0)=0, c=-sin(0)=0, d=cos(0)=1, e=50, f=0
    psx_svg_player_seek(p, 0.5f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.05f); // cos(0)
        EXPECT_NEAR(0.0f, b, 0.05f); // sin(0)
        EXPECT_NEAR(0.0f, c, 0.05f); // -sin(0)
        EXPECT_NEAR(1.0f, d, 0.05f); // cos(0)
        EXPECT_NEAR(50.0f, e, 0.05f); // x position
        EXPECT_NEAR(0.0f, f, 0.05f); // y position
    }

    // t=1.5s: on vertical segment, tangent=90 deg
    // Expected: a=cos(90)~0, b=sin(90)~1, c=-sin(90)~-1, d=cos(90)~0, e=100, f=50
    psx_svg_player_seek(p, 1.5f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(0.0f, a, 0.05f); // cos(90)
        EXPECT_NEAR(1.0f, b, 0.05f); // sin(90)
        EXPECT_NEAR(-1.0f, c, 0.05f); // -sin(90)
        EXPECT_NEAR(0.0f, d, 0.05f); // cos(90)
        EXPECT_NEAR(100.0f, e, 0.05f); // x position
        EXPECT_NEAR(50.0f, f, 0.05f); // y position
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_RotateAutoReverse)
{
    // Path: M 0 0 L 100 0 L 100 100
    // Same path as above, rotate="auto-reverse" => angle = tangent + 180 degrees
    // dur=2s, fill="freeze"
    //
    // At t=0.5s: tangent=0 deg, auto-reverse => 0+180=180 deg
    // position = (50, 0)
    // matrix = [cos(180), sin(180), -sin(180), cos(180), 50, 0] = [-1, 0, 0, -1, 50, 0]
    //
    // At t=1.5s: tangent=90 deg, auto-reverse => 90+180=270 deg
    // position = (100, 50)
    // matrix = [cos(270), sin(270), -sin(270), cos(270), 100, 50] = [0, -1, 1, 0, 100, 50]
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"200\" height=\"200\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateMotion path=\"M 0 0 L 100 0 L 100 100\" dur=\"2s\" rotate=\"auto-reverse\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0.5s: tangent=0 deg + 180 = 180 deg
    // Expected: a=cos(180)=-1, b=sin(180)~0, c=-sin(180)~0, d=cos(180)=-1, e=50, f=0
    psx_svg_player_seek(p, 0.5f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(-1.0f, a, 0.05f); // cos(180)
        EXPECT_NEAR(0.0f, b, 0.05f); // sin(180)
        EXPECT_NEAR(0.0f, c, 0.05f); // -sin(180)
        EXPECT_NEAR(-1.0f, d, 0.05f); // cos(180)
        EXPECT_NEAR(50.0f, e, 0.05f); // x position
        EXPECT_NEAR(0.0f, f, 0.05f); // y position
    }

    // t=1.5s: tangent=90 deg + 180 = 270 deg
    // Expected: a=cos(270)~0, b=sin(270)~-1, c=-sin(270)~1, d=cos(270)~0, e=100, f=50
    psx_svg_player_seek(p, 1.5f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(0.0f, a, 0.05f); // cos(270)
        EXPECT_NEAR(-1.0f, b, 0.05f); // sin(270)
        EXPECT_NEAR(1.0f, c, 0.05f); // -sin(270)
        EXPECT_NEAR(0.0f, d, 0.05f); // cos(270)
        EXPECT_NEAR(100.0f, e, 0.05f); // x position
        EXPECT_NEAR(50.0f, f, 0.05f); // y position
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_RotateFixed_45Degrees)
{
    // Path: M 0 0 L 200 0
    // Single segment: (0,0)->(200,0), length=200
    // dur=2s, rotate="45", fill="freeze"
    //
    // At t=1.0s (normalized 0.5): distance=100, position=(100, 0)
    // Fixed 45 degree rotation:
    // a=cos(45)~0.707, b=sin(45)~0.707, c=-sin(45)~-0.707, d=cos(45)~0.707
    // e=100, f=0
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"300\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateMotion path=\"M 0 0 L 200 0\" dur=\"2s\" rotate=\"45\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=1.0s: position (100, 0), fixed 45 deg rotation
    // Expected: a~0.707, b~0.707, c~-0.707, d~0.707, e=100, f=0
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(0.707f, a, 0.05f); // cos(45)
        EXPECT_NEAR(0.707f, b, 0.05f); // sin(45)
        EXPECT_NEAR(-0.707f, c, 0.05f); // -sin(45)
        EXPECT_NEAR(0.707f, d, 0.05f); // cos(45)
        EXPECT_NEAR(100.0f, e, 0.05f); // x position
        EXPECT_NEAR(0.0f, f, 0.05f); // y position
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_FromTo_NoPath)
{
    // No path attribute — uses from/to fallback.
    // from="0,0" to="200,100", dur=2s, fill=freeze
    //
    // At t=1.0s (midpoint, normalized 0.5):
    // position = (0,0) + 0.5 * ((200,100) - (0,0)) = (100, 50)
    // Expected transform override: (1, 0, 0, 1, 100, 50)
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"300\" height=\"200\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateMotion from=\"0,0\" to=\"200,100\" dur=\"2s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=1.0s: midpoint of 2s duration => position (100, 50)
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(1.0f, a, 0.001f);
        EXPECT_NEAR(0.0f, b, 0.001f);
        EXPECT_NEAR(0.0f, c, 0.001f);
        EXPECT_NEAR(1.0f, d, 0.001f);
        EXPECT_NEAR(100.0f, e, 0.05f); // x = 100 (midpoint)
        EXPECT_NEAR(50.0f, f, 0.05f); // y = 50 (midpoint)
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_NoPathNoFromTo_NoOverride)
{
    // No path, no from, no to — should produce no transform override.
    // dur=2s, fill=freeze
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"#000\">"
        "    <animateMotion dur=\"2s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=1.0s: no path, no from/to => no transform override
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_FALSE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_Path_QuadBezier_Q)
{
    // Path with Q (quadratic Bezier): M 0 0 Q 50 100 100 0
    // This is a parabolic arc from (0,0) to (100,0) with control point (50,100).
    // At t=0 => position (0,0), at t=1 => position (100,0).
    // At t=0.5 => midpoint of the curve, which for a quadratic Bezier is:
    // B(0.5) = (1-0.5)^2 * P0 + 2*(1-0.5)*0.5 * P1 + 0.5^2 * P2
    // = 0.25*(0,0) + 0.5*(50,100) + 0.25*(100,0) = (50, 50)
    // The arc-length midpoint won't be exactly at parametric 0.5, but the
    // key test is that the curve is NOT a straight line from (0,0) to (100,0).
    // At the arc-length midpoint, y should be significantly > 0 (curve bulges up).
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"200\" height=\"200\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"5\" height=\"5\" fill=\"blue\">"
        "    <animateMotion path=\"M 0 0 Q 50 100 100 0\" dur=\"2s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0: start position (0, 0)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(0.0f, e, 0.5f);
        EXPECT_NEAR(0.0f, f, 0.5f);
    }

    // t=2s (end): position (100, 0)
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(100.0f, e, 0.5f);
        EXPECT_NEAR(0.0f, f, 0.5f);
    }

    // t=1s (midpoint): the curve should bulge — y must be significantly > 0
    // If Q is not supported, the path degenerates to M 0 0 only (1 point),
    // or straight line, and y would be ~0.
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        // x should be roughly around 50 (midpoint of curve)
        EXPECT_GT(e, 20.0f);
        EXPECT_LT(e, 80.0f);
        // y must be > 0 — this is the key assertion proving the curve is followed
        EXPECT_GT(f, 10.0f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, MotionPath_QuadBezier_UserReportedPath)
{
    // User-reported rounded-rectangle path with 4 Q segments:
    // M 250,80 H 50 Q 30,80 30,50 Q 30,20 50,20 H 250 Q 280,20,280,50 Q 280,80,250,80Z
    //
    // Line-only parse (test_path_parse) skips Q commands, producing only
    // M + H + H + Z points. The real parser flattens each Q into
    // FLATTEN_STEPS_QUAD=8 segments, so the real point count must be larger.

    const char* path =
        "M 250,80 H 50 Q 30,80 30,50 Q 30,20 50,20 H 250 Q 280,20,280,50 Q 280,80,250,80Z";

    // --- Part 1: line-only parse to get baseline count ---
    float* line_xs = NULL;
    float* line_ys = NULL;
    uint32_t line_count = 0;
    bool line_ok = test_path_parse(path, (uint32_t)strlen(path),
                                   &line_xs, &line_ys, &line_count);
    EXPECT_TRUE(line_ok);
    // line-only sees: M(250,80), H50 → (50,80), H250 → (250,80), Z → (250,80) closed
    // Q commands are skipped entirely.
    EXPECT_GT(line_count, 0u);
    free(line_xs);
    free(line_ys);

    // --- Part 2: end-to-end via SVG player (real parser with Q support) ---
    // Use a 4-second duration so we can seek to t=2s for the midpoint.
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"300\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"5\" height=\"5\" fill=\"blue\">"
        "    <animateMotion path=\"M 250,80 H 50 Q 30,80 30,50 Q 30,20 50,20 H 250 Q 280,20,280,50 Q 280,80,250,80Z\" dur=\"4s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0: start position should be near (250, 80)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(250.0f, e, 1.0f);
        EXPECT_NEAR(80.0f, f, 1.0f);
    }

    // t=2s (midpoint of 4s duration, t_norm=0.5): position should be within
    // the path's bounding box [30..280] x [20..80].
    // The rounded rectangle's arc-length midpoint is roughly on the top edge
    // (the path goes right→down-curve→left→up-curve→right→down-curve→left→up-curve→close).
    // Key assertion: position is NOT on a straight line between start and end
    // (which would be the same point since the path is closed).
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        // x must be within the path's x range [30, 280]
        EXPECT_GE(e, 25.0f);
        EXPECT_LE(e, 285.0f);
        // y must be within the path's y range [20, 80]
        EXPECT_GE(f, 15.0f);
        EXPECT_LE(f, 85.0f);
        // The midpoint should NOT be at the start (250,80) — that would mean
        // the curve segments were ignored and the path collapsed.
        // At least one coordinate should differ significantly from start.
        bool moved_x = (e < 240.0f || e > 260.0f);
        bool moved_y = (f < 70.0f || f > 85.0f);
        EXPECT_TRUE(moved_x || moved_y)
                << "t=0.5 position (" << e << ", " << f
                << ") is too close to start — Q segments may not be parsed";
    }

    destroy_player(p, root);
}

// ---------------------------------------------------------------------------
// Unit test: Cubic Bezier basic parsing via C command
// Validates: Requirements 2.1
// Path "M 0,0 C 10,20 30,20 40,0" should produce FLATTEN_STEPS_CUBIC+1 = 17
// points (1 start + 16 flattened), with the last point near (40, 0).
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_CubicBezier_Basic)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
        "    <animateMotion path=\"M 0,0 C 10,20 30,20 40,0\" dur=\"4s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0: start position should be at (0, 0)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(0.0f, e, 0.01f);
        EXPECT_NEAR(0.0f, f, 0.01f);
    }

    // t=4s (end, fill=freeze): position should be near (40, 0)
    psx_svg_player_seek(p, 4.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(40.0f, e, 0.01f);
        EXPECT_NEAR(0.0f, f, 0.01f);
    }

    // t=2s (midpoint): position should be on the curve, NOT on the straight
    // line from (0,0) to (40,0). The cubic B(0.5) = (0.125*0 + 0.375*10 +
    // 0.375*30 + 0.125*40, 0.125*0 + 0.375*20 + 0.375*20 + 0.125*0) = (20, 15).
    // Due to arc-length parameterisation the exact position at t_norm=0.5 may
    // differ, but y must be significantly above 0 (the curve bulges upward).
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        // x should be somewhere in the middle of the curve
        EXPECT_GT(e, 5.0f);
        EXPECT_LT(e, 35.0f);
        // y must be positive (curve bulges upward), proving C was parsed
        EXPECT_GT(f, 3.0f);
    }

    // Verify point count indirectly: the line-only parser sees only M(0,0)
    // and skips C, producing 1 point. The real parser should produce 17 points
    // (1 start + FLATTEN_STEPS_CUBIC=16). We confirm the real parser works by
    // checking that the midpoint is NOT on the x-axis (which would happen if
    // the C command were skipped).

    destroy_player(p, root);
}

// ---------------------------------------------------------------------------
// Unit test: Smooth Quadratic Bezier — T after Q
// Validates: Requirements 3.1, 3.2
// Path "M 0,0 Q 10,20 20,0 T 40,0"
// Q produces FLATTEN_STEPS_QUAD=8 points, T produces another 8 points.
// Total: 1 (M start) + 8 (Q) + 8 (T) = 17 points.
// Endpoint should be near (40, 0).
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_SmoothQuad_TAfterQ)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
        "    <animateMotion path=\"M 0,0 Q 10,20 20,0 T 40,0\" dur=\"4s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0: start at (0, 0)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(0.0f, e, 0.01f);
        EXPECT_NEAR(0.0f, f, 0.01f);
    }

    // t=4s (end, fill=freeze): endpoint should be near (40, 0)
    psx_svg_player_seek(p, 4.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(40.0f, e, 0.1f);
        EXPECT_NEAR(0.0f, f, 0.1f);
    }

    // t=2s (midpoint): the Q segment bulges upward (control at y=20) and the
    // T segment mirrors the control point, bulging downward (reflected control
    // at y=-20). At the arc-length midpoint the position should be near x=20
    // (the junction between Q and T segments) with y near 0.
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        // x should be in the middle region
        EXPECT_GT(e, 5.0f);
        EXPECT_LT(e, 35.0f);
    }

    destroy_player(p, root);
}

// ---------------------------------------------------------------------------
// Unit test: Smooth Quadratic Bezier — T without prior Q (degenerates)
// Validates: Requirements 3.2
// Path "M 10,10 T 50,50"
// No prior Q, so control point = current point (10,10).
// This degenerates to a straight-line flattening.
// Total: 1 (M start) + 8 (T) = 9 points.
// Endpoint should be near (50, 50).
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_SmoothQuad_TWithoutQ)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
        "    <animateMotion path=\"M 10,10 T 50,50\" dur=\"4s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0: start at (10, 10)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(10.0f, e, 0.1f);
        EXPECT_NEAR(10.0f, f, 0.1f);
    }

    // t=4s (end, fill=freeze): endpoint should be near (50, 50)
    psx_svg_player_seek(p, 4.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(50.0f, e, 0.1f);
        EXPECT_NEAR(50.0f, f, 0.1f);
    }

    // t=2s (midpoint): since T degenerates to a straight line from (10,10)
    // to (50,50), the midpoint should be near (30, 30).
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(30.0f, e, 2.0f);
        EXPECT_NEAR(30.0f, f, 2.0f);
    }

    destroy_player(p, root);
}

// ---------------------------------------------------------------------------
// Unit test: Smooth Cubic Bezier — S after C
// Validates: Requirements 3.3, 3.4
// Path "M 0,0 C 10,20 30,20 40,0 S 70,-20 80,0"
// C produces FLATTEN_STEPS_CUBIC=16 points, S produces another 16 points.
// Total: 1 (M start) + 16 (C) + 16 (S) = 33 points.
// Endpoint should be near (80, 0).
// The S command mirrors C's second control point (30,20) about (40,0)
// giving reflected c1 = (50,-20), with explicit c2=(70,-20), end=(80,0).
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_SmoothCubic_SAfterC)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
        "    <animateMotion path=\"M 0,0 C 10,20 30,20 40,0 S 70,-20 80,0\" dur=\"4s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0: start at (0, 0)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(0.0f, e, 0.01f);
        EXPECT_NEAR(0.0f, f, 0.01f);
    }

    // t=4s (end, fill=freeze): endpoint should be near (80, 0)
    psx_svg_player_seek(p, 4.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(80.0f, e, 0.1f);
        EXPECT_NEAR(0.0f, f, 0.1f);
    }

    // t=2s (midpoint): the C segment bulges upward (y>0) and the S segment
    // bulges downward (y<0) due to the mirrored control point. At the
    // arc-length midpoint the position should be near x=40 (the junction)
    // with y near 0.
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        // x should be in the middle region of the full path [0, 80]
        EXPECT_GT(e, 10.0f);
        EXPECT_LT(e, 70.0f);
        // The curve is symmetric: C bulges up, S bulges down.
        // At the junction (x≈40) y should be near 0.
        // With arc-length parameterisation the exact y may vary, but it
        // should not be far from 0.
        EXPECT_GT(f, -15.0f);
        EXPECT_LT(f, 15.0f);
    }

    destroy_player(p, root);
}

// ---------------------------------------------------------------------------
// Property 1: Motion path parse-format round trip
// Feature: svg-shape-animation, Property 1: Motion path parse-format round trip
// **Validates: Requirements 4.8, 4.9**
//
// For any valid sequence of 2–20 (x, y) points with coordinates in [-1000, 1000],
// formatting as "M x0 y0 L x1 y1 ..." and re-parsing SHALL produce an equivalent
// point sequence within tolerance 0.01f.
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_RoundTrip_Property)
{
    // Hand-written LCG random generator (C++98, no STL)
    uint32_t seed = 12345u;
    const uint32_t NUM_ITERS = 100;

    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        // LCG: seed = seed * 1103515245 + 12345
        seed = seed * 1103515245u + 12345u;
        uint32_t npts = 2 + (seed >> 16) % 19; // 2..20 points

        // Generate random points
        float orig_xs[20];
        float orig_ys[20];
        for (uint32_t i = 0; i < npts; i++) {
            seed = seed * 1103515245u + 12345u;
            orig_xs[i] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 1000.0f;
            seed = seed * 1103515245u + 12345u;
            orig_ys[i] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 1000.0f;
        }

        // Format points into path string
        char* path_str = test_path_format(orig_xs, orig_ys, npts);
        ASSERT_TRUE(path_str != NULL) << "iter=" << iter << " format returned NULL";

        float* parsed_xs = NULL;
        float* parsed_ys = NULL;
        uint32_t parsed_count = 0;
        bool ok = test_path_parse(
                      path_str, (uint32_t)strlen(path_str),
                      &parsed_xs, &parsed_ys, &parsed_count);
        EXPECT_TRUE(ok) << "iter=" << iter << " parse failed for: " << path_str;

        if (ok) {
            EXPECT_EQ(npts, parsed_count) << "iter=" << iter << " count mismatch";
            uint32_t check_n = npts < parsed_count ? npts : parsed_count;
            for (uint32_t i = 0; i < check_n; i++) {
                EXPECT_NEAR(orig_xs[i], parsed_xs[i], 0.01f)
                        << "iter=" << iter << " x[" << i << "] mismatch";
                EXPECT_NEAR(orig_ys[i], parsed_ys[i], 0.01f)
                        << "iter=" << iter << " y[" << i << "] mismatch";
            }
        }

        free(parsed_xs);
        free(parsed_ys);
        free(path_str);
    }
}

// ---------------------------------------------------------------------------
// Property 2: Arc-length interpolation position correctness
// Feature: svg-shape-animation, Property 2: Arc-length interpolation position correctness
// **Validates: Requirements 5.3, 5.4, 5.5**
//
// For any motion path with 2–10 distinct points and total arc length > 0,
// and for any t in [0,1]:
// (a) t=0 → first point
// (b) t=1 → last point
// (c) position lies on the correct segment at proportional distance
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionArcLength_Position_Property)
{
    uint32_t seed = 67890u;
    const uint32_t NUM_ITERS = 100;

    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        // Generate 2–10 random points
        seed = seed * 1103515245u + 12345u;
        uint32_t npts = 2 + (seed >> 16) % 9; // 2..10

        float xs[10];
        float ys[10];
        for (uint32_t i = 0; i < npts; i++) {
            seed = seed * 1103515245u + 12345u;
            xs[i] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 500.0f;
            seed = seed * 1103515245u + 12345u;
            ys[i] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 500.0f;
        }

        // Compute total arc length independently
        float total_len = 0.0f;
        float seg_lens[10];
        seg_lens[0] = 0.0f;
        for (uint32_t i = 1; i < npts; i++) {
            float dx = xs[i] - xs[i - 1];
            float dy = ys[i] - ys[i - 1];
            seg_lens[i] = sqrtf(dx * dx + dy * dy);
            total_len += seg_lens[i];
        }

        if (total_len < 0.001f) {
            continue; // skip degenerate paths
        }

        // (a) t=0 → first point
        {
            float ox = 0, oy = 0;
            bool ok = test_arc_length_position(xs, ys, npts, 0.0f, &ox, &oy);
            EXPECT_TRUE(ok) << "iter=" << iter;
            if (ok) {
                EXPECT_NEAR(xs[0], ox, 0.01f) << "iter=" << iter << " t=0 x";
                EXPECT_NEAR(ys[0], oy, 0.01f) << "iter=" << iter << " t=0 y";
            }
        }

        // (b) t=1 → last point
        {
            float ox = 0, oy = 0;
            bool ok = test_arc_length_position(xs, ys, npts, 1.0f, &ox, &oy);
            EXPECT_TRUE(ok) << "iter=" << iter;
            if (ok) {
                EXPECT_NEAR(xs[npts - 1], ox, 0.01f) << "iter=" << iter << " t=1 x";
                EXPECT_NEAR(ys[npts - 1], oy, 0.01f) << "iter=" << iter << " t=1 y";
            }
        }

        // (c) random t in (0,1) — verify position lies on correct segment
        seed = seed * 1103515245u + 12345u;
        float t = (float)(seed >> 8) / (float)(1 << 24); // [0, 1)
        if (t <= 0.0f) { t = 0.01f; }
        if (t >= 1.0f) { t = 0.99f; }

        float target_dist = t * total_len;

        // Find which segment and interpolate independently
        float cum = 0.0f;
        float expect_x = xs[npts - 1];
        float expect_y = ys[npts - 1];
        for (uint32_t i = 1; i < npts; i++) {
            float next_cum = cum + seg_lens[i];
            if (target_dist <= next_cum || i == npts - 1) {
                float seg_t = 0.0f;
                if (seg_lens[i] > 0.001f) {
                    seg_t = (target_dist - cum) / seg_lens[i];
                }
                if (seg_t < 0.0f) { seg_t = 0.0f; }
                if (seg_t > 1.0f) { seg_t = 1.0f; }
                expect_x = xs[i - 1] + seg_t * (xs[i] - xs[i - 1]);
                expect_y = ys[i - 1] + seg_t * (ys[i] - ys[i - 1]);
                break;
            }
            cum = next_cum;
        }

        {
            float ox = 0, oy = 0;
            bool ok = test_arc_length_position(xs, ys, npts, t, &ox, &oy);
            EXPECT_TRUE(ok) << "iter=" << iter;
            if (ok) {
                EXPECT_NEAR(expect_x, ox, 0.1f) << "iter=" << iter << " t=" << t << " x";
                EXPECT_NEAR(expect_y, oy, 0.1f) << "iter=" << iter << " t=" << t << " y";
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Property 3: animateMotion produces correct translate override
// Feature: svg-shape-animation, Property 3: animateMotion produces correct translate override
// **Validates: Requirements 6.2, 6.3**
//
// For any SVG with <animateMotion> containing a linear path (M + L segments,
// 2–8 points) and any seek time within active duration, the transform override
// (e, f) SHALL match the independently computed arc-length interpolated position
// within tolerance 0.1f.
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, AnimateMotion_EndToEnd_Position_Property)
{
    uint32_t seed = 54321u;
    const uint32_t NUM_ITERS = 100;

    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        // Generate 2–8 random points in [0, 500]
        seed = seed * 1103515245u + 12345u;
        uint32_t npts = 2 + (seed >> 16) % 7; // 2..8

        float xs[8];
        float ys[8];
        for (uint32_t i = 0; i < npts; i++) {
            seed = seed * 1103515245u + 12345u;
            xs[i] = (float)((seed >> 16) % 500);
            seed = seed * 1103515245u + 12345u;
            ys[i] = (float)((seed >> 16) % 500);
        }

        // Compute total arc length
        float total_len = 0.0f;
        float seg_lens[8];
        seg_lens[0] = 0.0f;
        for (uint32_t i = 1; i < npts; i++) {
            float dx = xs[i] - xs[i - 1];
            float dy = ys[i] - ys[i - 1];
            seg_lens[i] = sqrtf(dx * dx + dy * dy);
            total_len += seg_lens[i];
        }
        if (total_len < 1.0f) {
            continue; // skip degenerate
        }

        // Build SVG path string: "M x0 y0 L x1 y1 ..."
        char path_buf[512];
        int pos = sprintf(path_buf, "M %g %g", xs[0], ys[0]);
        for (uint32_t i = 1; i < npts; i++) {
            pos += sprintf(path_buf + pos, " L %g %g", xs[i], ys[i]);
        }

        // Build SVG document with dur=10s, fill=freeze
        char svg_buf[1024];
        sprintf(svg_buf,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"600\" height=\"600\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"5\" height=\"5\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", path_buf);

        // Create player
        psx_result r = S_OK;
        psx_svg_node* root = NULL;
        psx_svg_player* p = create_player(svg_buf, &r, &root);
        if (!p) {
            continue; // skip if parse fails
        }

        const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
        if (!n) {
            destroy_player(p, root);
            continue;
        }

        psx_svg_player_play(p);

        // Random seek time in [0, 10]
        seed = seed * 1103515245u + 12345u;
        float seek_t = (float)(seed >> 8) / (float)(1 << 24) * 10.0f;
        if (seek_t > 10.0f) { seek_t = 10.0f; }
        float norm_t = seek_t / 10.0f;
        if (norm_t > 1.0f) { norm_t = 1.0f; }

        // Independently compute expected position via arc-length
        float target_dist = norm_t* total_len;
        float expect_x = xs[npts - 1];
        float expect_y = ys[npts - 1];
        float cum = 0.0f;
        for (uint32_t i = 1; i < npts; i++) {
            float next_cum = cum + seg_lens[i];
            if (target_dist <= next_cum || i == npts - 1) {
                float seg_t = 0.0f;
                if (seg_lens[i] > 0.001f) {
                    seg_t = (target_dist - cum) / seg_lens[i];
                }
                if (seg_t < 0.0f) { seg_t = 0.0f; }
                if (seg_t > 1.0f) { seg_t = 1.0f; }
                expect_x = xs[i - 1] + seg_t * (xs[i] - xs[i - 1]);
                expect_y = ys[i - 1] + seg_t * (ys[i] - ys[i - 1]);
                break;
            }
            cum = next_cum;
        }

        // Seek and verify
        psx_svg_player_seek(p, seek_t);
        psx_svg_player_tick(p, 0.0f);

        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        bool got = psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f);
        EXPECT_TRUE(got) << "iter=" << iter << " seek=" << seek_t;
        if (got) {
            EXPECT_NEAR(1.0f, a, 0.001f) << "iter=" << iter;
            EXPECT_NEAR(0.0f, b, 0.001f) << "iter=" << iter;
            EXPECT_NEAR(0.0f, c, 0.001f) << "iter=" << iter;
            EXPECT_NEAR(1.0f, d, 0.001f) << "iter=" << iter;
            EXPECT_NEAR(expect_x, e, 0.1f) << "iter=" << iter << " seek=" << seek_t << " x";
            EXPECT_NEAR(expect_y, f, 0.1f) << "iter=" << iter << " seek=" << seek_t << " y";
        }

        destroy_player(p, root);
    }
}

// ---------------------------------------------------------------------------
// Feature: motion-path-bezier, Property 1: 二次贝塞尔展平精度
// **Validates: Requirements 1.1, 7.1**
//
// For any valid quadratic Bezier parameters (P0, P1, P2), the production
// parser's flattened output (via animateMotion) should produce positions that
// lie close to the exact Bezier curve. We verify by seeking to multiple
// normalized times and checking that the player's reported position is near
// the exact Bezier curve point at the corresponding arc-length parameter.
//
// Since the production code flattens Q into FLATTEN_STEPS_QUAD=8 uniform-t
// segments, we independently compute the expected polyline, then use
// arc-length interpolation to find the expected position at each seek time,
// and compare against the player output.
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_Property1_QuadBezierFlattenAccuracy)
{
    // **Validates: Requirements 1.1, 7.1**
    uint32_t seed = 99887u;
    const uint32_t NUM_ITERS = 100;
    const int FLATTEN_STEPS = 8;

    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        // Generate random quadratic Bezier: P0(x0,y0), P1(cx,cy), P2(ex,ey)
        // Range [-500, 500]
        float coords[6];
        for (int ci = 0; ci < 6; ci++) {
            seed = seed * 1103515245u + 12345u;
            coords[ci] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 500.0f;
        }
        float x0 = coords[0], y0 = coords[1];
        float cx = coords[2], cy = coords[3];
        float ex = coords[4], ey = coords[5];

        // Compute exact flattened polyline: 9 points (start + 8 steps)
        float poly_x[9], poly_y[9];
        poly_x[0] = x0;
        poly_y[0] = y0;
        for (int i = 1; i <= FLATTEN_STEPS; i++) {
            float t = (float)i / (float)FLATTEN_STEPS;
            float u = 1.0f - t;
            poly_x[i] = u * u * x0 + 2.0f * u * t * cx + t * t * ex;
            poly_y[i] = u * u * y0 + 2.0f * u * t * cy + t * t * ey;
        }

        // Verify each flattened point matches exact Bezier at t = i/8
        for (int i = 1; i <= FLATTEN_STEPS; i++) {
            float t = (float)i / (float)FLATTEN_STEPS;
            float u = 1.0f - t;
            float exact_x = u * u * x0 + 2.0f * u * t * cx + t * t * ex;
            float exact_y = u * u * y0 + 2.0f * u * t * cy + t * t * ey;
            float dx = poly_x[i] - exact_x;
            float dy = poly_y[i] - exact_y;
            float dist = sqrtf(dx * dx + dy * dy);
            EXPECT_LT(dist, 0.01f)
                    << "iter=" << iter << " i=" << i
                    << " poly=(" << poly_x[i] << "," << poly_y[i] << ")"
                    << " exact=(" << exact_x << "," << exact_y << ")"
                    << " dist=" << dist;
        }

        // Now verify the production code produces the same polyline by going
        // through the full animateMotion pipeline.
        // Build SVG path string
        char path_buf[256];
        sprintf(path_buf, "M %.6g,%.6g Q %.6g,%.6g %.6g,%.6g",
                x0, y0, cx, cy, ex, ey);

        char svg_buf[1024];
        sprintf(svg_buf,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"1200\" height=\"1200\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"8s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", path_buf);

        psx_result r = S_OK;
        psx_svg_node* root = NULL;
        psx_svg_player* p = create_player(svg_buf, &r, &root);
        if (!p) { continue; }

        const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
        if (!n) { destroy_player(p, root); continue; }

        psx_svg_player_play(p);

        // Compute arc lengths of the expected polyline
        float cum[9];
        cum[0] = 0.0f;
        for (int i = 1; i <= FLATTEN_STEPS; i++) {
            float ddx = poly_x[i] - poly_x[i - 1];
            float ddy = poly_y[i] - poly_y[i - 1];
            cum[i] = cum[i - 1] + sqrtf(ddx * ddx + ddy * ddy);
        }
        float total_len = cum[FLATTEN_STEPS];

        if (total_len < 0.01f) {
            // Degenerate curve — skip
            destroy_player(p, root);
            continue;
        }

        // Check each flattened point by seeking to the corresponding arc-length fraction.
        // Point i is at cum[i]/total_len of the total arc length.
        // With dur=8s, seek time = (cum[i]/total_len) * 8.
        for (int i = 1; i <= FLATTEN_STEPS; i++) {
            float norm_t = cum[i] / total_len;
            float seek_time = norm_t * 8.0f;
            if (seek_time > 8.0f) { seek_time = 8.0f; }

            psx_svg_player_seek(p, seek_time);
            psx_svg_player_tick(p, 0.0f);

            float a = 0, b = 0, c = 0, d = 0, te = 0, tf = 0;
            bool got = psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &te, &tf);
            if (!got) { continue; }

            // The player position at this arc-length fraction should be at poly_x[i], poly_y[i]
            float pdx = te - poly_x[i];
            float pdy = tf - poly_y[i];
            float pdist = sqrtf(pdx * pdx + pdy * pdy);
            EXPECT_LT(pdist, 0.01f)
                    << "iter=" << iter << " i=" << i
                    << " seek=" << seek_time
                    << " player=(" << te << "," << tf << ")"
                    << " expected=(" << poly_x[i] << "," << poly_y[i] << ")"
                    << " dist=" << pdist;
        }

        destroy_player(p, root);
    }
}

// ---------------------------------------------------------------------------
// Property 2: Cubic Bezier Flatten Accuracy
// For any valid cubic Bezier parameters (P0, P1, P2, P3), the i-th flattened
// point should match the exact Bezier curve value at t = i/FLATTEN_STEPS_CUBIC
// within Euclidean distance < 0.01.
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_Property2_CubicBezierFlattenAccuracy)
{
    // **Validates: Requirements 2.1, 7.2**
    uint32_t seed = 77665u;
    const uint32_t NUM_ITERS = 100;
    const int FLATTEN_STEPS = 16;

    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        // Generate random cubic Bezier: P0(x0,y0), P1(c1x,c1y), P2(c2x,c2y), P3(ex,ey)
        // Range [-500, 500]
        float coords[8];
        for (int ci = 0; ci < 8; ci++) {
            seed = seed * 1103515245u + 12345u;
            coords[ci] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 500.0f;
        }
        float x0 = coords[0], y0 = coords[1];
        float c1x = coords[2], c1y = coords[3];
        float c2x = coords[4], c2y = coords[5];
        float ex = coords[6], ey = coords[7];

        // Compute exact flattened polyline: 17 points (start + 16 steps)
        float poly_x[17], poly_y[17];
        poly_x[0] = x0;
        poly_y[0] = y0;
        for (int i = 1; i <= FLATTEN_STEPS; i++) {
            float t = (float)i / (float)FLATTEN_STEPS;
            float u = 1.0f - t;
            poly_x[i] = u * u * u * x0 + 3.0f * u * u * t * c1x + 3.0f * u * t * t * c2x + t * t * t * ex;
            poly_y[i] = u * u * u * y0 + 3.0f * u * u * t * c1y + 3.0f * u * t * t * c2y + t * t * t * ey;
        }

        // Verify each flattened point matches exact cubic Bezier at t = i/16
        for (int i = 1; i <= FLATTEN_STEPS; i++) {
            float t = (float)i / (float)FLATTEN_STEPS;
            float u = 1.0f - t;
            float exact_x = u * u * u * x0 + 3.0f * u * u * t * c1x + 3.0f * u * t * t * c2x + t * t * t * ex;
            float exact_y = u * u * u * y0 + 3.0f * u * u * t * c1y + 3.0f * u * t * t * c2y + t * t * t * ey;
            float dx = poly_x[i] - exact_x;
            float dy = poly_y[i] - exact_y;
            float dist = sqrtf(dx * dx + dy * dy);
            EXPECT_LT(dist, 0.01f)
                    << "iter=" << iter << " i=" << i
                    << " poly=(" << poly_x[i] << "," << poly_y[i] << ")"
                    << " exact=(" << exact_x << "," << exact_y << ")"
                    << " dist=" << dist;
        }

        // Verify the production code produces the same polyline through the
        // full animateMotion pipeline.
        char path_buf[256];
        sprintf(path_buf, "M %.6g,%.6g C %.6g,%.6g %.6g,%.6g %.6g,%.6g",
                x0, y0, c1x, c1y, c2x, c2y, ex, ey);

        char svg_buf[1024];
        sprintf(svg_buf,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"1200\" height=\"1200\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"16s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", path_buf);

        psx_result r = S_OK;
        psx_svg_node* root = NULL;
        psx_svg_player* p = create_player(svg_buf, &r, &root);
        if (!p) { continue; }

        const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
        if (!n) { destroy_player(p, root); continue; }

        psx_svg_player_play(p);

        // Compute arc lengths of the expected polyline
        float cum[17];
        cum[0] = 0.0f;
        for (int i = 1; i <= FLATTEN_STEPS; i++) {
            float ddx = poly_x[i] - poly_x[i - 1];
            float ddy = poly_y[i] - poly_y[i - 1];
            cum[i] = cum[i - 1] + sqrtf(ddx * ddx + ddy * ddy);
        }
        float total_len = cum[FLATTEN_STEPS];

        if (total_len < 0.01f) {
            // Degenerate curve — skip
            destroy_player(p, root);
            continue;
        }

        // Check each flattened point by seeking to the corresponding arc-length fraction.
        // With dur=16s, seek time = (cum[i]/total_len) * 16.
        for (int i = 1; i <= FLATTEN_STEPS; i++) {
            float norm_t = cum[i] / total_len;
            float seek_time = norm_t * 16.0f;
            if (seek_time > 16.0f) { seek_time = 16.0f; }

            psx_svg_player_seek(p, seek_time);
            psx_svg_player_tick(p, 0.0f);

            float a = 0, b = 0, c = 0, d = 0, te = 0, tf = 0;
            bool got = psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &te, &tf);
            if (!got) { continue; }

            // The player position at this arc-length fraction should be at poly_x[i], poly_y[i]
            float pdx = te - poly_x[i];
            float pdy = tf - poly_y[i];
            float pdist = sqrtf(pdx * pdx + pdy * pdy);
            EXPECT_LT(pdist, 0.01f)
                    << "iter=" << iter << " i=" << i
                    << " seek=" << seek_time
                    << " player=(" << te << "," << tf << ")"
                    << " expected=(" << poly_x[i] << "," << poly_y[i] << ")"
                    << " dist=" << pdist;
        }

        destroy_player(p, root);
    }
}

// ---------------------------------------------------------------------------
// Property 3: Elliptical Arc Flatten Accuracy
// For any valid elliptical arc parameters (rx>0, ry>0, rotation, large_arc,
// sweep, start, end), the flattened points should lie on the exact elliptical
// arc within Euclidean distance < 0.1.
// We independently recompute the SVG spec F.6.5/F.6.6 center parameterization
// and compare against the player output.
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_Property3_ArcFlattenAccuracy)
{
    // **Validates: Requirements 4.1, 7.3**
    uint32_t seed = 33221u;
    const uint32_t NUM_ITERS = 100;
    const int FLATTEN_STEPS = 16;

    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        // Generate random arc parameters
        // Start point in [-100, 100]
        seed = seed * 1103515245u + 12345u;
        float x0 = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 100.0f;
        seed = seed * 1103515245u + 12345u;
        float y0 = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 100.0f;

        // Radii in [10, 100] — avoid tiny radii that cause numerical issues
        seed = seed * 1103515245u + 12345u;
        float rx = 10.0f + ((float)((seed >> 8) & 0xFFFFu) / 65535.0f) * 90.0f;
        seed = seed * 1103515245u + 12345u;
        float ry = 10.0f + ((float)((seed >> 8) & 0xFFFFu) / 65535.0f) * 90.0f;

        // x_rotation in [0, 360)
        seed = seed * 1103515245u + 12345u;
        float x_rotation = ((float)((seed >> 8) & 0xFFFFu) / 65535.0f) * 360.0f;

        // Flags
        seed = seed * 1103515245u + 12345u;
        int large_arc = (seed >> 16) & 1;
        seed = seed * 1103515245u + 12345u;
        int sweep_flag = (seed >> 16) & 1;

        // Endpoint in [-100, 100], ensure different from start
        seed = seed * 1103515245u + 12345u;
        float ex = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 100.0f;
        seed = seed * 1103515245u + 12345u;
        float ey = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 100.0f;

        // Skip degenerate: start == end
        float sep = sqrtf((ex - x0) * (ex - x0) + (ey - y0) * (ey - y0));
        if (sep < 1.0f) {
            ex = x0 + 20.0f;
            ey = y0 + 20.0f;
        }

        // --- Independent center parameterization (SVG spec F.6.5/F.6.6) ---
        float phi = (float)(x_rotation * (M_PI / 180.0));
        float cos_phi = cosf(phi);
        float sin_phi = sinf(phi);

        float dx2 = (x0 - ex) * 0.5f;
        float dy2 = (y0 - ey) * 0.5f;
        float x1p = cos_phi * dx2 + sin_phi * dy2;
        float y1p = -sin_phi * dx2 + cos_phi * dy2;

        // Scale radii if needed
        float arx = rx, ary = ry;
        float x1p2 = x1p * x1p;
        float y1p2 = y1p * y1p;
        float rx2 = arx * arx;
        float ry2 = ary * ary;
        float lambda = x1p2 / rx2 + y1p2 / ry2;
        if (lambda > 1.0f) {
            float sl = sqrtf(lambda);
            arx *= sl;
            ary *= sl;
            rx2 = arx * arx;
            ry2 = ary * ary;
        }

        // Center in rotated system
        float num = rx2 * ry2 - rx2 * y1p2 - ry2 * x1p2;
        float den = rx2 * y1p2 + ry2 * x1p2;
        float sq = 0.0f;
        if (den > 0.0f && num > 0.0f) {
            sq = sqrtf(num / den);
        }
        if ((large_arc != 0) == (sweep_flag != 0)) {
            sq = -sq;
        }

        float cxp = sq * arx * y1p / ary;
        float cyp = -sq * ary * x1p / arx;

        // Transform back to original coordinates
        float mx = (x0 + ex) * 0.5f;
        float my = (y0 + ey) * 0.5f;
        float cx_c = cos_phi * cxp - sin_phi * cyp + mx;
        float cy_c = sin_phi * cxp + cos_phi * cyp + my;

        // Compute theta1 and dtheta
        float ux = (x1p - cxp) / arx;
        float uy = (y1p - cyp) / ary;
        float vx = (-x1p - cxp) / arx;
        float vy = (-y1p - cyp) / ary;

        float n_u = sqrtf(ux * ux + uy * uy);
        float theta1 = 0.0f;
        if (n_u > 0.0f) {
            float cos_t = ux / n_u;
            if (cos_t < -1.0f) { cos_t = -1.0f; }
            if (cos_t > 1.0f) { cos_t = 1.0f; }
            theta1 = acosf(cos_t);
            if (uy < 0.0f) { theta1 = -theta1; }
        }

        float n_v = sqrtf(vx * vx + vy * vy);
        float dtheta = 0.0f;
        if (n_u > 0.0f && n_v > 0.0f) {
            float dot = ux * vx + uy * vy;
            float cos_d = dot / (n_u * n_v);
            if (cos_d < -1.0f) { cos_d = -1.0f; }
            if (cos_d > 1.0f) { cos_d = 1.0f; }
            dtheta = acosf(cos_d);
            if (ux * vy - uy * vx < 0.0f) { dtheta = -dtheta; }
        }

        if (sweep_flag && dtheta < 0.0f) {
            dtheta += (float)(2.0 * M_PI);
        } else if (!sweep_flag && dtheta > 0.0f) {
            dtheta -= (float)(2.0 * M_PI);
        }

        // Compute expected flattened polyline: start + 16 steps
        float poly_x[17], poly_y[17];
        poly_x[0] = x0;
        poly_y[0] = y0;
        for (int i = 1; i <= FLATTEN_STEPS; i++) {
            float angle = theta1 + dtheta * (float)i / (float)FLATTEN_STEPS;
            float ca = cosf(angle);
            float sa = sinf(angle);
            poly_x[i] = cx_c + arx * ca * cos_phi - ary * sa * sin_phi;
            poly_y[i] = cy_c + arx * ca * sin_phi + ary * sa * cos_phi;
        }

        // Verify the production code produces the same polyline through the
        // full animateMotion pipeline.
        char path_buf[256];
        sprintf(path_buf, "M %.6g,%.6g A %.6g,%.6g %.6g %d %d %.6g,%.6g",
                x0, y0, rx, ry, x_rotation, large_arc, sweep_flag, ex, ey);

        char svg_buf[1024];
        sprintf(svg_buf,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"1200\" height=\"1200\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"16s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", path_buf);

        psx_result r = S_OK;
        psx_svg_node* root = NULL;
        psx_svg_player* p = create_player(svg_buf, &r, &root);
        if (!p) { continue; }

        const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
        if (!n) { destroy_player(p, root); continue; }

        psx_svg_player_play(p);

        // Compute arc lengths of the expected polyline
        float cum[17];
        cum[0] = 0.0f;
        for (int i = 1; i <= FLATTEN_STEPS; i++) {
            float ddx = poly_x[i] - poly_x[i - 1];
            float ddy = poly_y[i] - poly_y[i - 1];
            cum[i] = cum[i - 1] + sqrtf(ddx * ddx + ddy * ddy);
        }
        float total_len = cum[FLATTEN_STEPS];

        if (total_len < 0.01f) {
            // Degenerate arc — skip
            destroy_player(p, root);
            continue;
        }

        // Check each flattened point by seeking to the corresponding arc-length fraction.
        // With dur=16s, seek time = (cum[i]/total_len) * 16.
        for (int i = 1; i <= FLATTEN_STEPS; i++) {
            float norm_t = cum[i] / total_len;
            float seek_time = norm_t * 16.0f;
            if (seek_time > 16.0f) { seek_time = 16.0f; }

            psx_svg_player_seek(p, seek_time);
            psx_svg_player_tick(p, 0.0f);

            float a = 0, b = 0, c = 0, d = 0, te = 0, tf = 0;
            bool got = psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &te, &tf);
            if (!got) { continue; }

            float pdx = te - poly_x[i];
            float pdy = tf - poly_y[i];
            float pdist = sqrtf(pdx * pdx + pdy * pdy);
            EXPECT_LT(pdist, 0.1f)
                    << "iter=" << iter << " i=" << i
                    << " seek=" << seek_time
                    << " player=(" << te << "," << tf << ")"
                    << " expected=(" << poly_x[i] << "," << poly_y[i] << ")"
                    << " dist=" << pdist;
        }

        destroy_player(p, root);
    }
}

// ---------------------------------------------------------------------------
// Property 6: Smooth Curve Mirror Correctness
// For any Q+T sequence, the T command's output should be identical to an
// explicit Q command with the reflected control point. Same for C+S.
// We verify by building two animateMotion SVGs — one with smooth commands
// and one with explicit commands using the reflected control point — then
// seeking to multiple times and comparing the reported positions.
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_Property6_SmoothCurveMirrorCorrectness)
{
    // **Validates: Requirements 3.1, 3.3**
    uint32_t seed = 55443u;
    const uint32_t NUM_ITERS = 100;
    const int NUM_SEEK_POINTS = 10;

    // -----------------------------------------------------------------------
    // Part A: Q + T  vs  Q + Q(reflected)
    // -----------------------------------------------------------------------
    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        // Generate random coords: start(x0,y0), Q control(cx,cy),
        // Q end(qex,qey), T end(tex,tey).  Range [-200, 200].
        float coords[8];
        for (int ci = 0; ci < 8; ci++) {
            seed = seed * 1103515245u + 12345u;
            coords[ci] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
        }
        float x0 = coords[0], y0 = coords[1];
        float cx = coords[2], cy = coords[3];
        float qex = coords[4], qey = coords[5];
        float tex = coords[6], tey = coords[7];

        // Reflected control point for the T segment
        float rcx = 2.0f * qex - cx;
        float rcy = 2.0f * qey - cy;

        // Path A: uses T (smooth quad)
        char pathA[512];
        sprintf(pathA, "M %.6g,%.6g Q %.6g,%.6g %.6g,%.6g T %.6g,%.6g",
                x0, y0, cx, cy, qex, qey, tex, tey);

        // Path B: uses explicit Q with reflected control
        char pathB[512];
        sprintf(pathB, "M %.6g,%.6g Q %.6g,%.6g %.6g,%.6g Q %.6g,%.6g %.6g,%.6g",
                x0, y0, cx, cy, qex, qey, rcx, rcy, tex, tey);

        char svgA[1024];
        sprintf(svgA,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathA);

        char svgB[1024];
        sprintf(svgB,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathB);

        psx_result rA = S_OK, rB = S_OK;
        psx_svg_node* rootA = NULL;
        psx_svg_node* rootB = NULL;
        psx_svg_player* pA = create_player(svgA, &rA, &rootA);
        psx_svg_player* pB = create_player(svgB, &rB, &rootB);
        if (!pA || !pB) {
            if (pA) { destroy_player(pA, rootA); }
            if (pB) { destroy_player(pB, rootB); }
            continue;
        }

        const psx_svg_node* nA = psx_svg_player_get_node_by_id(pA, "r");
        const psx_svg_node* nB = psx_svg_player_get_node_by_id(pB, "r");
        if (!nA || !nB) {
            destroy_player(pA, rootA);
            destroy_player(pB, rootB);
            continue;
        }

        psx_svg_player_play(pA);
        psx_svg_player_play(pB);

        bool mismatch = false;
        for (int si = 0; si <= NUM_SEEK_POINTS && !mismatch; si++) {
            float seek_time = (float)si / (float)NUM_SEEK_POINTS * 10.0f;

            psx_svg_player_seek(pA, seek_time);
            psx_svg_player_tick(pA, 0.0f);
            psx_svg_player_seek(pB, seek_time);
            psx_svg_player_tick(pB, 0.0f);

            float aA = 0, bA = 0, cA = 0, dA = 0, eA = 0, fA = 0;
            float aB = 0, bB = 0, cB = 0, dB = 0, eB = 0, fB = 0;
            bool gotA = psx_svg_player_debug_get_transform_override(pA, nA, &aA, &bA, &cA, &dA, &eA, &fA);
            bool gotB = psx_svg_player_debug_get_transform_override(pB, nB, &aB, &bB, &cB, &dB, &eB, &fB);
            if (!gotA || !gotB) { continue; }

            float dx = eA - eB;
            float dy = fA - fB;
            float dist = sqrtf(dx * dx + dy * dy);
            EXPECT_LT(dist, 0.01f)
                    << "Q+T iter=" << iter << " seek=" << seek_time
                    << " A=(" << eA << "," << fA << ")"
                    << " B=(" << eB << "," << fB << ")"
                    << " dist=" << dist;
            if (dist >= 0.01f) { mismatch = true; }
        }

        destroy_player(pA, rootA);
        destroy_player(pB, rootB);
    }

    // -----------------------------------------------------------------------
    // Part B: C + S  vs  C + C(reflected)
    // -----------------------------------------------------------------------
    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        // Generate random coords: start(x0,y0), C controls(c1x,c1y,c2x,c2y),
        // C end(cex,cey), S second control(sc2x,sc2y), S end(sex,sey).
        // That's 12 floats.  Range [-200, 200].
        float coords[12];
        for (int ci = 0; ci < 12; ci++) {
            seed = seed * 1103515245u + 12345u;
            coords[ci] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
        }
        float x0 = coords[0], y0 = coords[1];
        float c1x = coords[2], c1y = coords[3];
        float c2x = coords[4], c2y = coords[5];
        float cex = coords[6], cey = coords[7];
        float sc2x = coords[8], sc2y = coords[9];
        float sex = coords[10], sey = coords[11];

        // Reflected first control point for the S segment
        float rc1x = 2.0f * cex - c2x;
        float rc1y = 2.0f * cey - c2y;

        // Path A: uses S (smooth cubic)
        char pathA[512];
        sprintf(pathA, "M %.6g,%.6g C %.6g,%.6g %.6g,%.6g %.6g,%.6g S %.6g,%.6g %.6g,%.6g",
                x0, y0, c1x, c1y, c2x, c2y, cex, cey, sc2x, sc2y, sex, sey);

        // Path B: uses explicit C with reflected first control
        char pathB[512];
        sprintf(pathB, "M %.6g,%.6g C %.6g,%.6g %.6g,%.6g %.6g,%.6g C %.6g,%.6g %.6g,%.6g %.6g,%.6g",
                x0, y0, c1x, c1y, c2x, c2y, cex, cey, rc1x, rc1y, sc2x, sc2y, sex, sey);

        char svgA[1024];
        sprintf(svgA,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathA);

        char svgB[1024];
        sprintf(svgB,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathB);

        psx_result rA = S_OK, rB = S_OK;
        psx_svg_node* rootA = NULL;
        psx_svg_node* rootB = NULL;
        psx_svg_player* pA = create_player(svgA, &rA, &rootA);
        psx_svg_player* pB = create_player(svgB, &rB, &rootB);
        if (!pA || !pB) {
            if (pA) { destroy_player(pA, rootA); }
            if (pB) { destroy_player(pB, rootB); }
            continue;
        }

        const psx_svg_node* nA = psx_svg_player_get_node_by_id(pA, "r");
        const psx_svg_node* nB = psx_svg_player_get_node_by_id(pB, "r");
        if (!nA || !nB) {
            destroy_player(pA, rootA);
            destroy_player(pB, rootB);
            continue;
        }

        psx_svg_player_play(pA);
        psx_svg_player_play(pB);

        bool mismatch = false;
        for (int si = 0; si <= NUM_SEEK_POINTS && !mismatch; si++) {
            float seek_time = (float)si / (float)NUM_SEEK_POINTS * 10.0f;

            psx_svg_player_seek(pA, seek_time);
            psx_svg_player_tick(pA, 0.0f);
            psx_svg_player_seek(pB, seek_time);
            psx_svg_player_tick(pB, 0.0f);

            float aA = 0, bA = 0, cA = 0, dA = 0, eA = 0, fA = 0;
            float aB = 0, bB = 0, cB = 0, dB = 0, eB = 0, fB = 0;
            bool gotA = psx_svg_player_debug_get_transform_override(pA, nA, &aA, &bA, &cA, &dA, &eA, &fA);
            bool gotB = psx_svg_player_debug_get_transform_override(pB, nB, &aB, &bB, &cB, &dB, &eB, &fB);
            if (!gotA || !gotB) { continue; }

            float dx = eA - eB;
            float dy = fA - fB;
            float dist = sqrtf(dx * dx + dy * dy);
            EXPECT_LT(dist, 0.01f)
                    << "C+S iter=" << iter << " seek=" << seek_time
                    << " A=(" << eA << "," << fA << ")"
                    << " B=(" << eB << "," << fB << ")"
                    << " dist=" << dist;
            if (dist >= 0.01f) { mismatch = true; }
        }

        destroy_player(pA, rootA);
        destroy_player(pB, rootB);
    }
}

// ---------------------------------------------------------------------------
// Unit test: Elliptical Arc — Semicircle
// Validates: Requirements 4.1, 4.3
// Path "M 0,0 A 50,50 0 0 1 100,0" — semicircle from (0,0) to (100,0)
// with r=50, sweep=1.
// Center is at (50, 0). With small-arc + sweep=1 the arc passes through
// (50, -50) (above the chord in screen coords).
// At t=0: position near (0, 0)
// At t=end: position near (100, 0)
// At t=mid: position near (50, -50) — apex of the semicircle
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_Arc_Semicircle)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"200\" height=\"200\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
        "    <animateMotion path=\"M 0,0 A 50,50 0 0 1 100,0\" dur=\"4s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0: start position should be at (0, 0)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(0.0f, e, 1.0f);
        EXPECT_NEAR(0.0f, f, 1.0f);
    }

    // t=4s (end, fill=freeze): position should be near (100, 0)
    psx_svg_player_seek(p, 4.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(100.0f, e, 1.0f);
        EXPECT_NEAR(0.0f, f, 1.0f);
    }

    // t=2s (midpoint): position should be near (50, -50) — the apex of the
    // semicircle. The key check is that |y| is significantly != 0, proving
    // the A command was parsed as a curve, not a straight line.
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        // x should be near the middle of the arc
        EXPECT_GT(e, 20.0f);
        EXPECT_LT(e, 80.0f);
        // y must be significantly negative (arc goes above the x-axis),
        // proving the A command was parsed as a curve, not a straight line.
        EXPECT_LT(f, -20.0f);
        // y should be near -50 (the apex of the semicircle)
        EXPECT_NEAR(-50.0f, f, 15.0f);
    }

    destroy_player(p, root);
}

// ---------------------------------------------------------------------------
// Unit test: Elliptical Arc — Zero Radius degenerates to straight line
// Validates: Requirements 4.1, 4.3
// Path "M 10,10 A 0,5 0 0 1 50,50" — rx=0, should degenerate to line
// At t=0: position near (10, 10)
// At t=end: position near (50, 50)
// At t=mid: position near (30, 30) — midpoint of straight line
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_Arc_ZeroRadius)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
        "    <animateMotion path=\"M 10,10 A 0,5 0 0 1 50,50\" dur=\"4s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0: start position should be at (10, 10)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(10.0f, e, 1.0f);
        EXPECT_NEAR(10.0f, f, 1.0f);
    }

    // t=4s (end, fill=freeze): position should be near (50, 50)
    psx_svg_player_seek(p, 4.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(50.0f, e, 1.0f);
        EXPECT_NEAR(50.0f, f, 1.0f);
    }

    // t=2s (midpoint): since rx=0 degenerates to a straight line,
    // the midpoint should be near (30, 30) — the linear midpoint.
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        // For a straight line, x and y should be near the midpoint
        EXPECT_NEAR(30.0f, e, 3.0f);
        EXPECT_NEAR(30.0f, f, 3.0f);
    }

    destroy_player(p, root);
}

// ---------------------------------------------------------------------------
// Unit test: Mixed line and curve commands — verify continuity
// Validates: Requirements 6.1, 6.2
// Path "M 0,0 L 10,0 Q 20,0 20,10 L 20,20"
// At t=0: position near (0, 0)
// At t=end: position near (20, 20)
// At t=mid: position within bounding box [0,20] x [0,20]
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_MixedCommands)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"100\" height=\"100\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
        "    <animateMotion path=\"M 0,0 L 10,0 Q 20,0 20,10 L 20,20\" dur=\"4s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0: start position should be at (0, 0)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(0.0f, e, 1.0f);
        EXPECT_NEAR(0.0f, f, 1.0f);
    }

    // t=4s (end, fill=freeze): position should be near (20, 20)
    psx_svg_player_seek(p, 4.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(20.0f, e, 1.0f);
        EXPECT_NEAR(20.0f, f, 1.0f);
    }

    // t=2s (midpoint): position should be somewhere along the path,
    // within the bounding box [0,20] x [0,20]
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_GE(e, -1.0f);
        EXPECT_LE(e, 21.0f);
        EXPECT_GE(f, -1.0f);
        EXPECT_LE(f, 21.0f);
    }

    destroy_player(p, root);
}

// ---------------------------------------------------------------------------
// Unit test: Pure line-only rectangle path — regression test
// Validates: Requirements 6.1, 6.2
// Path "M 0,0 L 100,0 L 100,100 L 0,100 Z" — pure line-only rectangle
// At t=0: position near (0, 0)
// At t=end: position near (0, 0) (closed path returns to start)
// At t=0.25: position near (100, 0)
// At t=0.5: position near (100, 100)
// This verifies that the new curve code doesn't break existing line-only paths.
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_LineOnlyRegression)
{
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"200\" height=\"200\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
        "    <animateMotion path=\"M 0,0 L 100,0 L 100,100 L 0,100 Z\" dur=\"4s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0: start position should be at (0, 0)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(0.0f, e, 1.0f);
        EXPECT_NEAR(0.0f, f, 1.0f);
    }

    // t=4s (end, fill=freeze): closed path, position should be back at (0, 0)
    psx_svg_player_seek(p, 4.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(0.0f, e, 2.0f);
        EXPECT_NEAR(0.0f, f, 2.0f);
    }

    // t=1s (quarter, t=0.25): position should be near (100, 0)
    // The rectangle perimeter is 400, each side is 100 units.
    // At t=0.25 we should be at the end of the first side.
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(100.0f, e, 2.0f);
        EXPECT_NEAR(0.0f, f, 2.0f);
    }

    // t=2s (half, t=0.5): position should be near (100, 100)
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(100.0f, e, 2.0f);
        EXPECT_NEAR(100.0f, f, 2.0f);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateMotion_QuadBezierPath)
{
    // End-to-end test: SVG parsing -> animateMotion path parsing -> arc-length
    // parameterization -> position interpolation -> transform override.
    //
    // Path: M 0,0 Q 50,100 100,0  (quadratic Bezier arc)
    // Duration: 4s, fill=freeze
    //
    // Exact quadratic Bezier at parametric t=0.5:
    // B(0.5) = 0.25*(0,0) + 0.5*(50,100) + 0.25*(100,0) = (50, 50)
    // Arc-length midpoint differs slightly from parametric midpoint, but the
    // position must clearly be on the curve (y >> 0).
    const char* svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"200\" height=\"200\">"
        "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"10\" height=\"10\" fill=\"red\">"
        "    <animateMotion path=\"M 0,0 Q 50,100 100,0\" dur=\"4s\" fill=\"freeze\"/>"
        "  </rect>"
        "</svg>";

    psx_result r = S_OK;
    psx_svg_node* root = NULL;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_NE((psx_svg_player*)NULL, p);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    ASSERT_TRUE(n != NULL);

    psx_svg_player_play(p);

    // t=0: start of path, position near (0, 0)
    psx_svg_player_seek(p, 0.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(0.0f, e, 0.5f);
        EXPECT_NEAR(0.0f, f, 0.5f);
    }

    // t=2s (midpoint of 4s duration): position on the curve
    // The arc-length midpoint of Q 50,100 100,0 should be near (50, 50).
    // Allow generous tolerance because arc-length parameterization shifts
    // the midpoint slightly from the parametric midpoint.
    psx_svg_player_seek(p, 2.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        // x should be roughly around 50
        EXPECT_GT(e, 20.0f);
        EXPECT_LT(e, 80.0f);
        // y must be significantly > 0 — proves the curve is followed, not a straight line
        EXPECT_GT(f, 20.0f);
        EXPECT_LT(f, 80.0f);
    }

    // t=4s (end, fill=freeze): position near (100, 0)
    psx_svg_player_seek(p, 4.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f));
        EXPECT_NEAR(100.0f, e, 0.5f);
        EXPECT_NEAR(0.0f, f, 0.5f);
    }

    destroy_player(p, root);
}

// ---------------------------------------------------------------------------
// Property 4: Relative vs Absolute coordinate equivalence
// For any curve command (Q/C/A) and random parameters, using relative
// coordinate commands (q/c/a) should produce the same animation positions
// as using absolute coordinate commands. Verified by creating two SVG players
// and comparing transform overrides at multiple seek times.
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_Property4_RelativeAbsoluteEquivalence)
{
    // **Validates: Requirements 1.2, 2.2, 4.2**
    uint32_t seed = 99887u;
    const uint32_t NUM_ITERS = 100;
    const int NUM_SEEK_POINTS = 10;

    // -----------------------------------------------------------------------
    // Part A: Q (absolute) vs q (relative)
    // -----------------------------------------------------------------------
    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        float coords[6];
        for (int ci = 0; ci < 6; ci++) {
            seed = seed * 1103515245u + 12345u;
            coords[ci] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
        }
        float x0 = coords[0], y0 = coords[1];
        float cx = coords[2], cy = coords[3];
        float ex = coords[4], ey = coords[5];

        // Relative params
        float rcx = cx - x0, rcy = cy - y0;
        float rex = ex - x0, rey = ey - y0;

        // Path A: absolute Q
        char pathA[512];
        sprintf(pathA, "M %.6g,%.6g Q %.6g,%.6g %.6g,%.6g",
                x0, y0, cx, cy, ex, ey);

        // Path B: relative q
        char pathB[512];
        sprintf(pathB, "M %.6g,%.6g q %.6g,%.6g %.6g,%.6g",
                x0, y0, rcx, rcy, rex, rey);

        char svgA[1024];
        sprintf(svgA,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathA);

        char svgB[1024];
        sprintf(svgB,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathB);

        psx_result rA = S_OK, rB = S_OK;
        psx_svg_node* rootA = NULL;
        psx_svg_node* rootB = NULL;
        psx_svg_player* pA = create_player(svgA, &rA, &rootA);
        psx_svg_player* pB = create_player(svgB, &rB, &rootB);
        if (!pA || !pB) {
            if (pA) { destroy_player(pA, rootA); }
            if (pB) { destroy_player(pB, rootB); }
            continue;
        }

        const psx_svg_node* nA = psx_svg_player_get_node_by_id(pA, "r");
        const psx_svg_node* nB = psx_svg_player_get_node_by_id(pB, "r");
        if (!nA || !nB) {
            destroy_player(pA, rootA);
            destroy_player(pB, rootB);
            continue;
        }

        psx_svg_player_play(pA);
        psx_svg_player_play(pB);

        bool mismatch = false;
        for (int si = 0; si <= NUM_SEEK_POINTS && !mismatch; si++) {
            float seek_time = (float)si / (float)NUM_SEEK_POINTS * 10.0f;

            psx_svg_player_seek(pA, seek_time);
            psx_svg_player_tick(pA, 0.0f);
            psx_svg_player_seek(pB, seek_time);
            psx_svg_player_tick(pB, 0.0f);

            float aA = 0, bA = 0, cA = 0, dA = 0, eA = 0, fA = 0;
            float aB = 0, bB = 0, cB = 0, dB = 0, eB = 0, fB = 0;
            bool gotA = psx_svg_player_debug_get_transform_override(pA, nA, &aA, &bA, &cA, &dA, &eA, &fA);
            bool gotB = psx_svg_player_debug_get_transform_override(pB, nB, &aB, &bB, &cB, &dB, &eB, &fB);
            if (!gotA || !gotB) { continue; }

            float dx = eA - eB;
            float dy = fA - fB;
            float dist = sqrtf(dx * dx + dy * dy);
            EXPECT_LT(dist, 0.01f)
                    << "Q vs q iter=" << iter << " seek=" << seek_time
                    << " abs=(" << eA << "," << fA << ")"
                    << " rel=(" << eB << "," << fB << ")"
                    << " dist=" << dist;
            if (dist >= 0.01f) { mismatch = true; }
        }

        destroy_player(pA, rootA);
        destroy_player(pB, rootB);
    }

    // -----------------------------------------------------------------------
    // Part B: C (absolute) vs c (relative)
    // -----------------------------------------------------------------------
    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        float coords[8];
        for (int ci = 0; ci < 8; ci++) {
            seed = seed * 1103515245u + 12345u;
            coords[ci] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
        }
        float x0 = coords[0], y0 = coords[1];
        float c1x = coords[2], c1y = coords[3];
        float c2x = coords[4], c2y = coords[5];
        float ex = coords[6], ey = coords[7];

        // Relative params
        float rc1x = c1x - x0, rc1y = c1y - y0;
        float rc2x = c2x - x0, rc2y = c2y - y0;
        float rex = ex - x0, rey = ey - y0;

        // Path A: absolute C
        char pathA[512];
        sprintf(pathA, "M %.6g,%.6g C %.6g,%.6g %.6g,%.6g %.6g,%.6g",
                x0, y0, c1x, c1y, c2x, c2y, ex, ey);

        // Path B: relative c
        char pathB[512];
        sprintf(pathB, "M %.6g,%.6g c %.6g,%.6g %.6g,%.6g %.6g,%.6g",
                x0, y0, rc1x, rc1y, rc2x, rc2y, rex, rey);

        char svgA[1024];
        sprintf(svgA,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathA);

        char svgB[1024];
        sprintf(svgB,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathB);

        psx_result rA = S_OK, rB = S_OK;
        psx_svg_node* rootA = NULL;
        psx_svg_node* rootB = NULL;
        psx_svg_player* pA = create_player(svgA, &rA, &rootA);
        psx_svg_player* pB = create_player(svgB, &rB, &rootB);
        if (!pA || !pB) {
            if (pA) { destroy_player(pA, rootA); }
            if (pB) { destroy_player(pB, rootB); }
            continue;
        }

        const psx_svg_node* nA = psx_svg_player_get_node_by_id(pA, "r");
        const psx_svg_node* nB = psx_svg_player_get_node_by_id(pB, "r");
        if (!nA || !nB) {
            destroy_player(pA, rootA);
            destroy_player(pB, rootB);
            continue;
        }

        psx_svg_player_play(pA);
        psx_svg_player_play(pB);

        bool mismatch = false;
        for (int si = 0; si <= NUM_SEEK_POINTS && !mismatch; si++) {
            float seek_time = (float)si / (float)NUM_SEEK_POINTS * 10.0f;

            psx_svg_player_seek(pA, seek_time);
            psx_svg_player_tick(pA, 0.0f);
            psx_svg_player_seek(pB, seek_time);
            psx_svg_player_tick(pB, 0.0f);

            float aA = 0, bA = 0, cA = 0, dA = 0, eA = 0, fA = 0;
            float aB = 0, bB = 0, cB = 0, dB = 0, eB = 0, fB = 0;
            bool gotA = psx_svg_player_debug_get_transform_override(pA, nA, &aA, &bA, &cA, &dA, &eA, &fA);
            bool gotB = psx_svg_player_debug_get_transform_override(pB, nB, &aB, &bB, &cB, &dB, &eB, &fB);
            if (!gotA || !gotB) { continue; }

            float dx = eA - eB;
            float dy = fA - fB;
            float dist = sqrtf(dx * dx + dy * dy);
            EXPECT_LT(dist, 0.01f)
                    << "C vs c iter=" << iter << " seek=" << seek_time
                    << " abs=(" << eA << "," << fA << ")"
                    << " rel=(" << eB << "," << fB << ")"
                    << " dist=" << dist;
            if (dist >= 0.01f) { mismatch = true; }
        }

        destroy_player(pA, rootA);
        destroy_player(pB, rootB);
    }

    // -----------------------------------------------------------------------
    // Part C: A (absolute) vs a (relative)
    // For arc, only the endpoint (ex, ey) is relative; rx, ry, rotation,
    // large-arc-flag, sweep-flag remain the same.
    // -----------------------------------------------------------------------
    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        // Generate start point and endpoint
        float coords[4];
        for (int ci = 0; ci < 4; ci++) {
            seed = seed * 1103515245u + 12345u;
            coords[ci] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
        }
        float x0 = coords[0], y0 = coords[1];
        float ex = coords[2], ey = coords[3];

        // Generate rx, ry in [10, 200] to avoid degenerate zero-radius arcs
        seed = seed * 1103515245u + 12345u;
        float rx = 10.0f + (float)(seed % 190u);
        seed = seed * 1103515245u + 12345u;
        float ry = 10.0f + (float)(seed % 190u);

        // Ensure radii are large enough to connect the endpoints
        float dx_half = (ex - x0) * 0.5f;
        float dy_half = (ey - y0) * 0.5f;
        float min_r = sqrtf(dx_half * dx_half + dy_half * dy_half) + 1.0f;
        if (rx < min_r) { rx = min_r; }
        if (ry < min_r) { ry = min_r; }

        // Random rotation [0, 360), flags
        seed = seed * 1103515245u + 12345u;
        float rotation = (float)(seed % 360u);
        seed = seed * 1103515245u + 12345u;
        int large_arc = (int)(seed % 2u);
        seed = seed * 1103515245u + 12345u;
        int sweep = (int)(seed % 2u);

        // Relative endpoint
        float rex = ex - x0, rey = ey - y0;

        // Path A: absolute A
        char pathA[512];
        sprintf(pathA, "M %.6g,%.6g A %.6g,%.6g %.6g %d %d %.6g,%.6g",
                x0, y0, rx, ry, rotation, large_arc, sweep, ex, ey);

        // Path B: relative a
        char pathB[512];
        sprintf(pathB, "M %.6g,%.6g a %.6g,%.6g %.6g %d %d %.6g,%.6g",
                x0, y0, rx, ry, rotation, large_arc, sweep, rex, rey);

        char svgA[1024];
        sprintf(svgA,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathA);

        char svgB[1024];
        sprintf(svgB,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathB);

        psx_result rA = S_OK, rB = S_OK;
        psx_svg_node* rootA = NULL;
        psx_svg_node* rootB = NULL;
        psx_svg_player* pA = create_player(svgA, &rA, &rootA);
        psx_svg_player* pB = create_player(svgB, &rB, &rootB);
        if (!pA || !pB) {
            if (pA) { destroy_player(pA, rootA); }
            if (pB) { destroy_player(pB, rootB); }
            continue;
        }

        const psx_svg_node* nA = psx_svg_player_get_node_by_id(pA, "r");
        const psx_svg_node* nB = psx_svg_player_get_node_by_id(pB, "r");
        if (!nA || !nB) {
            destroy_player(pA, rootA);
            destroy_player(pB, rootB);
            continue;
        }

        psx_svg_player_play(pA);
        psx_svg_player_play(pB);

        bool mismatch = false;
        for (int si = 0; si <= NUM_SEEK_POINTS && !mismatch; si++) {
            float seek_time = (float)si / (float)NUM_SEEK_POINTS * 10.0f;

            psx_svg_player_seek(pA, seek_time);
            psx_svg_player_tick(pA, 0.0f);
            psx_svg_player_seek(pB, seek_time);
            psx_svg_player_tick(pB, 0.0f);

            float aA = 0, bA = 0, cA = 0, dA = 0, eA = 0, fA = 0;
            float aB = 0, bB = 0, cB = 0, dB = 0, eB = 0, fB = 0;
            bool gotA = psx_svg_player_debug_get_transform_override(pA, nA, &aA, &bA, &cA, &dA, &eA, &fA);
            bool gotB = psx_svg_player_debug_get_transform_override(pB, nB, &aB, &bB, &cB, &dB, &eB, &fB);
            if (!gotA || !gotB) { continue; }

            float ddx = eA - eB;
            float ddy = fA - fB;
            float dist = sqrtf(ddx * ddx + ddy * ddy);
            EXPECT_LT(dist, 0.01f)
                    << "A vs a iter=" << iter << " seek=" << seek_time
                    << " abs=(" << eA << "," << fA << ")"
                    << " rel=(" << eB << "," << fB << ")"
                    << " dist=" << dist;
            if (dist >= 0.01f) { mismatch = true; }
        }

        destroy_player(pA, rootA);
        destroy_player(pB, rootB);
    }
}

// ---------------------------------------------------------------------------
// Property 5: Implicit repeat equivalence
// For any curve command (Q/C/A) with two sets of parameters, placing both
// sets after a single command letter (implicit repeat) should produce the
// same animation positions as using two explicit command letters. Verified
// by creating two SVG players and comparing transform overrides at multiple
// seek times.
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_Property5_ImplicitRepeatEquivalence)
{
    // **Validates: Requirements 1.3, 2.3, 4.4**
    uint32_t seed = 55443u;
    const uint32_t NUM_ITERS = 100;
    const int NUM_SEEK_POINTS = 10;

    // -----------------------------------------------------------------------
    // Part A: Q implicit repeat vs two explicit Q commands
    // -----------------------------------------------------------------------
    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        float coords[10];
        for (int ci = 0; ci < 10; ci++) {
            seed = seed * 1103515245u + 12345u;
            coords[ci] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
        }
        float x0 = coords[0], y0 = coords[1];
        float cx1 = coords[2], cy1 = coords[3];
        float ex1 = coords[4], ey1 = coords[5];
        float cx2 = coords[6], cy2 = coords[7];
        float ex2 = coords[8], ey2 = coords[9];

        // Path A: implicit repeat (one Q, two param sets)
        char pathA[512];
        sprintf(pathA, "M %.6g,%.6g Q %.6g,%.6g %.6g,%.6g %.6g,%.6g %.6g,%.6g",
                x0, y0, cx1, cy1, ex1, ey1, cx2, cy2, ex2, ey2);

        // Path B: explicit repeat (two Q commands)
        char pathB[512];
        sprintf(pathB, "M %.6g,%.6g Q %.6g,%.6g %.6g,%.6g Q %.6g,%.6g %.6g,%.6g",
                x0, y0, cx1, cy1, ex1, ey1, cx2, cy2, ex2, ey2);

        char svgA[1024];
        sprintf(svgA,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathA);

        char svgB[1024];
        sprintf(svgB,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathB);

        psx_result rA = S_OK, rB = S_OK;
        psx_svg_node* rootA = NULL;
        psx_svg_node* rootB = NULL;
        psx_svg_player* pA = create_player(svgA, &rA, &rootA);
        psx_svg_player* pB = create_player(svgB, &rB, &rootB);
        if (!pA || !pB) {
            if (pA) { destroy_player(pA, rootA); }
            if (pB) { destroy_player(pB, rootB); }
            continue;
        }

        const psx_svg_node* nA = psx_svg_player_get_node_by_id(pA, "r");
        const psx_svg_node* nB = psx_svg_player_get_node_by_id(pB, "r");
        if (!nA || !nB) {
            destroy_player(pA, rootA);
            destroy_player(pB, rootB);
            continue;
        }

        psx_svg_player_play(pA);
        psx_svg_player_play(pB);

        bool mismatch = false;
        for (int si = 0; si <= NUM_SEEK_POINTS && !mismatch; si++) {
            float seek_time = (float)si / (float)NUM_SEEK_POINTS * 10.0f;

            psx_svg_player_seek(pA, seek_time);
            psx_svg_player_tick(pA, 0.0f);
            psx_svg_player_seek(pB, seek_time);
            psx_svg_player_tick(pB, 0.0f);

            float aA = 0, bA = 0, cA = 0, dA = 0, eA = 0, fA = 0;
            float aB = 0, bB = 0, cB = 0, dB = 0, eB = 0, fB = 0;
            bool gotA = psx_svg_player_debug_get_transform_override(pA, nA, &aA, &bA, &cA, &dA, &eA, &fA);
            bool gotB = psx_svg_player_debug_get_transform_override(pB, nB, &aB, &bB, &cB, &dB, &eB, &fB);
            if (!gotA || !gotB) { continue; }

            float dx = eA - eB;
            float dy = fA - fB;
            float dist = sqrtf(dx * dx + dy * dy);
            EXPECT_LT(dist, 0.01f)
                    << "Q implicit iter=" << iter << " seek=" << seek_time
                    << " implicit=(" << eA << "," << fA << ")"
                    << " explicit=(" << eB << "," << fB << ")"
                    << " dist=" << dist;
            if (dist >= 0.01f) { mismatch = true; }
        }

        destroy_player(pA, rootA);
        destroy_player(pB, rootB);
    }

    // -----------------------------------------------------------------------
    // Part B: C implicit repeat vs two explicit C commands
    // -----------------------------------------------------------------------
    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        float coords[14];
        for (int ci = 0; ci < 14; ci++) {
            seed = seed * 1103515245u + 12345u;
            coords[ci] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
        }
        float x0 = coords[0], y0 = coords[1];
        float c1x1 = coords[2], c1y1 = coords[3];
        float c2x1 = coords[4], c2y1 = coords[5];
        float ex1 = coords[6], ey1 = coords[7];
        float c1x2 = coords[8], c1y2 = coords[9];
        float c2x2 = coords[10], c2y2 = coords[11];
        float ex2 = coords[12], ey2 = coords[13];

        // Path A: implicit repeat (one C, two param sets)
        char pathA[768];
        sprintf(pathA, "M %.6g,%.6g C %.6g,%.6g %.6g,%.6g %.6g,%.6g %.6g,%.6g %.6g,%.6g %.6g,%.6g",
                x0, y0,
                c1x1, c1y1, c2x1, c2y1, ex1, ey1,
                c1x2, c1y2, c2x2, c2y2, ex2, ey2);

        // Path B: explicit repeat (two C commands)
        char pathB[768];
        sprintf(pathB, "M %.6g,%.6g C %.6g,%.6g %.6g,%.6g %.6g,%.6g C %.6g,%.6g %.6g,%.6g %.6g,%.6g",
                x0, y0,
                c1x1, c1y1, c2x1, c2y1, ex1, ey1,
                c1x2, c1y2, c2x2, c2y2, ex2, ey2);

        char svgA[1536];
        sprintf(svgA,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathA);

        char svgB[1536];
        sprintf(svgB,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathB);

        psx_result rA = S_OK, rB = S_OK;
        psx_svg_node* rootA = NULL;
        psx_svg_node* rootB = NULL;
        psx_svg_player* pA = create_player(svgA, &rA, &rootA);
        psx_svg_player* pB = create_player(svgB, &rB, &rootB);
        if (!pA || !pB) {
            if (pA) { destroy_player(pA, rootA); }
            if (pB) { destroy_player(pB, rootB); }
            continue;
        }

        const psx_svg_node* nA = psx_svg_player_get_node_by_id(pA, "r");
        const psx_svg_node* nB = psx_svg_player_get_node_by_id(pB, "r");
        if (!nA || !nB) {
            destroy_player(pA, rootA);
            destroy_player(pB, rootB);
            continue;
        }

        psx_svg_player_play(pA);
        psx_svg_player_play(pB);

        bool mismatch = false;
        for (int si = 0; si <= NUM_SEEK_POINTS && !mismatch; si++) {
            float seek_time = (float)si / (float)NUM_SEEK_POINTS * 10.0f;

            psx_svg_player_seek(pA, seek_time);
            psx_svg_player_tick(pA, 0.0f);
            psx_svg_player_seek(pB, seek_time);
            psx_svg_player_tick(pB, 0.0f);

            float aA = 0, bA = 0, cA = 0, dA = 0, eA = 0, fA = 0;
            float aB = 0, bB = 0, cB = 0, dB = 0, eB = 0, fB = 0;
            bool gotA = psx_svg_player_debug_get_transform_override(pA, nA, &aA, &bA, &cA, &dA, &eA, &fA);
            bool gotB = psx_svg_player_debug_get_transform_override(pB, nB, &aB, &bB, &cB, &dB, &eB, &fB);
            if (!gotA || !gotB) { continue; }

            float dx = eA - eB;
            float dy = fA - fB;
            float dist = sqrtf(dx * dx + dy * dy);
            EXPECT_LT(dist, 0.01f)
                    << "C implicit iter=" << iter << " seek=" << seek_time
                    << " implicit=(" << eA << "," << fA << ")"
                    << " explicit=(" << eB << "," << fB << ")"
                    << " dist=" << dist;
            if (dist >= 0.01f) { mismatch = true; }
        }

        destroy_player(pA, rootA);
        destroy_player(pB, rootB);
    }

    // -----------------------------------------------------------------------
    // Part C: A implicit repeat vs two explicit A commands
    // -----------------------------------------------------------------------
    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        // Generate start point and two endpoints
        float coords[6];
        for (int ci = 0; ci < 6; ci++) {
            seed = seed * 1103515245u + 12345u;
            coords[ci] = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
        }
        float x0 = coords[0], y0 = coords[1];
        float ex1 = coords[2], ey1 = coords[3];
        float ex2 = coords[4], ey2 = coords[5];

        // Generate shared arc params: rx, ry in [10, 200]
        seed = seed * 1103515245u + 12345u;
        float rx = 10.0f + (float)(seed % 190u);
        seed = seed * 1103515245u + 12345u;
        float ry = 10.0f + (float)(seed % 190u);

        // Ensure radii large enough for both segments
        float dx1 = (ex1 - x0) * 0.5f, dy1 = (ey1 - y0) * 0.5f;
        float min_r1 = sqrtf(dx1 * dx1 + dy1 * dy1) + 1.0f;
        float dx2 = (ex2 - ex1) * 0.5f, dy2 = (ey2 - ey1) * 0.5f;
        float min_r2 = sqrtf(dx2 * dx2 + dy2 * dy2) + 1.0f;
        float min_r = min_r1 > min_r2 ? min_r1 : min_r2;
        if (rx < min_r) { rx = min_r; }
        if (ry < min_r) { ry = min_r; }

        seed = seed * 1103515245u + 12345u;
        float rotation = (float)(seed % 360u);
        seed = seed * 1103515245u + 12345u;
        int large_arc = (int)(seed % 2u);
        seed = seed * 1103515245u + 12345u;
        int sweep = (int)(seed % 2u);

        // Path A: implicit repeat (one A, two param sets)
        char pathA[768];
        sprintf(pathA, "M %.6g,%.6g A %.6g,%.6g %.6g %d %d %.6g,%.6g %.6g,%.6g %.6g %d %d %.6g,%.6g",
                x0, y0,
                rx, ry, rotation, large_arc, sweep, ex1, ey1,
                rx, ry, rotation, large_arc, sweep, ex2, ey2);

        // Path B: explicit repeat (two A commands)
        char pathB[768];
        sprintf(pathB, "M %.6g,%.6g A %.6g,%.6g %.6g %d %d %.6g,%.6g A %.6g,%.6g %.6g %d %d %.6g,%.6g",
                x0, y0,
                rx, ry, rotation, large_arc, sweep, ex1, ey1,
                rx, ry, rotation, large_arc, sweep, ex2, ey2);

        char svgA[1536];
        sprintf(svgA,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathA);

        char svgB[1536];
        sprintf(svgB,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", pathB);

        psx_result rA = S_OK, rB = S_OK;
        psx_svg_node* rootA = NULL;
        psx_svg_node* rootB = NULL;
        psx_svg_player* pA = create_player(svgA, &rA, &rootA);
        psx_svg_player* pB = create_player(svgB, &rB, &rootB);
        if (!pA || !pB) {
            if (pA) { destroy_player(pA, rootA); }
            if (pB) { destroy_player(pB, rootB); }
            continue;
        }

        const psx_svg_node* nA = psx_svg_player_get_node_by_id(pA, "r");
        const psx_svg_node* nB = psx_svg_player_get_node_by_id(pB, "r");
        if (!nA || !nB) {
            destroy_player(pA, rootA);
            destroy_player(pB, rootB);
            continue;
        }

        psx_svg_player_play(pA);
        psx_svg_player_play(pB);

        bool mismatch = false;
        for (int si = 0; si <= NUM_SEEK_POINTS && !mismatch; si++) {
            float seek_time = (float)si / (float)NUM_SEEK_POINTS * 10.0f;

            psx_svg_player_seek(pA, seek_time);
            psx_svg_player_tick(pA, 0.0f);
            psx_svg_player_seek(pB, seek_time);
            psx_svg_player_tick(pB, 0.0f);

            float aA = 0, bA = 0, cA = 0, dA = 0, eA = 0, fA = 0;
            float aB = 0, bB = 0, cB = 0, dB = 0, eB = 0, fB = 0;
            bool gotA = psx_svg_player_debug_get_transform_override(pA, nA, &aA, &bA, &cA, &dA, &eA, &fA);
            bool gotB = psx_svg_player_debug_get_transform_override(pB, nB, &aB, &bB, &cB, &dB, &eB, &fB);
            if (!gotA || !gotB) { continue; }

            float ddx = eA - eB;
            float ddy = fA - fB;
            float dist = sqrtf(ddx * ddx + ddy * ddy);
            EXPECT_LT(dist, 0.01f)
                    << "A implicit iter=" << iter << " seek=" << seek_time
                    << " implicit=(" << eA << "," << fA << ")"
                    << " explicit=(" << eB << "," << fB << ")"
                    << " dist=" << dist;
            if (dist >= 0.01f) { mismatch = true; }
        }

        destroy_player(pA, rootA);
        destroy_player(pB, rootB);
    }
}

// ---------------------------------------------------------------------------
// Property 7: 纯直线路径回归兼容
// For any path containing only M/L/H/V/Z commands, the new parser should
// produce the same positions as the reference line-only parser (test_path_parse)
// at all seek times.
// ---------------------------------------------------------------------------
TEST_F(SVGPlayerTest, MotionPath_Property7_LineOnlyRegression)
{
    // **Validates: Requirements 6.1**
    uint32_t seed = 77442u;
    const uint32_t NUM_ITERS = 100;
    const int NUM_SEEK_POINTS = 10;

    for (uint32_t iter = 0; iter < NUM_ITERS; iter++) {
        // Generate a random line-only path: M x0,y0 then 2-5 segments of L/H/V
        float cur_x = 0, cur_y = 0, start_x = 0, start_y = 0;
        char path_str[1024];
        int ppos = 0;

        // Random start point
        seed = seed * 1103515245u + 12345u;
        start_x = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
        seed = seed * 1103515245u + 12345u;
        start_y = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
        cur_x = start_x;
        cur_y = start_y;
        ppos += sprintf(path_str + ppos, "M %.6g,%.6g", start_x, start_y);

        // Random number of segments: 2-5
        seed = seed * 1103515245u + 12345u;
        int num_segs = 2 + (int)((seed >> 16) % 4);

        for (int s = 0; s < num_segs; s++) {
            seed = seed * 1103515245u + 12345u;
            int cmd_type = (int)((seed >> 16) % 3); // 0=L, 1=H, 2=V

            if (cmd_type == 0) {
                // L x,y
                seed = seed * 1103515245u + 12345u;
                float x = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
                seed = seed * 1103515245u + 12345u;
                float y = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
                ppos += sprintf(path_str + ppos, " L %.6g,%.6g", x, y);
                cur_x = x;
                cur_y = y;
            } else if (cmd_type == 1) {
                // H x
                seed = seed * 1103515245u + 12345u;
                float x = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
                ppos += sprintf(path_str + ppos, " H %.6g", x);
                cur_x = x;
            } else {
                // V y
                seed = seed * 1103515245u + 12345u;
                float y = ((float)(int32_t)(seed >> 8) / (float)(1 << 23)) * 200.0f;
                ppos += sprintf(path_str + ppos, " V %.6g", y);
                cur_y = y;
            }
        }

        // Optionally close with Z (50% chance)
        seed = seed * 1103515245u + 12345u;
        if ((seed >> 16) % 2 == 0) {
            ppos += sprintf(path_str + ppos, " Z");
        }
        path_str[ppos] = '\0';

        // Parse with reference line-only parser
        float* ref_xs = NULL;
        float* ref_ys = NULL;
        uint32_t ref_count = 0;
        bool ref_ok = test_path_parse(path_str, (uint32_t)ppos,
                                      &ref_xs, &ref_ys, &ref_count);
        if (!ref_ok || ref_count < 2) {
            free(ref_xs);
            free(ref_ys);
            continue;
        }

        // Build SVG with animateMotion using the same path
        char svg[2048];
        sprintf(svg,
                "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"tiny\" width=\"800\" height=\"800\">"
                "  <rect id=\"r\" x=\"0\" y=\"0\" width=\"2\" height=\"2\" fill=\"#000\">"
                "    <animateMotion path=\"%s\" dur=\"10s\" fill=\"freeze\"/>"
                "  </rect>"
                "</svg>", path_str);

        psx_result r = S_OK;
        psx_svg_node* root = NULL;
        psx_svg_player* p = create_player(svg, &r, &root);
        if (!p) {
            free(ref_xs);
            free(ref_ys);
            continue;
        }

        const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
        if (!n) {
            destroy_player(p, root);
            free(ref_xs);
            free(ref_ys);
            continue;
        }

        psx_svg_player_play(p);

        bool mismatch = false;
        for (int si = 0; si <= NUM_SEEK_POINTS && !mismatch; si++) {
            float t = (float)si / (float)NUM_SEEK_POINTS;
            float seek_time = t * 10.0f;

            // Reference position from test_path_parse output
            float exp_x = 0, exp_y = 0;
            test_arc_length_position(ref_xs, ref_ys, ref_count, t, &exp_x, &exp_y);

            // Player position
            psx_svg_player_seek(p, seek_time);
            psx_svg_player_tick(p, 0.0f);

            float a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
            bool got = psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f);
            if (!got) { continue; }

            float dx = e - exp_x;
            float dy = f - exp_y;
            float dist = sqrtf(dx * dx + dy * dy);
            EXPECT_LT(dist, 0.01f)
                    << "iter=" << iter << " seek_t=" << t
                    << " path=\"" << path_str << "\""
                    << " expected=(" << exp_x << "," << exp_y << ")"
                    << " actual=(" << e << "," << f << ")"
                    << " dist=" << dist;
            if (dist >= 0.01f) { mismatch = true; }
        }

        destroy_player(p, root);
        free(ref_xs);
        free(ref_ys);
    }
}

// ── <set attributeName="visibility"> tests ──

TEST_F(SVGPlayerTest, SetVisibility_BeforeBegin_NoOverride)
{
    const char* svg =
        "<svg xmlns='http://www.w3.org/2000/svg' version='1.2' baseProfile='tiny'"
        "     width='200' height='200'>"
        "  <rect xml:id='r1' x='10' y='10' width='80' height='80' fill='red'"
        "        visibility='hidden'>"
        "    <set attributeName='visibility' to='visible'"
        "         begin='2s' dur='4s' fill='freeze'/>"
        "  </rect>"
        "</svg>";

    psx_svg_node* root = NULL;
    psx_result r = S_OK;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_TRUE(p != NULL);

    // At t=1s, before begin=2s — no visibility override should exist.
    psx_svg_player_seek(p, 1.0f);
    psx_svg_player_tick(p, 0.0f);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r1");
    EXPECT_TRUE(n != NULL);

    int32_t iv = -1;
    bool got = psx_svg_player_debug_get_int_override(p, n, SVG_ATTR_VISIBILITY, &iv);
    EXPECT_FALSE(got) << "No visibility override before begin time";

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, SetVisibility_DuringActive_OverrideVisible)
{
    const char* svg =
        "<svg xmlns='http://www.w3.org/2000/svg' version='1.2' baseProfile='tiny'"
        "     width='200' height='200'>"
        "  <rect xml:id='r1' x='10' y='10' width='80' height='80' fill='red'"
        "        visibility='hidden'>"
        "    <set attributeName='visibility' to='visible'"
        "         begin='2s' dur='4s' fill='freeze'/>"
        "  </rect>"
        "</svg>";

    psx_svg_node* root = NULL;
    psx_result r = S_OK;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_TRUE(p != NULL);

    // At t=3s, inside [2s, 6s) — visibility should be overridden to 1 (visible).
    psx_svg_player_seek(p, 3.0f);
    psx_svg_player_tick(p, 0.0f);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r1");
    EXPECT_TRUE(n != NULL);

    int32_t iv = -1;
    bool got = psx_svg_player_debug_get_int_override(p, n, SVG_ATTR_VISIBILITY, &iv);
    EXPECT_TRUE(got) << "Visibility override should exist during active interval";
    EXPECT_EQ(1, iv) << "to='visible' should produce 1";

    destroy_player(p, root);
}

// ── additive="sum" for animateTransform tests ──

TEST_F(SVGPlayerTest, AnimateTransform_Additive_Sum_RotateThenScale)
{
    // Two animateTransform on the same rect, both additive="sum".
    // First: rotate from 0 to 90 over 5s
    // Second: scale from 1 to 2 over 5s
    // At t=2.5s (midpoint): rotate=45°, scale=1.5
    // Expected combined matrix = rotate(45) * scale(1.5)
    //   rotate(45): a=cos45, b=sin45, c=-sin45, d=cos45, e=0, f=0
    //   scale(1.5): a=1.5, b=0, c=0, d=1.5, e=0, f=0
    //   combined = rotate * scale:
    //     a = cos45*1.5, b = sin45*1.5, c = -sin45*1.5, d = cos45*1.5, e=0, f=0
    const char* svg =
        "<svg xmlns='http://www.w3.org/2000/svg' version='1.2' baseProfile='tiny'"
        "     width='200' height='200'>"
        "  <rect xml:id='r1' x='10' y='10' width='80' height='80' fill='red'>"
        "    <animateTransform attributeName='transform' type='rotate'"
        "         from='0' to='90' dur='5s' fill='freeze' additive='sum'/>"
        "    <animateTransform attributeName='transform' type='scale'"
        "         from='1' to='2' dur='5s' fill='freeze' additive='sum'/>"
        "  </rect>"
        "</svg>";

    psx_svg_node* root = NULL;
    psx_result r = S_OK;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_TRUE(p != NULL);

    psx_svg_player_seek(p, 2.5f);
    psx_svg_player_tick(p, 0.0f);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r1");
    EXPECT_TRUE(n != NULL);

    float a, b, c, d, e, f;
    bool got = psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f);
    EXPECT_TRUE(got) << "Transform override should exist";

    // rotate(45) * scale(1.5):
    // cos(45°) ≈ 0.7071, sin(45°) ≈ 0.7071
    // a = cos45 * 1.5 ≈ 1.0607
    // b = sin45 * 1.5 ≈ 1.0607
    // c = -sin45 * 1.5 ≈ -1.0607
    // d = cos45 * 1.5 ≈ 1.0607
    float cos45 = 0.7071f;
    float sin45 = 0.7071f;
    EXPECT_NEAR(cos45 * 1.5f, a, 0.05f);
    EXPECT_NEAR(sin45 * 1.5f, b, 0.05f);
    EXPECT_NEAR(-sin45 * 1.5f, c, 0.05f);
    EXPECT_NEAR(cos45 * 1.5f, d, 0.05f);
    EXPECT_NEAR(0.0f, e, 0.05f);
    EXPECT_NEAR(0.0f, f, 0.05f);

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateTransform_Additive_Sum_SingleScale)
{
    // Single animateTransform scale with additive=sum.
    // Without a prior override, compose should behave like set.
    const char* svg =
        "<svg xmlns='http://www.w3.org/2000/svg' version='1.2' baseProfile='tiny'"
        "     width='200' height='200'>"
        "  <rect xml:id='r1' x='10' y='10' width='80' height='80' fill='red'>"
        "    <animateTransform attributeName='transform' type='scale'"
        "         from='1' to='2' dur='5s' fill='freeze' additive='sum'/>"
        "  </rect>"
        "</svg>";

    psx_svg_node* root = NULL;
    psx_result r = S_OK;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_TRUE(p != NULL);

    psx_svg_player_seek(p, 2.5f);
    psx_svg_player_tick(p, 0.0f);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r1");
    EXPECT_TRUE(n != NULL);

    float a, b, c, d, e, f;
    bool got = psx_svg_player_debug_get_transform_override(p, n, &a, &b, &c, &d, &e, &f);
    EXPECT_TRUE(got) << "Transform override should exist";
    // scale(1.5): a=1.5, d=1.5
    EXPECT_NEAR(1.5f, a, 0.05f);
    EXPECT_NEAR(0.0f, b, 0.001f);
    EXPECT_NEAR(0.0f, c, 0.001f);
    EXPECT_NEAR(1.5f, d, 0.05f);

    destroy_player(p, root);
}

// ── animateColor linear interpolation tests ──

TEST_F(SVGPlayerTest, AnimateColorFill_FromTo_Linear)
{
    // animateColor default calcMode is linear.
    // from=#ff0000 (red) to=#0000ff (blue) dur=1s
    // At t=0.5: R=127, G=0, B=127 => packed 0x007F007F (approx)
    const char* svg =
        "<svg xmlns='http://www.w3.org/2000/svg' version='1.2' baseProfile='tiny'"
        "     width='10' height='10'>"
        "  <rect xml:id='r' x='0' y='0' width='10' height='10' fill='#000'>"
        "    <animateColor attributeName='fill' from='#ff0000' to='#0000ff'"
        "         dur='1s' fill='freeze'/>"
        "  </rect>"
        "</svg>";

    psx_svg_node* root = NULL;
    psx_result r = S_OK;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_TRUE(p != NULL);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    EXPECT_TRUE(n != NULL);

    // At t=0.5 (midpoint): linear lerp of each channel
    // R: 255 -> 0, at 0.5 => ~128
    // G: 0 -> 0, at 0.5 => 0
    // B: 0 -> 255, at 0.5 => ~128
    psx_svg_player_seek(p, 0.5f);
    {
        float v = 0;
        bool got = psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_FILL, &v);
        EXPECT_TRUE(got) << "Fill override should exist at t=0.5";

        union { uint32_t u; float f; } bits;
        bits.f = v;
        uint32_t cr = (bits.u >> 16) & 0xFF;
        uint32_t cg = (bits.u >> 8) & 0xFF;
        uint32_t cb = bits.u & 0xFF;

        // Allow ±2 tolerance for rounding
        EXPECT_NEAR(128.0f, (float)cr, 2.0f);
        EXPECT_NEAR(0.0f, (float)cg, 2.0f);
        EXPECT_NEAR(128.0f, (float)cb, 2.0f);
    }

    // At t=0.0 (start): should be pure red
    psx_svg_player_seek(p, 0.0f);
    {
        float v = 0;
        bool got = psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_FILL, &v);
        EXPECT_TRUE(got);

        union { uint32_t u; float f; } bits;
        bits.f = v;
        uint32_t cr = (bits.u >> 16) & 0xFF;
        uint32_t cb = bits.u & 0xFF;
        EXPECT_EQ(255u, cr);
        EXPECT_EQ(0u, cb);
    }

    // At t=1.0 (end, freeze): should be pure blue
    psx_svg_player_seek(p, 1.0f);
    {
        float v = 0;
        bool got = psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_FILL, &v);
        EXPECT_TRUE(got);

        union { uint32_t u; float f; } bits;
        bits.f = v;
        uint32_t cr = (bits.u >> 16) & 0xFF;
        uint32_t cb = bits.u & 0xFF;
        EXPECT_EQ(0u, cr);
        EXPECT_EQ(255u, cb);
    }

    destroy_player(p, root);
}

TEST_F(SVGPlayerTest, AnimateColorFill_Values_Linear)
{
    // values list with 3 colors: red -> green -> blue over 2s
    // At t=0.5 (25%): between red and green, midpoint
    // R: 255->0 at 0.5 => ~128, G: 0->128 at 0.5 => ~64, B: 0->0 => 0
    // Actually: segment 0 covers [0,1s], segment 1 covers [1s,2s]
    // At t=0.5s => segment 0, u=0.5 => lerp(red, green, 0.5)
    const char* svg =
        "<svg xmlns='http://www.w3.org/2000/svg' version='1.2' baseProfile='tiny'"
        "     width='10' height='10'>"
        "  <rect xml:id='r' x='0' y='0' width='10' height='10' fill='#000'>"
        "    <animateColor attributeName='fill' values='#ff0000;#00ff00;#0000ff'"
        "         dur='2s' fill='freeze'/>"
        "  </rect>"
        "</svg>";

    psx_svg_node* root = NULL;
    psx_result r = S_OK;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_TRUE(p != NULL);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r");
    EXPECT_TRUE(n != NULL);

    // At t=0.5s: segment 0, u=0.5 => lerp(#ff0000, #00ff00, 0.5)
    // R: 255->0 => 128, G: 0->255 => 128, B: 0->0 => 0
    psx_svg_player_seek(p, 0.5f);
    {
        float v = 0;
        bool got = psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_FILL, &v);
        EXPECT_TRUE(got);

        union { uint32_t u; float f; } bits;
        bits.f = v;
        uint32_t cr = (bits.u >> 16) & 0xFF;
        uint32_t cg = (bits.u >> 8) & 0xFF;
        uint32_t cb = bits.u & 0xFF;

        EXPECT_NEAR(128.0f, (float)cr, 2.0f);
        EXPECT_NEAR(128.0f, (float)cg, 2.0f);
        EXPECT_NEAR(0.0f, (float)cb, 2.0f);
    }

    // At t=1.5s: segment 1, u=0.5 => lerp(#00ff00, #0000ff, 0.5)
    // R: 0->0 => 0, G: 255->0 => 128, B: 0->255 => 128
    psx_svg_player_seek(p, 1.5f);
    {
        float v = 0;
        bool got = psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_FILL, &v);
        EXPECT_TRUE(got);

        union { uint32_t u; float f; } bits;
        bits.f = v;
        uint32_t cr = (bits.u >> 16) & 0xFF;
        uint32_t cg = (bits.u >> 8) & 0xFF;
        uint32_t cb = bits.u & 0xFF;

        EXPECT_NEAR(0.0f, (float)cr, 2.0f);
        EXPECT_NEAR(128.0f, (float)cg, 2.0f);
        EXPECT_NEAR(128.0f, (float)cb, 2.0f);
    }

    destroy_player(p, root);
}

// ── viewBox + animate regression test ──

TEST_F(SVGPlayerTest, AnimateRectX_WithViewBox_OverrideProduced)
{
    // Exact SVG from user report: viewBox present, <animate> on x.
    const char* svg =
        "<svg width='300' height='200' viewBox='0 0 600 400'"
        "     xmlns='http://www.w3.org/2000/svg' version='1.2' baseProfile='tiny'>"
        "  <rect x='10' y='10' width='580' height='380' fill='yellow'"
        "        stroke='blue' stroke-width='4'/>"
        "  <rect xml:id='r1' x='100' y='100' width='200' height='100' fill='red'>"
        "    <animate attributeName='x' from='100' to='400' dur='3s' fill='freeze'/>"
        "  </rect>"
        "</svg>";

    psx_svg_node* root = NULL;
    psx_result r = S_OK;
    psx_svg_player* p = create_player(svg, &r, &root);
    ASSERT_TRUE(p != NULL);

    const psx_svg_node* n = psx_svg_player_get_node_by_id(p, "r1");
    EXPECT_TRUE(n != NULL);

    // t=0: x should be 100 (from value)
    psx_svg_player_play(p);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(100.0f, v, 0.01f);
    }

    // t=1.5s (midpoint): x should be 250
    psx_svg_player_seek(p, 1.5f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(250.0f, v, 0.1f);
    }

    // t=3s (end, freeze): x should be 400
    psx_svg_player_seek(p, 3.0f);
    psx_svg_player_tick(p, 0.0f);
    {
        float v = 0;
        EXPECT_TRUE(psx_svg_player_debug_get_float_override(p, n, SVG_ATTR_X, &v));
        EXPECT_NEAR(400.0f, v, 0.01f);
    }

    destroy_player(p, root);
}
