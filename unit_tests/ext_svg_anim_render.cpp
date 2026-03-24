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

#define SNAPSHOT_PATH "svg_anim"
#include "test.h"

#include "psx_svg_node.h"
#include "psx_svg_render.h"
#include "psx_svg_player.h"

#include <string.h>

class SVGAnimRenderTest : public ::testing::Test
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

    void SetUp()
    {
        root = NULL;
        player = NULL;
        clear_test_canvas();
    }

    void TearDown()
    {
        if (player) {
            psx_svg_player_destroy(player);
            player = NULL;
        }
        if (root) {
            psx_svg_node_destroy(root);
            root = NULL;
        }
    }

    void load_anim(const char* data)
    {
        root = psx_svg_load_data(data, (uint32_t)strlen(data));
        psx_result err = S_OK;
        player = psx_svg_player_create(root, &err);
    }

    void draw_frame(float t)
    {
        clear_test_canvas();
        psx_svg_player_seek(player, t);
        ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
        psx_svg_player_draw(player, ctx);
        ps_context_unref(ctx);
    }

    psx_svg_node* root;
    psx_svg_player* player;
};

/* ========================================================================
 * 1. <animate> — Rect geometry (x, y, width, height)
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateRectX_FromTo)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"80\" height=\"80\" fill=\"red\">"
        "  <animate attributeName=\"x\" from=\"10\" to=\"210\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_x_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_x_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_x_t2);
}

TEST_F(SVGAnimRenderTest, AnimateRectY_FromTo)
{
    const char* svg =
        "<svg width=\"200\" height=\"320\">"
        "<rect x=\"50\" y=\"10\" width=\"80\" height=\"80\" fill=\"blue\">"
        "  <animate attributeName=\"y\" from=\"10\" to=\"210\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_y_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_y_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_y_t2);
}

TEST_F(SVGAnimRenderTest, AnimateRectWidth_FromTo)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"40\" height=\"80\" fill=\"green\">"
        "  <animate attributeName=\"width\" from=\"40\" to=\"280\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_w_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_w_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_w_t2);
}

TEST_F(SVGAnimRenderTest, AnimateRectHeight_FromTo)
{
    const char* svg =
        "<svg width=\"200\" height=\"320\">"
        "<rect x=\"50\" y=\"10\" width=\"80\" height=\"40\" fill=\"orange\">"
        "  <animate attributeName=\"height\" from=\"40\" to=\"280\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_h_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_h_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_h_t2);
}

TEST_F(SVGAnimRenderTest, AnimateRectRxRy_FromTo)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<rect x=\"20\" y=\"20\" width=\"160\" height=\"160\" rx=\"0\" ry=\"0\" fill=\"purple\">"
        "  <animate attributeName=\"rx\" from=\"0\" to=\"80\" dur=\"2s\" fill=\"freeze\"/>"
        "  <animate attributeName=\"ry\" from=\"0\" to=\"80\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_rxry_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_rxry_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_rect_rxry_t2);
}

/* ========================================================================
 * 2. <animate> — Circle geometry (cx, cy, r)
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateCircleCx_FromTo)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<circle cx=\"50\" cy=\"100\" r=\"40\" fill=\"red\">"
        "  <animate attributeName=\"cx\" from=\"50\" to=\"270\" dur=\"2s\" fill=\"freeze\"/>"
        "</circle>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_circle_cx_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_circle_cx_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_circle_cx_t2);
}

TEST_F(SVGAnimRenderTest, AnimateCircleR_FromTo)
{
    const char* svg =
        "<svg width=\"300\" height=\"300\">"
        "<circle cx=\"150\" cy=\"150\" r=\"20\" fill=\"blue\">"
        "  <animate attributeName=\"r\" from=\"20\" to=\"140\" dur=\"2s\" fill=\"freeze\"/>"
        "</circle>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_circle_r_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_circle_r_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_circle_r_t2);
}

/* ========================================================================
 * 3. <animate> — Ellipse geometry (cx, cy, rx, ry)
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateEllipseRxRy_FromTo)
{
    const char* svg =
        "<svg width=\"400\" height=\"300\">"
        "<ellipse cx=\"200\" cy=\"150\" rx=\"30\" ry=\"20\" fill=\"green\">"
        "  <animate attributeName=\"rx\" from=\"30\" to=\"180\" dur=\"2s\" fill=\"freeze\"/>"
        "  <animate attributeName=\"ry\" from=\"20\" to=\"130\" dur=\"2s\" fill=\"freeze\"/>"
        "</ellipse>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_ellipse_rxry_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_ellipse_rxry_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_ellipse_rxry_t2);
}

/* ========================================================================
 * 4. <animate> — Line geometry (x1, y1, x2, y2)
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateLineEndpoint_FromTo)
{
    const char* svg =
        "<svg width=\"300\" height=\"300\">"
        "<line x1=\"10\" y1=\"150\" x2=\"10\" y2=\"150\" stroke=\"black\" stroke-width=\"4\">"
        "  <animate attributeName=\"x2\" from=\"10\" to=\"290\" dur=\"2s\" fill=\"freeze\"/>"
        "  <animate attributeName=\"y2\" from=\"150\" to=\"10\" dur=\"2s\" fill=\"freeze\"/>"
        "</line>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_line_endpoint_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_line_endpoint_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_line_endpoint_t2);
}

/* ========================================================================
 * 5. <animate> — Paint style (opacity, fill-opacity, stroke-opacity, stroke-width)
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateOpacity_FromTo)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<rect x=\"20\" y=\"20\" width=\"160\" height=\"160\" fill=\"red\" opacity=\"1\">"
        "  <animate attributeName=\"opacity\" from=\"1\" to=\"0\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_opacity_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_opacity_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_opacity_t2);
}

TEST_F(SVGAnimRenderTest, AnimateFillOpacity_FromTo)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<rect x=\"20\" y=\"20\" width=\"160\" height=\"160\" fill=\"blue\" fill-opacity=\"1\">"
        "  <animate attributeName=\"fill-opacity\" from=\"1\" to=\"0.1\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_fill_opacity_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_fill_opacity_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_fill_opacity_t2);
}

TEST_F(SVGAnimRenderTest, AnimateStrokeOpacity_FromTo)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<rect x=\"20\" y=\"20\" width=\"160\" height=\"160\" fill=\"none\" stroke=\"red\""
        " stroke-width=\"10\" stroke-opacity=\"1\">"
        "  <animate attributeName=\"stroke-opacity\" from=\"1\" to=\"0.1\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_stroke_opacity_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_stroke_opacity_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_stroke_opacity_t2);
}

TEST_F(SVGAnimRenderTest, AnimateStrokeWidth_FromTo)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<rect x=\"40\" y=\"40\" width=\"120\" height=\"120\" fill=\"none\" stroke=\"green\""
        " stroke-width=\"2\">"
        "  <animate attributeName=\"stroke-width\" from=\"2\" to=\"20\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_stroke_width_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_stroke_width_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_stroke_width_t2);
}

/* ========================================================================
 * 6. <animateColor> — fill / stroke color interpolation
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateColorFill_FromTo)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<rect x=\"20\" y=\"20\" width=\"160\" height=\"160\" fill=\"red\">"
        "  <animateColor attributeName=\"fill\" from=\"red\" to=\"blue\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_color_fill_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_color_fill_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_color_fill_t2);
}

TEST_F(SVGAnimRenderTest, AnimateColorStroke_FromTo)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<rect x=\"30\" y=\"30\" width=\"140\" height=\"140\" fill=\"none\" stroke=\"red\" stroke-width=\"10\">"
        "  <animateColor attributeName=\"stroke\" from=\"red\" to=\"green\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_color_stroke_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_color_stroke_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_color_stroke_t2);
}

/* ========================================================================
 * 7. <animateTransform> — translate, scale, rotate, skewX, skewY
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateTransformTranslate_FromTo)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"60\" height=\"60\" fill=\"red\">"
        "  <animateTransform attributeName=\"transform\" type=\"translate\""
        "   from=\"0,0\" to=\"200,0\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_translate_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_translate_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_translate_t2);
}

TEST_F(SVGAnimRenderTest, AnimateTransformScale_FromTo)
{
    const char* svg =
        "<svg width=\"300\" height=\"300\">"
        "<rect x=\"50\" y=\"50\" width=\"40\" height=\"40\" fill=\"blue\">"
        "  <animateTransform attributeName=\"transform\" type=\"scale\""
        "   from=\"1\" to=\"3\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_scale_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_scale_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_scale_t2);
}

TEST_F(SVGAnimRenderTest, AnimateTransformRotate_FromTo)
{
    const char* svg =
        "<svg width=\"300\" height=\"300\">"
        "<rect x=\"100\" y=\"100\" width=\"100\" height=\"100\" fill=\"green\">"
        "  <animateTransform attributeName=\"transform\" type=\"rotate\""
        "   from=\"0,150,150\" to=\"90,150,150\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_rotate_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_rotate_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_rotate_t2);
}

TEST_F(SVGAnimRenderTest, AnimateTransformSkewX_FromTo)
{
    const char* svg =
        "<svg width=\"300\" height=\"200\">"
        "<rect x=\"80\" y=\"40\" width=\"100\" height=\"100\" fill=\"orange\">"
        "  <animateTransform attributeName=\"transform\" type=\"skewX\""
        "   from=\"0\" to=\"30\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_skewx_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_skewx_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_skewx_t2);
}

TEST_F(SVGAnimRenderTest, AnimateTransformSkewY_FromTo)
{
    const char* svg =
        "<svg width=\"200\" height=\"300\">"
        "<rect x=\"40\" y=\"80\" width=\"100\" height=\"100\" fill=\"purple\">"
        "  <animateTransform attributeName=\"transform\" type=\"skewY\""
        "   from=\"0\" to=\"30\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_skewy_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_skewy_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_transform_skewy_t2);
}

/* ========================================================================
 * 8. <animateMotion> — path following
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateMotionPath)
{
    const char* svg =
        "<svg width=\"400\" height=\"300\">"
        "<rect x=\"-10\" y=\"-10\" width=\"20\" height=\"20\" fill=\"red\">"
        "  <animateMotion path=\"M 50,150 L 350,150\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_motion_path_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_motion_path_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_motion_path_t2);
}

TEST_F(SVGAnimRenderTest, AnimateMotionAutoRotate)
{
    const char* svg =
        "<svg width=\"400\" height=\"400\">"
        "<polygon points=\"-10,-5 10,0 -10,5\" fill=\"blue\">"
        "  <animateMotion path=\"M 50,200 C 100,50 300,50 350,200\" dur=\"2s\""
        "   rotate=\"auto\" fill=\"freeze\"/>"
        "</polygon>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_motion_autorotate_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_motion_autorotate_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_motion_autorotate_t2);
}

/* ========================================================================
 * 9. <set> — discrete property assignment (visibility, display)
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, SetVisibilityHidden)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<rect x=\"20\" y=\"20\" width=\"160\" height=\"160\" fill=\"red\">"
        "  <set attributeName=\"visibility\" to=\"hidden\" begin=\"1s\" dur=\"1s\" fill=\"remove\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    /* t=0.5: visible */
    draw_frame(0.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_vis_t0);

    /* t=1.5: hidden */
    draw_frame(1.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_vis_t1);

    /* t=2.5: visible again (fill=remove) */
    draw_frame(2.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_vis_t2);
}

TEST_F(SVGAnimRenderTest, SetDisplayNone)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<rect x=\"20\" y=\"20\" width=\"160\" height=\"160\" fill=\"blue\">"
        "  <set attributeName=\"display\" to=\"none\" begin=\"1s\" dur=\"1s\" fill=\"remove\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_display_t0);

    draw_frame(1.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_display_t1);

    draw_frame(2.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_display_t2);
}

/* ========================================================================
 * 10. fill="freeze" vs fill="remove"
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, FillFreeze_HoldsEndValue)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"80\" height=\"80\" fill=\"red\">"
        "  <animate attributeName=\"x\" from=\"10\" to=\"210\" dur=\"1s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    /* during animation */
    draw_frame(0.5f);
    EXPECT_SNAPSHOT_EQ(anim_freeze_t0);

    /* at end */
    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_freeze_t1);

    /* after end: freeze holds */
    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_freeze_t2);
}

TEST_F(SVGAnimRenderTest, FillRemove_RevertsToBase)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"80\" height=\"80\" fill=\"green\">"
        "  <animate attributeName=\"x\" from=\"10\" to=\"210\" dur=\"1s\" fill=\"remove\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    /* during animation */
    draw_frame(0.5f);
    EXPECT_SNAPSHOT_EQ(anim_remove_t0);

    /* after end: reverts to base x=10 */
    draw_frame(1.5f);
    EXPECT_SNAPSHOT_EQ(anim_remove_t1);
}

/* ========================================================================
 * 11. repeatCount
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, RepeatCount_SecondCycle)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"80\" height=\"80\" fill=\"blue\">"
        "  <animate attributeName=\"x\" from=\"10\" to=\"210\" dur=\"1s\""
        "   repeatCount=\"3\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    /* first cycle mid */
    draw_frame(0.5f);
    EXPECT_SNAPSHOT_EQ(anim_repeat_t0);

    /* second cycle mid (t=1.5) */
    draw_frame(1.5f);
    EXPECT_SNAPSHOT_EQ(anim_repeat_t1);

    /* after all repeats: freeze at end */
    draw_frame(3.5f);
    EXPECT_SNAPSHOT_EQ(anim_repeat_t2);
}

/* ========================================================================
 * 12. calcMode — discrete, spline, paced
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, CalcModeDiscrete)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"80\" height=\"80\" fill=\"red\">"
        "  <animate attributeName=\"x\" values=\"10;110;210\" dur=\"3s\""
        "   calcMode=\"discrete\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    /* t=0: first value */
    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_discrete_t0);

    /* t=1.5: second value */
    draw_frame(1.5f);
    EXPECT_SNAPSHOT_EQ(anim_discrete_t1);

    /* t=2.5: third value */
    draw_frame(2.5f);
    EXPECT_SNAPSHOT_EQ(anim_discrete_t2);
}

TEST_F(SVGAnimRenderTest, CalcModeSpline)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"80\" height=\"80\" fill=\"green\">"
        "  <animate attributeName=\"x\" values=\"10;210\" dur=\"2s\""
        "   calcMode=\"spline\" keyTimes=\"0;1\" keySplines=\"0.42 0 0.58 1\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_spline_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_spline_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_spline_t2);
}

TEST_F(SVGAnimRenderTest, CalcModePaced)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<circle cx=\"50\" cy=\"100\" r=\"30\" fill=\"blue\">"
        "  <animate attributeName=\"cx\" values=\"50;160;270\" dur=\"2s\""
        "   calcMode=\"paced\" fill=\"freeze\"/>"
        "</circle>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_paced_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_paced_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_paced_t2);
}

/* ========================================================================
 * 13. Text animation (x, y, font-size, opacity) — use EXPECT_SYS_SNAPSHOT_EQ
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateTextX_FromTo)
{
    const char* svg =
        "<svg width=\"400\" height=\"100\">"
        "<text x=\"10\" y=\"60\" font-family=\"sans-serif\" font-size=\"40\" fill=\"black\">"
        "Hello"
        "  <animate attributeName=\"x\" from=\"10\" to=\"300\" dur=\"2s\" fill=\"freeze\"/>"
        "</text>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_x_t0);

    draw_frame(1.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_x_t1);

    draw_frame(2.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_x_t2);
}

TEST_F(SVGAnimRenderTest, AnimateTextY_FromTo)
{
    const char* svg =
        "<svg width=\"200\" height=\"300\">"
        "<text x=\"20\" y=\"40\" font-family=\"sans-serif\" font-size=\"40\" fill=\"black\">"
        "Hi"
        "  <animate attributeName=\"y\" from=\"40\" to=\"260\" dur=\"2s\" fill=\"freeze\"/>"
        "</text>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_y_t0);

    draw_frame(1.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_y_t1);

    draw_frame(2.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_y_t2);
}

TEST_F(SVGAnimRenderTest, AnimateTextFontSize_FromTo)
{
    const char* svg =
        "<svg width=\"400\" height=\"200\">"
        "<text x=\"10\" y=\"100\" font-family=\"sans-serif\" font-size=\"12\" fill=\"red\">"
        "Grow"
        "  <animate attributeName=\"font-size\" from=\"12\" to=\"80\" dur=\"2s\" fill=\"freeze\"/>"
        "</text>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_fontsize_t0);

    draw_frame(1.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_fontsize_t1);

    draw_frame(2.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_fontsize_t2);
}

TEST_F(SVGAnimRenderTest, AnimateTextOpacity_FromTo)
{
    const char* svg =
        "<svg width=\"200\" height=\"100\">"
        "<text x=\"10\" y=\"60\" font-family=\"sans-serif\" font-size=\"40\" fill=\"blue\">"
        "Fade"
        "  <animate attributeName=\"opacity\" from=\"1\" to=\"0\" dur=\"2s\" fill=\"freeze\"/>"
        "</text>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_opacity_t0);

    draw_frame(1.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_opacity_t1);

    draw_frame(2.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_opacity_t2);
}

/* ========================================================================
 * 14. additive="sum" — transform composition
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AdditiveSum_TranslateOnRotated)
{
    const char* svg =
        "<svg width=\"300\" height=\"300\">"
        "<rect x=\"100\" y=\"100\" width=\"60\" height=\"60\" fill=\"red\""
        " transform=\"rotate(45,130,130)\">"
        "  <animateTransform attributeName=\"transform\" type=\"translate\""
        "   from=\"0,0\" to=\"100,0\" dur=\"2s\" fill=\"freeze\" additive=\"sum\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_additive_sum_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_additive_sum_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_additive_sum_t2);
}

/* ========================================================================
 * 15. accumulate="sum" — value accumulation across repeats
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AccumulateSum_AcrossRepeats)
{
    const char* svg =
        "<svg width=\"400\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"40\" height=\"40\" fill=\"blue\">"
        "  <animate attributeName=\"x\" from=\"0\" to=\"100\" dur=\"1s\""
        "   repeatCount=\"3\" accumulate=\"sum\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    /* first cycle mid: x ~ 50 */
    draw_frame(0.5f);
    EXPECT_SNAPSHOT_EQ(anim_accumulate_t0);

    /* second cycle mid: x ~ 100 + 50 = 150 */
    draw_frame(1.5f);
    EXPECT_SNAPSHOT_EQ(anim_accumulate_t1);

    /* third cycle end: x ~ 300, freeze */
    draw_frame(3.5f);
    EXPECT_SNAPSHOT_EQ(anim_accumulate_t2);
}

/* ========================================================================
 * 16. stroke-dasharray animation
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateStrokeDasharray)
{
    const char* svg =
        "<svg width=\"300\" height=\"100\">"
        "<line x1=\"10\" y1=\"50\" x2=\"290\" y2=\"50\" stroke=\"black\" stroke-width=\"4\""
        " stroke-dasharray=\"5,5\">"
        "  <animate attributeName=\"stroke-dasharray\" from=\"5,5\" to=\"20,10\" dur=\"2s\" fill=\"freeze\"/>"
        "</line>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_dasharray_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_dasharray_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_dasharray_t2);
}

/* ========================================================================
 * 17. stroke-dashoffset animation
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateStrokeDashoffset)
{
    const char* svg =
        "<svg width=\"300\" height=\"100\">"
        "<line x1=\"10\" y1=\"50\" x2=\"290\" y2=\"50\" stroke=\"black\" stroke-width=\"4\""
        " stroke-dasharray=\"20,10\" stroke-dashoffset=\"0\">"
        "  <animate attributeName=\"stroke-dashoffset\" from=\"0\" to=\"30\" dur=\"2s\" fill=\"freeze\"/>"
        "</line>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_dashoffset_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_dashoffset_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_dashoffset_t2);
}

/* ========================================================================
 * 18. Gradient stop animation (stop-color, stop-opacity, offset)
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateGradientStopColor)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<defs><linearGradient id=\"g1\">"
        "  <stop offset=\"0\" stop-color=\"red\">"
        "    <animateColor attributeName=\"stop-color\" from=\"red\" to=\"blue\" dur=\"2s\" fill=\"freeze\"/>"
        "  </stop>"
        "  <stop offset=\"1\" stop-color=\"yellow\"/>"
        "</linearGradient></defs>"
        "<rect x=\"0\" y=\"0\" width=\"200\" height=\"200\" fill=\"url(#g1)\"/>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_grad_stop_color_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_grad_stop_color_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_grad_stop_color_t2);
}

TEST_F(SVGAnimRenderTest, AnimateGradientStopOpacity)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<defs><linearGradient id=\"g1\">"
        "  <stop offset=\"0\" stop-color=\"red\" stop-opacity=\"1\">"
        "    <animate attributeName=\"stop-opacity\" from=\"1\" to=\"0\" dur=\"2s\" fill=\"freeze\"/>"
        "  </stop>"
        "  <stop offset=\"1\" stop-color=\"blue\"/>"
        "</linearGradient></defs>"
        "<rect x=\"0\" y=\"0\" width=\"200\" height=\"200\" fill=\"url(#g1)\"/>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_grad_stop_opacity_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_grad_stop_opacity_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_grad_stop_opacity_t2);
}

TEST_F(SVGAnimRenderTest, AnimateGradientOffset)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<defs><linearGradient id=\"g1\">"
        "  <stop offset=\"0.2\" stop-color=\"red\">"
        "    <animate attributeName=\"offset\" from=\"0.2\" to=\"0.8\" dur=\"2s\" fill=\"freeze\"/>"
        "  </stop>"
        "  <stop offset=\"1\" stop-color=\"blue\"/>"
        "</linearGradient></defs>"
        "<rect x=\"0\" y=\"0\" width=\"200\" height=\"200\" fill=\"url(#g1)\"/>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_grad_offset_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_grad_offset_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_grad_offset_t2);
}

/* ========================================================================
 * 19. begin offset — delayed start
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, BeginOffset_DelayedStart)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"80\" height=\"80\" fill=\"red\">"
        "  <animate attributeName=\"x\" from=\"10\" to=\"210\" begin=\"1s\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    /* before begin: static */
    draw_frame(0.5f);
    EXPECT_SNAPSHOT_EQ(anim_begin_offset_t0);

    /* mid animation */
    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_begin_offset_t1);

    /* after end: freeze */
    draw_frame(3.5f);
    EXPECT_SNAPSHOT_EQ(anim_begin_offset_t2);
}

/* ========================================================================
 * 20. by attribute — additive from base
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, ByAttribute_AdditiveFromBase)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"80\" height=\"80\" fill=\"orange\">"
        "  <animate attributeName=\"x\" by=\"200\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_by_attr_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_by_attr_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_by_attr_t2);
}

/* ========================================================================
 * 21. values list with keyTimes
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, ValuesWithKeyTimes)
{
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"80\" height=\"80\" fill=\"purple\">"
        "  <animate attributeName=\"x\" values=\"10;210;110\" keyTimes=\"0;0.5;1\""
        "   dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_keytimes_t0);

    /* t=1s: at keyTime=0.5, x=210 */
    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_keytimes_t1);

    /* t=2s: at keyTime=1.0, x=110 */
    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_keytimes_t2);
}

/* ========================================================================
 * 22. Multiple animations on same element
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, MultipleAnimations_SameElement)
{
    const char* svg =
        "<svg width=\"320\" height=\"320\">"
        "<rect x=\"10\" y=\"10\" width=\"60\" height=\"60\" fill=\"red\">"
        "  <animate attributeName=\"x\" from=\"10\" to=\"250\" dur=\"2s\" fill=\"freeze\"/>"
        "  <animate attributeName=\"y\" from=\"10\" to=\"250\" dur=\"2s\" fill=\"freeze\"/>"
        "  <animate attributeName=\"width\" from=\"60\" to=\"30\" dur=\"2s\" fill=\"freeze\"/>"
        "  <animate attributeName=\"height\" from=\"60\" to=\"30\" dur=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_multi_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_multi_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_multi_t2);
}

/* ========================================================================
 * 23. <set> stroke-linecap / stroke-linejoin
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, SetStrokeLinecap)
{
    const char* svg =
        "<svg width=\"200\" height=\"100\">"
        "<line x1=\"20\" y1=\"50\" x2=\"180\" y2=\"50\" stroke=\"black\" stroke-width=\"20\""
        " stroke-linecap=\"butt\">"
        "  <set attributeName=\"stroke-linecap\" to=\"round\" begin=\"1s\" dur=\"1s\" fill=\"remove\"/>"
        "</line>"
        "</svg>";
    load_anim(svg);

    /* butt */
    draw_frame(0.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_linecap_t0);

    /* round */
    draw_frame(1.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_linecap_t1);

    /* butt again */
    draw_frame(2.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_linecap_t2);
}

TEST_F(SVGAnimRenderTest, SetStrokeLinejoin)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<polyline points=\"20,180 100,20 180,180\" fill=\"none\" stroke=\"black\""
        " stroke-width=\"20\" stroke-linejoin=\"miter\">"
        "  <set attributeName=\"stroke-linejoin\" to=\"round\" begin=\"1s\" dur=\"1s\" fill=\"remove\"/>"
        "</polyline>"
        "</svg>";
    load_anim(svg);

    /* miter */
    draw_frame(0.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_linejoin_t0);

    /* round */
    draw_frame(1.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_linejoin_t1);

    /* miter again */
    draw_frame(2.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_linejoin_t2);
}

/* ========================================================================
 * 24. stroke-miterlimit animation
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateStrokeMiterlimit)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<polyline points=\"20,180 100,20 180,180\" fill=\"none\" stroke=\"black\""
        " stroke-width=\"15\" stroke-linejoin=\"miter\" stroke-miterlimit=\"4\">"
        "  <animate attributeName=\"stroke-miterlimit\" from=\"4\" to=\"1\" dur=\"2s\" fill=\"freeze\"/>"
        "</polyline>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_miterlimit_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_miterlimit_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_miterlimit_t2);
}

/* ========================================================================
 * 25. <set> fill-rule
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, SetFillRule)
{
    const char* svg =
        "<svg width=\"200\" height=\"200\">"
        "<path d=\"M 40,20 L 180,20 L 60,160 L 100,0 L 140,160 Z\" fill=\"red\" fill-rule=\"nonzero\">"
        "  <set attributeName=\"fill-rule\" to=\"evenodd\" begin=\"1s\" dur=\"1s\" fill=\"remove\"/>"
        "</path>"
        "</svg>";
    load_anim(svg);

    /* nonzero */
    draw_frame(0.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_fillrule_t0);

    /* evenodd */
    draw_frame(1.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_fillrule_t1);

    /* nonzero again */
    draw_frame(2.5f);
    EXPECT_SNAPSHOT_EQ(anim_set_fillrule_t2);
}

/* ========================================================================
 * 26. Combined: color + geometry animation
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, CombinedColorAndGeometry)
{
    const char* svg =
        "<svg width=\"300\" height=\"200\">"
        "<circle cx=\"50\" cy=\"100\" r=\"40\" fill=\"red\">"
        "  <animate attributeName=\"cx\" from=\"50\" to=\"250\" dur=\"2s\" fill=\"freeze\"/>"
        "  <animate attributeName=\"r\" from=\"40\" to=\"20\" dur=\"2s\" fill=\"freeze\"/>"
        "  <animateColor attributeName=\"fill\" from=\"red\" to=\"blue\" dur=\"2s\" fill=\"freeze\"/>"
        "</circle>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SNAPSHOT_EQ(anim_combined_t0);

    draw_frame(1.0f);
    EXPECT_SNAPSHOT_EQ(anim_combined_t1);

    draw_frame(2.0f);
    EXPECT_SNAPSHOT_EQ(anim_combined_t2);
}

/* ========================================================================
 * 27. Text with animateColor — use EXPECT_SYS_SNAPSHOT_EQ
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, AnimateTextColor_FromTo)
{
    const char* svg =
        "<svg width=\"300\" height=\"100\">"
        "<text x=\"10\" y=\"60\" font-family=\"sans-serif\" font-size=\"40\" fill=\"red\">"
        "Color"
        "  <animateColor attributeName=\"fill\" from=\"red\" to=\"blue\" dur=\"2s\" fill=\"freeze\"/>"
        "</text>"
        "</svg>";
    load_anim(svg);

    draw_frame(0.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_color_t0);

    draw_frame(1.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_color_t1);

    draw_frame(2.0f);
    EXPECT_SYS_SNAPSHOT_EQ(anim_text_color_t2);
}

/* ========================================================================
 * 28. min/max timing constraints
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, MinMaxConstraint)
{
    /* dur=1s, min=2s => active duration clamped to 2s */
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"80\" height=\"80\" fill=\"red\">"
        "  <animate attributeName=\"x\" from=\"10\" to=\"210\" dur=\"1s\" min=\"2s\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    /* t=0.5: mid first cycle */
    draw_frame(0.5f);
    EXPECT_SNAPSHOT_EQ(anim_minmax_t0);

    /* t=1.5: in extended duration (second cycle due to min) */
    draw_frame(1.5f);
    EXPECT_SNAPSHOT_EQ(anim_minmax_t1);

    /* t=2.5: after min-extended end, freeze */
    draw_frame(2.5f);
    EXPECT_SNAPSHOT_EQ(anim_minmax_t2);
}

/* ========================================================================
 * 29. restart attribute
 * ======================================================================== */

TEST_F(SVGAnimRenderTest, RestartNever_BeginList)
{
    /* restart=never with begin list: only first activation */
    const char* svg =
        "<svg width=\"320\" height=\"200\">"
        "<rect x=\"10\" y=\"50\" width=\"80\" height=\"80\" fill=\"green\">"
        "  <animate attributeName=\"x\" from=\"10\" to=\"210\" begin=\"0s;2s\" dur=\"1s\""
        "   restart=\"never\" fill=\"freeze\"/>"
        "</rect>"
        "</svg>";
    load_anim(svg);

    /* first activation mid */
    draw_frame(0.5f);
    EXPECT_SNAPSHOT_EQ(anim_restart_never_t0);

    /* after first end, freeze at 210 */
    draw_frame(1.5f);
    EXPECT_SNAPSHOT_EQ(anim_restart_never_t1);

    /* t=2.5: second begin ignored, still frozen at 210 */
    draw_frame(2.5f);
    EXPECT_SNAPSHOT_EQ(anim_restart_never_t2);
}
