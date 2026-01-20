/*
 * Copyright (c) 2026, Zhang Ji Peng
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

PERF_TEST_DEFINE(Gradient);

// Test 1: Linear gradient creation and destruction
PERF_TEST(Gradient, LinearGradientCreateDestroy)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};

    for (int i = 0; i < 10000; i++) {
        ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
        ps_gradient_unref(gradient);
    }
}

// Test 2: Radial gradient creation and destruction
PERF_TEST(Gradient, RadialGradientCreateDestroy)
{
    ps_point start = {50, 50};
    ps_point end = {100, 100};

    for (int i = 0; i < 10000; i++) {
        ps_gradient* gradient = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &start, 25.0f, &end, 50.0f);
        ps_gradient_unref(gradient);
    }
}

// Test 3: Conic gradient creation and destruction
PERF_TEST(Gradient, ConicGradientCreateDestroy)
{
    ps_point origin = {50, 50};

    for (int i = 0; i < 10000; i++) {
        ps_gradient* gradient = ps_gradient_create_conic(GRADIENT_SPREAD_PAD, &origin, 0.0f);
        ps_gradient_unref(gradient);
    }
}

// Test 4: All spread modes performance
PERF_TEST_RUN(Gradient, AllSpreadModes)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_point origin = {50, 50};

    auto result = RunBenchmark(Gradient_AllSpreadModes, [&]() {
        for (int i = 0; i < 5000; i++) {
            // Test all spread modes for linear gradients
            ps_gradient* linear_pad = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
            ps_gradient* linear_repeat = ps_gradient_create_linear(GRADIENT_SPREAD_REPEAT, &start, &end);
            ps_gradient* linear_reflect = ps_gradient_create_linear(GRADIENT_SPREAD_REFLECT, &start, &end);

            // Test all spread modes for radial gradients
            ps_gradient* radial_pad = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &start, 25.0f, &end, 50.0f);
            ps_gradient* radial_repeat = ps_gradient_create_radial(GRADIENT_SPREAD_REPEAT, &start, 25.0f, &end, 50.0f);
            ps_gradient* radial_reflect = ps_gradient_create_radial(GRADIENT_SPREAD_REFLECT, &start, 25.0f, &end, 50.0f);

            // Test conic gradients (only PAD and REFLECT supported)
            ps_gradient* conic_pad = ps_gradient_create_conic(GRADIENT_SPREAD_PAD, &origin, 0.0f);
            ps_gradient* conic_reflect = ps_gradient_create_conic(GRADIENT_SPREAD_REFLECT, &origin, 0.0f);

            // Cleanup
            ps_gradient_unref(linear_pad);
            ps_gradient_unref(linear_repeat);
            ps_gradient_unref(linear_reflect);
            ps_gradient_unref(radial_pad);
            ps_gradient_unref(radial_repeat);
            ps_gradient_unref(radial_reflect);
            ps_gradient_unref(conic_pad);
            ps_gradient_unref(conic_reflect);
        }
    });

    CompareToBenchmark(Gradient_AllSpreadModes, result);
}

// Test 5: Gradient reference counting
PERF_TEST_RUN(Gradient, GradientReferenceOperations)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);

    auto result = RunBenchmark(Gradient_GradientReferenceOperations, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_gradient_ref(gradient);
            ps_gradient_unref(gradient);
        }
    });

    CompareToBenchmark(Gradient_GradientReferenceOperations, result);
    ps_gradient_unref(gradient);
}

// Test 6: Color stop operations
PERF_TEST_RUN(Gradient, ColorStopOperations)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);

    ps_color colors[] = {
        {1.0f, 0.0f, 0.0f, 1.0f}, // Red
        {0.0f, 1.0f, 0.0f, 1.0f}, // Green
        {0.0f, 0.0f, 1.0f, 1.0f}, // Blue
        {1.0f, 1.0f, 0.0f, 1.0f}, // Yellow
        {1.0f, 0.0f, 1.0f, 1.0f}, // Magenta
        {0.0f, 1.0f, 1.0f, 1.0f}, // Cyan
        {1.0f, 1.0f, 1.0f, 1.0f}, // White
        {0.0f, 0.0f, 0.0f, 1.0f} // Black
    };

    auto result = RunBenchmark(Gradient_ColorStopOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            // Add multiple color stops
            for (int j = 0; j < 8; j++) {
                ps_gradient_add_color_stop(gradient, j * 0.125f, &colors[j]);
            }
            // Clear all stops
            ps_gradient_clear_color_stops(gradient);
        }
    });

    CompareToBenchmark(Gradient_ColorStopOperations, result);
    ps_gradient_unref(gradient);
}

// Test 7: Complex gradient with many color stops
PERF_TEST_RUN(Gradient, ComplexColorStops)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);

    auto result = RunBenchmark(Gradient_ComplexColorStops, [&]() {
        for (int i = 0; i < 1000; i++) {
            // Create gradient with many color stops
            for (int j = 0; j < 100; j++) {
                ps_color color = {
                    (float)j / 100.0f, // Red gradient
                    1.0f - (float)j / 100.0f, // Green inverse gradient
                     0.5f, // Constant blue
                     1.0f // Full alpha
                };
                ps_gradient_add_color_stop(gradient, j * 0.01f, &color);
            }
            ps_gradient_clear_color_stops(gradient);
        }
    });

    CompareToBenchmark(Gradient_ComplexColorStops, result);
    ps_gradient_unref(gradient);
}

// Test 8: Gradient transformation operations
PERF_TEST_RUN(Gradient, GradientTransformOperations)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
    ps_matrix* matrix = ps_matrix_create();

    auto result = RunBenchmark(Gradient_GradientTransformOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            // Apply various transformations
            ps_matrix_identity(matrix);
            ps_matrix_translate(matrix, i * 0.1f, i * 0.1f);
            ps_matrix_rotate(matrix, i * 0.01f);
            ps_matrix_scale(matrix, 1.0f + i * 0.001f, 1.0f + i * 0.001f);
            ps_gradient_transform(gradient, matrix);
        }
    });

    CompareToBenchmark(Gradient_GradientTransformOperations, result);
    ps_gradient_unref(gradient);
    ps_matrix_unref(matrix);
}

// Test 9: Stress test with many gradients
PERF_TEST_RUN(Gradient, GradientStressTest)
{
    const int num_gradients = 1000;
    ps_gradient* gradients[num_gradients];
    ps_point start = {0, 0};
    ps_point end = {100, 100};

    auto result = RunBenchmark(Gradient_GradientStressTest, [&]() {
        // Create many gradients
        for (int i = 0; i < num_gradients; i++) {
            gradients[i] = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);

            // Add some color stops
            ps_color color = {(float)i / num_gradients, 0.5f, 1.0f - (float)i / num_gradients, 1.0f};
            ps_gradient_add_color_stop(gradients[i], 0.0f, &color);
            ps_gradient_add_color_stop(gradients[i], 1.0f, &color);
        }

        // Reference operations
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < num_gradients; j++) {
                ps_gradient_ref(gradients[j]);
            }
            for (int j = 0; j < num_gradients; j++) {
                ps_gradient_unref(gradients[j]);
            }
        }

        // Clean up
        for (int i = 0; i < num_gradients; i++) {
            ps_gradient_unref(gradients[i]);
        }
    });

    CompareToBenchmark(Gradient_GradientStressTest, result);
}

// Test 10: Different gradient types comparison
PERF_TEST_RUN(Gradient, GradientTypeComparison)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_point origin = {50, 50};

    auto result = RunBenchmark(Gradient_GradientTypeComparison, [&]() {
        for (int i = 0; i < 5000; i++) {
            // Create different types of gradients
            ps_gradient* linear = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);
            ps_gradient* radial = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &start, 25.0f, &end, 50.0f);
            ps_gradient* conic = ps_gradient_create_conic(GRADIENT_SPREAD_PAD, &origin, 0.0f);

            // Add color stops to each
            ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
            ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};

            ps_gradient_add_color_stop(linear, 0.0f, &red);
            ps_gradient_add_color_stop(linear, 1.0f, &blue);

            ps_gradient_add_color_stop(radial, 0.0f, &red);
            ps_gradient_add_color_stop(radial, 1.0f, &blue);

            ps_gradient_add_color_stop(conic, 0.0f, &red);
            ps_gradient_add_color_stop(conic, 1.0f, &blue);

            // Cleanup
            ps_gradient_unref(linear);
            ps_gradient_unref(radial);
            ps_gradient_unref(conic);
        }
    });

    CompareToBenchmark(Gradient_GradientTypeComparison, result);
}

// Test 11: Error handling with NULL parameters
PERF_TEST_RUN(Gradient, GradientErrorHandling)
{
    auto result = RunBenchmark(Gradient_GradientErrorHandling, [&]() {
        for (int i = 0; i < 10000; i++) {
            // NULL pointer tests
            ps_gradient_create_linear(GRADIENT_SPREAD_PAD, nullptr, nullptr);
            ps_gradient_create_radial(GRADIENT_SPREAD_PAD, nullptr, 0, nullptr, 0);
            ps_gradient_create_conic(GRADIENT_SPREAD_PAD, nullptr, 0);

            // NULL gradient operations
            ps_gradient_ref(nullptr);
            ps_gradient_unref(nullptr);
            ps_gradient_transform(nullptr, nullptr);
            ps_gradient_add_color_stop(nullptr, 0, nullptr);
            ps_gradient_clear_color_stops(nullptr);
        }
    });

    CompareToBenchmark(Gradient_GradientErrorHandling, result);
}

// Test 12: Gradient with extreme values
PERF_TEST_RUN(Gradient, GradientExtremeValues)
{
    auto result = RunBenchmark(Gradient_GradientExtremeValues, [&]() {
        for (int i = 0; i < 5000; i++) {
            // Test extreme coordinate values
            ps_point far_start = {-10000.0f, -10000.0f};
            ps_point far_end = {10000.0f, 10000.0f};
            ps_gradient* linear = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &far_start, &far_end);

            // Test extreme radius values
            ps_point center = {0.0f, 0.0f};
            ps_gradient* radial = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &center, 0.0f, &center, 10000.0f);

            // Test extreme angle values
            ps_gradient* conic = ps_gradient_create_conic(GRADIENT_SPREAD_PAD, &center, 3.14159f * 2.0f);

            // Test extreme offset values (should be clamped)
            ps_color color = {1.0f, 1.0f, 1.0f, 1.0f};
            ps_gradient_add_color_stop(linear, -1.0f, &color);
            ps_gradient_add_color_stop(linear, 2.0f, &color);

            // Cleanup
            ps_gradient_unref(linear);
            ps_gradient_unref(radial);
            ps_gradient_unref(conic);
        }
    });

    CompareToBenchmark(Gradient_GradientExtremeValues, result);
}

// Test 13: Gradient reuse performance
PERF_TEST_RUN(Gradient, GradientReusePerformance)
{
    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &start, &end);

    // Pre-populate with color stops
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    ps_gradient_add_color_stop(gradient, 0.0f, &red);
    ps_gradient_add_color_stop(gradient, 1.0f, &blue);

    auto result = RunBenchmark(Gradient_GradientReusePerformance, [&]() {
        for (int i = 0; i < 10000; i++) {
            // Clear and re-add color stops
            ps_gradient_clear_color_stops(gradient);
            ps_gradient_add_color_stop(gradient, 0.0f, &red);
            ps_gradient_add_color_stop(gradient, 0.5f, &blue);
            ps_gradient_add_color_stop(gradient, 1.0f, &red);
        }
    });

    CompareToBenchmark(Gradient_GradientReusePerformance, result);
    ps_gradient_unref(gradient);
}
