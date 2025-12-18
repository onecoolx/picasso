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

class CanvasTest : public ::testing::Test
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
        canvas = nullptr;
        clear_test_canvas();
    }

    void TearDown() override
    {
        if (canvas) {
            ps_canvas_unref(canvas);
        }
    }

    ps_canvas* canvas;
};

// Test compatible canvas creation
TEST_F(CanvasTest, CanvasWithDataAndCompatible)
{
    uint8_t buf[200] = {0};
    ps_canvas* newCanvas = ps_canvas_create_with_data(buf, COLOR_FORMAT_RGB565, 10, 10, 20);
    ps_canvas* newCanvas2 = ps_canvas_create_compatible(newCanvas, 20, 20);

    ASSERT_EQ(ps_canvas_get_format(newCanvas), ps_canvas_get_format(newCanvas2));

    ps_size s;
    ps_bool r = ps_canvas_get_size(newCanvas2, &s);
    ASSERT_FLOAT_EQ(s.w, 20.0f);
    ASSERT_FLOAT_EQ(s.h, 20.0f);
    ASSERT_EQ(r, True);

    ps_canvas_unref(newCanvas);
    ps_canvas_unref(newCanvas2);

    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Test canvas creation with various formats
TEST_F(CanvasTest, CreateWithDifferentFormats)
{
    // Test all supported color formats
    ps_color_format formats[] = {
        COLOR_FORMAT_RGBA, COLOR_FORMAT_ARGB, COLOR_FORMAT_ABGR, COLOR_FORMAT_BGRA,
        COLOR_FORMAT_RGB, COLOR_FORMAT_BGR, COLOR_FORMAT_RGB565, COLOR_FORMAT_RGB555,
        COLOR_FORMAT_A8
    };

    for (auto format : formats) {
        canvas = ps_canvas_create(format, 100, 100);
        ASSERT_NE(nullptr, canvas) << "Failed to create canvas with format: " << format;
        EXPECT_EQ(format, ps_canvas_get_format(canvas));

        ps_size size;
        EXPECT_TRUE(ps_canvas_get_size(canvas, &size));
        EXPECT_FLOAT_EQ(100.0f, size.w);
        EXPECT_FLOAT_EQ(100.0f, size.h);

        ps_canvas_unref(canvas);
        canvas = nullptr;
    }

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(CanvasTest, CreateWithInvalidParameters)
{
    // Test null dimensions
    canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 0, 100);
    EXPECT_EQ(nullptr, canvas);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 0);
    EXPECT_EQ(nullptr, canvas);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    canvas = ps_canvas_create(COLOR_FORMAT_RGBA, -10, 100);
    EXPECT_EQ(nullptr, canvas);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());
}

// Test canvas creation with data
TEST_F(CanvasTest, CreateWithData)
{
    uint8_t buffer[100 * 100 * 4] = {0};
    canvas = ps_canvas_create_with_data(buffer, COLOR_FORMAT_RGBA, 100, 100, 100 * 4);
    ASSERT_NE(nullptr, canvas);
    EXPECT_EQ(COLOR_FORMAT_RGBA, ps_canvas_get_format(canvas));

    ps_size size;
    EXPECT_TRUE(ps_canvas_get_size(canvas, &size));
    EXPECT_FLOAT_EQ(100.0f, size.w);
    EXPECT_FLOAT_EQ(100.0f, size.h);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Test canvas creation with data - bad cases
TEST_F(CanvasTest, CreateWithDataBadCases)
{
    uint8_t buffer[100 * 100 * 4] = {0};

    // Test null buffer
    canvas = ps_canvas_create_with_data(nullptr, COLOR_FORMAT_RGBA, 100, 100, 100 * 4);
    EXPECT_EQ(nullptr, canvas);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test invalid dimensions
    canvas = ps_canvas_create_with_data(buffer, COLOR_FORMAT_RGBA, 0, 100, 100 * 4);
    EXPECT_EQ(nullptr, canvas);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test invalid pitch
    canvas = ps_canvas_create_with_data(buffer, COLOR_FORMAT_RGBA, 100, 100, 0);
    EXPECT_EQ(nullptr, canvas);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());
}

// Test compatible canvas creation - bad cases
TEST_F(CanvasTest, CreateCompatibleBadCases)
{
    // Test null source canvas
    canvas = ps_canvas_create_compatible(nullptr, 100, 100);
    EXPECT_EQ(nullptr, canvas);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test with invalid source (simulate device error)
    // This would need to be tested by mocking is_valid_system_device() to return false
}

// Test reference counting
TEST_F(CanvasTest, ReferenceCounting)
{
    canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_NE(nullptr, canvas);

    // Test ref
    ps_canvas* ref = ps_canvas_ref(canvas);
    EXPECT_EQ(canvas, ref);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    // Test unref (should not destroy yet)
    ps_canvas_unref(canvas);
    canvas = nullptr; // Avoid double unref in TearDown

    // Test final unref
    ps_canvas_unref(ref);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Test reference counting - bad cases
TEST_F(CanvasTest, ReferenceCountingBadCases)
{
    // Test ref null canvas
    ps_canvas* ref = ps_canvas_ref(nullptr);
    EXPECT_EQ(nullptr, ref);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test unref null canvas (should not crash)
    ps_canvas_unref(nullptr);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());
}

// Test canvas properties
TEST_F(CanvasTest, CanvasProperties)
{
    canvas = ps_canvas_create(COLOR_FORMAT_ARGB, 200, 150);
    ASSERT_NE(nullptr, canvas);

    // Test format
    EXPECT_EQ(COLOR_FORMAT_ARGB, ps_canvas_get_format(canvas));

    // Test size
    ps_size size;
    EXPECT_TRUE(ps_canvas_get_size(canvas, &size));
    EXPECT_FLOAT_EQ(200.0f, size.w);
    EXPECT_FLOAT_EQ(150.0f, size.h);

    // Test size with null parameter
    EXPECT_FALSE(ps_canvas_get_size(canvas, nullptr));
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Test canvas properties - bad cases
TEST_F(CanvasTest, CanvasPropertiesBadCases)
{
    // Test format with null canvas
    EXPECT_EQ(COLOR_FORMAT_UNKNOWN, ps_canvas_get_format(nullptr));
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test size with null canvas
    ps_size size;
    EXPECT_FALSE(ps_canvas_get_size(nullptr, &size));
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());
}

// Test canvas rendering and image comparison
TEST_F(CanvasTest, CanvasRendering)
{
    // Create context and draw something
    ps_context* ctx = ps_context_create(get_test_canvas(), nullptr);
    ASSERT_NE(nullptr, ctx);

    // Clear with red color
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &red);
    ps_clear(ctx);

    // Draw a rectangle
    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_rect rc = {10, 10, 80, 80};
    ps_rectangle(ctx, &rc);
    ps_set_source_color(ctx, &blue);
    ps_fill(ctx);

    ps_context_unref(ctx);

    // Compare with snapshot
    EXPECT_SNAPSHOT_EQ(canvas_simple_rect);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Test canvas mask operations
TEST_F(CanvasTest, CanvasMaskOperations)
{
    canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_NE(nullptr, canvas);

    // Create a mask
    ps_mask* mask = ps_mask_create(100, 100);
    ASSERT_NE(nullptr, mask);

    // Set mask
    ps_canvas_set_mask(canvas, mask);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    // Reset mask
    ps_canvas_reset_mask(canvas);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_mask_unref(mask);
}

// Test canvas mask operations - bad cases
TEST_F(CanvasTest, CanvasMaskOperationsBadCases)
{
    // Test set mask with null canvas
    ps_mask* mask = ps_mask_create(100, 100);
    ps_canvas_set_mask(nullptr, mask);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test set mask with null mask
    canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ps_canvas_set_mask(canvas, nullptr);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test reset mask with null canvas
    ps_canvas_reset_mask(nullptr);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    ps_mask_unref(mask);
}

// Test BitBlt operations
TEST_F(CanvasTest, CanvasBitBlt)
{
    // Create source canvas
    ps_canvas* source = ps_canvas_create(COLOR_FORMAT_RGBA, 50, 50);
    ASSERT_NE(nullptr, source);

    // Fill source with pattern
    ps_context* src_ctx = ps_context_create(source, nullptr);
    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(src_ctx, &blue);
    ps_clear(src_ctx);
    ps_context_unref(src_ctx);

    // Copy from source to destination
    ps_rect src_rc = {0, 0, 50, 50};
    ps_point dst_pos = {100, 100};

    ps_canvas_bitblt(source, &src_rc, get_test_canvas(), &dst_pos);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    // Compare result
    EXPECT_SNAPSHOT_EQ(canvas_bitblt_result);

    ps_canvas_unref(source);
}

// Test BitBlt with format mismatch
TEST_F(CanvasTest, BitBltFormatMismatch)
{
    ps_canvas* src = ps_canvas_create(COLOR_FORMAT_RGBA, 50, 50);
    ps_canvas* dst = ps_canvas_create(COLOR_FORMAT_RGB565, 100, 100);

    ps_rect src_rc = {0, 0, 50, 50};
    ps_point dst_pos = {0, 0};

    // Should fail due to format mismatch
    ps_canvas_bitblt(src, &src_rc, dst, &dst_pos);
    EXPECT_EQ(STATUS_MISMATCHING_FORMAT, ps_last_status());

    ps_canvas_unref(src);
    ps_canvas_unref(dst);
}

// Test BitBlt operations - bad cases
TEST_F(CanvasTest, CanvasBitBltBadCases)
{
    ps_canvas* source = ps_canvas_create(COLOR_FORMAT_RGBA, 50, 50);
    canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);

    ps_rect src_rc = {0, 0, 50, 50};
    ps_point dst_pos = {25, 25};

    // Test null destination canvas
    ps_canvas_bitblt(source, &src_rc, nullptr, &dst_pos);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test null source canvas
    ps_canvas_bitblt(nullptr, &src_rc, canvas, &dst_pos);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test copy size out of bounds
    ps_rect large_rc = {0, 0, 200, 200};
    ps_canvas_bitblt(source, &large_rc, canvas, &dst_pos);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_canvas_unref(source);
}

// Test canvas from canvas
TEST_F(CanvasTest, CreateFromCanvas)
{
    ps_canvas* source = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_NE(nullptr, source);

    // Create canvas from portion of source
    ps_rect rect = {10, 10, 50, 50};
    canvas = ps_canvas_create_from_canvas(source, &rect);
    ASSERT_NE(nullptr, canvas);

    EXPECT_EQ(COLOR_FORMAT_RGBA, ps_canvas_get_format(canvas));

    ps_size size;
    EXPECT_TRUE(ps_canvas_get_size(canvas, &size));
    EXPECT_FLOAT_EQ(50.0f, size.w);
    EXPECT_FLOAT_EQ(50.0f, size.h);

    ps_canvas_unref(source);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Test canvas from canvas - bad cases
TEST_F(CanvasTest, CreateFromCanvasBadCases)
{
    ps_canvas* source = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);

    // Test null source
    ps_rect rect = {10, 10, 50, 50};
    canvas = ps_canvas_create_from_canvas(nullptr, &rect);
    EXPECT_EQ(nullptr, canvas);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    ps_canvas_unref(source);
}

// Test canvas replace data
TEST_F(CanvasTest, ReplaceData)
{
    uint8_t buffer1[100 * 100 * 4] = {0};
    uint8_t buffer2[100 * 100 * 4] = {255};

    // Create canvas with data
    canvas = ps_canvas_create_with_data(buffer1, COLOR_FORMAT_RGBA, 100, 100, 100 * 4);
    ASSERT_NE(nullptr, canvas);

    // Replace data
    ps_canvas* result = ps_canvas_replace_data(canvas, buffer2, COLOR_FORMAT_RGBA, 100, 100, 100 * 4);
    EXPECT_EQ(canvas, result);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    // Verify format unchanged
    EXPECT_EQ(COLOR_FORMAT_RGBA, ps_canvas_get_format(canvas));
}

// Test canvas replace data - bad cases
TEST_F(CanvasTest, ReplaceDataBadCases)
{
    uint8_t buffer[100 * 100 * 4] = {0};

    // Test null canvas
    ps_canvas* result = ps_canvas_replace_data(nullptr, buffer, COLOR_FORMAT_RGBA, 100, 100, 100 * 4);
    EXPECT_EQ(nullptr, result);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test null buffer
    canvas = ps_canvas_create_with_data(buffer, COLOR_FORMAT_RGBA, 100, 100, 100 * 4);
    result = ps_canvas_replace_data(canvas, nullptr, COLOR_FORMAT_RGBA, 100, 100, 100 * 4);
    EXPECT_EQ(nullptr, result);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test invalid dimensions
    result = ps_canvas_replace_data(canvas, buffer, COLOR_FORMAT_RGBA, 0, 100, 100 * 4);
    EXPECT_EQ(nullptr, result);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test invalid pitch
    result = ps_canvas_replace_data(canvas, buffer, COLOR_FORMAT_RGBA, 100, 100, 0);
    EXPECT_EQ(nullptr, result);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test format mismatch
    result = ps_canvas_replace_data(canvas, buffer, COLOR_FORMAT_RGB565, 100, 100, 200);
    EXPECT_EQ(nullptr, result);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    // Test replace on non-data canvas (created without ps_canvas_create_with_data)
    ps_canvas* normal_canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    result = ps_canvas_replace_data(normal_canvas, buffer, COLOR_FORMAT_RGBA, 100, 100, 100 * 4);
    EXPECT_EQ(nullptr, result);
    EXPECT_NE(STATUS_SUCCEED, ps_last_status());

    ps_canvas_unref(normal_canvas);
}

// Test canvas from image with rect
TEST_F(CanvasTest, CreateFromImageWithRect)
{
    // Create a test image first
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 100, 100);
    if (img) {
        // Fill image with test pattern
        ps_canvas* img_canvas = ps_canvas_create_from_image(img, nullptr);
        ASSERT_NE(nullptr, img_canvas);
        ps_context* img_ctx = ps_context_create(img_canvas, nullptr);
        ASSERT_NE(nullptr, img_ctx);

        ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
        ps_set_source_color(img_ctx, &red);
        ps_clear(img_ctx);
        ps_context_unref(img_ctx);
        ps_canvas_unref(img_canvas);

        // Create canvas from portion of image
        ps_rect rect = {25, 25, 50, 50};
        canvas = ps_canvas_create_from_image(img, &rect);
        ASSERT_NE(nullptr, canvas);

        ps_size size;
        EXPECT_TRUE(ps_canvas_get_size(canvas, &size));
        EXPECT_FLOAT_EQ(50.0f, size.w);
        EXPECT_FLOAT_EQ(50.0f, size.h);

        ps_image_unref(img);
        EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
    }
}

// Test canvas from mask with rect
TEST_F(CanvasTest, CreateFromMaskWithRect)
{
    ps_mask* mask = ps_mask_create(200, 200);
    ASSERT_NE(nullptr, mask);

    // Create canvas from portion of mask
    ps_rect rect = {50, 50, 100, 100};
    canvas = ps_canvas_create_from_mask(mask, &rect);
    ASSERT_NE(nullptr, canvas);

    EXPECT_EQ(COLOR_FORMAT_A8, ps_canvas_get_format(canvas));

    ps_size size;
    EXPECT_TRUE(ps_canvas_get_size(canvas, &size));
    EXPECT_FLOAT_EQ(100.0f, size.w);
    EXPECT_FLOAT_EQ(100.0f, size.h);

    ps_mask_unref(mask);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Test canvas from mask with invalid rect
TEST_F(CanvasTest, CreateFromMaskWithInvalidRect)
{
    ps_mask* mask = ps_mask_create(100, 100);

    // Test rect out of bounds
    ps_rect large_rect = {50, 50, 100, 100};
    canvas = ps_canvas_create_from_mask(mask, &large_rect);
    // Should still create but clamp to mask bounds
    if (canvas) {
        ps_size size;
        ps_canvas_get_size(canvas, &size);
        EXPECT_LE(size.w, 50.0f);
        EXPECT_LE(size.h, 50.0f);
    }

    ps_mask_unref(mask);
}

// Test multiple reference counts
TEST_F(CanvasTest, MultipleReferenceCounts)
{
    canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_NE(nullptr, canvas);

    // Create multiple references
    ps_canvas* ref1 = ps_canvas_ref(canvas);
    ps_canvas* ref2 = ps_canvas_ref(canvas);
    ps_canvas* ref3 = ps_canvas_ref(canvas);

    EXPECT_EQ(canvas, ref1);
    EXPECT_EQ(canvas, ref2);
    EXPECT_EQ(canvas, ref3);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    // Unref multiple times
    ps_canvas_unref(canvas);
    ps_canvas_unref(ref1);
    ps_canvas_unref(ref2);
    canvas = nullptr; // Avoid double unref in TearDown

    // Should still be valid
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    // Final unref
    ps_canvas_unref(ref3);
}

// Test canvas edge cases
TEST_F(CanvasTest, EdgeCases)
{
    // Test 1x1 canvas
    canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 1, 1);
    ASSERT_NE(nullptr, canvas);

    ps_size size;
    EXPECT_TRUE(ps_canvas_get_size(canvas, &size));
    EXPECT_FLOAT_EQ(1.0f, size.w);
    EXPECT_FLOAT_EQ(1.0f, size.h);

    ps_canvas_unref(canvas);
    canvas = nullptr;

    // Test minimum pitch for different formats
    uint8_t buffer[4] = {0};
    canvas = ps_canvas_create_with_data(buffer, COLOR_FORMAT_RGBA, 1, 1, 4);
    ASSERT_NE(nullptr, canvas);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Test canvas thread safety (basic test)
TEST_F(CanvasTest, ThreadSafety)
{
    // Create multiple contexts for the same canvas
    ps_context* ctx1 = ps_context_create(get_test_canvas(), nullptr);
    ps_context* ctx2 = ps_context_create(get_test_canvas(), nullptr);

    ASSERT_NE(nullptr, ctx1);
    ASSERT_NE(nullptr, ctx2);

    // Draw with both contexts
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};

    ps_rect rc1 = { 10, 10, 100, 100 };
    ps_set_source_color(ctx1, &red);
    ps_rectangle(ctx1, &rc1);
    ps_fill(ctx1);

    ps_rect rc2 = { 150, 150, 50, 50 };
    ps_set_source_color(ctx2, &blue);
    ps_rectangle(ctx2, &rc2);
    ps_fill(ctx2);

    ps_context_unref(ctx1);
    ps_context_unref(ctx2);

    EXPECT_SNAPSHOT_EQ(multi_context_draw);

    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
}
