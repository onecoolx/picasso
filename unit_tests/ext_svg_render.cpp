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

#define SNAPSHOT_PATH "svg"
#include "test.h"
#include "timeuse.h"

#include "psx_svg_node.h"
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
        draw_list = psx_svg_render_list_create(root);
    }

    void release(void)
    {
        if (root) {
            psx_svg_node_destroy(root);
            root = NULL;
        }
        if (draw_list) {
            psx_svg_render_list_destroy(draw_list);
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
        psx_svg_render_list_draw(ctx, draw_list);
        ps_context_unref(ctx);
    }

    void TearDown() override
    {
        release();
    }

    psx_svg_node* root;
    psx_svg_render_list* draw_list;
};

TEST_F(SVGRenderTest, ViewportTest)
{
    const char* svg_data = "<svg width=100 height=100 viewport-fill=\"blue\" viewport-fill-opacity=0.6></svg>";
    load(svg_data);
    EXPECT_NE(nullptr, draw_list);
    draw_svg_list();
    EXPECT_SNAPSHOT_EQ(svg_simple);
    release();

    const char* svg_viewport = "<svg width=\"100px\" height=\"100px\" viewport-fill=\"green\"></svg>";

    draw_svg(svg_viewport);
    EXPECT_SNAPSHOT_EQ(svg_viewport);

    const char* svg_viewport_2 = "<svg width=\"300\" height=\"200\" viewBox=\"100 0 1500 1000\""
                                 "preserveAspectRatio=\"none\">"
                                 "<rect x=\"0\" y=\"0\" width=\"1500\" height=\"1000\""
                                 "fill=\"yellow\" stroke=\"blue\" stroke-width=\"12\"/>"
                                 "<path fill=\"red\"  d=\"M 750,100 L 250,900 L 1250,900 z\"/>"
                                 "<text x=\"100\" y=\"600\" font-size=\"200\" font-family=\"Verdana\">"
                                 "Stretch to fit</text>"
                                 "</svg>";
    draw_svg(svg_viewport_2);
    EXPECT_SYS_SNAPSHOT_EQ(svg_viewport_2);
}

TEST_F(SVGRenderTest, ShapesTest)
{
    const char* svg_rect_transform = "<svg><rect fill=\"rgb(0, 0, 200)\" x=\"0\" y=\"0\" width=\"100\" height=\"100\""
                                     " transform=\"translate(50, 50) rotate(45) translate(-50, -50)\"/></svg>";
    draw_svg(svg_rect_transform);
    EXPECT_SNAPSHOT_EQ(svg_rect_transform);

    const char* svg_shapes = "<svg><rect fill=\"red\" x=\"0\" y=\"0\" width=\"100\" height=\"100\"/>"
                             "<circle fill=\"red\" cx=\"100\" cy=\"100\" r=\"50\"/>"
                             "<ellipse stroke=\"red\" fill=\"none\" cx=\"200\" cy=\"200\" rx=\"100\" ry=50/></svg>";
    draw_svg(svg_shapes);
    EXPECT_SNAPSHOT_EQ(svg_shapes);

    const char* svg_shapes_solid = "<svg width=\"480\" height=\"360\" viewBox=\"0 0 480 360\">"
                                   "<defs><solidColor xml:id=\"solidMaroon\" solid-color=\"maroon\" solid-opacity=\"0.7\"/>"
                                   "</defs><g><circle transform=\"translate(100, 150)\" fill=\"url(#solidMaroon)\" r=\"30\"/>"
                                   "<rect fill=\"url(#solidMaroon)\" transform=\"translate(190, 150)\" "
                                   "x=\"-30\" y=\"-30\" width=\"60\" height=\"60\"/>"
                                   "<path fill=\"url(#solidMaroon)\" transform=\"translate(270, 150)\"  d=\"M 0 -30 L 30 30 L -30 30 Z\" />"
                                   "<text fill=\"url(#solidMaroon)\" transform=\"translate(340, 150)\" "
                                   "y=\"21\" font-weight=\"bold\" font-size=\"60\">A</text>"
                                   "</g></svg>";
    draw_svg(svg_shapes_solid);
    EXPECT_SYS_SNAPSHOT_EQ(svg_shapes_solid);

    const char* svg_linear_gradient = "<svg width=\"4cm\" height=\"2cm\" viewBox=\"0 0 400 200\">"
                                      "<defs><linearGradient xml:id=\"MyGradient\">"
                                      "<stop offset=\"0.05\" stop-color=\"#F60\"/>"
                                      "<stop offset=\"0.95\" stop-color=\"#FF6\"/>"
                                      "</linearGradient></defs>"
                                      "<rect fill=\"url(#MyGradient)\" stroke=\"black\" stroke-width=\"5\""
                                      " x=\"100\" y=\"100\" width=\"600\" height=\"200\"/></svg>";
    draw_svg(svg_linear_gradient);
    EXPECT_SNAPSHOT_EQ(svg_linear_gradient);

    const char* svg_shapes_linear = "<svg width=\"8cm\" height=\"4cm\" viewBox=\"0 0 800 400\">"
                                    "<defs><radialGradient id=\"MyGradient\" gradientUnits=\"userSpaceOnUse\""
                                    "  cx=\"400\" cy=\"200\" r=\"300\">"
                                    "<stop offset=\"0.2\" stop-color=\"black\"/>"
                                    "<stop offset=\"0.75\" stop-color=\"white\"/>"
                                    "</radialGradient></defs>"
                                    "<rect fill=\"url(#MyGradient)\" stroke=\"black\" stroke-width=\"5\""
                                    " x=\"100\" y=\"100\" width=\"600\" height=\"200\"/></svg>";
    draw_svg(svg_shapes_linear);
    EXPECT_SNAPSHOT_EQ(svg_shapes_linear);

    const char* svg_shapes_radial = "<svg width=\"8cm\" height=\"4cm\" viewBox=\"0 0 800 400\">"
                                    "<defs><radialGradient id=\"MyGradient\">"
                                    "<stop offset=\"0.2\" stop-color=\"white\"/>"
                                    "<stop offset=\"0.75\" stop-color=\"black\"/>"
                                    "</radialGradient></defs>"
                                    "<rect fill=\"url(#MyGradient)\" stroke=\"black\" stroke-width=\"5\""
                                    "x=\"100\" y=\"100\" width=\"300\" height=\"300\"/></svg>";
    draw_svg(svg_shapes_radial);
    EXPECT_SNAPSHOT_EQ(svg_shapes_radial);

    const char* svg_shapes_linear2 = "<svg width=\"8cm\" height=\"4cm\" viewBox=\"0 0 800 400\">"
                                     "<defs><linearGradient xml:id=\"MyGradient\" x1=0 y1=0 x2=500 y2=350 gradientUnits=\"userSpaceOnUse\">"
                                     "<stop offset=\"0.05\" stop-color=\"#F60\"/>"
                                     "<stop offset=\"0.95\" stop-color=\"#FF6\"/>"
                                     "</linearGradient></defs>"
                                     "<rect fill=\"url(#MyGradient)\" stroke=\"black\" stroke-width=\"5\""
                                     " x=\"100\" y=\"100\" width=\"600\" height=\"200\"/></svg>";
    draw_svg(svg_shapes_linear2);
    EXPECT_SNAPSHOT_EQ(svg_shapes_linear2);

    const char* svg_shapes_linear3 = "<svg width=\"7cm\" height=\"2cm\" viewBox=\"0 0 700 200\">"
                                     "<g><defs><linearGradient id=\"MyGradient\" gradientUnits=\"objectBoundingBox\">"
                                     "<stop offset=\"0\" stop-color=\"#F60\"/>"
                                     "<stop offset=\"1\" stop-color=\"#FF6\"/>"
                                     "</linearGradient></defs>"
                                     "<rect x=\"1\" y=\"1\" width=\"698\" height=\"198\" fill=\"none\" stroke=\"blue\" stroke-width=\"2\"/>"
                                     "<g fill=\"url(#MyGradient)\">"
                                     "<rect x=\"100\" y=\"50\" width=\"200\" height=\"100\"/>"
                                     "<rect x=\"400\" y=\"50\" width=\"200\" height=\"100\"/>"
                                     "</g></g></svg>";
    draw_svg(svg_shapes_linear3);
    EXPECT_SNAPSHOT_EQ(svg_shapes_linear3);

    const char* svg_shapes_linear4 = "<svg width=\"8cm\" height=\"4cm\" viewBox=\"0 0 800 400\">"
                                     "<g><defs><linearGradient id=\"MyGradient\" gradientUnits=\"userSpaceOnUse\""
                                     " x1=0 y1=0 x2=350 y2=350>"
                                     "<stop offset=\"0\" stop-color=\"red\"/>"
                                     "<stop offset=\"0.5\" stop-color=\"blue\"/>"
                                     "</linearGradient></defs>"
                                     "<rect fill=\"url(#MyGradient)\" stroke=\"black\" stroke-width=\"5\""
                                     " x=\"0\" y=\"0\" width=\"200\" height=\"600\"/></g></svg>";
    draw_svg(svg_shapes_linear4);
    EXPECT_SNAPSHOT_EQ(svg_shapes_linear4);

    const char* svg_shapes_linear5 = \
                                     "<svg width='144' height='144' viewBox='0 0 144 144'><g>"
                                     "<rect x='4' y='4' width='136' height='136' fill='url(#paint2_linear_13691_50994)' fill-opacity='1.0'/>"
                                     "</g><defs><linearGradient id='paint2_linear_13691_50994' x1='4' y1='4' x2='280' y2='280'"
                                     " gradientUnits='userSpaceOnUse'>"
                                     "<stop stop-color='white'/><stop offset='1' stop-color='#000000'/></linearGradient>"
                                     "</defs></svg>";
    draw_svg(svg_shapes_linear5);
    EXPECT_SNAPSHOT_EQ(svg_shapes_linear5);

    const char* svg_shapes_linear6 = "<svg width=\'800\' height=\'800\' viewBox=\'0 0 800 800\' fill=\'none\' xmlns=\'http://www.w3.org/2000/svg\'>"
                                     "<rect x=\'10\' y=\'10\' width=\'200\' height=\'200\' fill=\'url(#paint0_linear_13691_50971)\'/>"
                                     "<rect x=\'212\' y=\'10\' width=\'200\' height=\'200\' fill=\'url(#paint0_linear_13691_50971)\'/>"
                                     "<rect x=\'10\' y=\'212\' width=\'200\' height=\'200\' fill=\'url(#paint0_linear_13691_50971)\'/>"
                                     "<rect x=\'212\' y=\'212\' width=\'200\' height=\'200\' fill=\'url(#paint0_linear_13691_50971)\'/>"
                                     "<defs><linearGradient id=\'paint0_linear_13691_50971\' x1=\'10\' y1=\'10\' "
                                     "x2=\'400\' y2=\'400\' gradientUnits=\'userSpaceOnUse\' >"
                                     "<stop offset=\'0\' stop-color=\'red\' stop-opacity=\'1\'/>"
                                     "<stop offset=\'1\' stop-color=\'green\' stop-opacity=\'1\'/>"
                                     "</linearGradient></defs></svg>";
    draw_svg(svg_shapes_linear6);
    EXPECT_SNAPSHOT_EQ(svg_shapes_linear6);

    const char* svg_shapes_linear7 = "<svg width=\'400\' height=\'400\' viewBox=\'0 0 400 400\' fill=\'none\' xmlns=\'http://www.w3.org/2000/svg\'>"
                                     "<rect x=\'10\' y=\'10\' width=\'100\' height=\'100\' fill=\'url(#gradient1)\'/>"
                                     "<rect x=\'120\' y=\'10\' width=\'100\' height=\'100\' fill=\'url(#gradient1)\'/>"
                                     "<rect x=\'230\' y=\'10\' width=\'100\' height=\'100\' fill=\'url(#gradient1)\'/>"
                                     "<defs><linearGradient id=\'gradient1\' x1=\'%0\' y1=\'0%\' "
                                     "x2=\'100%\' y2=\'0%\' gradientUnits=\'userSpaceOnUse\' >"
                                     "<stop offset=\'0\' stop-color=\'red\' stop-opacity=\'1\'/>"
                                     "<stop offset=\'1\' stop-color=\'green\' stop-opacity=\'1\'/>"
                                     "</linearGradient></defs></svg>";
    draw_svg(svg_shapes_linear7);
    EXPECT_SNAPSHOT_EQ(svg_shapes_linear7);

    const char* svg_shapes_svg2ext_arc1 = "<?xml version=\"1.0\" standalone=\"no\"?>"
                                          "<svg width=\"12cm\" height=\"5.25cm\" viewBox=\"0 0 1200 400\""
                                          "xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"
                                          "<rect x=\"1\" y=\"1\" width=\"1198\" height=\"398\""
                                          " fill=\"none\" stroke=\"blue\" stroke-width=\"1\" />"
                                          " <path d=\"M300,200 h-150 a150,150 0 1,0 150,-150 z\""
                                          " fill=\"red\" stroke=\"blue\" stroke-width=\"5\" />"
                                          " <path d=\"M275,175 v-150 a150,150 0 0,0 -150,150 z\""
                                          " fill=\"yellow\" stroke=\"blue\" stroke-width=\"5\" />"
                                          "<path d=\"M600,350 l 50,-25"
                                          " a25,25 -30 0,1 50,-25 l 50,-25"
                                          " a25,50 -30 0,1 50,-25 l 50,-25"
                                          " a25,75 -30 0,1 50,-25 l 50,-25"
                                          " a25,100 -30 0,1 50,-25 l 50,-25\""
                                          " fill=\"none\" stroke=\"red\" stroke-width=\"5\"/></svg>";
    draw_svg(svg_shapes_svg2ext_arc1);
    EXPECT_SNAPSHOT_EQ(svg_shapes_svg2ext_arc1);

    const char* svg_shapes_use_transform = "<?xml version='1.0'?>"
                                           "<svg><rect id='r1' x='50' y='50' width='100' height='100' fill='green'/>"
                                           "<g transform='rotate(45 220,250)'>"
                                           "<use x='80' y='80' xlink:href='#r1' transform='scale(0.5)'/>"
                                           "</g></svg>";
    draw_svg(svg_shapes_use_transform);
    EXPECT_SNAPSHOT_EQ(svg_shapes_use_transform);
}

TEST_F(SVGRenderTest, ComplexPath)
{
    const char* svg_complex_1 = "<svg width=5cm height=4cm viewBox=\"0 0 500 400\">"
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
    EXPECT_SYS_SNAPSHOT_EQ(svg_complex_1);

    const char* svg_complex_2 = "<?xml version=\"1.0\"?><svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\""
                                "version=\"1.2\" baseProfile=\"tiny\" width=\"10cm\" height=\"3cm\" viewBox=\"0 0 100 30\">"
                                "<desc>'use' with a 'transform' attribute</desc>"
                                "<defs><rect xml:id=\"MyRect\" x=\"0\" y=\"0\" width=\"60\" height=\"10\"/></defs>"
                                "<rect x=\".1\" y=\".1\" width=\"99.8\" height=\"29.8\" fill=\"none\" stroke=\"blue\" stroke-width=\".2\"/>"
                                "<use xlink:href=\"#MyRect\" fill=\"red\" transform=\"translate(20,2.5) rotate(10)\"/>"
                                "<use xlink:href=\"#MyRect\" fill=\"green\" transform=\"translate(20,2.5) rotate(-10)\"/>"
                                "</svg>";
    draw_svg(svg_complex_2);
    EXPECT_SNAPSHOT_EQ(svg_complex_2);

    const char* svg_complex_3 = "<svg width=\"10cm\" height=\"3cm\" viewBox=\"0 0 100 30\">"
                                "<g id=g1><rect id=\"MyRect1\" x=\"0\" y=\"0\" width=\"60\" height=\"10\"/>"
                                "<rect id=\"MyRect2\" x=\"0\" y=\"12\" width=\"60\" height=\"10\"/>"
                                "</g><rect x=\".1\" y=\".1\" width=\"99.8\" height=\"29.8\""
                                " fill=\"none\" stroke=\"blue\" stroke-width=\".2\"/>"
                                "<use xlink:href=\"#g1\" fill=\"green\" transform=\"translate(20,2.5) rotate(-10)\"/>"
                                "<use xlink:href=\"#MyRect1\" fill=\"red\" transform=\"translate(20,2.5) rotate(10)\"/>"
                                "</svg>";
    draw_svg(svg_complex_3);
    EXPECT_SNAPSHOT_EQ(svg_complex_3);

    const char* svg_complex_4 = "<?xml version='1.0'?>"
                                "<svg width='12cm' height='4cm' viewBox='0 0 1200 400' "
                                "xmlns='http://www.w3.org/2000/svg' version='1.2' baseProfile='tiny'>"
                                "<desc>Example rect02 - rounded rectangles</desc>"
                                "<!-- Show outline of canvas using 'rect' element -->"
                                "<rect x='1' y='1' width='1198' height='398'"
                                " fill='none' stroke='blue' stroke-width='2'/>"
                                "<rect x='100' y='100' width='400' height='200' rx='50'"
                                " fill='green' />"
                                "<g transform='translate(700 210) rotate(-30)'>"
                                "<rect x='0' y='0' width='400' height='200' rx='50'"
                                " fill='none' stroke='purple' stroke-width='30' />"
                                "</g></svg>";
    draw_svg(svg_complex_4);
    EXPECT_SNAPSHOT_EQ(svg_complex_4);

    const char* svg_complex_5 = "<svg width='12cm' height='4cm' viewBox='0 0 1200 400'>"
                                "<rect x='1' y='1' width='1198' height='398' "
                                " fill='none' stroke='blue' stroke-width='2' />"
                                "<g transform='translate(300 200)'>"
                                "<ellipse rx='250' ry='100' "
                                "fill='red'  /></g>"
                                "<ellipse transform='translate(900 200) rotate(-30)' "
                                "rx='250' ry='100' fill='none' stroke='blue' stroke-width='20'  />"
                                "</svg>";
    draw_svg(svg_complex_5);
    EXPECT_SNAPSHOT_EQ(svg_complex_5);

    const char* svg_complex_6 = "<svg width='12cm' height='4cm' viewBox='0 0 1200 400'>"
                                "<rect x='1' y='1' width='1198' height='398'"
                                " fill='none' stroke='blue' stroke-width='2' />"
                                "<g stroke='green' >"
                                "<line x1='100' y1='300' x2='300' y2='100'"
                                " stroke-width='5'  />"
                                "<line x1='300' y1='300' x2='500' y2='100'"
                                "     stroke-width='10'  />"
                                "<line x1='500' y1='300' x2='700' y2='100'"
                                "    stroke-width='15'  />"
                                "<line x1='700' y1='300' x2='900' y2='100'"
                                "      stroke-width='20'  />"
                                "<line x1='900' y1='300' x2='1100' y2='100'"
                                "     stroke-width='25'  />"
                                "</g></svg>";
    draw_svg(svg_complex_6);
    EXPECT_SNAPSHOT_EQ(svg_complex_6);

    const char* svg_complex_7 = "<svg width='12cm' height='4cm' viewBox='0 0 1200 400'>"
                                "<rect x='1' y='1' width='1198' height='398'"
                                " fill='none' stroke='blue' stroke-width='2' />"
                                "<polyline fill='none' stroke='blue' stroke-width='10' "
                                " points='50,375"
                                "    150,375 150,325 250,325 250,375"
                                "    350,375 350,250 450,250 450,375"
                                "    550,375 550,175 650,175 650,375"
                                "    750,375 750,100 850,100 850,375"
                                "    950,375 950,25 1050,25 1050,375"
                                "    1150,375' />"
                                "</svg>";
    draw_svg(svg_complex_7);
    EXPECT_SNAPSHOT_EQ(svg_complex_7);

    const char* svg_complex_8 = "<svg xmlns='http://www.w3.org/2000/svg' "
                                "width='100%' height='100%' viewBox='0 0 400 400' "
                                "direction='rtl' xml:lang='fa'>"
                                "<text x='200' y='200' font-size='20'>داستان SVG Tiny 1.2 طولا ني است.</text>"
                                "</svg>";
    draw_svg(svg_complex_8);
    EXPECT_SYS_SNAPSHOT_EQ(svg_complex_8);

    const char* svg_complex_9 = "<svg width=\"400\" height=\"400\" viewBox=\"0 0 800 800\">"
                                "<defs><g id=\"g1\" fill=\"red\">"
                                "<circle cx=\"100\" cy=\"100\" r=\"40\"/>"
                                "<rect x=\"20\" y=\"160\" width=\"100\" height=\"100\"/>"
                                "</g>"
                                "<linearGradient id=\"gad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\">"
                                "<stop offset=\"0\" stop-color=\"red\" />"
                                "<stop offset=\"0.5\" stop-color=\"green\" />"
                                "<stop offset=\"1\" stop-color=\"blue\" />"
                                "</linearGradient></defs>"
                                "<use xlink:href=\"#g1\" x=\"50\" y=\"0\" />"
                                "<use xlink:href=\"#g1\" x=\"250\" y=\"0\" fill=\"url(#gad)\" />"
                                "</svg>";
    draw_svg(svg_complex_9);
    EXPECT_SYS_SNAPSHOT_EQ(svg_complex_9);


    const char* svg_complex_10 = "<svg width=\"400\" height=\"400\" viewBox=\"0 0 800 800\">"
                                "<defs>"
                                "<circle id=\"g1\" cx=\"100\" cy=\"100\" r=\"40\"/>"
                                "<linearGradient id=\"gad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\">"
                                "<stop offset=\"0\" stop-color=\"red\" />"
                                "<stop offset=\"0.5\" stop-color=\"green\" />"
                                "<stop offset=\"1\" stop-color=\"blue\" />"
                                "</linearGradient></defs>"
                                "<use xlink:href=\"#g1\" x=\"50\" y=\"0\" />"
                                "<use xlink:href=\"#g1\" x=\"250\" y=\"0\" fill=\"url(#gad)\" />"
                                "</svg>";
    draw_svg(svg_complex_10);
    EXPECT_SYS_SNAPSHOT_EQ(svg_complex_10);
}

TEST_F(SVGRenderTest, GroupTest)
{
    const char* svg_use_simple = "<svg><use xlink:href=\"#r1\" x=100 y=100/>"
                                 "<rect xml:id=\"r1\" x=20 width=50 height=50 fill=green/>"
                                 "</svg>";

    draw_svg(svg_use_simple);
    EXPECT_SNAPSHOT_EQ(svg_use_simple);

    const char* svg_use_group = "<svg><use xlink:href=\"#g1\" x=50 y=50/>"
                                "<g fill=\"blue\" xml:id=\"g1\">"
                                "<rect width=20 height=20/>"
                                "<rect x=30 width=20 height=20 fill=green/>"
                                "</g></svg>";
    draw_svg(svg_use_group);
    EXPECT_SNAPSHOT_EQ(svg_use_group);

    const char* svg_group_circle = "<svg><g fill=\"#FF0000\">"
                                   "<rect x=\"0\" y=\"0\" width=\"100\" height=\"100\"/>"
                                   "<circle cx=\"100\" cy=\"100\" r=\"50\"/>"
                                   "<ellipse fill=\"#00F\" cx=\"200\" cy=\"200\" rx=\"100\" ry=50/>"
                                   "</g></svg>";
    draw_svg(svg_group_circle);
    EXPECT_SNAPSHOT_EQ(svg_group_circle);

    const char* svg_group_inherit = "<svg><g fill=\"blue\"><g>"
                                    "<rect width=\"100\" height=\"100\" fill=\"inherit\"/>"
                                    "<circle cx=\"200\" cy=\"200\" r=\"50\" fill=\"green\"/>"
                                    "</g></g></svg>";
    draw_svg(svg_group_inherit);
    EXPECT_SNAPSHOT_EQ(svg_group_inherit);

    const char* svg_group_gradient = "<svg><defs><linearGradient id=\"s1\">"
                                     "<stop offset=\"0.1\" stop-color=\"red\"/>"
                                     "<stop offset=\"0.8\" stop-color=\"green\"/>"
                                     "</linearGradient></defs>"
                                     "<g fill=\"url(#s1)\" id=\"g1\">"
                                     "<rect width=20 height=20/><g>"
                                     "<rect x=30 width=20 height=20 fill=blue/>"
                                     "<rect x=30 x=80 y= 30 width=20 height =20 fill=red/></g></g>"
                                     "<use xlink:href=\"#g1\" x=150 y=100/>"
                                     "<use xlink:href=\"#g1\" x=250 y=100/></svg>";
    draw_svg(svg_group_gradient);
    EXPECT_SNAPSHOT_EQ(svg_group_gradient);
}

TEST_F(SVGRenderTest, TextTest)
{
    const char* svg_text_tspan = "<svg><text x=20 y=60 font-family=\"sans-serif\" font-size=\"24\">"
                                 "hello <tspan fill=\"red\" font-size=\"36\">all</tspan> world"
                                 "</text></svg>";
    draw_svg(svg_text_tspan);
    EXPECT_SYS_SNAPSHOT_EQ(svg_text_tspan);

    const char* svg_text_gradient = "<svg><defs><linearGradient id=\"g1\">"
                                    "<stop offset=\"0.1\" stop-color=\"blue\"/>"
                                    "<stop offset =\"0.8\" stop-color=\"red\"/>"
                                    "</linearGradient></defs>"
                                    "<text fill=\"url(#g1)\" x=20 y=60 font-family=\"sans-serif\" font-size=\"48px\" font-weight=\"bold\">"
                                    "hello <tspan fill=\"green\" font-size=\"24px\">all</tspan> world"
                                    "</text></svg>";
    draw_svg(svg_text_gradient);
    EXPECT_SYS_SNAPSHOT_EQ(svg_text_gradient);

    const char* svg_text_use_gradient = "<svg><defs><linearGradient id=\"g1\">"
                                        "<stop offset=\"0.1\" stop-color=\"blue\"/>"
                                        "<stop offset =\"0.8\" stop-color=\"red\"/>"
                                        "</linearGradient></defs>"
                                        "<text id=\"text1\" fill=\"url(#g1)\" x=20 y=60"
                                        " font-family=\"sans-serif\" font-size=\"48px\" font-weight=\"bold\">"
                                        "hello <tspan fill=\"green\" font-size=\"24px\">all</tspan> world"
                                        "</text><use x=\"20\" y=\"100\" xlink:href=\"#text1\"/></svg>";
    draw_svg(svg_text_use_gradient);
    EXPECT_SYS_SNAPSHOT_EQ(svg_text_use_gradient);

}

TEST_F(SVGRenderTest, ImageTest)
{

    const char* svg_image_1 = "<svg width=\"400\" height=\"400\">"
                              "<image x=\"0\" y=\"0\" width=\"360\" height=\"240\" preserveAspectRatio=\"none\""
                              "xlink:href=\"test.png\" />"
                              "<rect x=\"0\" y=\"0\" width=\"360\" height=\"240\" fill=\"none\" stroke=\"blue\"/></svg>";
    draw_svg(svg_image_1);
    EXPECT_SNAPSHOT_EQ(svg_image_1);

    const char* svg_image_2 = "<svg width=\"400\" height=\"400\">"
                              "<image x=\"0\" y=\"0\" width=\"360\" height=\"240\""
                              "xlink:href=\"test.png\" />"
                              "<rect x=\"0\" y=\"0\" width=\"360\" height=\"240\" fill=\"none\" stroke=\"blue\"/></svg>";
    draw_svg(svg_image_2);
    EXPECT_SNAPSHOT_EQ(svg_image_2);

    const char* svg_image_3 = "<svg width=\"400\" height=\"400\">"
                              "<image x=\"0\" y=\"0\" width=\"360\" height=\"540\" preserveAspectRatio=\"xMinYMin\" "
                              "xlink:href=\"test.png\" />"
                              "<rect x=\"0\" y=\"0\" width=\"360\" height=\"540\" fill=\"none\" stroke=\"blue\"/></svg>";
    draw_svg(svg_image_3);
    EXPECT_SNAPSHOT_EQ(svg_image_3);

    const char* svg_image_4 = "<svg width=\"400\" height=\"400\">"
                              "<image x=\"0\" y=\"0\" width=\"360\" height=\"540\" preserveAspectRatio=\" xMinYMid\" "
                              "xlink:href=\"test.png\" />"
                              "<rect x=\"0\" y=\"0\" width=\"360\" height=\"540\" fill=\"none\" stroke=\"blue\"/></svg>";
    draw_svg(svg_image_4);
    EXPECT_SNAPSHOT_EQ(svg_image_4);

    const char* svg_image_5 = "<svg width=\"400\" height=\"400\">"
                              "<image x=\"0\" y=\"0\" width=\"360\" height=\"540\" opacity=\"0.5\" preserveAspectRatio=\" xMinYMax\" "
                              "xlink:href=\"test.png\" />"
                              "<rect x=\"0\" y=\"0\" width=\"360\" height=\"540\" fill=\"none\" stroke=\"blue\"/></svg>";
    draw_svg(svg_image_5);
    EXPECT_SNAPSHOT_EQ(svg_image_5);

    const char* svg_image_6 = "<svg width=\"400\" height=\"400\">"
                              "<image x=\"0\" y=\"0\" width=\"360\" height=\"240\" preserveAspectRatio=\"xMidYMin\" "
                              "xlink:href=\"test.png\" />"
                              "<rect x=\"0\" y=\"0\" width=\"360\" height=\"240\" fill=\"none\" stroke=\"blue\"/></svg>";
    draw_svg(svg_image_6);
    EXPECT_SNAPSHOT_EQ(svg_image_6);

    const char* svg_image_7 = "<svg width=\"400\" height=\"400\">"
                              "<image x=\"0\" y=\"0\" width=\"360\" height=\"240\" preserveAspectRatio=\"xMidYMax\" "
                              "xlink:href=\"test.png\" />"
                              "<rect x=\"0\" y=\"0\" width=\"360\" height=\"240\" fill=\"none\" stroke=\"blue\"/></svg>";
    draw_svg(svg_image_7);
    EXPECT_SNAPSHOT_EQ(svg_image_7);

    const char* svg_image_8 = "<svg width=\"400\" height=\"400\">"
                              "<image x=\"0\" y=\"0\" width=\"360\" height=\"240\" preserveAspectRatio=\"xMaxYMin\" "
                              "xlink:href=\"test.png\" />"
                              "<rect x=\"0\" y=\"0\" width=\"360\" height=\"240\" fill=\"none\" stroke=\"blue\"/></svg>";
    draw_svg(svg_image_8);
    EXPECT_SNAPSHOT_EQ(svg_image_8);

    const char* svg_image_9 = "<svg width=\"400\" height=\"400\">"
                              "<image x=\"0\" y=\"0\" width=\"360\" height=\"240\" preserveAspectRatio=\"xMaxYMid\" "
                              "xlink:href=\"test.png\" />"
                              "<rect x=\"0\" y=\"0\" width=\"360\" height=\"240\" fill=\"none\" stroke=\"blue\"/></svg>";
    draw_svg(svg_image_9);
    EXPECT_SNAPSHOT_EQ(svg_image_9);

    const char* svg_image_10 = "<svg width=\"400\" height=\"400\">"
                               "<image x=\"0\" y=\"0\" width=\"360\" height=\"240\" preserveAspectRatio=\"xMaxYMax\" "
                               "xlink:href=\"test.png\" />"
                               "<rect x=\"0\" y=\"0\" width=\"360\" height=\"240\" fill=\"none\" stroke=\"blue\"/></svg>";
    draw_svg(svg_image_10);
    EXPECT_SNAPSHOT_EQ(svg_image_10);

    const char* svg_image_11 = "<svg width=\"400\" height=\"400\">"
                               "<image x=\"0\" y=\"0\" width=\"50\" height=\"30\" preserveAspectRatio=\"meet\""
                               "xlink:href=\"test.png\" />"
                               "<rect x=\"0\" y=\"0\" width=\"50\" height=\"30\" fill=\"none\" stroke=\"blue\"/></svg>";
    draw_svg(svg_image_11);
    EXPECT_SNAPSHOT_EQ(svg_image_11);

    const char* svg_image_12 = "<svg width=\"400\" height=\"400\">"
                               "<image x=\"0\" y=\"0\" width=\"50\" height=\"30\" preserveAspectRatio=\"xMidYMid slice\""
                               "xlink:href=\"test.png\" />"
                               "<rect x=\"0\" y=\"0\" width=\"50\" height=\"30\" fill=\"none\" stroke=\"blue\"/></svg>";
    draw_svg(svg_image_12);
    EXPECT_SNAPSHOT_EQ(svg_image_12);

    const char* svg_image_13 = "<svg width=\"200\" height=\"200\">"
                               "<image x=\"90\" y=\"-65\" width=\"80\" height=\"90\" transform=\"rotate(45)\""
                               " xlink:href=\"test.png\"/>"
                               "</svg>";
    draw_svg(svg_image_13);
    EXPECT_SNAPSHOT_EQ(svg_image_13);
}

TEST_F(SVGRenderTest, BadCaseTest)
{
    // invalid <use> xlink:href attr
    const char* svg_badcase_1 = "<svg width=\"200\" height=\"200\">"
                                "<rect id=\"rect1\" x=\"10\" y=\"10\" width=\"100\" height=\"100\" />"
                                "<use xlink=\"#rect1\"></svg>";
    draw_svg(svg_badcase_1);
}
