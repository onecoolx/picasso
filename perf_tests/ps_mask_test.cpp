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

PERF_TEST_DEFINE(Mask);

// Test 1: Basic mask creation and destruction
PERF_TEST(Mask, MaskCreateDestroy)
{
    for (int i = 0; i < 10000; i++) {
        ps_mask* mask = ps_mask_create(64, 64);
        ps_mask_unref(mask);
    }
}

// Test 2: Mask creation with different dimensions
PERF_TEST_RUN(Mask, MaskCreateDifferentDimensions)
{
    auto result = RunBenchmark(Mask_MaskCreateDifferentDimensions, [&]() {
        for (int i = 0; i < 2000; i++) {
            ps_mask* mask1 = ps_mask_create(1, 1);
            ps_mask* mask2 = ps_mask_create(8, 8);
            ps_mask* mask3 = ps_mask_create(32, 32);
            ps_mask* mask4 = ps_mask_create(64, 64);

            ps_mask_unref(mask1);
            ps_mask_unref(mask2);
            ps_mask_unref(mask3);
            ps_mask_unref(mask4);
        }
    });

    CompareToBenchmark(Mask_MaskCreateDifferentDimensions, result);
}

// Test 3: Mask creation with data
PERF_TEST_RUN(Mask, MaskCreateWithData)
{
    uint8_t* buffer = (uint8_t*)malloc(64 * 64);

    // Initialize buffer with test data
    for (int i = 0; i < 64 * 64; i++) {
        buffer[i] = (uint8_t)(i % 256);
    }

    auto result = RunBenchmark(Mask_MaskCreateWithData, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_mask* mask = ps_mask_create_with_data(buffer, 64, 64);
            ps_mask_unref(mask);
        }
    });

    CompareToBenchmark(Mask_MaskCreateWithData, result);
    free(buffer);
}

// Test 4: Mask creation with multiple data buffers
PERF_TEST_RUN(Mask, MaskCreateWithMultipleData)
{
    uint8_t* buffers[8];

    // Create multiple buffers with different sizes
    for (int i = 0; i < 8; i++) {
        int size = (i + 1) * 32 * (i + 1) * 32;
        buffers[i] = (uint8_t*)malloc(size);
        memset(buffers[i], 128 + i * 16, size);
    }

    auto result = RunBenchmark(Mask_MaskCreateWithMultipleData, [&]() {
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 8; j++) {
                ps_mask* mask = ps_mask_create_with_data(buffers[j], (j + 1) * 32, (j + 1) * 32);
                ps_mask_unref(mask);
            }
        }
    });

    CompareToBenchmark(Mask_MaskCreateWithMultipleData, result);

    for (int i = 0; i < 8; i++) {
        free(buffers[i]);
    }
}

// Test 5: Mask reference counting
PERF_TEST_RUN(Mask, MaskReferenceOperations)
{
    ps_mask* mask = ps_mask_create(64, 64);

    auto result = RunBenchmark(Mask_MaskReferenceOperations, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_mask_ref(mask);
            ps_mask_unref(mask);
        }
    });

    CompareToBenchmark(Mask_MaskReferenceOperations, result);
    ps_mask_unref(mask);
}

// Test 6: Multiple mask references
PERF_TEST_RUN(Mask, MaskMultipleReferences)
{
    auto result = RunBenchmark(Mask_MaskMultipleReferences, [&]() {
        for (int i = 0; i < 1000; i++) {
            ps_mask* mask = ps_mask_create(32, 32);
            ps_mask_ref(mask);
            ps_mask_ref(mask);
            ps_mask_unref(mask);
            ps_mask_unref(mask);
            ps_mask_unref(mask);
        }
    });

    CompareToBenchmark(Mask_MaskMultipleReferences, result);
}

// Test 7: Color filter operations
PERF_TEST_RUN(Mask, MaskColorFilterOperations)
{
    ps_mask* mask = ps_mask_create(64, 64);

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

    auto result = RunBenchmark(Mask_MaskColorFilterOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            // Add multiple color filters
            for (int j = 0; j < 8; j++) {
                ps_mask_add_color_filter(mask, &colors[j]);
            }
            // Clear all filters
            ps_mask_clear_color_filters(mask);
        }
    });

    CompareToBenchmark(Mask_MaskColorFilterOperations, result);
    ps_mask_unref(mask);
}

// Test 8: Complex color filter operations
PERF_TEST_RUN(Mask, MaskComplexColorFilters)
{
    ps_mask* mask = ps_mask_create(128, 128);

    auto result = RunBenchmark(Mask_MaskComplexColorFilters, [&]() {
        for (int i = 0; i < 5000; i++) {
            // Add many color filters with varying values
            for (int j = 0; j < 100; j++) {
                ps_color color = {
                    (float)j / 100.0f, // Red gradient
                    1.0f - (float)j / 100.0f, // Green inverse gradient
                     0.5f, // Constant blue
                     1.0f // Full alpha
                };
                ps_mask_add_color_filter(mask, &color);
            }
            ps_mask_clear_color_filters(mask);
        }
    });

    CompareToBenchmark(Mask_MaskComplexColorFilters, result);
    ps_mask_unref(mask);
}

// Test 9: Stress test with many masks
PERF_TEST_RUN(Mask, MaskStressTest)
{
    const int num_masks = 1000;
    ps_mask* masks[num_masks];

    auto result = RunBenchmark(Mask_MaskStressTest, [&]() {
        // Create many masks
        for (int i = 0; i < num_masks; i++) {
            masks[i] = ps_mask_create(16, 16);
        }

        // Reference operations
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < num_masks; j++) {
                ps_mask_ref(masks[j]);
            }
            for (int j = 0; j < num_masks; j++) {
                ps_mask_unref(masks[j]);
            }
        }

        // Clean up
        for (int i = 0; i < num_masks; i++) {
            ps_mask_unref(masks[i]);
        }
    });

    CompareToBenchmark(Mask_MaskStressTest, result);
}

// Test 10: Mask operations with canvas integration
PERF_TEST_RUN(Mask, MaskCanvasIntegration)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 640, 480);
    ps_mask* mask = ps_mask_create(640, 480);

    auto result = RunBenchmark(Mask_MaskCanvasIntegration, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_canvas_set_mask(canvas, mask);
            ps_canvas_reset_mask(canvas);
        }
    });

    CompareToBenchmark(Mask_MaskCanvasIntegration, result);
    ps_canvas_unref(canvas);
    ps_mask_unref(mask);
}

// Test 11: Error handling with NULL parameters
PERF_TEST_RUN(Mask, MaskErrorHandling)
{
    auto result = RunBenchmark(Mask_MaskErrorHandling, [&]() {
        for (int i = 0; i < 10000; i++) {
            // NULL pointer tests
            ps_mask_create(-1, 64);
            ps_mask_create(64, -1);
            ps_mask_create(-1, -1);
            ps_mask_create_with_data(NULL, 64, 64);
            ps_mask_ref(NULL);
            ps_mask_unref(NULL);
            ps_mask_add_color_filter(NULL, NULL);
            ps_mask_clear_color_filters(NULL);
        }
    });

    CompareToBenchmark(Mask_MaskErrorHandling, result);
}

// Test 12: Mask reuse performance
PERF_TEST_RUN(Mask, MaskReusePerformance)
{
    ps_mask* mask = ps_mask_create(64, 64);
    ps_color red = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_color blue = {0.0f, 0.0f, 1.0f, 1.0f};

    auto result = RunBenchmark(Mask_MaskReusePerformance, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_mask_add_color_filter(mask, &red);
            ps_mask_add_color_filter(mask, &blue);
            ps_mask_clear_color_filters(mask);
        }
    });

    CompareToBenchmark(Mask_MaskReusePerformance, result);
    ps_mask_unref(mask);
}

// Test 13: Mask data buffer operations
PERF_TEST_RUN(Mask, MaskDataBufferOperations)
{
    uint8_t* buffer = (uint8_t*)malloc(128 * 128);

    // Create pattern data
    for (int y = 0; y < 128; y++) {
        for (int x = 0; x < 128; x++) {
            buffer[y * 128 + x] = (uint8_t)((x + y) % 256);
        }
    }

    auto result = RunBenchmark(Mask_MaskDataBufferOperations, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_mask* mask = ps_mask_create_with_data(buffer, 128, 128);
            ps_color color = {0.5f, 0.5f, 0.5f, 1.0f};
            ps_mask_add_color_filter(mask, &color);
            ps_mask_unref(mask);
        }
    });

    CompareToBenchmark(Mask_MaskDataBufferOperations, result);
    free(buffer);
}
