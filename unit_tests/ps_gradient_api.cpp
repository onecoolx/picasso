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

class GradientTest : public ::testing::Test
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
        clear_test_canvas();
        ctx = ps_context_create(get_test_canvas(), nullptr);
    }

    void TearDown() override
    {
        if (ctx) {
            ps_context_unref(ctx);
            ctx = nullptr;
        }
    }
    ps_context* ctx;
};

// Test gradient creation functions
TEST_F(GradientTest, CreateLinear)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};

    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_NE(gradient, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    ps_gradient_unref(gradient);
}

TEST_F(GradientTest, CreateRadial)
{
    ps_point start = {50, 50};
    ps_point end = {100, 100};

    ps_gradient* gradient = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &start, 10, &end, 50);
    ASSERT_NE(gradient, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    ps_gradient_unref(gradient);
}

TEST_F(GradientTest, CreateConic)
{
    ps_point origin = {50, 50};

    ps_gradient* gradient = ps_gradient_create_conic(GRADIENT_SPREAD_PAD, &origin, 0);
    ASSERT_NE(gradient, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    ps_gradient_unref(gradient);
}

// Test all spread modes
TEST_F(GradientTest, SpreadModes)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};

    ps_gradient* pad = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_NE(pad, nullptr);
    ps_gradient_unref(pad);

    ps_gradient* repeat = ps_gradient_create_linear(GRADIENT_SPREAD_REPEAT, &start, &end);
    ASSERT_NE(repeat, nullptr);
    ps_gradient_unref(repeat);

    ps_gradient* reflect = ps_gradient_create_linear(GRADIENT_SPREAD_REFLECT, &start, &end);
    ASSERT_NE(reflect, nullptr);
    ps_gradient_unref(reflect);
}

// Test reference counting
TEST_F(GradientTest, ReferenceCounting)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};

    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_NE(gradient, nullptr);

    // Test ref increases reference count
    ps_gradient* ref = ps_gradient_ref(gradient);
    EXPECT_EQ(ref, gradient);

    // Test unref doesn't destroy yet
    ps_gradient_unref(gradient);

    // Second unref should destroy
    ps_gradient_unref(ref);
}

// Test color stops
TEST_F(GradientTest, ColorStops)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};

    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_NE(gradient, nullptr);

    // Add color stops
    ps_color red = {1, 0, 0, 1};
    ps_color blue = {0, 0, 1, 1};

    ps_gradient_add_color_stop(gradient, 0.0f, &red);
    ps_gradient_add_color_stop(gradient, 1.0f, &blue);

    // Clear color stops
    ps_gradient_clear_color_stops(gradient);

    ps_gradient_unref(gradient);
}

// Test transformation
TEST_F(GradientTest, Transform)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};

    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_NE(gradient, nullptr);

    ps_matrix* matrix = ps_matrix_create();
    ps_matrix_scale(matrix, 2, 2);

    ps_gradient_transform(gradient, matrix);

    ps_matrix_unref(matrix);
    ps_gradient_unref(gradient);
}

// Test bad cases
TEST_F(GradientTest, BadCases)
{
    // Null pointer tests
    EXPECT_EQ(ps_gradient_create_linear(GRADIENT_SPREAD_PAD, nullptr, nullptr), nullptr);
    EXPECT_EQ(ps_gradient_create_radial(GRADIENT_SPREAD_PAD, nullptr, 0, nullptr, 0), nullptr);
    EXPECT_EQ(ps_gradient_create_conic(GRADIENT_SPREAD_PAD, nullptr, 0), nullptr);

    // Null gradient operations
    ps_gradient_ref(nullptr);
    ps_gradient_unref(nullptr);
    ps_gradient_transform(nullptr, nullptr);
    ps_gradient_add_color_stop(nullptr, 0, nullptr);
    ps_gradient_clear_color_stops(nullptr);

    // Null matrix
    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ps_gradient_transform(gradient, nullptr);
    ps_gradient_unref(gradient);
}

// Test rendering with linear gradient
TEST_F(GradientTest, DrawLinearGradient)
{
    ps_point start = {0, 0};
    ps_point end = {200, 200};

    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_NE(gradient, nullptr);

    // Add color stops
    ps_color red = {1, 0, 0, 1};
    ps_color blue = {0, 0, 1, 1};
    ps_gradient_add_color_stop(gradient, 0.0f, &red);
    ps_gradient_add_color_stop(gradient, 1.0f, &blue);

    // Set gradient as source and draw rectangle
    ps_set_source_gradient(ctx, gradient);
    ps_rect rc = { 50, 50, 200, 200 };
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(gradient_linear_draw);

    ps_gradient_unref(gradient);
}

// Test rendering with radial gradient
TEST_F(GradientTest, DrawRadialGradient)
{
    ps_point start = {80, 80};
    ps_point end = {150, 150};

    ps_gradient* gradient = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &start, 10, &end, 100);
    ASSERT_NE(gradient, nullptr);

    // Add color stops
    ps_color white = {1, 1, 1, 1};
    ps_color black = {0, 0, 0, 1};
    ps_gradient_add_color_stop(gradient, 0.0f, &white);
    ps_gradient_add_color_stop(gradient, 1.0f, &black);

    // Set gradient as source and draw circle
    ps_set_source_gradient(ctx, gradient);
    ps_rect rc = { 50, 50, 200, 200 };
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(gradient_radial_draw);

    ps_gradient_unref(gradient);
}

// Test rendering with conic gradient
TEST_F(GradientTest, DrawConicGradient)
{
    ps_point origin = {125, 125};

    ps_gradient* gradient = ps_gradient_create_conic(GRADIENT_SPREAD_PAD, &origin, 0);
    ASSERT_NE(gradient, nullptr);

    // Add multiple color stops for rainbow effect
    ps_color red = {1, 0, 0, 1};
    ps_color yellow = {1, 1, 0, 1};
    ps_color green = {0, 1, 0, 1};
    ps_color cyan = {0, 1, 1, 1};
    ps_color blue = {0, 0, 1, 1};
    ps_color magenta = {1, 0, 1, 1};

    ps_gradient_add_color_stop(gradient, 0.0f, &red);
    ps_gradient_add_color_stop(gradient, 0.17f, &yellow);
    ps_gradient_add_color_stop(gradient, 0.33f, &green);
    ps_gradient_add_color_stop(gradient, 0.5f, &cyan);
    ps_gradient_add_color_stop(gradient, 0.67f, &blue);
    ps_gradient_add_color_stop(gradient, 0.83f, &magenta);
    ps_gradient_add_color_stop(gradient, 1.0f, &red);

    // Set gradient as source and draw circle
    ps_set_source_gradient(ctx, gradient);
    ps_rect rc = { 50, 50, 200, 200 };
    ps_ellipse(ctx, &rc);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(gradient_conic_draw);

    ps_gradient_unref(gradient);
}

// Test gradient with different spread modes
TEST_F(GradientTest, DrawSpreadModes)
{
    ps_point start = {100, 100};
    ps_point end = {200, 100};
    ps_rect rc = { 0, 50, 300, 50 };

    // Test PAD spread
    ps_gradient* pad = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ps_color red = {1, 0, 0, 1};
    ps_color blue = {0, 0, 1, 1};
    ps_gradient_add_color_stop(pad, 0.0f, &red);
    ps_gradient_add_color_stop(pad, 1.0f, &blue);

    ps_set_source_gradient(ctx, pad);
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(gradient_spread_pad);
    ps_gradient_unref(pad);

    clear_test_canvas();

    // Test REPEAT spread
    ps_gradient* repeat = ps_gradient_create_linear(GRADIENT_SPREAD_REPEAT, &start, &end);
    ps_gradient_add_color_stop(repeat, 0.0f, &red);
    ps_gradient_add_color_stop(repeat, 0.5f, &blue);
    ps_gradient_add_color_stop(repeat, 1.0f, &red);

    ps_set_source_gradient(ctx, repeat);
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(gradient_spread_repeat);
    ps_gradient_unref(repeat);

    clear_test_canvas();

    // Test REFLECT spread
    ps_gradient* reflect = ps_gradient_create_linear(GRADIENT_SPREAD_REFLECT, &start, &end);
    ps_gradient_add_color_stop(reflect, 0.0f, &red);
    ps_gradient_add_color_stop(reflect, 1.0f, &blue);

    ps_set_source_gradient(ctx, reflect);
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(gradient_spread_reflect);
    ps_gradient_unref(reflect);
}

TEST_F(GradientTest, DrawTransformedGradient)
{
    ps_point start = {0, 0};
    ps_point end = {100, 0};

    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_NE(gradient, nullptr);

    ps_color red = {1, 0, 0, 1};
    ps_color blue = {0, 0, 1, 1};
    ps_gradient_add_color_stop(gradient, 0.0f, &red);
    ps_gradient_add_color_stop(gradient, 1.0f, &blue);

    // Apply rotation transformation
    ps_matrix* matrix = ps_matrix_create();
    ps_matrix_translate(matrix, 100, 100);
    ps_matrix_translate(matrix, -50, -50);
    ps_matrix_rotate(matrix, (float)(45 * M_PI / 180.0f));
    ps_matrix_translate(matrix, 50, 50);
    ps_gradient_transform(gradient, matrix);

    ps_set_source_gradient(ctx, gradient);
    ps_rect rc = {100, 100, 100, 100};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(gradient_transformed);

    ps_matrix_unref(matrix);
    ps_gradient_unref(gradient);
}

TEST_F(GradientTest, RenderGradientWithAlpha)
{
    ps_point start = {0, 0};
    ps_point end = {200, 200};

    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_NE(gradient, nullptr);

    // Add color stops with alpha
    ps_color red = {1, 0, 0, 0.5f};
    ps_color blue = {0, 0, 1, 0.8f};
    ps_gradient_add_color_stop(gradient, 0.0f, &red);
    ps_gradient_add_color_stop(gradient, 1.0f, &blue);

    // Draw over existing content to test alpha blending
    ps_color green = {0, 1, 0, 1};
    ps_set_source_color(ctx, &green);
    ps_rect rc1 = {50, 50, 200, 200};
    ps_rectangle(ctx, &rc1);
    ps_fill(ctx);

    ps_set_source_gradient(ctx, gradient);
    ps_rect rc2 = {100, 100, 200, 200};
    ps_rectangle(ctx, &rc2);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(gradient_with_alpha);

    ps_gradient_unref(gradient);
}

// Test edge cases
TEST_F(GradientTest, EdgeCases)
{
    // Zero length gradient
    ps_point start = {100, 100};
    ps_point end = {100, 100};
    ps_gradient* zero = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ASSERT_NE(zero, nullptr);
    ps_gradient_unref(zero);

    // Zero radius radial gradient
    ps_gradient* zero_radius = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &start, 0, &end, 0);
    ASSERT_NE(zero_radius, nullptr);
    ps_gradient_unref(zero_radius);

    // Single color stop
    ps_point start2 = {0, 0};
    ps_point end2 = {100, 100};
    ps_gradient* single = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start2, &end2);
    ASSERT_NE(single, nullptr);

    ps_color red = {1, 0, 0, 1};
    ps_gradient_add_color_stop(single, 5.0f, &red);

    ps_set_source_gradient(ctx, single);
    ps_rect rc = {50, 50, 200, 200};
    ps_rectangle(ctx, &rc);
    ps_paint(ctx);

    EXPECT_SNAPSHOT_EQ(gradient_single_stop);
    ps_gradient_unref(single);

    // Invalid offset values
    ps_gradient* invalid = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start2, &end2);
    ASSERT_NE(invalid, nullptr);

    // These should be handled gracefully
    ps_gradient_add_color_stop(invalid, -0.5f, &red);
    ps_gradient_add_color_stop(invalid, 1.5f, &red);

    ps_gradient_unref(invalid);
}
