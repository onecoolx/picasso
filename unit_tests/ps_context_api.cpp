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

class ContextTest : public ::testing::Test
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
        canvas = get_test_canvas();
        ctx = ps_context_create(canvas, NULL);
    }

    void TearDown() override
    {
        ps_context_unref(ctx);
    }

    ps_canvas* canvas;
    ps_context* ctx;
};

TEST_F(ContextTest, CreateAndRef)
{
    ps_context* ctx2 = ps_context_create(canvas, NULL);
    ASSERT_TRUE(ctx2);
    ps_context_unref(ctx2);

    ps_context* ctx3 = ps_context_create(canvas, ctx);
    ASSERT_TRUE(ctx3);
    ps_context_unref(ctx3);

    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(ContextTest, CreateWithNullCanvas)
{
    ps_context* null_ctx = ps_context_create(NULL, NULL);
    ASSERT_EQ(NULL, null_ctx);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(ContextTest, ReferenceCounting)
{
    ps_context* ref_ctx = ps_context_ref(ctx);
    ASSERT_EQ(ctx, ref_ctx);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Multiple refs should work
    ps_context* ref_ctx2 = ps_context_ref(ctx);
    ASSERT_EQ(ctx, ref_ctx2);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Unref twice (once for each extra ref)
    ps_context_unref(ref_ctx);
    ps_context_unref(ref_ctx2);
}

TEST_F(ContextTest, UnrefNull)
{
    // Should not crash
    ps_context_unref(NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Canvas association tests

TEST_F(ContextTest, CanvasAssociation)
{
    ps_canvas* newCanvas = ps_canvas_create(COLOR_FORMAT_ARGB, 400, 300);
    ps_context_set_canvas(ctx, newCanvas);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_canvas* curCanvas = ps_context_get_canvas(ctx);
    ASSERT_EQ(curCanvas, newCanvas);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Test setting NULL canvas
    ps_canvas* oldCanvas = ps_context_set_canvas(ctx, NULL);
    ASSERT_EQ(nullptr, oldCanvas);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_canvas_unref(newCanvas);

    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(ContextTest, SetCanvasWithNullContext)
{
    ps_canvas* result = ps_context_set_canvas(NULL, canvas);
    ASSERT_EQ(NULL, result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(ContextTest, GetCanvasWithNullContext)
{
    ps_canvas* result = ps_context_get_canvas(NULL);
    ASSERT_EQ(NULL, result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(ContextTest, AntialiasAndGamma)
{
    // gamma
    float g = 1.5;
    float old = ps_set_gamma(ctx, g);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    old = ps_set_gamma(ctx, old);
    EXPECT_EQ(g, old);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_set_antialias(ctx, True);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    ps_set_antialias(ctx, False);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Test with null context
    ps_set_gamma(NULL, 1.0f);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_set_antialias(NULL, True);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Rendering and state tests
TEST_F(ContextTest, BasicRendering)
{
    clear_test_canvas();

    // Set up drawing state
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f}; // Red
    ps_set_source_color(ctx, &color);
    ps_set_line_width(ctx, 2.0f);

    // Draw simple rectangle

    ps_rect rc1 = {50, 50, 100, 100};
    ps_rectangle(ctx, &rc1);
    ps_fill(ctx);

    ps_rect rc2 = {200, 50, 100, 100};
    ps_rectangle(ctx, &rc2);
    ps_stroke(ctx);

    EXPECT_SNAPSHOT_EQ(context_basic_rectangle);
}

TEST_F(ContextTest, FillAndStroke)
{
    clear_test_canvas();

    ps_color fillColor = {0.0f, 1.0f, 0.0f, 1.0f}; // Green
    ps_color strokeColor = {0.0f, 0.0f, 1.0f, 1.0f}; // Blue
    ps_set_source_color(ctx, &fillColor);
    ps_set_line_width(ctx, 3.0f);

    // Draw filled circle with stroke
    ps_rect rc1 = {150, 150, 100, 60};
    ps_ellipse(ctx, &rc1);
    ps_fill(ctx);

    ps_set_source_color(ctx, &strokeColor);
    ps_rect rc2 = {300, 150, 100, 60};
    ps_ellipse(ctx, &rc2);
    ps_stroke(ctx);

    EXPECT_SNAPSHOT_EQ(context_fill_stroke_circle);
}

TEST_F(ContextTest, PathOperations)
{
    clear_test_canvas();

    ps_color color = {1.0f, 0.5f, 0.0f, 1.0f}; // Orange
    ps_set_source_color(ctx, &color);
    ps_set_line_width(ctx, 2.0f);

    // Create a complex path
    ps_point p;
    p.x = 50.0f;
    p.y = 50.0f;
    ps_move_to(ctx, &p);

    p.x = 150.0f;
    p.y = 50.0f;
    ps_line_to(ctx, &p);

    p.x = 150.0f;
    p.y = 150.0f;
    ps_line_to(ctx, &p);

    p.x = 50.0f;
    p.y = 150.0f;
    ps_line_to(ctx, &p);
    ps_close_path(ctx);

    // Add a bezier curve
    p.x = 200.0f;
    p.y = 100.0f;
    ps_move_to(ctx, &p);

    ps_point c1 = {250.0f, 50.0f};
    ps_point c2 = {300.0f, 150.0f};
    ps_point p2 = {350.0f, 100.0f};
    ps_bezier_curve_to(ctx, &c1, &c2, &p2);

    ps_stroke(ctx);

    EXPECT_SNAPSHOT_EQ(context_path_operations);
}

TEST_F(ContextTest, TransformOperations)
{
    clear_test_canvas();

    ps_color color = {1.0f, 0.0f, 1.0f, 1.0f}; // Magenta
    ps_set_source_color(ctx, &color);
    ps_set_line_width(ctx, 2.0f);

    // Test transformations
    ps_save(ctx);

    // Original rectangle
    ps_rect rc = { 20, 20, 100, 100 };
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    // Translated rectangle
    ps_translate(ctx, 150, 0);
    ps_rectangle(ctx, &rc);
    ps_stroke(ctx);

    // Rotated rectangle
    ps_translate(ctx, 150, 0);
    ps_rotate(ctx, 45.0f * 3.14159f / 180.0f);
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    ps_restore(ctx);

    // Scaled rectangle
    ps_save(ctx);
    ps_translate(ctx, 0, 280);
    ps_scale(ctx, 2.0f, 0.5f);
    ps_rectangle(ctx, &rc);
    ps_stroke(ctx);

    ps_restore(ctx);

    EXPECT_SNAPSHOT_EQ(context_transform_operations);
}

TEST_F(ContextTest, ClippingOperations)
{
    clear_test_canvas();

    // Set up clipping region
    ps_rect rc = { 50, 50, 100, 100 };
    ps_rect rc2 = {0, 0, 200, 200};
    ps_rectangle(ctx, &rc2);
    ps_stroke(ctx);

    ps_rectangle(ctx, &rc);
    ps_clip(ctx);

    // Draw something that will be clipped
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f}; // Red
    ps_set_source_color(ctx, &color);
    ps_rectangle(ctx, &rc2);
    ps_fill(ctx);

    // Reset clip and draw full rectangle
    ps_reset_clip(ctx);
    ps_color blueColor = {0.0f, 0.0f, 1.0f, 0.5f}; // Semi-transparent blue
    ps_set_source_color(ctx, &blueColor);
    ps_rect rc3 = {75, 75, 100, 100};
    ps_rectangle(ctx, &rc3);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(context_clipping_operations);
}

// State management tests
TEST_F(ContextTest, StateSaveRestore)
{
    clear_test_canvas();

    // Set initial state
    ps_color redColor = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &redColor);
    ps_set_line_width(ctx, 2.0f);

    // Save state
    ps_save(ctx);

    // Change state
    ps_color blueColor = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &blueColor);
    ps_set_line_width(ctx, 5.0f);

    // Draw with new state
    ps_rect rc = {20, 20, 60, 60};
    ps_rectangle(ctx, &rc);
    ps_paint(ctx);

    // Restore state
    ps_restore(ctx);

    // Draw with restored state
    ps_rect rc2 = {100, 100, 60, 60};
    ps_rectangle(ctx, &rc2);
    ps_paint(ctx);

    EXPECT_SNAPSHOT_EQ(context_state_save_restore);
}

// Bad case tests
TEST_F(ContextTest, BadCases)
{
    // Test operations with null context
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(NULL, &color);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_set_line_width(NULL, 2.0f);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_point p = {10, 10};
    ps_move_to(NULL, &p);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_line_to(NULL, &p);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_rect rc = {0, 0, 10, 10};
    ps_rectangle(NULL, &rc);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_stroke(NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_fill(NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_save(NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_restore(NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_reset_clip(NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

// Memory stress tests
TEST_F(ContextTest, MultipleContexts)
{
    const int numContexts = 10;
    ps_context* contexts[numContexts];

    // Create multiple contexts
    for (int i = 0; i < numContexts; i++) {
        contexts[i] = ps_context_create(canvas, NULL);
        ASSERT_TRUE(contexts[i]);
    }

    // Use all contexts
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    for (int i = 0; i < numContexts; i++) {
        ps_set_source_color(contexts[i], &color);
        ps_rect rc = {i * 10.0f, i * 10.0f, 50.0f, 50.0f};
        ps_rectangle(contexts[i], &rc);
        ps_fill(contexts[i]);
    }

    // Clean up
    for (int i = 0; i < numContexts; i++) {
        ps_context_unref(contexts[i]);
    }

    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(ContextTest, ContextWithSharedResources)
{
    // Create parent context
    ps_context* parentCtx = ps_context_create(canvas, NULL);
    ASSERT_TRUE(parentCtx);

    // Create child context with shared resources
    ps_context* childCtx = ps_context_create(canvas, parentCtx);
    ASSERT_TRUE(childCtx);

    // Both contexts should work
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(parentCtx, &color);
    ps_rect rc = {10, 10, 50, 50};
    ps_rectangle(parentCtx, &rc);
    ps_fill(parentCtx);

    ps_color blueColor = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(childCtx, &blueColor);
    ps_rect rc2 = {70, 70, 50, 50};
    ps_rectangle(childCtx, &rc2);
    ps_fill(childCtx);

    ps_context_unref(childCtx);
    ps_context_unref(parentCtx);

    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Visual regression tests
TEST_F(ContextTest, DrawRegressionText)
{
    clear_test_canvas();

    ps_color color = {0.0f, 0.0f, 1.0f, 1.0f}; // Blue
    ps_set_source_color(ctx, &color);

    ps_font* f = ps_font_create("Sans-Serif", CHARSET_ANSI, 24, FONT_WEIGHT_REGULAR, False);

    // Draw text (if font system is available)
    f = ps_set_font(ctx, f);
    ps_text_out_length(ctx, 50, 100, "Hello Picasso", 13);

    f = ps_set_font(ctx, f);
    ps_font_unref(f);

    EXPECT_SYS_SNAPSHOT_EQ(context_text_rendering);
}

TEST_F(ContextTest, DrawRegressionRects)
{
    clear_test_canvas();

    // Create a simple gradient effect using multiple rectangles
    for (int i = 0; i < 10; i++) {
        float intensity = i / 10.0f;
        ps_color color = {intensity, 0.0f, 1.0f - intensity, 1.0f};
        ps_set_source_color(ctx, &color);
        ps_rect rc = {i * 20.0f, 50.0f, 20.0f, 100.0f};
        ps_rectangle(ctx, &rc);
        ps_fill(ctx);
    }

    EXPECT_SNAPSHOT_EQ(context_rects_effect);
}

TEST_F(ContextTest, DrawComplexPath)
{
    clear_test_canvas();

    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_stroke_color(ctx, &color);
    ps_set_line_width(ctx, 2.0f);

    // Create a star shape
    const int numPoints = 5;
    const float outerRadius = 80.0f;
    const float innerRadius = 30.0f;
    const float centerX = 150.0f;
    const float centerY = 150.0f;

    for (int i = 0; i < numPoints * 2; i++) {
        float angle = i * 3.14159f / numPoints;
        float radius = (i % 2 == 0) ? outerRadius : innerRadius;
        float x = centerX + cos(angle) * radius;
        float y = centerY + sin(angle) * radius;

        ps_point p = {x, y};

        if (i == 0) {
            ps_move_to(ctx, &p);
        } else {
            ps_line_to(ctx, &p);
        }
    }
    ps_close_path(ctx);
    ps_stroke(ctx);

    EXPECT_SNAPSHOT_EQ(context_complex_star_path);
}

// Blur attribute tests
TEST_F(ContextTest, BlurBasicFunctionality)
{
    // Test default blur is 0
    float oldBlur = ps_set_blur(ctx, 0.5f);
    EXPECT_FLOAT_EQ(0.0f, oldBlur);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Test setting blur returns previous value
    oldBlur = ps_set_blur(ctx, 0.8f);
    EXPECT_FLOAT_EQ(0.5f, oldBlur);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(ContextTest, BlurRangeValidation)
{
    // Test lower bound clamping
    float oldBlur = ps_set_blur(ctx, -0.5f);
    EXPECT_FLOAT_EQ(0.0f, oldBlur); // Previous value from last test
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Test upper bound clamping
    oldBlur = ps_set_blur(ctx, 1.5f);
    EXPECT_FLOAT_EQ(0.0f, oldBlur); // Should be clamped to 0
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Test exact boundary values
    ps_set_blur(ctx, 0.0f);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_set_blur(ctx, 1.0f);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(ContextTest, BlurWithDrawingOperations)
{
    clear_test_canvas();

    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f}; // Red
    ps_set_source_color(ctx, &color);
    ps_set_blur(ctx, 0.5f);

    // Test blur with stroke
    ps_rect rc = {20, 20, 60, 60};
    ps_rectangle(ctx, &rc);
    ps_stroke(ctx);

    // Test blur with fill
    ps_set_blur(ctx, 0.8f);
    ps_rect rc1 = {100, 100, 60, 60};
    ps_rectangle(ctx, &rc1);
    ps_fill(ctx);

    // Test blur with paint
    ps_set_blur(ctx, 0.3f);
    ps_rect rc2 = {180, 20, 60, 60};
    ps_rectangle(ctx, &rc2);
    ps_paint(ctx);

    EXPECT_SNAPSHOT_EQ(context_blur_drawing_operations);
}

TEST_F(ContextTest, BlurWithShadows)
{
    clear_test_canvas();

    ps_color color = {0.0f, 1.0f, 0.0f, 1.0f}; // Green
    ps_set_source_color(ctx, &color);

    // Set shadow with blur
    ps_set_shadow(ctx, 5.0f, 5.0f, 0.5f);
    ps_color c = {0.0f, 0.0f, 0.0f, 0.5f};
    ps_set_shadow_color(ctx, &c); // Semi-transparent black

    // Set context blur as well
    ps_set_blur(ctx, 0.3f);

    ps_rect rc = {50, 50, 100, 100};
    ps_rectangle(ctx, &rc);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(context_blur_with_shadows);
}

TEST_F(ContextTest, BlurStateSaveRestore)
{
    clear_test_canvas();

    // Set initial blur
    ps_set_blur(ctx, 0.5f);

    // Save state
    ps_save(ctx);

    // Change blur
    ps_set_blur(ctx, 0.8f);

    // Draw with new blur
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_rect rc1 = { 20, 20, 80, 80 };
    ps_rectangle(ctx, &rc1);
    ps_fill(ctx);

    // Restore state
    ps_restore(ctx);

    // Draw with restored blur
    ps_color blueColor = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_set_source_color(ctx, &blueColor);
    ps_rect rc2 = { 120, 120, 80, 80 };
    ps_rectangle(ctx, &rc2);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(context_blur_state_save_restore);
}

TEST_F(ContextTest, BlurBadCases)
{
    // Test with null context
    float result = ps_set_blur(NULL, 0.5f);
    EXPECT_FLOAT_EQ(0.0f, result);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(ContextTest, BlurVisualRegression)
{
    clear_test_canvas();

    ps_color color = {1.0f, 0.5f, 0.0f, 1.0f}; // Orange
    ps_set_source_color(ctx, &color);

    // Create gradient blur effect
    for (int i = 0; i < 5; i++) {
        ps_set_blur(ctx, i * 0.2f);
        ps_rect rc = {20.0f + i * 50.0f, 50.0f, 25.0f, 80.0f};
        ps_rectangle(ctx, &rc);
        ps_fill(ctx);
    }

    EXPECT_SNAPSHOT_EQ(context_blur_gradient_effect);
}

TEST_F(ContextTest, BlurWithComplexPaths)
{
    clear_test_canvas();

    ps_color color = {0.5f, 0.0f, 1.0f, 1.0f}; // Purple
    ps_set_source_color(ctx, &color);
    ps_set_blur(ctx, 0.6f);

    ps_point startPt = {100.0f, 50.0f};
    ps_move_to(ctx, &startPt);

    ps_point fcp1 = {150.0f, 20.0f};
    ps_point scp1 = {200.0f, 80.0f};
    ps_point ep1 = {250.0f, 50.0f};
    ps_bezier_curve_to(ctx, &fcp1, &scp1, &ep1);

    ps_point fcp2 = {200.0f, 120.0f};
    ps_point scp2 = {150.0f, 80.0f};
    ps_point ep2 = {100.0f, 150.0f};
    ps_bezier_curve_to(ctx, &fcp2, &scp2, &ep2);

    ps_close_path(ctx);

    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(context_blur_complex_path);
}

TEST_F(ContextTest, BlurWithAntialias)
{
    clear_test_canvas();

    ps_color color = {0.0f, 0.8f, 0.8f, 1.0f}; // Cyan
    ps_set_source_color(ctx, &color);
    ps_set_blur(ctx, 0.4f);

    ps_set_antialias(ctx, True);
    ps_rect ellipseRect1 = {40.0f, 50.0f, 80.0f, 60.0f};
    ps_ellipse(ctx, &ellipseRect1);
    ps_fill(ctx);

    // Test with antialias off
    ps_set_antialias(ctx, False);
    ps_rect ellipseRect2 = {140.0f, 50.0f, 80.0f, 60.0f};
    ps_ellipse(ctx, &ellipseRect2);
    ps_fill(ctx);

    EXPECT_SNAPSHOT_EQ(context_blur_antialias);
}

