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
        root = psx_svg_load(data, (uint32_t)strlen(data));
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
    ASSERT_NE(root, nullptr);
}

TEST_F(SVGParserTest, SVGElementTest)
{
    const char* svg_data1 = "<svg version=\"1.2\" baseProfile=\"tiny\"></svg>";
    load(svg_data1);
    ASSERT_EQ(root->child_count(), 0);
    ASSERT_EQ(root->attr_count(), 2);
    ASSERT_STREQ(root->attr_at(0)->value.sval, "1.2");
    ASSERT_STREQ(root->attr_at(1)->value.sval, "tiny");
    release();

    const char* svg_viewbox0 = "<svg viewBox=\"none\"></svg>";
    load(svg_viewbox0);
    ASSERT_EQ(root->attr_at(0)->class_type, SVG_ATTR_VALUE_NONE);
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
    ASSERT_EQ(root->attr_at(0)->class_type, SVG_ATTR_VALUE_NONE);
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
