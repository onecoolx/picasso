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

#include "test.h"
#include "timeuse.h"

#include "psx_svg.h"

class SVGParserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        root = NULL;
    }

    void TearDown() override
    {
        release();
    }

    void load(const char* data)
    {
        root = psx_svg_load_data(data, (uint32_t)strlen(data));
    }

    void release(void)
    {
        if (root) {
            psx_svg_node_destroy(root);
            root = NULL;
        }
    }

    psx_svg_node* root;
};

TEST_F(SVGParserTest, SimpleSVGTest)
{
    const char* svg_data = "<svg></svg>";
    load(svg_data);
    EXPECT_NE(root, nullptr);
    release();
}

TEST_F(SVGParserTest, SVGElementTest)
{
    const char* svg_data1 = "<svg version=\"1.2\" baseProfile=\"tiny\"></svg>";
    load(svg_data1);
    EXPECT_EQ(root->child_count(), 0);
    EXPECT_EQ(root->attr_count(), 2);
    EXPECT_STREQ(root->attr_at(0)->value.sval, "1.2");
    EXPECT_STREQ(root->attr_at(1)->value.sval, "tiny");
    release();

    const char* svg_viewbox0 = "<svg viewBox=\"none\"></svg>";
    load(svg_viewbox0);
    EXPECT_EQ(root->attr_at(0)->class_type, SVG_ATTR_VALUE_NONE);
    release();

    const char* svg_viewbox1 = "<svg viewBox=\"0 0 10 10\"></svg>";
    load(svg_viewbox1);
    float ret1[4] = {0.0f, 0.0f, 10.0f, 10.0f};
    EXPECT_ARRAY_EQ(ret1, (float*)(root->attr_at(0)->value.val), 4);
    release();

    const char* svg_viewbox2 = "<svg viewBox=\"-5,10 +10,-10\"></svg>";
    load(svg_viewbox2);
    float ret2[4] = {-5.0f, 10.0f, 10.0f, -10.0f};
    EXPECT_ARRAY_EQ(ret2, (float*)(root->attr_at(0)->value.val), 4);
    release();

    const char* svg_viewbox3 = "<svg viewBox=\"-5,-5\"></svg>";
    load(svg_viewbox3);
    EXPECT_EQ(root->attr_at(0)->class_type, SVG_ATTR_VALUE_NONE);
    release();

    const char* svg_viewbox4 = "<svg viewBox=\"-5,-5 .2 1.5E+1\"></svg>";
    load(svg_viewbox4);
    float ret4[4] = {-5.0f, -5.0f, 0.2f, 15.0f};
    EXPECT_ARRAY_EQ(ret4, (float*)(root->attr_at(0)->value.val), 4);
    release();

    /* width and height */
    const char* svg_wh = "<svg width=\"100\" height=\"100px\"></svg>";
    load(svg_wh);
    EXPECT_FLOAT_EQ(root->attr_at(0)->value.fval, 100.0f);
    EXPECT_FLOAT_EQ(root->attr_at(1)->value.fval, 100.0f);
    release();

    const char* svg_wh2 = "<svg width=\"10cm\" height=\"100mm\"></svg>";
    load(svg_wh2);
    EXPECT_FLOAT_EQ(root->attr_at(0)->value.fval, 377.9528f);
    EXPECT_FLOAT_EQ(root->attr_at(1)->value.fval, 377.9528f);
    release();

    const char* svg_wh3 = "<svg width=\"10in\" height=\"10pc\"></svg>";
    load(svg_wh3);
    EXPECT_FLOAT_EQ(root->attr_at(0)->value.fval, 960.0f);
    EXPECT_FLOAT_EQ(root->attr_at(1)->value.fval, 160.0f);
    release();

    const char* svg_wh4 = "<svg width=\"10em\" height=\"10ex\"></svg>";
    load(svg_wh4);
    EXPECT_FLOAT_EQ(root->attr_at(0)->value.fval, 160.0f);
    EXPECT_FLOAT_EQ(root->attr_at(1)->value.fval, 83.2f);
    release();

    const char* svg_wh5 = "<svg width=\"10pt\" height=\"100%\"></svg>";
    load(svg_wh5);
    EXPECT_FLOAT_EQ(root->attr_at(0)->value.fval, 13.333334f);
    EXPECT_FLOAT_EQ(root->attr_at(1)->value.fval, 1.0f);
    release();

    /* preserveAspectRatio */
    const char* svg_ar0 = "<svg preserveAspectRatio=\"none meet\"></svg>";
    load(svg_ar0);
    EXPECT_EQ(root->attr_count(), 1);
    release();

    const char* svg_ar1 = "<svg preserveAspectRatio=\"xMinYMin meet\"></svg>";
    load(svg_ar1);
    EXPECT_EQ(root->attr_at(0)->value.uval, 2);
    release();

    const char* svg_ar2 = "<svg preserveAspectRatio=\"  xMidYMin slice\"></svg>";
    load(svg_ar2);
    EXPECT_EQ(root->attr_at(0)->value.uval, 5);
    release();

    const char* svg_ar3 = "<svg preserveAspectRatio=\"xMaxYMin unknow\"></svg>";
    load(svg_ar3);
    EXPECT_EQ(root->attr_at(0)->value.uval, 6);
    release();

    const char* svg_ar4 = "<svg preserveAspectRatio=\"xMinYMid meet\"></svg>";
    load(svg_ar4);
    EXPECT_EQ(root->attr_at(0)->value.uval, 8);
    release();

    const char* svg_ar5 = "<svg preserveAspectRatio=\"xMidYMid\"></svg>";
    load(svg_ar5);
    EXPECT_EQ(root->attr_at(0)->value.uval, 10);
    release();

    const char* svg_ar6 = "<svg preserveAspectRatio=\"xMaxYMid slice\"></svg>";
    load(svg_ar6);
    EXPECT_EQ(root->attr_at(0)->value.uval, 13);
    release();

    const char* svg_ar7 = "<svg preserveAspectRatio=\"xMinYMax slice\"></svg>";
    load(svg_ar7);
    EXPECT_EQ(root->attr_at(0)->value.uval, 15);
    release();

    const char* svg_ar8 = "<svg preserveAspectRatio=\"xMidYMax meet\"></svg>";
    load(svg_ar8);
    EXPECT_EQ(root->attr_at(0)->value.uval, 16);
    release();

    const char* svg_ar9 = "<svg preserveAspectRatio=\"xMaxYMax meet\"></svg>";
    load(svg_ar9);
    EXPECT_EQ(root->attr_at(0)->value.uval, 18);
    release();

    const char* svg_ar10 = "<svg preserveAspectRatio=\"unknown\"></svg>";
    load(svg_ar10);
    EXPECT_EQ(root->attr_count(), 1);
    release();
}

TEST_F(SVGParserTest, PolylineElementTest)
{
    const char* svg_poly1 = "<svg><polyline points=\"100.0,50 200,150.0 180,110 200,200 210,340\"/></svg>";
    load(svg_poly1);
    psx_svg_node* svg_node = root->get_child(0);
    EXPECT_EQ(((psx_svg_attr_values_list*)(svg_node->attr_at(0)->value.val))->length, 5);
    release();

    const char* svg_poly2 = "<svg><polyline points=\"\n100.0,50\t200,150.0\n180,110\r \"/></svg>";
    load(svg_poly2);
    svg_node = root->get_child(0);
    EXPECT_EQ(((psx_svg_attr_values_list*)(svg_node->attr_at(0)->value.val))->length, 3);
    release();

    const char* svg_poly3 = "<svg><polyline points=\"100.0 200,150.0 edf,err\"/></svg>";
    load(svg_poly3);
    svg_node = root->get_child(0);
    EXPECT_EQ(((psx_svg_attr_values_list*)(svg_node->attr_at(0)->value.val))->length, 1);
    release();

    const char* svg_poly4 = "<svg><polyline points=\"100.0\"/></svg>";
    load(svg_poly4);
    svg_node = root->get_child(0);
    EXPECT_EQ(((psx_svg_attr_values_list*)(svg_node->attr_at(0)->value.val))->length, 0);
    release();

    const char* svg_poly5 = "<svg><polyline points=\" \"/></svg>";
    load(svg_poly5);
    svg_node = root->get_child(0);
    EXPECT_EQ(svg_node->attr_count(), 0);
    release();
}

TEST_F(SVGParserTest, PathElementTest)
{
    const char* svg_path1 = "<svg><path d=\" \"/></svg>";
    load(svg_path1);
    psx_svg_node* svg_node = root->get_child(0);
    EXPECT_EQ(svg_node->attr_count(), 0);
    release();

    const char* svg_path2 = "<svg><path d=\"REt321\"/></svg>";
    load(svg_path2);
    svg_node = root->get_child(0);
    EXPECT_FLOAT_EQ(ps_path_get_length((ps_path*)(svg_node->attr_at(0)->value.val)), 0);
    release();

    const char* svg_path3 = "<svg><path d=\"1223\"/></svg>";
    load(svg_path3);
    svg_node = root->get_child(0);
    EXPECT_FLOAT_EQ(ps_path_get_length((ps_path*)(svg_node->attr_at(0)->value.val)), 0);
    release();

    const char* svg_path4 = "<svg><path d=\"m 100 200 150 180 L300 500 C 400 450 320.0 280.5 400 400 Z\"/></svg>";
    load(svg_path4);
    svg_node = root->get_child(0);
    EXPECT_FLOAT_EQ(ps_path_get_length((ps_path*)(svg_node->attr_at(0)->value.val)), 945.00311f);
    release();

    const char* svg_path5 = "<svg><path d=\"M 100 200 300 500 L400 450 l 150 100 H 500 h 100 H 600 v 100 s 100 200 300 400\"/></svg>";
    load(svg_path5);
    svg_node = root->get_child(0);
    EXPECT_FLOAT_EQ(ps_path_get_length((ps_path*)(svg_node->attr_at(0)->value.val)), 1404.3647f);

    ps_path* path = (ps_path*)(svg_node->attr_at(0)->value.val);

    ps_point p;
    ps_path_cmd cmd = ps_path_get_vertex(path, 0, &p);
    EXPECT_EQ(cmd, PATH_CMD_MOVE_TO);

    cmd = ps_path_get_vertex(path, 1, &p);
    EXPECT_EQ(cmd, PATH_CMD_LINE_TO);

    cmd = ps_path_get_vertex(path, 2, &p);
    EXPECT_EQ(cmd, PATH_CMD_LINE_TO);

    EXPECT_FLOAT_EQ(p.x, 400.0f);
    EXPECT_FLOAT_EQ(p.y, 450.0f);

    cmd = ps_path_get_vertex(path, 7, &p);
    EXPECT_EQ(cmd, PATH_CMD_LINE_TO);

    EXPECT_FLOAT_EQ(p.x, 600.0f);
    EXPECT_FLOAT_EQ(p.y, 650.0f);
    release();
}

TEST_F(SVGParserTest, TransformTest)
{
    const char* svg_tr1 = "<svg><g transform=\" \"/></svg>";
    load(svg_tr1);
    psx_svg_node* svg_node = root->get_child(0);
    EXPECT_EQ(svg_node->attr_count(), 0);
    release();

    const char* svg_tr2 = "<svg><g transform=\"none\"/></svg>";
    load(svg_tr2);
    svg_node = root->get_child(0);
    EXPECT_EQ(svg_node->attr_count(), 1);
    release();

    const char* svg_tr3 = "<svg><g transform=\"matrix()\"/></svg>";
    load(svg_tr3);
    svg_node = root->get_child(0);
    ps_matrix* matrix = (ps_matrix*)(svg_node->attr_at(0)->value.val);
    EXPECT_EQ(ps_matrix_is_identity(matrix), True);
    release();

    const char* svg_tr4 = "<svg><g transform=\"matrix(1.5, 0, 2.0, 2, 10, 20)\"/></svg>";
    load(svg_tr4);
    svg_node = root->get_child(0);
    matrix = (ps_matrix*)(svg_node->attr_at(0)->value.val);
    float shx, shy;
    ps_matrix_get_shear_factor(matrix, &shx, &shy);
    EXPECT_FLOAT_EQ(shy, 0.0f);
    EXPECT_FLOAT_EQ(shx, 2.0f);
    float sx, sy;
    ps_matrix_get_scale_factor(matrix, &sx, &sy);
    EXPECT_FLOAT_EQ(sx, 1.5f);
    EXPECT_FLOAT_EQ(sy, 2.0f);
    float tx, ty;
    ps_matrix_get_translate_factor(matrix, &tx, &ty);
    EXPECT_FLOAT_EQ(tx, 10.0f);
    EXPECT_FLOAT_EQ(ty, 20.0f);
    release();

    const char* svg_tr5 = "<svg><g transform=\"translate(1, 2) translate(2.0, 2)\"/></svg>";
    load(svg_tr5);
    svg_node = root->get_child(0);
    matrix = (ps_matrix*)(svg_node->attr_at(0)->value.val);
    ps_matrix_get_translate_factor(matrix, &tx, &ty);
    EXPECT_FLOAT_EQ(tx, 3.0f);
    EXPECT_FLOAT_EQ(ty, 4.0f);
    release();

    const char* svg_tr6 = "<svg><g transform=\" scale(0.5) scale(0.5 0.5)\"/></svg>";
    load(svg_tr6);
    svg_node = root->get_child(0);
    matrix = (ps_matrix*)(svg_node->attr_at(0)->value.val);
    ps_matrix_get_scale_factor(matrix, &sx, &sy);
    EXPECT_FLOAT_EQ(sx, 0.25f);
    EXPECT_FLOAT_EQ(sy, 0.25f);
    release();

    const char* svg_tr7 = "<svg><g transform=\" translate(10 ) scale( 0.5  0.5) scale(0.5 ) \"/></svg>";
    load(svg_tr7);
    svg_node = root->get_child(0);
    matrix = (ps_matrix*)(svg_node->attr_at(0)->value.val);
    ps_matrix_get_translate_factor(matrix, &tx, &ty);
    EXPECT_FLOAT_EQ(tx, 2.5f);
    EXPECT_FLOAT_EQ(ty, 0.0f);
    ps_matrix_get_scale_factor(matrix, &sx, &sy);
    EXPECT_FLOAT_EQ(sx, 0.25f);
    EXPECT_FLOAT_EQ(sy, 0.25f);
    release();

    const char* svg_tr8 = "<svg><g transform=\" rotate(90 10 10) \"/></svg>";
    load(svg_tr8);
    svg_node = root->get_child(0);
    matrix = (ps_matrix*)(svg_node->attr_at(0)->value.val);
    ps_matrix_get_translate_factor(matrix, &tx, &ty);
    EXPECT_FLOAT_EQ(tx, 20.0f);
    EXPECT_FLOAT_EQ(ty, 0.0f);
    release();

    const char* svg_tr9 = "<svg><g transform=\" rotate(90 ) \"/></svg>";
    load(svg_tr9);
    svg_node = root->get_child(0);
    matrix = (ps_matrix*)(svg_node->attr_at(0)->value.val);
    ps_matrix_get_translate_factor(matrix, &tx, &ty);
    EXPECT_FLOAT_EQ(tx, 0.0f);
    EXPECT_FLOAT_EQ(ty, 0.0f);
    release();

    const char* svg_tr10 = "<svg><g transform=\" skewX(10) skewY(10) \"/></svg>";
    load(svg_tr10);
    svg_node = root->get_child(0);
    matrix = (ps_matrix*)(svg_node->attr_at(0)->value.val);
    ps_matrix_get_shear_factor(matrix, &shx, &shy);
    EXPECT_FLOAT_EQ(shy, 0.17453294f);
    EXPECT_FLOAT_EQ(shx, 0.17453294f);
    release();
}

TEST_F(SVGParserTest, FillStrokeTest)
{
    const char* svg_sf1 = "<svg><g fill=\" \"/></svg>";
    load(svg_sf1);
    psx_svg_node* svg_node = root->get_child(0);
    EXPECT_EQ(svg_node->attr_count(), 0);
    release();

    const char* svg_sf2 = "<svg><g fill=\"none\" stroke=\"inherit\"/></svg>";
    load(svg_sf2);
    svg_node = root->get_child(0);
    EXPECT_EQ(svg_node->attr_count(), 2);
    release();

    const char* svg_sf3 = "<svg><g fill=\" url(#grad1) \"/></svg>";
    load(svg_sf3);
    svg_node = root->get_child(0);
    const char* str = svg_node->attr_at(0)->value.sval;
    EXPECT_STREQ(str, "grad1");
    release();

    const char* svg_sf4 = "<svg><g fill=\" url( # grad2 ) \"/></svg>";
    load(svg_sf4);
    svg_node = root->get_child(0);
    str = svg_node->attr_at(0)->value.sval;
    EXPECT_STREQ(str, "");
    release();

    const char* svg_sf5 = "<svg><g fill=\" url( #grad2 ) \"/></svg>";
    load(svg_sf5);
    svg_node = root->get_child(0);
    str = svg_node->attr_at(0)->value.sval;
    EXPECT_STREQ(str, "grad2");
    release();

    const char* svg_sf6 = "<svg><g fill=\" url (#grad2) \"/></svg>";
    load(svg_sf6);
    svg_node = root->get_child(0);
    uint32_t c = svg_node->attr_at(0)->value.uval;
    EXPECT_EQ(c, 0);
    release();

    const char* svg_sf7 = "<svg><g fill=\"rgb(255, 255, 255)\"/></svg>";
    load(svg_sf7);
    svg_node = root->get_child(0);
    c = svg_node->attr_at(0)->value.uval;
    EXPECT_EQ(c, 0xffffff);
    release();

    const char* svg_sf8 = "<svg><g fill=\"rgb(50%, 50%, 50%)\"/></svg>";
    load(svg_sf8);
    svg_node = root->get_child(0);
    c = svg_node->attr_at(0)->value.uval;
    EXPECT_EQ(c, 0x808080);
    release();

    const char* svg_sf9 = "<svg><g fill=\"#F00\"/></svg>";
    load(svg_sf9);
    svg_node = root->get_child(0);
    c = svg_node->attr_at(0)->value.uval;
    EXPECT_EQ(c, 0xff0000);
    release();

    const char* svg_sf10 = "<svg><g fill=\"#FF8000\"/></svg>";
    load(svg_sf10);
    svg_node = root->get_child(0);
    c = svg_node->attr_at(0)->value.uval;
    EXPECT_EQ(c, 0xff8000);
    release();

    const char* svg_sf11 = "<svg><g fill=\"red\"/></svg>";
    load(svg_sf11);
    svg_node = root->get_child(0);
    c = svg_node->attr_at(0)->value.uval;
    EXPECT_EQ(c, 0xff0000);
    release();

    const char* svg_sf12 = "<svg><g fill=\"rgba(255, 255, 255, 1.0)\"/></svg>";
    load(svg_sf12);
    svg_node = root->get_child(0);
    c = svg_node->attr_at(0)->value.uval;
    EXPECT_EQ(c, 0xffffffff);
    release();

    const char* svg_sf13 = "<svg><g fill=\"rgba(255, 255, 255, 128)\"/></svg>";
    load(svg_sf13);
    svg_node = root->get_child(0);
    c = svg_node->attr_at(0)->value.uval;
    EXPECT_EQ(c, 0x80ffffff);
    release();
}

TEST_F(SVGParserTest, FillStrokeAttrsTest)
{
    const char* svg_sf0 = "<svg><g fill-rule=\'\'/></svg>";
    load(svg_sf0);
    psx_svg_node* svg_node = root->get_child(0);
    EXPECT_EQ(svg_node->attr_count(), 0);
    release();

    const char* svg_sf1 = "<svg><g stroke-width=1 stroke-miterlimit=3 fill-rule=\'inherit\' /></svg>";
    load(svg_sf1);
    svg_node = root->get_child(0);
    float f1 = svg_node->attr_at(0)->value.fval;
    EXPECT_FLOAT_EQ(f1, 1.0f);
    uint32_t f2 = svg_node->attr_at(1)->value.ival;
    EXPECT_EQ(f2, 3);
    release();

    const char* svg_sf2 = "<svg><g fill-rule=\"evenodd\" stroke-width=\'-1\' stroke-miterlimit=\'-5.0\'/></svg>";
    load(svg_sf2);
    svg_node = root->get_child(0);
    uint32_t r1 = svg_node->attr_at(0)->value.ival;
    EXPECT_EQ(r1, FILL_RULE_EVEN_ODD);
    float w1 = svg_node->attr_at(1)->value.fval;
    EXPECT_FLOAT_EQ(w1, 0.0f);
    uint32_t l1 = svg_node->attr_at(2)->value.uval;
    EXPECT_EQ(l1, 1);
    release();

    const char* svg_sf3 = "<svg><g stroke-linecap=\"round\" stroke-linejoin=\' bevel\' fill-rule=nonzero/></svg>";
    load(svg_sf3);
    svg_node = root->get_child(0);
    uint32_t c1 = svg_node->attr_at(0)->value.ival;
    EXPECT_EQ(c1, LINE_CAP_ROUND);
    uint32_t c2 = svg_node->attr_at(1)->value.ival;
    EXPECT_EQ(c2, LINE_JOIN_BEVEL);
    uint32_t r2 = svg_node->attr_at(2)->value.ival;
    EXPECT_EQ(r2, FILL_RULE_WINDING);
    release();

    const char* svg_sf4 = "<svg><g stroke-linecap=\" square \" stroke-linejoin=round/></svg>";
    load(svg_sf4);
    svg_node = root->get_child(0);
    uint32_t c3 = svg_node->attr_at(0)->value.ival;
    EXPECT_EQ(c3, LINE_CAP_SQUARE);
    uint32_t c4 = svg_node->attr_at(1)->value.ival;
    EXPECT_EQ(c4, LINE_JOIN_ROUND);
    release();

    const char* svg_sf5 = "<svg><g stroke-linecap=\"a\" stroke-linejoin=\'b\' stroke-dasharray=\"none\" stroke-opacity=\"inherit\"/></svg>";
    load(svg_sf5);
    svg_node = root->get_child(0);
    uint32_t c5 = svg_node->attr_at(0)->value.ival;
    EXPECT_EQ(c5, LINE_CAP_BUTT);
    uint32_t c6 = svg_node->attr_at(1)->value.ival;
    EXPECT_EQ(c6, LINE_JOIN_MITER);
    release();

    const char* svg_sf6 = "<svg><g stroke-dasharray=\"1,2,3, 2.5, 3 \" stroke-dashoffset=1.2"
                          " fill-opacity=\"2.0\" stroke-dasharray=\"inherit\" /></svg>";
    load(svg_sf6);
    svg_node = root->get_child(0);
    psx_svg_attr_values_list* list = (psx_svg_attr_values_list*)svg_node->attr_at(0)->value.val;
    EXPECT_EQ(list->length, 5);
    float* arr = (float*)(&list->data);
    float ret[5] = {1.0f, 2.0f, 3.0f, 2.5f, 3.0f};
    EXPECT_ARRAY_EQ(ret, arr, 5);

    float c8 = svg_node->attr_at(1)->value.fval;
    EXPECT_FLOAT_EQ(c8, 1.2f);
    float c9 = svg_node->attr_at(2)->value.fval;
    EXPECT_FLOAT_EQ(c9, 1.0f);
    release();
}

TEST_F(SVGParserTest, TextAttrsTest)
{
    const char* svg_sf0 = "<svg><text font-size=\'16\' font-family=\"arial\" font-variant=inherit>hello world!</text></svg>";
    load(svg_sf0);
    psx_svg_node* svg_node = root->get_child(0);

    float font_size = svg_node->attr_at(0)->value.fval;
    EXPECT_FLOAT_EQ(font_size, 16.0f);
    const char* font_family = svg_node->attr_at(1)->value.sval;
    EXPECT_STREQ(font_family, "arial");
    psx_svg_node* svg_node1 = svg_node->get_child(0);
    const char* content = svg_node1->content();
    EXPECT_STREQ(content, "hello world!");
    release();

    const char* svg_sf1 = "<svg><text font-size=\'16em\' font=\'user\' font-style=\" italic \" >hello<tspan>my\n</tspan>world!</text></svg>";
    load(svg_sf1);
    svg_node = root->get_child(0);

    font_size = svg_node->attr_at(0)->value.fval;
    EXPECT_FLOAT_EQ(font_size, 256.0f);
    const char* font_style = svg_node->attr_at(1)->value.sval;
    EXPECT_STREQ(font_style, "italic ");
    svg_node1 = svg_node->get_child(0);
    const char* content1 = svg_node1->content();
    EXPECT_STREQ(content1, "hello");

    svg_node1 = svg_node->get_child(2);
    const char* content2 = svg_node1->content();
    EXPECT_STREQ(content2, "world!");

    svg_node1 = svg_node->get_child(1);
    psx_svg_node* svg_node2 = svg_node1->get_child(0);
    const char* content3 = svg_node2->content();
    EXPECT_STREQ(content3, "my");
    release();
}

TEST_F(SVGParserTest, GradientTest)
{
    const char* svg_gt1 = "<svg><linearGradient id=\"gt1\" gradientUnits= objectBoundingBox>"
                          "<stop stop-color='red' offset=0.1/>"
                          "<stop stop-color=\'black\' stop-opacity=\"0.5\" offset=1.0/>"
                          "</linearGradient></svg>";
    load(svg_gt1);
    psx_svg_node* svg_node = root->get_child(0);
    EXPECT_STREQ(svg_node->content(), "gt1");

    psx_svg_node* svg_node1 = svg_node->get_child(1);
    uint32_t c1 = svg_node1->attr_at(0)->value.uval;
    EXPECT_EQ(c1, 0);

    float o1 = svg_node1->attr_at(1)->value.fval;
    EXPECT_FLOAT_EQ(o1, 0.5f);

    float o2 = svg_node1->attr_at(2)->value.fval;
    EXPECT_FLOAT_EQ(o2, 1.0f);
    release();

    const char* svg_gt2 = "<svg><defs><radialGradient xml:id=\"gt2\" gradientUnits=\"userSpaceOnUse\""
                          "cx=\"400\" cy=\"200\" r=\"300\">"
                          "<stop offset=\"0\" stop-color=\"red\"/>"
                          "<stop offset=\"0.5\" stop-color=\"blue\"/>"
                          "<stop offset=\"1\" stop-color=\"red\"/>"
                          "</radialGradient></defs></svg>";
    load(svg_gt2);
    svg_node = root->get_child(0);

    svg_node1 = svg_node->get_child(0);
    EXPECT_STREQ(svg_node1->content(), "gt2");

    uint32_t g = svg_node1->attr_at(0)->value.ival;
    EXPECT_EQ(g, SVG_GRADIENT_UNITS_USER_SPACE);

    float cx = svg_node1->attr_at(1)->value.fval;
    EXPECT_FLOAT_EQ(cx, 400.0f);

    float cy = svg_node1->attr_at(2)->value.fval;
    EXPECT_FLOAT_EQ(cy, 200.0f);
    release();
}

TEST_F(SVGParserTest, BadCaseTest)
{
    const char* svg_b1 = "<rect x=10 y=10 width=100 height=100/>";
    load(svg_b1);
    EXPECT_EQ(NULL, root);
    release();

    const char* svg_b2 = "<svg><text><rect x=10 y=10 width=200 height=200/></svg>";
    load(svg_b2);
    EXPECT_EQ(NULL, root);
    release();

    const char* svg_b3 = "<svg><rect x=10 y=10 width=200 height/></svg>";
    load(svg_b3);
    EXPECT_EQ(NULL, root);
    release();

    const char* svg_b4 = "<svg><g fill=\"url( \"/></svg>";
    load(svg_b4);
    psx_svg_node* svg_node = root->get_child(0);
    EXPECT_EQ(0, svg_node->attr_at(0)->class_type);
    release();

    const char* svg_b5 = "<svg><g transform=matrix/></svg>";
    load(svg_b5);
    svg_node = root->get_child(0);
    ps_matrix* matrix = (ps_matrix*)(svg_node->attr_at(0)->value.val);
    EXPECT_EQ(ps_matrix_is_identity(matrix), True);
    release();

    const char* svg_b6 = "<svg><123><123></svg>";
    load(svg_b6);
    EXPECT_EQ(NULL, root);
    release();

    const char* svg_b7 = "<svg><my> bad case <you></svg>";
    load(svg_b7);
    EXPECT_EQ(NULL, root);
    release();

    const char* svg_b8 = "<svg><path d=\"M 100  L150 180 L 150 Z\"/></svg>";
    load(svg_b8);
    svg_node = root->get_child(0);

    ps_path* path = (ps_path*)(svg_node->attr_at(0)->value.val);

    ps_point p;
    ps_path_cmd cmd = ps_path_get_vertex(path, 0, &p);
    EXPECT_EQ(cmd, PATH_CMD_MOVE_TO);

    EXPECT_FLOAT_EQ(p.x, 100.0f);
    EXPECT_FLOAT_EQ(p.y, 150.0f);

    cmd = ps_path_get_vertex(path, 1, &p);
    EXPECT_EQ(cmd, PATH_CMD_LINE_TO);

    EXPECT_FLOAT_EQ(p.x, 180.0f);
    EXPECT_FLOAT_EQ(p.y, 150.0f);
    release();

    const char* svg_b9 = "<svg><path d=\"M 100 200 L150 Z\"/></svg>";
    load(svg_b9);
    svg_node = root->get_child(0);

    path = (ps_path*)(svg_node->attr_at(0)->value.val);

    cmd = ps_path_get_vertex(path, 0, &p);
    EXPECT_EQ(cmd, PATH_CMD_MOVE_TO);

    EXPECT_FLOAT_EQ(p.x, 100.0f);
    EXPECT_FLOAT_EQ(p.y, 200.0f);
    release();

    const char* svg_b10 = "<svg></text>bad case</text></svg>";
    load(svg_b10);
    EXPECT_EQ(NULL, root);
    release();

    const char* svg_b11 = "<svg><text font-size></text></svg>";
    load(svg_b11);
    svg_node = root->get_child(0);
    EXPECT_EQ(svg_node->attr_count(), 0);
    release();

    const char* svg_b12 = "<svg><text font-size=></text></svg>";
    load(svg_b12);
    svg_node = root->get_child(0);
    EXPECT_EQ(svg_node->attr_count(), 0);
    EXPECT_EQ(svg_node->child_count(), 0);
    release();

    const char* svg_b13 = "<svg><!-aaaa /></svg>";
    load(svg_b13);
    EXPECT_NE(nullptr, root);
    release();

    const char* svg_b14 = "<svg><rect"
                          "x=1 y=1 width=10 height=10"
                          "x=1 y=1 width=10 height=10"
                          "x=1 y=1 width=10 height=10"
                          " /></svg>";
    load(svg_b14);
    svg_node = root->get_child(0);
    EXPECT_NE(nullptr, root);
    release();

    const char* svg_b15 = "<svg>"
                          "<path d=\'m-122.3,84.285s0.1,1.894-0.73,1.875c-0.82-0.019-17.27-48.094-37.8-45.851,0,0,17.78-7.353,38.53,43.976z\'/>"
                          "</svg>";
    load(svg_b15);
    svg_node = root->get_child(0);

    path = (ps_path*)(svg_node->attr_at(0)->value.val);

    cmd = ps_path_get_vertex(path, 0, &p);
    EXPECT_EQ(cmd, PATH_CMD_MOVE_TO);

    EXPECT_FLOAT_EQ(p.x, -122.3f);
    EXPECT_FLOAT_EQ(p.y, 84.285f);

    release();
}

TEST_F(SVGParserTest, AnimateTest)
{
    const char* svg_anim0 = "<svg><rect xml:id=\"RectElement\" x=\"300\" y=\"100\" width=\"300\" height=\"100\">"
                            "<animate attributeName=\"x\" dur=\"9s\" fill=\"freeze\" from=\"300\" to=\"0\"/>"
                            "</rect></svg>";
    load(svg_anim0);
    psx_svg_node* svg_node = root->get_child(0);
    psx_svg_node* anim_node = svg_node->get_child(0);
    EXPECT_NE(nullptr, anim_node);

    EXPECT_EQ(SVG_TAG_ANIMATE, anim_node->type());
    psx_svg_attr_type at = (psx_svg_attr_type)anim_node->attr_at(0)->value.ival;
    EXPECT_EQ(SVG_ATTR_X, at);

    float dur = anim_node->attr_at(1)->value.fval;
    EXPECT_FLOAT_EQ(dur, 9000.0f);

    int ft = anim_node->attr_at(2)->value.ival;
    EXPECT_EQ(ft, SVG_ANIMATION_FREEZE);

    float fr = anim_node->attr_at(3)->value.fval;
    EXPECT_FLOAT_EQ(fr, 300.0f);

    float to = anim_node->attr_at(4)->value.fval;
    EXPECT_FLOAT_EQ(to, 0.0f);
    release();
}

TEST_F(SVGParserTest, SetTest)
{
    const char* svg_anim0 = "<svg><rect xml:id=\"RectElement\" x=\"300\" y=\"100\" width=\"300\" height=\"100\">"
                            "<set attributeName=\"x\" to=\"500\" values=\"0\"/>"
                            "</rect></svg>";
    load(svg_anim0);
    psx_svg_node* svg_node = root->get_child(0);
    psx_svg_node* anim_node = svg_node->get_child(0);
    EXPECT_NE(nullptr, anim_node);

    EXPECT_EQ(SVG_TAG_SET, anim_node->type());
    psx_svg_attr_type at = (psx_svg_attr_type)anim_node->attr_at(0)->value.ival;
    EXPECT_EQ(SVG_ATTR_X, at);

    float to = anim_node->attr_at(1)->value.fval;
    EXPECT_FLOAT_EQ(to, 500.0f);
    release();
}

TEST_F(SVGParserTest, AnimateMotionTest)
{
    const char* svg_anim0 = "<svg><path xml:id=\"path1\" d=\"M100,250 C 100,50 400,50 400,250\" "
                            "fill=\"none\" stroke=\"blue\" stroke-width=\"7.06\"/>"
                            "<animateMotion dur=\"6s\" repeatCount=\"indefinite\" rotate=\"auto\">"
                            "<mpath xlink:href=\"#path1\"/>"
                            "</animateMotion>"
                            "</svg>";

    load(svg_anim0);
    psx_svg_node* path_node = root->get_child(0);
    EXPECT_NE(nullptr, path_node);
    psx_svg_node* anim_node = root->get_child(1);
    EXPECT_NE(nullptr, anim_node);

    EXPECT_EQ(SVG_TAG_ANIMATE_MOTION, anim_node->type());

    psx_svg_node* mpath_node = anim_node->get_child(0);
    EXPECT_NE(nullptr, mpath_node);

    EXPECT_EQ(SVG_TAG_MPATH, mpath_node->type());

    const char* xlink = mpath_node->attr_at(0)->value.sval;
    EXPECT_STREQ(xlink, path_node->content());

    uint32_t rp = anim_node->attr_at(1)->value.uval;
    EXPECT_EQ(rp, 0);

    float rt = anim_node->attr_at(2)->value.fval;
    EXPECT_FLOAT_EQ(rt, 0.0f);
    release();

    const char* svg_anim1 = "<svg><circle r=\"5\" fill=\"blue\">"
                            "<animateMotion begin=\"500ms\" dur=\"3.1s\" calcMode=\"linear\" "
                            "keyPoints=\"0.5; 0.8; 1.0\" path=\"M15,43 C15,43 36,20 65,33\"/>"
                            "</circle>"
                            "</svg>";

    load(svg_anim1);
    path_node = root->get_child(0);
    EXPECT_NE(nullptr, path_node);
    anim_node = path_node->get_child(0);
    EXPECT_NE(nullptr, anim_node);

    EXPECT_EQ(SVG_TAG_ANIMATE_MOTION, anim_node->type());

    psx_svg_attr_values_list* lb = (psx_svg_attr_values_list*)anim_node->attr_at(0)->value.val;
    EXPECT_EQ(lb->length, 1);

    float* fb = (float*)(&lb->data);
    EXPECT_FLOAT_EQ(*fb, 500.0f);

    float dr = anim_node->attr_at(1)->value.fval;
    EXPECT_FLOAT_EQ(dr, 3100.0f);

    int cm = anim_node->attr_at(2)->value.ival;
    EXPECT_EQ(cm, SVG_ANIMATION_CALC_MODE_LINEAR);

    psx_svg_attr_values_list* l = (psx_svg_attr_values_list*)anim_node->attr_at(3)->value.val;
    EXPECT_EQ(l->length, 3);

    float* pt = (float*)(&l->data);
    EXPECT_FLOAT_EQ(pt[0], 0.5f);
    EXPECT_FLOAT_EQ(pt[1], 0.8f);
    EXPECT_FLOAT_EQ(pt[2], 1.0f);

    release();

    const char* svg_anim2 = "<svg><circle r=\"5\" fill=\"blue\">"
                            "<animateMotion begin=\"5s;2s\" end=\"8s;10s\" values=\"100, 50;200 200\" keyTimes=\"100ms;200ms\""
                            " keySplines=\"0 0 1.5 1.0; 0.5 0.5, 2.0,1.5\" additive=\"sum\" accumulate=\"none\"/>"
                            "</circle>"
                            "</svg>";

    load(svg_anim2);
    path_node = root->get_child(0);
    EXPECT_NE(nullptr, path_node);
    anim_node = path_node->get_child(0);
    EXPECT_NE(nullptr, anim_node);

    EXPECT_EQ(SVG_TAG_ANIMATE_MOTION, anim_node->type());

    lb = (psx_svg_attr_values_list*)anim_node->attr_at(0)->value.val;
    EXPECT_EQ(lb->length, 2);
    fb = (float*)(&lb->data);
    EXPECT_FLOAT_EQ(fb[0], 5000.0f);
    EXPECT_FLOAT_EQ(fb[1], 2000.0f);

    lb = (psx_svg_attr_values_list*)anim_node->attr_at(1)->value.val;
    EXPECT_EQ(lb->length, 2);
    fb = (float*)(&lb->data);
    EXPECT_FLOAT_EQ(fb[0], 8000.0f);
    EXPECT_FLOAT_EQ(fb[1], 10000.0f);

    lb = (psx_svg_attr_values_list*)anim_node->attr_at(2)->value.val;
    EXPECT_EQ(lb->length, 2);
    psx_svg_point* ps = (psx_svg_point*)(&lb->data);
    EXPECT_FLOAT_EQ(ps[0].x, 100.0f);
    EXPECT_FLOAT_EQ(ps[0].y, 50.0f);
    EXPECT_FLOAT_EQ(ps[1].x, 200.0f);
    EXPECT_FLOAT_EQ(ps[1].y, 200.0f);

    lb = (psx_svg_attr_values_list*)anim_node->attr_at(3)->value.val;
    EXPECT_EQ(lb->length, 2);
    fb = (float*)(&lb->data);
    EXPECT_FLOAT_EQ(fb[0], 100.0f);
    EXPECT_FLOAT_EQ(fb[1], 200.0f);

    l = (psx_svg_attr_values_list*)anim_node->attr_at(4)->value.val;
    EXPECT_EQ(l->length, 4);

    ps = (psx_svg_point*)(&l->data);
    EXPECT_FLOAT_EQ(ps[0].x, 0.0f);
    EXPECT_FLOAT_EQ(ps[0].y, 0.0f);
    EXPECT_FLOAT_EQ(ps[1].x, 1.5f);
    EXPECT_FLOAT_EQ(ps[1].y, 1.0f);
    EXPECT_FLOAT_EQ(ps[2].x, 0.5f);
    EXPECT_FLOAT_EQ(ps[2].y, 0.5f);
    EXPECT_FLOAT_EQ(ps[3].x, 2.0f);
    EXPECT_FLOAT_EQ(ps[3].y, 1.5f);

    release();
}

TEST_F(SVGParserTest, AnimateTransformTest)
{
    const char* svg_anim0 = "<svg><rect transform=\"skewX(30)\" x=0 y=0 width=100 height=100>"
                            "<animateTransform attributeName=\"transform\" attributeType=\"XML\""
                            "type=\"rotate\" from=\"0\" to=\"90\" dur=\"5s\""
                            "additive=\"sum\" fill=\"freeze\"/>"
                            "<animateTransform attributeName=\"transform\" attributeType=\"XML\""
                            "type=\"scale\" from=\"1\" to=\"2\" dur=\"5s\" values=\"0.5; 0.2, 0.2\""
                            "additive=\"sum\" fill=\"freeze\"/>"
                            "</rect></svg>";

    load(svg_anim0);
    psx_svg_node* path_node = root->get_child(0);
    EXPECT_NE(nullptr, path_node);
    psx_svg_node* anim_node1 = path_node->get_child(0);
    EXPECT_NE(nullptr, anim_node1);
    EXPECT_EQ(SVG_TAG_ANIMATE_TRANSFORM, anim_node1->type());

    psx_svg_node* anim_node2 = path_node->get_child(1);
    EXPECT_NE(nullptr, anim_node2);
    EXPECT_EQ(SVG_TAG_ANIMATE_TRANSFORM, anim_node2->type());

    psx_svg_attr_type at = (psx_svg_attr_type)anim_node1->attr_at(0)->value.ival;
    EXPECT_EQ(SVG_ATTR_TRANSFORM, at);

    int32_t tt = anim_node1->attr_at(1)->value.ival;
    EXPECT_EQ(SVG_TRANSFORM_TYPE_ROTATE, tt);

    psx_svg_attr_values_list* l = (psx_svg_attr_values_list*)anim_node1->attr_at(2)->value.val;
    EXPECT_EQ(l->length, 1);
    float* pt = (float*)(&l->data);
    EXPECT_FLOAT_EQ(pt[0], 0.0f);

    l = (psx_svg_attr_values_list*)anim_node2->attr_at(5)->value.val;
    EXPECT_EQ(l->length, 2);
    psx_svg_attr_values_list* ll = (psx_svg_attr_values_list*)(&l->data);
    EXPECT_EQ(ll->length, 1);
    pt = (float*)(&ll->data);
    EXPECT_FLOAT_EQ(pt[0], 0.5f);

    ll = (psx_svg_attr_values_list*)((uint8_t*)(&l->data) + sizeof(uint32_t) + sizeof(float) * 4);
    EXPECT_EQ(ll->length, 2);
    pt = (float*)(&ll->data);
    EXPECT_FLOAT_EQ(pt[0], 0.2f);
    EXPECT_FLOAT_EQ(pt[1], 0.2f);

    release();
}

TEST_F(SVGParserTest, AnimateColorTest)
{
    const char* svg_anim0 = "<svg><rect color=\"yellow\" fill=\"black\">"
                            "<animateColor attributeName=\"fill\" from=\"red\" to=\"#DDF\" "
                            "begin=\"1s\" dur=\"5s\" fill=\"freeze\" additive=\"sum\" repeatCount=5"
                            " restart=\"whenNotActive\" values=\"rgb(0,255,0);black\" />"
                            "</rect></svg>";

    load(svg_anim0);
    psx_svg_node* path_node = root->get_child(0);
    EXPECT_NE(nullptr, path_node);
    psx_svg_node* anim_node = path_node->get_child(0);
    EXPECT_NE(nullptr, anim_node);
    EXPECT_EQ(SVG_TAG_ANIMATE_COLOR, anim_node->type());

    psx_svg_attr_type at = (psx_svg_attr_type)anim_node->attr_at(0)->value.ival;
    EXPECT_EQ(SVG_ATTR_FILL, at);

    uint32_t c = anim_node->attr_at(1)->value.uval;
    EXPECT_EQ(c, 0xFF0000);

    c = anim_node->attr_at(2)->value.uval;
    EXPECT_EQ(c, 0xDDDDFF);

    psx_svg_attr_values_list* l = (psx_svg_attr_values_list*)anim_node->attr_at(9)->value.val;
    EXPECT_EQ(l->length, 2);
    uint32_t* pc = (uint32_t*)(&l->data);
    EXPECT_EQ(pc[0], 0x00FF00);
    EXPECT_EQ(pc[1], 0x000000);

    release();
}
