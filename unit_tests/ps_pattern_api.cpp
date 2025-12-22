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
#include "pat565.h"

#include "picasso_objects.h"

class PatternTest : public ::testing::Test
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
        ctx = nullptr;
        img = nullptr;
        pattern = nullptr;
        matrix = nullptr;
    }

    void TearDown() override
    {
        cleanup();
    }

    void cleanup()
    {
        if (pattern) {
            ps_pattern_unref(pattern);
            pattern = nullptr;
        }
        if (img) {
            ps_image_unref(img);
            img = nullptr;
        }
        if (matrix) {
            ps_matrix_unref(matrix);
            matrix = nullptr;
        }
        if (ctx) {
            ps_context_unref(ctx);
            ctx = nullptr;
        }
    }

    void create_test_context()
    {
        ps_canvas* canvas = get_test_canvas();
        ctx = ps_context_create(canvas, nullptr);
        ASSERT_NE(nullptr, ctx);
    }

    void create_test_image()
    {
        img = ps_image_create_with_data(pat565.bits, COLOR_FORMAT_RGB565, pat565.width, pat565.height, pat565.pitch);
        ASSERT_NE(nullptr, img);
    }

    void create_test_matrix()
    {
        matrix = ps_matrix_create();
        ASSERT_NE(nullptr, matrix);
        ps_matrix_scale(matrix, 1.5f, 1.5f);
        ps_matrix_rotate(matrix, 1.34f);
    }

    ps_context* ctx;
    ps_image* img;
    ps_pattern* pattern;
    ps_matrix* matrix;
};

// Test pattern creation with valid parameters
TEST_F(PatternTest, CreateImagePatternValid)
{
    create_test_image();

    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ASSERT_NE(nullptr, pattern);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Test pattern creation with different wrap types
TEST_F(PatternTest, CreateImagePatternWrapTypes)
{
    create_test_image();

    // Test REPEAT wrap
    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ASSERT_NE(nullptr, pattern);
    cleanup();

    // Test REFLECT wrap
    pattern = ps_pattern_create_image(img, WRAP_TYPE_REFLECT, WRAP_TYPE_REFLECT, nullptr);
    ASSERT_EQ(nullptr, pattern);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Test pattern creation with transform matrix
TEST_F(PatternTest, CreateImagePatternWithTransform)
{
    create_test_image();
    create_test_matrix();

    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, matrix);
    ASSERT_NE(nullptr, pattern);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Test pattern creation with null image (bad case)
TEST_F(PatternTest, CreateImagePatternNullImage)
{
    pattern = ps_pattern_create_image(nullptr, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ASSERT_EQ(nullptr, pattern);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Test reference counting
TEST_F(PatternTest, ReferenceCounting)
{
    create_test_image();

    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ASSERT_NE(nullptr, pattern);

    // Test ref
    ps_pattern* refed = ps_pattern_ref(pattern);
    ASSERT_EQ(pattern, refed);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Test unref (should not destroy yet)
    ps_pattern_unref(pattern);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Test final unref
    ps_pattern_unref(refed);
    pattern = nullptr; // Avoid double unref in teardown
}

// Test pattern transform
TEST_F(PatternTest, PatternTransform)
{
    create_test_image();
    create_test_matrix();

    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ASSERT_NE(nullptr, pattern);

    ps_pattern_transform(pattern, matrix);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Test pattern transform with null pattern (bad case)
TEST_F(PatternTest, PatternTransformNullPattern)
{
    create_test_matrix();

    ps_pattern_transform(nullptr, matrix);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Test pattern transform with null matrix (bad case)
TEST_F(PatternTest, PatternTransformNullMatrix)
{
    create_test_image();
    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ASSERT_NE(nullptr, pattern);

    ps_pattern_transform(pattern, nullptr);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Test setting pattern as source for fill
TEST_F(PatternTest, SetSourcePattern)
{
    create_test_context();
    create_test_image();

    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ASSERT_NE(nullptr, pattern);

    ps_set_source_pattern(ctx, pattern);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Test setting pattern as source for stroke
TEST_F(PatternTest, SetStrokePattern)
{
    create_test_context();
    create_test_image();

    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ASSERT_NE(nullptr, pattern);

    ps_set_stroke_pattern(ctx, pattern);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Test setting null pattern as source (bad case)
TEST_F(PatternTest, SetSourcePatternNull)
{
    create_test_context();

    ps_set_source_pattern(ctx, nullptr);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Test setting null pattern as stroke (bad case)
TEST_F(PatternTest, SetStrokePatternNull)
{
    create_test_context();

    ps_set_stroke_pattern(ctx, nullptr);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Visual test: Pattern fill rendering
TEST_F(PatternTest, DrawPatternFill)
{
    clear_test_canvas();
    create_test_context();
    create_test_image();

    // Create pattern
    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ASSERT_NE(nullptr, pattern);

    // Set as fill source
    ps_set_source_pattern(ctx, pattern);

    // Draw rectangle filled with pattern
    ps_rect rc = { 50, 50, 200, 150 };
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    // Verify visual output
    EXPECT_SNAPSHOT_EQ(pattern_fill_test);
}

// Visual test: Pattern stroke rendering
TEST_F(PatternTest, DrawPatternStroke)
{
    clear_test_canvas();
    create_test_context();
    create_test_image();

    // Create pattern
    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ASSERT_NE(nullptr, pattern);

    // Set as stroke source
    ps_set_stroke_pattern(ctx, pattern);
    ps_set_line_width(ctx, 10.0f);

    // Draw rectangle with pattern stroke
    ps_rect rc = { 100, 100, 300, 200 };
    ps_rectangle(ctx, &rc);
    ps_stroke(ctx);

    // Verify visual output
    EXPECT_SNAPSHOT_EQ(pattern_stroke_test);
}

// Visual test: Pattern with transform
TEST_F(PatternTest, DrawPatternTransform)
{
    clear_test_canvas();
    create_test_context();
    create_test_image();
    create_test_matrix();

    // Create pattern with transform
    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, matrix);
    ASSERT_NE(nullptr, pattern);

    // Set as fill source
    ps_set_source_pattern(ctx, pattern);

    // Draw rectangle with transformed pattern
    ps_rect rc = { 50, 50, 400, 300 };
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    // Verify visual output
    EXPECT_SNAPSHOT_EQ(pattern_transform_test);
}

// Visual test: Pattern with reflect wrap
TEST_F(PatternTest, DrawPatternReflect)
{
    clear_test_canvas();
    create_test_context();
    create_test_image();

    // Create pattern with reflect wrap
    pattern = ps_pattern_create_image(img, WRAP_TYPE_REFLECT, WRAP_TYPE_REFLECT, nullptr);
    ASSERT_NE(nullptr, pattern);

    // Set as fill source
    ps_set_source_pattern(ctx, pattern);

    // Draw rectangle with reflected pattern
    ps_rect rc = { 50, 50, 400, 300 };
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    // Verify visual output
    EXPECT_SNAPSHOT_EQ(pattern_reflect_test);
}

// Test pattern with complex drawing
TEST_F(PatternTest, DrawPatternComplex)
{
    clear_test_canvas();
    create_test_context();
    create_test_image();

    // Create pattern
    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ASSERT_NE(nullptr, pattern);

    // Set as fill source
    ps_set_source_pattern(ctx, pattern);

    // Draw complex shape
    ps_point p;
    p.x = 100;
    p.y = 100;
    ps_move_to(ctx, &p);

    p.x = 300;
    p.y = 100;
    ps_line_to(ctx, &p);

    p.x = 350;
    p.y = 200;
    ps_line_to(ctx, &p);

    p.x = 250;
    p.y = 300;
    ps_line_to(ctx, &p);

    p.x = 150;
    p.y = 300;
    ps_line_to(ctx, &p);

    p.x = 50;
    p.y = 200;
    ps_line_to(ctx, &p);
    ps_close_path(ctx);
    ps_fill(ctx);

    // Verify visual output
    EXPECT_SNAPSHOT_EQ(pattern_complex_test);
}

// Test pattern lifecycle with multiple operations
TEST_F(PatternTest, PatternLifecycle)
{
    create_test_context();
    create_test_image();

    // Create multiple patterns
    ps_pattern* pattern1 = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ps_pattern* pattern2 = ps_pattern_create_image(img, WRAP_TYPE_REFLECT, WRAP_TYPE_REFLECT, nullptr);

    ASSERT_NE(nullptr, pattern1);
    ASSERT_NE(nullptr, pattern2);

    // Use patterns
    ps_set_source_pattern(ctx, pattern1);
    ps_set_stroke_pattern(ctx, pattern2);

    // Clean up
    ps_pattern_unref(pattern1);
    ps_pattern_unref(pattern2);
}

// Test error handling with invalid context
TEST_F(PatternTest, SetPatternInvalidContext)
{
    create_test_image();
    pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, nullptr);
    ASSERT_NE(nullptr, pattern);

    // Try to set pattern with null context
    ps_set_source_pattern(nullptr, pattern);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_set_stroke_pattern(nullptr, pattern);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}
