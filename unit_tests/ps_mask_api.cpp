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

class PsMaskTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        PS_Init();
        clear_test_canvas();
        canvas = get_test_canvas();
        context = ps_context_create(canvas, NULL);
    }

    void TearDown() override
    {
        if (context) {
            ps_context_unref(context);
        }
        PS_Shutdown();
    }

    ps_canvas* canvas = nullptr;
    ps_context* context = nullptr;
};

// Test ps_mask_create with valid dimensions
TEST_F(PsMaskTest, CreateValidMask)
{
    ps_mask* mask = ps_mask_create(100, 100);
    ASSERT_NE(mask, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Verify mask is properly initialized
    EXPECT_GT(mask->refcount, 0);
    ps_mask_unref(mask);
}

// Test ps_mask_create with invalid dimensions
TEST_F(PsMaskTest, CreateInvalidMask)
{
    // Test zero width
    ps_mask* mask = ps_mask_create(0, 100);
    EXPECT_EQ(mask, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    // Test zero height
    mask = ps_mask_create(100, 0);
    EXPECT_EQ(mask, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    // Test negative dimensions
    mask = ps_mask_create(-10, 100);
    EXPECT_EQ(mask, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

// Test ps_mask_create_with_data with valid data
TEST_F(PsMaskTest, CreateMaskWithData)
{
    const int width = 50;
    const int height = 50;
    std::vector<ps_byte> data(width * height, 128); // Gray mask

    ps_mask* mask = ps_mask_create_with_data(data.data(), width, height);
    ASSERT_NE(mask, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
    EXPECT_EQ(mask->flage, buffer_alloc_none);

    ps_mask_unref(mask);
}

// Test ps_mask_create_with_data with invalid parameters
TEST_F(PsMaskTest, CreateMaskWithInvalidData)
{
    // Test null data
    ps_mask* mask = ps_mask_create_with_data(nullptr, 100, 100);
    EXPECT_EQ(mask, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    // Test invalid dimensions with valid data
    std::vector<ps_byte> data(100, 128);
    mask = ps_mask_create_with_data(data.data(), 0, 100);
    EXPECT_EQ(mask, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

// Test reference counting
TEST_F(PsMaskTest, ReferenceCounting)
{
    ps_mask* mask = ps_mask_create(100, 100);
    ASSERT_NE(mask, nullptr);

    // Test ref
    ps_mask* ref_mask = ps_mask_ref(mask);
    EXPECT_EQ(ref_mask, mask);
    EXPECT_EQ(mask->refcount, 2);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Test unref once
    ps_mask_unref(mask);
    EXPECT_EQ(mask->refcount, 1);

    // Test unref again (should free)
    ps_mask_unref(mask);
}

// Test ps_mask_ref with null pointer
TEST_F(PsMaskTest, RefNullMask)
{
    ps_mask* result = ps_mask_ref(nullptr);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

// Test ps_mask_unref with null pointer
TEST_F(PsMaskTest, UnrefNullMask)
{
    ps_mask_unref(nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

// Test color filter functionality
TEST_F(PsMaskTest, ColorFilters)
{
    ps_mask* mask = ps_mask_create(100, 100);
    ASSERT_NE(mask, nullptr);

    // Add color filter
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_mask_add_color_filter(mask, &red);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Add another color filter
    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_mask_add_color_filter(mask, &blue);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Clear color filters
    ps_mask_clear_color_filters(mask);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    ps_mask_unref(mask);
}

// Test color filter with null parameters
TEST_F(PsMaskTest, ColorFiltersNullParams)
{
    // Test null mask
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_mask_add_color_filter(nullptr, &red);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    // Test null color
    ps_mask* mask = ps_mask_create(100, 100);
    ASSERT_NE(mask, nullptr);
    ps_mask_add_color_filter(mask, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    // Test clear with null mask
    ps_mask_clear_color_filters(nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_mask_unref(mask);
}

// Visual test: Basic mask application
TEST_F(PsMaskTest, DrawBasicMask)
{
    // Create a simple mask
    std::vector<ps_byte> data(TEST_WIDTH * TEST_HEIGHT, 96); // Light gray mask
    ps_mask* mask = ps_mask_create_with_data(data.data(), TEST_WIDTH, TEST_HEIGHT);
    ASSERT_NE(mask, nullptr);

    // Apply mask to canvas
    ps_canvas_set_mask(canvas, mask);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Draw a rectangle
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(context, &red);
    ps_rect rc = { 100, 100, 200, 200 };
    ps_rectangle(context, &rc);
    ps_fill(context);

    // Compare with snapshot
    EXPECT_SNAPSHOT_EQ(mask_basic_test);

    // Reset mask
    ps_canvas_reset_mask(canvas);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    ps_mask_unref(mask);
}

// Visual test: Mask with color filter
TEST_F(PsMaskTest, DrawMaskWithColorFilter)
{
    // Create mask
    ps_mask* mask = ps_mask_create(TEST_WIDTH, TEST_HEIGHT);
    ASSERT_NE(mask, nullptr);

    // Add color filter for red only
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_mask_add_color_filter(mask, &red);

    // Apply mask
    ps_canvas_set_mask(canvas, mask);

    // Draw multiple colored rectangles
    ps_color colors[] = {
        {1.0f, 0.0f, 0.0f, 1.0f}, // Red
        {0.0f, 1.0f, 0.0f, 1.0f}, // Green
        {0.0f, 0.0f, 1.0f, 1.0f}, // Blue
    };

    for (int i = 0; i < 3; i++) {
        ps_set_source_color(context, &colors[i]);
        ps_rect rc = { 50.0f + i * 150.0f, 150.0f, 100.0f, 100.0f };
        ps_rectangle(context, &rc);
        ps_fill(context);
    }

    // Compare with snapshot
    EXPECT_SNAPSHOT_EQ(mask_color_filter_test);

    // Cleanup
    ps_canvas_reset_mask(canvas);
    ps_mask_unref(mask);
}

// Visual test: Mask with custom data
TEST_F(PsMaskTest, DrawMaskWithCustomData)
{
    const int width = TEST_WIDTH;
    const int height = TEST_HEIGHT;

    // Create a gradient mask data
    std::vector<ps_byte> mask_data(width * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Create horizontal gradient
            mask_data[y * width + x] = (ps_byte)(255 * x / width);
        }
    }

    // Create mask with custom data
    ps_mask* mask = ps_mask_create_with_data(mask_data.data(), width, height);
    ASSERT_NE(mask, nullptr);

    // Apply mask and draw
    ps_canvas_set_mask(canvas, mask);

    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(context, &blue);
    ps_rect rc = { 0, 0, TEST_WIDTH, TEST_HEIGHT };
    ps_rectangle(context, &rc);
    ps_fill(context);

    // Compare with snapshot
    EXPECT_SNAPSHOT_EQ(mask_gradient_test);

    // Cleanup
    ps_canvas_reset_mask(canvas);
    ps_mask_unref(mask);
}
