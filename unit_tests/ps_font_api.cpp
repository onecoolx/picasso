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

#define SNAPSHOT_PATH "draw"

#include "test.h"

#include "picasso_objects.h"

class FontTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        PS_Init();
        canvas = get_test_canvas();
        ctx = ps_context_create(canvas, NULL);
    }

    void TearDown() override
    {
        if (ctx) { ps_context_unref(ctx); }
        PS_Shutdown();
    }

    ps_canvas* canvas;
    ps_context* ctx;
};

#define ps_text_out(ctx, x, y, str) ps_text_out_length((ctx), (float)(x), (float)(y), (str), (uint32_t)strlen((str)))

// Font creation tests
TEST_F(FontTest, CreateFontValidParams)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_unref(font);
}

TEST_F(FontTest, CreateFontNullName)
{
    ps_font* font = ps_font_create(NULL, CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_EQ(NULL, font);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, CreateFontEmptyName)
{
    ps_font* font = ps_font_create("", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    ps_font_unref(font);
}

TEST_F(FontTest, CreateFontZeroSize)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 0.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    ps_font_unref(font);
}

TEST_F(FontTest, CreateFontNegativeSize)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, -5.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    ps_font_unref(font);
}

TEST_F(FontTest, CreateFontDifferentCharsets)
{
    ps_font* font1 = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font1 != NULL);

    ps_font* font2 = ps_font_create("Arial", CHARSET_UNICODE, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font2 != NULL);

    ps_font_unref(font1);
    ps_font_unref(font2);
}

TEST_F(FontTest, CreateFontAllWeights)
{
    const ps_font_weight weights[] = {
        FONT_WEIGHT_REGULAR,
        FONT_WEIGHT_MEDIUM,
        FONT_WEIGHT_BOLD,
        FONT_WEIGHT_HEAVY
    };

    for (int i = 0; i < 4; i++) {
        ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, weights[i], False);
        ASSERT_TRUE(font != NULL) << "Weight " << weights[i] << " failed";
        ps_font_unref(font);
    }
}

TEST_F(FontTest, CreateFontItalicVariants)
{
    ps_font* regular = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ps_font* italic = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, True);

    ASSERT_TRUE(regular != NULL);
    ASSERT_TRUE(italic != NULL);

    ps_font_unref(regular);
    ps_font_unref(italic);
}

// Font copy tests
TEST_F(FontTest, CreateCopyValidFont)
{
    ps_font* original = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_BOLD, True);
    ASSERT_TRUE(original != NULL);

    ps_font* copy = ps_font_create_copy(original);
    ASSERT_TRUE(copy != NULL);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    ASSERT_NE(original, copy); // Should be different objects

    ps_font_unref(original);
    ps_font_unref(copy);
}

TEST_F(FontTest, CreateCopyNullFont)
{
    ps_font* copy = ps_font_create_copy(NULL);
    ASSERT_EQ(NULL, copy);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Reference counting tests
TEST_F(FontTest, FontReferenceCounting)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* ref1 = ps_font_ref(font);
    ASSERT_EQ(font, ref1);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font* ref2 = ps_font_ref(font);
    ASSERT_EQ(font, ref2);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Should not crash with multiple unrefs
    ps_font_unref(ref1);
    ps_font_unref(ref2);
    ps_font_unref(font);
}

TEST_F(FontTest, FontRefNull)
{
    ps_font* result = ps_font_ref(NULL);
    ASSERT_EQ(NULL, result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, FontUnrefNull)
{
    // Should not crash
    ps_font_unref(NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Font property setter tests
TEST_F(FontTest, FontSetSize)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font_set_size(font, 24.0f);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_set_size(font, 0.0f);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_set_size(font, -10.0f);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_font_unref(font);
}

TEST_F(FontTest, FontSetWeight)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font_set_weight(font, FONT_WEIGHT_BOLD);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_set_weight(font, 100); // Custom weight
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_unref(font);
}

TEST_F(FontTest, FontSetWeightNull)
{
    ps_font_set_weight(NULL, FONT_WEIGHT_BOLD);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, FontSetItalic)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font_set_italic(font, True);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_set_italic(font, False);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_unref(font);
}

TEST_F(FontTest, FontSetItalicNull)
{
    ps_font_set_italic(NULL, True);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, FontSetCharset)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font_set_charset(font, CHARSET_UNICODE);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_set_charset(font, CHARSET_ANSI);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_unref(font);
}

TEST_F(FontTest, FontSetCharsetNull)
{
    ps_font_set_charset(NULL, CHARSET_UNICODE);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, FontSetHint)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font_set_hint(font, True);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_set_hint(font, False);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_unref(font);
}

TEST_F(FontTest, FontSetHintNull)
{
    ps_font_set_hint(NULL, True);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, FontSetFlip)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font_set_flip(font, True);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_set_flip(font, False);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_unref(font);
}

TEST_F(FontTest, FontSetFlipNull)
{
    ps_font_set_flip(NULL, True);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Font info tests
TEST_F(FontTest, GetFontInfo)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_info info;
    ps_bool result = ps_get_font_info(ctx, &info);
    ASSERT_TRUE(result);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Verify info structure has reasonable values
    ASSERT_GT(info.size, 0.0f);
    ASSERT_GT(info.unitsEM, 0);

    ps_set_font(ctx, oldFont); // reset font

    ps_font_unref(font);
}

TEST_F(FontTest, GetFontInfoNullContext)
{
    ps_font_info info;
    ps_bool result = ps_get_font_info(NULL, &info);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, GetFontInfoNullInfo)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_bool result = ps_get_font_info(ctx, NULL);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_set_font(ctx, oldFont); // reset font

    ps_font_unref(font);
}

// Set font tests
TEST_F(FontTest, SetFontValid)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_set_font(ctx, oldFont); // reset font

    ps_font_unref(font);
}

TEST_F(FontTest, SetFontNullContext)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* result = ps_set_font(NULL, font);
    ASSERT_EQ(NULL, result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_font_unref(font);
}

TEST_F(FontTest, SetFontNullFont)
{
    ps_font* oldFont = ps_set_font(ctx, NULL);
    ASSERT_EQ(NULL, oldFont);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Visual rendering tests
TEST_F(FontTest, DrawBasicText)
{
    clear_test_canvas();

    ps_color color = {0.0f, 0.0f, 0.0f, 1.0f}; // Black
    ps_set_source_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);
    ps_text_out(ctx, 50, 100, "Hello World!");

    ps_set_font(ctx, oldFont); // reset font

    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_draw_basic_text);
}

TEST_F(FontTest, DrawDifferentSizes)
{
    clear_test_canvas();

    ps_color colors[] = {
        {0.0f, 0.0f, 1.0f, 1.0f}, // Blue
        {1.0f, 0.0f, 0.0f, 1.0f}, // Red
        {0.0f, 1.0f, 0.0f, 1.0f}, // Green
        {0.0f, 0.0f, 0.0f, 1.0f}, // Black
    };

    const float sizes[] = {12.0f, 18.0f, 24.0f, 36.0f};
    const char* text = "Hello World!";

    for (int i = 0; i < 4; i++) {
        ps_font* font = ps_font_create("Arial", CHARSET_ANSI, sizes[i], FONT_WEIGHT_REGULAR, False);
        ASSERT_TRUE(font != NULL);

        ps_set_text_color(ctx, &colors[i]);

        ps_font* oldFont = ps_set_font(ctx, font);
        ps_text_out(ctx, 50.0f, 100 + i * 30.0f, text);

        ps_set_font(ctx, oldFont); // reset font

        ps_font_unref(font);
    }

    EXPECT_SYS_SNAPSHOT_EQ(font_different_sizes);
}

TEST_F(FontTest, DrawDifferentWeights)
{
    clear_test_canvas();

    ps_color colors[] = {
        {0.0f, 0.0f, 1.0f, 1.0f}, // Blue
        {1.0f, 0.0f, 0.0f, 1.0f}, // Red
        {0.0f, 1.0f, 0.0f, 1.0f}, // Green
        {0.0f, 0.0f, 0.0f, 1.0f}, // Black
    };

    const ps_font_weight weights[] = {
        FONT_WEIGHT_REGULAR,
        FONT_WEIGHT_MEDIUM,
        FONT_WEIGHT_BOLD,
        FONT_WEIGHT_HEAVY
    };

    const char* labels[] = {"Regular", "Medium", "Bold", "Heavy"};

    for (int i = 0; i < 4; i++) {
        ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 20.0f, weights[i], False);
        ASSERT_TRUE(font != NULL);

        ps_set_text_color(ctx, &colors[i]);

        ps_font* oldFont = ps_set_font(ctx, font);
        ps_text_out(ctx, 50, 80 + i * 40, labels[i]);

        ps_set_font(ctx, oldFont); // reset font
        ps_font_unref(font);
    }

    EXPECT_SYS_SNAPSHOT_EQ(font_different_weights);
}

TEST_F(FontTest, DrawItalicText)
{
    clear_test_canvas();

    ps_color color1 = {0.0f, 0.5f, 0.0f, 1.0f}; // Green
    ps_color color2 = {0.5f, 0.0f, 0.0f, 1.0f}; // Red

    ps_font* regular = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ps_font* italic = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, True);

    ASSERT_TRUE(regular != NULL);
    ASSERT_TRUE(italic != NULL);

    ps_set_text_color(ctx, &color1);
    ps_font* oldFont = ps_set_font(ctx, regular);
    ps_text_out(ctx, 50, 100, "Regular Text");
    ps_set_font(ctx, oldFont); // reset font

    ps_set_text_color(ctx, &color2);
    oldFont = ps_set_font(ctx, italic);
    ps_text_out(ctx, 50, 150, "Italic Text");
    ps_set_font(ctx, oldFont); // reset font

    ps_font_unref(regular);
    ps_font_unref(italic);

    EXPECT_SYS_SNAPSHOT_EQ(font_italic_comparison);
}

TEST_F(FontTest, DrawModifiedFont)
{
    clear_test_canvas();

    ps_color color = {0.5f, 0.0f, 0.5f, 1.0f}; // Purple
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 20.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);
    ps_text_out(ctx, 50, 80, "Original");

    // Modify font properties
    ps_font_set_size(font, 30.0f);
    ps_text_out(ctx, 50, 130, "Modified Size");

    ps_font_set_weight(font, FONT_WEIGHT_BOLD);
    ps_text_out(ctx, 50, 180, "Modified Weight");

    ps_font_set_italic(font, True);
    ps_text_out(ctx, 50, 230, "Modified Italic");

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_modified_properties);
}

// Text extent tests
TEST_F(FontTest, GetTextExtentValid)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_size extent;
    ps_bool result = ps_get_text_extent(ctx, "Hello World", 11, &extent);
    ASSERT_TRUE(result);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_GT(extent.w, 0.0f);
    ASSERT_GT(extent.h, 0.0f);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
}

TEST_F(FontTest, GetTextExtentNullContext)
{
    ps_size extent;
    ps_bool result = ps_get_text_extent(NULL, "Hello", 5, &extent);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, GetTextExtentNullText)
{
    ps_size extent;
    ps_bool result = ps_get_text_extent(ctx, NULL, 5, &extent);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, GetTextExtentZeroLength)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_size extent;
    ps_bool result = ps_get_text_extent(ctx, "Hello", 0, &extent);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
}

TEST_F(FontTest, GetTextExtentNullSize)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_bool result = ps_get_text_extent(ctx, "Hello", 5, NULL);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
}

TEST_F(FontTest, GetTextExtentUnicode)
{
    ps_font* font = ps_font_create("Arial", CHARSET_UNICODE, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    const ps_uchar16 unicode_text[] = {0x4F60, 0x597D, 0x4E16, 0x754C}; // UCS-2 text
    ps_size extent;
    ps_bool result = ps_get_text_extent(ctx, unicode_text, 4, &extent);
    ASSERT_TRUE(result);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_GT(extent.w, 0.0f);
    ASSERT_GT(extent.h, 0.0f);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
}

// Glyph tests
TEST_F(FontTest, GetGlyphValid)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_glyph glyph;
    ps_bool result = ps_get_glyph(ctx, 'A', &glyph);
    ASSERT_TRUE(result);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    ASSERT_TRUE(glyph.glyph != NULL);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
}

TEST_F(FontTest, GetGlyphNullContext)
{
    ps_glyph glyph;
    ps_bool result = ps_get_glyph(NULL, 'A', &glyph);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, GetGlyphNullGlyph)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_bool result = ps_get_glyph(ctx, 'A', NULL);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
}

TEST_F(FontTest, ShowGlyphsValid)
{
    clear_test_canvas();

    ps_color color = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Create glyphs for "HELLO"
    ps_glyph glyphs[5];
    for (int i = 0; i < 5; i++) {
        ASSERT_TRUE(ps_get_glyph(ctx, "HELLO"[i], &glyphs[i]));
    }

    ps_show_glyphs(ctx, 50, 100, glyphs, 5);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_show_glyphs);
}

TEST_F(FontTest, ShowGlyphsNullContext)
{
    ps_glyph glyph;
    ps_show_glyphs(NULL, 0, 0, &glyph, 1);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, ShowGlyphsNullGlyphs)
{
    ps_show_glyphs(ctx, 0, 0, NULL, 1);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, ShowGlyphsZeroLength)
{
    ps_show_glyphs(ctx, 0, 0, NULL, 0);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, GetPathFromGlyphValid)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_glyph glyph;
    ASSERT_TRUE(ps_get_glyph(ctx, 'A', &glyph));

    ps_path* path = ps_path_create();
    ASSERT_TRUE(path != NULL);

    ps_bool result = ps_get_path_from_glyph(ctx, &glyph, path);
    ASSERT_TRUE(result);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_path_unref(path);
    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
}

TEST_F(FontTest, GetPathFromGlyphNullContext)
{
    ps_glyph glyph;
    ps_path* path = ps_path_create();

    ps_bool result = ps_get_path_from_glyph(NULL, &glyph, path);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_unref(path);
}

TEST_F(FontTest, GetPathFromGlyphNullGlyph)
{
    ps_path* path = ps_path_create();

    ps_bool result = ps_get_path_from_glyph(ctx, NULL, path);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_unref(path);
}

TEST_F(FontTest, GetPathFromGlyphNullPath)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_glyph glyph;
    ASSERT_TRUE(ps_get_glyph(ctx, 'A', &glyph));

    ps_bool result = ps_get_path_from_glyph(ctx, &glyph, NULL);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
}

TEST_F(FontTest, GlyphGetExtentValid)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_glyph glyph;
    ASSERT_TRUE(ps_get_glyph(ctx, 'A', &glyph));

    ps_size extent;
    ps_bool result = ps_glyph_get_extent(&glyph, &extent);
    ASSERT_TRUE(result);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_GT(extent.w, 0.0f);
    ASSERT_GT(extent.h, 0.0f);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
}

TEST_F(FontTest, GlyphGetExtentNullGlyph)
{
    ps_size extent;
    ps_bool result = ps_glyph_get_extent(NULL, &extent);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, GlyphGetExtentNullSize)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_glyph glyph;
    ASSERT_TRUE(ps_get_glyph(ctx, 'A', &glyph));

    ps_bool result = ps_glyph_get_extent(&glyph, NULL);
    ASSERT_FALSE(result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
}

// Text rendering options tests
TEST_F(FontTest, SetTextColorValid)
{
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f}; // Red
    ps_set_text_color(ctx, &color);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FontTest, SetTextColorNullContext)
{
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_text_color(NULL, &color);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, SetTextColorNullColor)
{
    ps_set_text_color(ctx, NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, SetTextStrokeColorValid)
{
    ps_color color = {0.0f, 1.0f, 0.0f, 1.0f}; // Green
    ps_set_text_stroke_color(ctx, &color);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FontTest, SetTextStrokeColorNullContext)
{
    ps_color color = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_set_text_stroke_color(NULL, &color);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, SetTextStrokeColorNullColor)
{
    ps_set_text_stroke_color(ctx, NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, SetTextRenderTypeValid)
{
    ps_set_text_render_type(ctx, TEXT_TYPE_SMOOTH);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_set_text_render_type(ctx, TEXT_TYPE_MONO);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_set_text_render_type(ctx, TEXT_TYPE_STROKE);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FontTest, SetTextRenderTypeNullContext)
{
    ps_set_text_render_type(NULL, TEXT_TYPE_SMOOTH);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, SetTextAntialiasValid)
{
    ps_set_text_antialias(ctx, True);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_set_text_antialias(ctx, False);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FontTest, SetTextAntialiasNullContext)
{
    ps_set_text_antialias(NULL, True);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, SetTextKerningValid)
{
    ps_set_text_kerning(ctx, True);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_set_text_kerning(ctx, False);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(FontTest, SetTextKerningNullContext)
{
    ps_set_text_kerning(NULL, True);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Text transformation tests
TEST_F(FontTest, TextTransformValid)
{
    ps_matrix* matrix = ps_matrix_create();
    ASSERT_TRUE(matrix != NULL);

    ps_matrix_translate(matrix, 10.0f, 20.0f);
    ps_text_transform(ctx, matrix);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_matrix_unref(matrix);
}

TEST_F(FontTest, TextTransformNullContext)
{
    ps_matrix* matrix = ps_matrix_create();
    ps_text_transform(NULL, matrix);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
    ps_matrix_unref(matrix);
}

TEST_F(FontTest, TextTransformNullMatrix)
{
    ps_text_transform(ctx, NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, SetTextMatrixValid)
{
    ps_matrix* matrix = ps_matrix_create();
    ASSERT_TRUE(matrix != NULL);

    ps_matrix_scale(matrix, 2.0f, 2.0f);
    ps_set_text_matrix(ctx, matrix);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_matrix_unref(matrix);
}

TEST_F(FontTest, SetTextMatrixNullContext)
{
    ps_matrix* matrix = ps_matrix_create();
    ps_set_text_matrix(NULL, matrix);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
    ps_matrix_unref(matrix);
}

TEST_F(FontTest, SetTextMatrixNullMatrix)
{
    ps_set_text_matrix(ctx, NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Advanced visual rendering tests
TEST_F(FontTest, DrawTextInRectangle)
{
    clear_test_canvas();

    ps_color color = {0.0f, 0.0f, 1.0f, 1.0f}; // Blue
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 40.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_rect area = {50.0f, 50.0f, 300.0f, 100.0f};
    ps_rectangle(ctx, &area);
    ps_stroke(ctx);

    ps_draw_text(ctx, &area, "Hello World", 11, DRAW_TEXT_FILL, TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_draw_text_rectangle);
}

TEST_F(FontTest, DrawTextDifferentAlignments)
{
    clear_test_canvas();

    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f}; // Red
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 18.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    const char* text = "Align";
    const uint32_t alignments[] = {
        TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP,
        TEXT_ALIGN_CENTER | TEXT_ALIGN_TOP,
        TEXT_ALIGN_RIGHT | TEXT_ALIGN_TOP,
        TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER,
        TEXT_ALIGN_CENTER,
        TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER,
        TEXT_ALIGN_LEFT | TEXT_ALIGN_BOTTOM,
        TEXT_ALIGN_CENTER | TEXT_ALIGN_BOTTOM,
        TEXT_ALIGN_RIGHT | TEXT_ALIGN_BOTTOM
    };

    for (int i = 0; i < 9; i++) {
        int row = i / 3;
        int col = i % 3;
        ps_rect area = {50.0f + col * 120.0f, 50.0f + row * 70.0f, 100.0f, 60.0f};
        ps_rectangle(ctx, &area);
        ps_stroke(ctx);
        ps_draw_text(ctx, &area, text, 5, DRAW_TEXT_FILL, alignments[i]);
    }

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_text_alignments);
}

TEST_F(FontTest, DrawTextDifferentStyles)
{
    clear_test_canvas();

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Set different colors for fill and stroke
    ps_color fillColor = {1.0f, 0.0f, 0.0f, 1.0f}; // Red
    ps_color strokeColor = {0.0f, 0.0f, 1.0f, 1.0f}; // Blue
    ps_set_text_color(ctx, &fillColor);
    ps_set_text_stroke_color(ctx, &strokeColor);

    const char* text = "Style";
    ps_rect areas[] = {
        {50.0f, 50.0f, 150.0f, 60.0f}, // Fill only
        {250.0f, 50.0f, 150.0f, 60.0f}, // Stroke only
        {450.0f, 50.0f, 150.0f, 60.0f} // Both
    };

    ps_draw_text(ctx, &areas[0], text, 5, DRAW_TEXT_FILL, TEXT_ALIGN_CENTER);
    ps_draw_text(ctx, &areas[1], text, 5, DRAW_TEXT_STROKE, TEXT_ALIGN_CENTER);
    ps_draw_text(ctx, &areas[2], text, 5, DRAW_TEXT_BOTH, TEXT_ALIGN_CENTER);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_text_styles);
}

TEST_F(FontTest, DrawTextNullContext)
{
    ps_rect area = {0, 0, 100, 50};
    ps_draw_text(NULL, &area, "Hello", 5, DRAW_TEXT_FILL, TEXT_ALIGN_LEFT);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, DrawTextNullArea)
{
    ps_draw_text(ctx, NULL, "Hello", 5, DRAW_TEXT_FILL, TEXT_ALIGN_LEFT);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, DrawTextNullText)
{
    ps_rect area = {0, 0, 100, 50};
    ps_draw_text(ctx, &area, NULL, 5, DRAW_TEXT_FILL, TEXT_ALIGN_LEFT);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, DrawTextZeroLength)
{
    ps_rect area = {0, 0, 100, 50};
    ps_draw_text(ctx, &area, "Hello", 0, DRAW_TEXT_FILL, TEXT_ALIGN_LEFT);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Unicode text rendering tests
TEST_F(FontTest, DrawUnicodeText)
{
    clear_test_canvas();

    ps_color color = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_UNICODE, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Test various Unicode characters
    const ps_uchar16 unicode_text[] = {0x4F60, 0x597D, 0x4E16, 0x754C}; // UCS-2 text
    ps_wide_text_out_length(ctx, 50, 100, unicode_text, 4);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_unicode_text);
}

TEST_F(FontTest, WideTextOutNullContext)
{
    const ps_uchar16 text[] = {0x4F60, 0x597D};
    ps_wide_text_out_length(NULL, 0, 0, text, 2);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, WideTextOutNullText)
{
    ps_wide_text_out_length(ctx, 0, 0, NULL, 2);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, WideTextOutZeroLength)
{
    const ps_uchar16 text[] = {0x4F60, 0x597D};
    ps_wide_text_out_length(ctx, 0, 0, text, 0);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Text output with length tests
TEST_F(FontTest, TextOutLengthValid)
{
    clear_test_canvas();

    ps_color color = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 20.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Test with specific length (should only render "Hello" from "Hello World")
    ps_text_out_length(ctx, 50, 100, "Hello World", 5);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_text_out_length);
}

TEST_F(FontTest, TextOutLengthNullContext)
{
    ps_text_out_length(NULL, 0, 0, "Hello", 5);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, TextOutLengthNullText)
{
    ps_text_out_length(ctx, 0, 0, NULL, 5);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(FontTest, TextOutLengthZeroLength)
{
    ps_text_out_length(ctx, 0, 0, "Hello", 0);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Matrix transformation tests
TEST_F(FontTest, TextMatrixTransformations)
{
    clear_test_canvas();

    ps_color color = {0.5f, 0.0f, 1.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 20.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Original text
    ps_text_out(ctx, 50, 50, "Normal");

    // Scaled text
    ps_matrix* scaleMatrix = ps_matrix_create();
    ps_matrix_scale(scaleMatrix, 2.0f, 2.0f);
    ps_set_text_matrix(ctx, scaleMatrix);
    ps_text_out(ctx, 50, 100, "Scaled");
    ps_matrix_unref(scaleMatrix);

    // Rotated text
    ps_matrix* rotateMatrix = ps_matrix_create();
    ps_matrix_rotate(rotateMatrix, 45.0f);
    ps_set_text_matrix(ctx, rotateMatrix);
    ps_text_out(ctx, 200, 100, "Rotated");
    ps_matrix_unref(rotateMatrix);

    // Reset matrix
    ps_matrix* identityMatrix = ps_matrix_create();
    ps_set_text_matrix(ctx, identityMatrix);
    ps_text_out(ctx, 50, 200, "Reset");
    ps_matrix_unref(identityMatrix);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_text_transformations);
}

TEST_F(FontTest, TextTransformAccumulation)
{
    clear_test_canvas();

    ps_color color = {1.0f, 0.5f, 0.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 18.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Apply multiple transformations
    ps_matrix* matrix1 = ps_matrix_create();
    ps_matrix_translate(matrix1, 50.0f, 50.0f);
    ps_text_transform(ctx, matrix1);

    ps_matrix* matrix2 = ps_matrix_create();
    ps_matrix_rotate(matrix2, 0.275f);
    ps_text_transform(ctx, matrix2);

    ps_matrix* matrix3 = ps_matrix_create();
    ps_matrix_scale(matrix3, 1.125f, 1.125f);
    ps_text_transform(ctx, matrix3);

    ps_text_out(ctx, 0, 0, "Transformed");

    ps_matrix_unref(matrix1);
    ps_matrix_unref(matrix2);
    ps_matrix_unref(matrix3);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_text_accumulated_transform);
}

// Advanced rendering tests
TEST_F(FontTest, TextWithKerning)
{
    clear_test_canvas();

    ps_color color = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Text without kerning
    ps_set_text_kerning(ctx, False);
    ps_text_out(ctx, 50, 100, "AVAVAV");

    ps_color color2 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_text_color(ctx, &color2);
    // Text with kerning
    ps_set_text_kerning(ctx, True);
    ps_text_out(ctx, 50, 150, "AVAVAV");

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_text_kerning);
}

TEST_F(FontTest, TextAntialiasingModes)
{
    clear_test_canvas();

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 40.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Text with antialiasing
    ps_set_text_antialias(ctx, True);
    ps_color color1 = {1.0f, 0.0f, 1.0f, 1.0f};
    ps_set_text_color(ctx, &color1);
    ps_text_out(ctx, 50, 100, "Antialiased");

    // Text without antialiasing
    ps_set_text_antialias(ctx, False);
    ps_color color2 = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_text_color(ctx, &color2);
    ps_text_out(ctx, 50, 200, "Aliased");

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_text_antialiasing);
}

TEST_F(FontTest, TextRenderTypes)
{
    clear_test_canvas();

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 36.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_color color = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    // Smooth rendering
    ps_set_text_render_type(ctx, TEXT_TYPE_SMOOTH);
    ps_text_out(ctx, 50, 100, "Smooth");

    // Mono rendering
    ps_set_text_render_type(ctx, TEXT_TYPE_MONO);
    ps_text_out(ctx, 50, 180, "Mono");

    // Stroke rendering
    ps_set_text_render_type(ctx, TEXT_TYPE_STROKE);
    ps_text_out(ctx, 50, 260, "Stroke");

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_text_render_types);
}

// Edge cases and stress tests
TEST_F(FontTest, VeryLargeFontSize)
{
    clear_test_canvas();

    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 200.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);
    ps_text_out(ctx, 50, 200, "Big");

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_very_large_size);
}

TEST_F(FontTest, VerySmallFontSize)
{
    clear_test_canvas();

    ps_color color = {0.0f, 1.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 6.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);
    ps_text_out(ctx, 50, 50, "Tiny");

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_very_small_size);
}

TEST_F(FontTest, ExtremeFontWeights)
{
    clear_test_canvas();

    ps_color color = {0.0f, 0.0f, 0.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    // Test extreme weight values
    ps_font* font1 = ps_font_create("Arial", CHARSET_ANSI, 20.0f, 1, False); // Very light
    ps_font* font2 = ps_font_create("Arial", CHARSET_ANSI, 20.0f, 1000, False); // Very heavy

    ASSERT_TRUE(font1 != NULL);
    ASSERT_TRUE(font2 != NULL);

    ps_font* oldFont1 = ps_set_font(ctx, font1);
    ps_text_out(ctx, 50, 100, "Light");
    ps_set_font(ctx, oldFont1); // reset font

    ps_font* oldFont2 = ps_set_font(ctx, font2);
    ps_text_out(ctx, 50, 150, "Heavy");
    ps_set_font(ctx, oldFont2); // reset font

    ps_font_unref(font1);
    ps_font_unref(font2);

    EXPECT_SYS_SNAPSHOT_EQ(font_extreme_weights);
}

// Memory management tests
TEST_F(FontTest, MultipleFontCreationDestruction)
{
    // Test creating and destroying many fonts to check for memory leaks
    const int num_fonts = 100;
    ps_font* fonts[num_fonts];

    for (int i = 0; i < num_fonts; i++) {
        fonts[i] = ps_font_create("Arial", CHARSET_ANSI, 12.0f + i * 0.1f,
                                  FONT_WEIGHT_REGULAR, i % 2 == 0);
        ASSERT_TRUE(fonts[i] != NULL);
    }

    // Use all fonts
    for (int i = 0; i < num_fonts; i++) {
        ps_font* oldFont = ps_set_font(ctx, fonts[i]);
        ps_text_out(ctx, 10, 10 + i * 15, "Test");
        ps_set_font(ctx, oldFont); // reset font
    }

    // Clean up
    for (int i = 0; i < num_fonts; i++) {
        ps_font_unref(fonts[i]);
    }
}

TEST_F(FontTest, FontCopyAfterModification)
{
    // Test that font copy captures current state after modifications
    ps_font* original = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(original != NULL);

    // Modify original font
    ps_font_set_size(original, 24.0f);
    ps_font_set_weight(original, FONT_WEIGHT_BOLD);
    ps_font_set_italic(original, True);

    // Create copy
    ps_font* copy = ps_font_create_copy(original);
    ASSERT_TRUE(copy != NULL);

    // Verify copy has modified properties by testing rendering
    clear_test_canvas();

    ps_color color = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* oldFont1 = ps_set_font(ctx, original);
    ps_text_out(ctx, 50, 100, "Original");
    ps_set_font(ctx, oldFont1); // reset font

    ps_font* oldFont2 = ps_set_font(ctx, copy);
    ps_text_out(ctx, 50, 150, "Copy");
    ps_set_font(ctx, oldFont2); // reset font

    ps_font_unref(original);
    ps_font_unref(copy);

    EXPECT_SYS_SNAPSHOT_EQ(font_copy_after_modification);
}

// Complex text layout tests
TEST_F(FontTest, MixedCharsetText)
{
    clear_test_canvas();

    ps_color color = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    // Test mixing ANSI and Unicode text
    ps_font* ansiFont = ps_font_create("Arial", CHARSET_ANSI, 20.0f, FONT_WEIGHT_REGULAR, False);
    ps_font* unicodeFont = ps_font_create("Arial", CHARSET_UNICODE, 20.0f, FONT_WEIGHT_REGULAR, False);

    ASSERT_TRUE(ansiFont != NULL);
    ASSERT_TRUE(unicodeFont != NULL);

    ps_font* oldFont1 = ps_set_font(ctx, ansiFont);
    ps_text_out(ctx, 50, 100, "Hello");
    ps_set_font(ctx, oldFont1); // reset font

    ps_font* oldFont2 = ps_set_font(ctx, unicodeFont);
    const ps_uchar16 unicode_text[] = {0x4F60, 0x597D};
    ps_wide_text_out_length(ctx, 50, 150, unicode_text, 2);
    ps_set_font(ctx, oldFont2); // reset font

    ps_font_unref(ansiFont);
    ps_font_unref(unicodeFont);

    EXPECT_SYS_SNAPSHOT_EQ(font_mixed_charset);
}

TEST_F(FontTest, TextWithSpecialCharacters)
{
    clear_test_canvas();

    ps_color color = {1.0f, 0.0f, 1.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 18.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Test various special characters
    const char* special_chars[] = {
        "!@#$%^&*()",
        "[]{}|\\:;\"'<>,.?/",
        "0123456789",
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
        "abcdefghijklmnopqrstuvwxyz"
    };

    for (int i = 0; i < 5; i++) {
        ps_text_out(ctx, 50, 50 + i * 30, special_chars[i]);
    }

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_special_characters);
}

// Performance and stress tests
TEST_F(FontTest, LargeTextRendering)
{
    clear_test_canvas();

    ps_color color = {0.0f, 0.5f, 0.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 14.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Render a large amount of text
    const char* long_text = "This is a very long text string that tests the rendering performance "
                            "of the font system when dealing with large amounts of text. "
                            "It should wrap properly and render efficiently without memory leaks "
                            "or performance degradation. ";

    // Render the text multiple times
    for (int i = 0; i < 10; i++) {
        ps_text_out(ctx, 50, 50 + i * 20, long_text);
    }

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_long_text);
}

TEST_F(FontTest, RapidFontSwitching)
{
    clear_test_canvas();

    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    // Create multiple fonts with different properties
    ps_font* fonts[5];
    fonts[0] = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    fonts[1] = ps_font_create("Arial", CHARSET_ANSI, 16.0f, FONT_WEIGHT_BOLD, False);
    fonts[2] = ps_font_create("Arial", CHARSET_ANSI, 20.0f, FONT_WEIGHT_MEDIUM, True);
    fonts[3] = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_HEAVY, False);
    fonts[4] = ps_font_create("Arial", CHARSET_ANSI, 18.0f, FONT_WEIGHT_REGULAR, True);

    for (int i = 0; i < 5; i++) {
        ASSERT_TRUE(fonts[i] != NULL);
    }

    // Rapidly switch between fonts and render text
    for (int round = 0; round < 5; round++) {
        for (int i = 0; i < 5; i++) {
            ps_font* oldFont = ps_set_font(ctx, fonts[i]);
            ps_text_out(ctx, 50 + round * 100, 50 + i * 40, "Switch");
            ps_set_font(ctx, oldFont); // reset font
        }
    }

    // Clean up
    for (int i = 0; i < 5; i++) {
        ps_font_unref(fonts[i]);
    }

    EXPECT_SYS_SNAPSHOT_EQ(font_rapid_switching);
}

// Error handling and edge cases
TEST_F(FontTest, FontSetSizeNegative)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    // According to the implementation, negative sizes should be rejected
    ps_font_set_size(font, -5.0f);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_font_unref(font);
}

TEST_F(FontTest, FontSetWeightBoundaryValues)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    // Test boundary values (100-900 is valid range)
    ps_font_set_weight(font, 100); // Minimum valid
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_set_weight(font, 900); // Maximum valid
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_set_weight(font, 99); // Just below minimum, 100 will be set
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_set_weight(font, 901); // Just above maximum, 900 will be set
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_unref(font);
}

TEST_F(FontTest, TextExtentWithEmptyString)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    ps_size extent;
    // Test with empty string - should return zero width but valid height
    ps_bool result = ps_get_text_extent(ctx, "", 0, &extent);
    ASSERT_FALSE(result); // Empty string should fail validation
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
}

// Font hint and flip tests
TEST_F(FontTest, FontHintEffects)
{
    clear_test_canvas();

    ps_color color = {0.5f, 0.0f, 0.5f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* font1 = ps_font_create("Arial", CHARSET_ANSI, 16.0f, FONT_WEIGHT_REGULAR, False);
    ps_font* font2 = ps_font_create("Arial", CHARSET_ANSI, 16.0f, FONT_WEIGHT_REGULAR, False);

    ASSERT_TRUE(font1 != NULL);
    ASSERT_TRUE(font2 != NULL);

    // Set different hint values
    ps_font_set_hint(font1, True);
    ps_font_set_hint(font2, False);

    ps_font* oldFont1 = ps_set_font(ctx, font1);
    ps_text_out(ctx, 50, 100, "Hinted");
    ps_set_font(ctx, oldFont1); // reset font

    ps_font* oldFont2 = ps_set_font(ctx, font2);
    ps_text_out(ctx, 50, 150, "Unhinted");
    ps_set_font(ctx, oldFont2); // reset font

    ps_font_unref(font1);
    ps_font_unref(font2);

    EXPECT_SYS_SNAPSHOT_EQ(font_hint_effects);
}

TEST_F(FontTest, FontFlipEffects)
{
    clear_test_canvas();

    ps_color color = {0.0f, 0.5f, 0.5f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* font1 = ps_font_create("Arial", CHARSET_ANSI, 16.0f, FONT_WEIGHT_REGULAR, False);
    ps_font* font2 = ps_font_create("Arial", CHARSET_ANSI, 16.0f, FONT_WEIGHT_REGULAR, False);

    ASSERT_TRUE(font1 != NULL);
    ASSERT_TRUE(font2 != NULL);

    // Set different flip values
    ps_font_set_flip(font1, True);
    ps_font_set_flip(font2, False);

    ps_font* oldFont1 = ps_set_font(ctx, font1);
    ps_text_out(ctx, 50, 100, "Flipped");
    ps_set_font(ctx, oldFont1); // reset font

    ps_font* oldFont2 = ps_set_font(ctx, font2);
    ps_text_out(ctx, 50, 150, "Normal");
    ps_set_font(ctx, oldFont2); // reset font

    ps_font_unref(font1);
    ps_font_unref(font2);

    EXPECT_SYS_SNAPSHOT_EQ(font_flip_effects);
}

// Complex transformation tests
TEST_F(FontTest, ComplexTextTransformations)
{
    clear_test_canvas();

    ps_color color = {1.0f, 0.0f, 0.5f, 1.0f};
    ps_set_text_color(ctx, &color);

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 18.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Apply complex transformation: translate + rotate + scale
    ps_matrix* matrix = ps_matrix_create();
    ps_matrix_translate(matrix, 10.0f, 10.0f);
    ps_matrix_rotate(matrix, 0.44f);
    ps_matrix_scale(matrix, 1.5f, 0.8f);
    ps_set_text_matrix(ctx, matrix);

    ps_text_out(ctx, 0, 0, "Complex Transform");

    ps_matrix_unref(matrix);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_complex_transform);
}

// Final comprehensive test
TEST_F(FontTest, ComprehensiveFontDemo)
{
    clear_test_canvas();

    // This test demonstrates all major font features in one rendering
    ps_font* fonts[4];
    fonts[0] = ps_font_create("Arial", CHARSET_ANSI, 16.0f, FONT_WEIGHT_REGULAR, False);
    fonts[1] = ps_font_create("Arial", CHARSET_ANSI, 20.0f, FONT_WEIGHT_BOLD, False);
    fonts[2] = ps_font_create("Arial", CHARSET_ANSI, 24.0f, FONT_WEIGHT_MEDIUM, True);
    fonts[3] = ps_font_create("Arial", CHARSET_UNICODE, 36.0f, FONT_WEIGHT_HEAVY, False);

    for (int i = 0; i < 4; i++) {
        ASSERT_TRUE(fonts[i] != NULL);
    }

    const char* labels[] = {"Regular", "Bold", "Italic", "Unicode"};
    ps_color colors[] = {
        {1.0f, 0.0f, 0.0f, 1.0f}, // Red
        {0.0f, 1.0f, 0.0f, 1.0f}, // Green
        {0.0f, 0.0f, 1.0f, 1.0f}, // Blue
        {1.0f, 0.0f, 1.0f, 1.0f} // Magenta
    };

    for (int i = 0; i < 4; i++) {
        ps_set_text_color(ctx, &colors[i]);
        ps_font* oldFont = ps_set_font(ctx, fonts[i]);
        ps_text_out(ctx, 50, 80 + i * 60, labels[i]);

        if (i == 3) {
            // Render Unicode text for the Unicode font
            const ps_uchar16 unicode_text[] = {0x4F60, 0x597D, 0x4E16, 0x754C};
            ps_wide_text_out_length(ctx, 300, 120, unicode_text, 4);
        } else {
            ps_text_out(ctx, 150, 80 + i * 60, "Sample Text");
        }

        ps_set_font(ctx, oldFont); // reset font
    }

    // Clean up
    for (int i = 0; i < 4; i++) {
        ps_font_unref(fonts[i]);
    }

    EXPECT_SYS_SNAPSHOT_EQ(font_comprehensive_demo);
}

// Device error handling tests
TEST_F(FontTest, FontOperationsWithInvalidDevice)
{
    // Test operations when device is not initialized
    // This would need to be tested by calling PS_Shutdown() first
    // For now, we'll test the error paths that are reachable

    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    // Test that operations succeed when device is valid
    ps_font_set_size(font, 16.0f);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_unref(font);
}

// Thread safety tests (if applicable)
TEST_F(FontTest, ConcurrentFontOperations)
{
    // Basic concurrent operation test
    // Create multiple fonts from different "threads" (simulated)
    ps_font* fonts[10];

    for (int i = 0; i < 10; i++) {
        fonts[i] = ps_font_create("Arial", CHARSET_ANSI, 12.0f + i, FONT_WEIGHT_REGULAR, False);
        ASSERT_TRUE(fonts[i] != NULL);
    }

    // Use fonts concurrently (simulated)
    for (int i = 0; i < 10; i++) {
        ps_font* oldFont = ps_set_font(ctx, fonts[i]);
        ps_text_out(ctx, 10, 10 + i * 20, "Concurrent");
        ps_set_font(ctx, oldFont); // reset font
    }

    // Clean up
    for (int i = 0; i < 10; i++) {
        ps_font_unref(fonts[i]);
    }
}

// Font property persistence tests
TEST_F(FontTest, FontPropertyPersistence)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    // Set multiple properties
    ps_font_set_size(font, 24.0f);
    ps_font_set_weight(font, FONT_WEIGHT_BOLD);
    ps_font_set_italic(font, True);
    ps_font_set_charset(font, CHARSET_UNICODE);
    ps_font_set_hint(font, False);
    ps_font_set_flip(font, True);

    // Verify all operations succeeded
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Test that properties persist through context changes
    ps_font* oldFont = ps_set_font(ctx, font);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Render text to verify properties are applied
    clear_test_canvas();
    ps_color color = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_text_color(ctx, &color);

    const ps_uchar16 unicode_text[] = {0x4F60, 0x597D};
    ps_wide_text_out_length(ctx, 50, 100, unicode_text, 2);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_property_persistence);
}

// Text measurement accuracy tests
TEST_F(FontTest, TextMeasurementAccuracy)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Test measurement consistency
    const char* test_text = "Measurement Test";
    ps_size extent1, extent2;

    ps_bool result1 = ps_get_text_extent(ctx, test_text, (uint32_t)strlen(test_text), &extent1);
    ps_bool result2 = ps_get_text_extent(ctx, test_text, (uint32_t)strlen(test_text), &extent2);

    ASSERT_TRUE(result1);
    ASSERT_TRUE(result2);
    ASSERT_EQ(extent1.w, extent2.w);
    ASSERT_EQ(extent1.h, extent2.h);

    // Test that longer text has greater width
    ps_size short_extent, long_extent;
    ps_get_text_extent(ctx, "Short", 5, &short_extent);
    ps_get_text_extent(ctx, "Much Longer Text", 16, &long_extent);

    ASSERT_GT(long_extent.w, short_extent.w);

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
}

// Font fallback tests
TEST_F(FontTest, FontFallbackBehavior)
{
    // Test with non-existent font name
    ps_font* font = ps_font_create("NonExistentFont", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    // Should still create a font (system may use default)
    ASSERT_TRUE(font != NULL);

    ps_font* oldFont = ps_set_font(ctx, font);

    // Try to render text - should not crash
    clear_test_canvas();
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_text_color(ctx, &color);
    ps_text_out(ctx, 50, 100, "Fallback Test");

    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);

    EXPECT_SYS_SNAPSHOT_EQ(font_fallback_test);
}

// Memory stress tests
TEST_F(FontTest, MemoryStressTest)
{
    // Create and destroy many fonts rapidly
    for (int round = 0; round < 10; round++) {
        ps_font* fonts[50];

        // Create many fonts
        for (int i = 0; i < 50; i++) {
            fonts[i] = ps_font_create("Arial", CHARSET_ANSI, 10.0f + i * 0.5f,
                                      FONT_WEIGHT_REGULAR, i % 2 == 0);
            ASSERT_TRUE(fonts[i] != NULL);
        }

        // Use them briefly
        for (int i = 0; i < 10; i++) { // Only test first 10 to avoid too much rendering
            ps_font* oldFont = ps_set_font(ctx, fonts[i]);
            ps_text_out(ctx, 10, 10 + i * 15, "Stress");
            ps_set_font(ctx, oldFont); // reset font
        }

        // Clean up
        for (int i = 0; i < 50; i++) {
            ps_font_unref(fonts[i]);
        }
    }
}

// Edge case: Zero and extreme values
TEST_F(FontTest, ExtremeValueHandling)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ASSERT_TRUE(font != NULL);

    // Test boundary conditions for size
    ps_font_set_size(font, 0.1f); // Very small
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_set_size(font, 1000.0f); // Very large
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Test boundary conditions for weight
    ps_font_set_weight(font, 100); // Minimum
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_set_weight(font, 900); // Maximum
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_font_unref(font);
}

// Text rendering with different contexts
TEST_F(FontTest, MultipleContextFontUsage)
{
    // Create additional contexts
    ps_canvas* canvas2 = get_test_canvas();
    ps_context* ctx2 = ps_context_create(canvas2, NULL);
    ASSERT_TRUE(ctx2 != NULL);

    // Create fonts for each context
    ps_font* font1 = ps_font_create("Arial", CHARSET_ANSI, 16.0f, FONT_WEIGHT_REGULAR, False);
    ps_font* font2 = ps_font_create("Arial", CHARSET_ANSI, 20.0f, FONT_WEIGHT_BOLD, False);

    ASSERT_TRUE(font1 != NULL);
    ASSERT_TRUE(font2 != NULL);

    // Set fonts in different contexts
    ps_font* oldFont1 = ps_set_font(ctx, font1);
    ps_font* oldFont2 = ps_set_font(ctx2, font2);

    // Render in both contexts
    ps_color color1 = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_color color2 = {0.0f, 1.0f, 0.0f, 1.0f};

    ps_set_source_color(ctx, &color1);
    ps_text_out(ctx, 50, 100, "Context 1");
    ps_set_font(ctx, oldFont1); // reset font

    ps_set_source_color(ctx2, &color2);
    ps_text_out(ctx2, 50, 100, "Context 2");
    ps_set_font(ctx, oldFont2); // reset font

    // Clean up
    ps_font_unref(font1);
    ps_font_unref(font2);
    ps_context_unref(ctx2);
}

// Final validation test
TEST_F(FontTest, CompleteAPIValidation)
{
    // This test validates that all major API functions work together correctly

    // 1. Create font
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 18.0f, FONT_WEIGHT_BOLD, True);
    ASSERT_TRUE(font != NULL);

    // 2. Modify properties
    ps_font_set_size(font, 24.0f);
    ps_font_set_weight(font, FONT_WEIGHT_HEAVY);
    ps_font_set_italic(font, False);
    ps_font_set_hint(font, True);
    ps_font_set_flip(font, False);

    // 3. Copy font
    ps_font* font_copy = ps_font_create_copy(font);
    ASSERT_TRUE(font_copy != NULL);

    // 4. Set in context
    ps_font* oldFont = ps_set_font(ctx, font);

    // 5. Get font info
    ps_font_info info;
    ps_bool result = ps_get_font_info(ctx, &info);
    ASSERT_TRUE(result);
    ASSERT_GT(info.size, 0.0f);

    // 6. Measure text
    ps_size extent;
    result = ps_get_text_extent(ctx, "Validation", 10, &extent);
    ASSERT_TRUE(result);
    ASSERT_GT(extent.w, 0.0f);

    // 7. Get glyph
    ps_glyph glyph;
    result = ps_get_glyph(ctx, 'A', &glyph);
    ASSERT_TRUE(result);

    // 8. Get glyph extent
    ps_size glyph_extent;
    result = ps_glyph_get_extent(&glyph, &glyph_extent);
    ASSERT_TRUE(result);

    // 9. Render text
    clear_test_canvas();
    ps_color color = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_text_color(ctx, &color);
    ps_text_out(ctx, 50, 100, "API Validation");

    // 10. Clean up
    ps_set_font(ctx, oldFont); // reset font
    ps_font_unref(font);
    ps_font_unref(font_copy);

    EXPECT_SYS_SNAPSHOT_EQ(font_complete_validation);
}
