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
#include "timeuse.h"

#include "psx_svg.h"
#include "psx_svg_render.h"

class SVGRenderTest : public ::testing::Test
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
        root = NULL;
        clear_test_canvas();
    }

    void clear()
    {
        clear_test_canvas();
    }

    void load(const char* data)
    {
        root = psx_svg_load_data(data, (uint32_t)strlen(data));
        draw_list = psx_svg_render_create(root);
    }

    void release(void)
    {
        if (root) {
            psx_svg_node_destroy(root);
            root = NULL;
        }
        if (draw_list) {
            psx_svg_render_destroy(draw_list);
            draw_list = NULL;
        }
    }

    void draw_svg(const char* data)
    {
        load(data);
        draw_svg_list();
        release();
    }

    void draw_svg_list(void)
    {
        clear();
        ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
        psx_svg_draw(ctx, draw_list);
        ps_context_unref(ctx);
    }

    void TearDown() override
    {
        release();
    }

    psx_svg_node* root;
    psx_svg_render_list* draw_list;
};

TEST_F(SVGRenderTest, SimpleTest)
{
    const char* svg_data = "<svg width=100 height=100 viewport-fill=\"blue\" viewport-fill-opacity=0.6></svg>";
    load(svg_data);
    EXPECT_NE(nullptr, draw_list);
    draw_svg_list();
    EXPECT_SNAPSHOT_EQ(svg_simple);
    release();
}

TEST_F(SVGRenderTest, ShapesTest)
{
    const char * svg_rect_transform = "<svg><rect fill=\"rgb(0, 0, 200)\" x=\"0\" y=\"0\" width=\"100\" height=\"100\""
                                  " transform=\"translate(50, 50) rotate(45) translate(-50, -50)\"/></svg>";
    draw_svg(svg_rect_transform);
    EXPECT_SNAPSHOT_EQ(svg_rect_transform);


    const char * svg_shapes = "<svg><rect fill=\"red\" x=\"0\" y=\"0\" width=\"100\" height=\"100\"/>"
                              "<circle fill=\"red\" cx=\"100\" cy=\"100\" r=\"50\"/>"
                              "<ellipse stroke=\"red\" fill=\"none\" cx=\"200\" cy=\"200\" rx=\"100\" ry=50/></svg>";
    draw_svg(svg_shapes);
    EXPECT_SNAPSHOT_EQ(svg_shapes);


#if 0
    const char * svg_linear_gradient = "<svg width=\"4cm\" height=\"2cm\" viewBox=\"0 0 400 200\">"
                                       "<defs><linearGradient xml:id=\"MyGradient\">"
                                       "<stop offset=\"0.05\" stop-color=\"#F60\"/>"
                                       "<stop offset=\"0.95\" stop-color=\"#FF6\"/>"
                                       "</linearGradient></defs>"
                                       "<rect fill=\"url(#MyGradient)\" stroke=\"black\" stroke-width=\"5\""
                                       " x=\"100\" y=\"100\" width=\"600\" height=\"200\"/></svg>";
    draw_svg(svg_linear_gradient);
    EXPECT_SNAPSHOT_EQ(svg_linear_gradient);
#endif
    

}

TEST_F(SVGRenderTest, ComplexPath)
{
    const char * svg_complex_1 = "<svg width=5cm height=4cm viewBox=\"0 0 500 400\">"
                             "<rect fill=none stroke=blue stroke-width=1 x=1 y=1 width=498 height=398 />"
                             "<polyline fill=none stroke=#888888 stroke-width=1 points=\"100,200 100,100\" />"
                             "<polyline fill=none stroke=#888888 stroke-width=1 points=\"250,100 250,200\" />"
                             "<polyline fill=none stroke=#888888 stroke-width=1 points=\"250,200 250,300\" />"
                             "<polyline fill=none stroke=#888888 stroke-width=1 points=\"400,300 400,200\" />"
                             "<path fill=none stroke=red stroke-width=5 d=\"M100,200 C100,100 250,100 250,200"
                             " S400,300 400,200\" />"
                             "<circle fill=#888888 stroke=none stroke-width=2 cx=100 cy=200 r=10 />"
                             "<circle fill=#888888 stroke=none stroke-width=2 cx=250 cy=200 r=10 />"
                             "<circle fill=#888888 stroke=none stroke-width=2 cx=400 cy=200 r=10 />"
                             "<circle fill=#888888 stroke=none cx=100 cy=100 r=10 />"
                             "<circle fill=#888888 stroke=none cx=250 cy=100 r=10 />"
                             "<circle fill=#888888 stroke=none cx=400 cy=300 r=10 />"
                             "<circle fill=none stroke=blue stroke-width=4 cx=250 cy=300 r=9 />"
                             "<text font-size=22 font-family=\"Verdana\" x=25 y=70>M100,200 C100,100 250,100 250,200</text>"
                             "<text font-size=22 font-family=\"Verdana\" x=325 y=350>S400,300 400,200</text></svg>";
    draw_svg(svg_complex_1);
    EXPECT_SNAPSHOT_EQ(svg_complex_1);



}

TEST_F(SVGRenderTest, GroupTest)
{
    const char * svg_use_simple = "<svg><use xlink:href=\"#r1\" x=100 y=100/>"
                           "<rect xml:id=\"r1\" x=20 width=50 height=50 fill=green/>"
                           "</svg>";

    draw_svg(svg_use_simple);
    EXPECT_SNAPSHOT_EQ(svg_use_simple);

    const char * svg_use_group = "<svg><use xlink:href=\"#g1\" x=50 y=50/>"
                               "<g fill=\"blue\" xml:id=\"g1\">"
                               "<rect width=20 height=20/>"
                               "<rect x=30 width=20 height=20 fill=green/>"
                               "</g></svg>";
    draw_svg(svg_use_group);
    EXPECT_SNAPSHOT_EQ(svg_use_group);
}
