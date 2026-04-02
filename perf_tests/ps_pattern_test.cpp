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

PERF_TEST_DEFINE(Pattern);

// Test 1: Basic pattern creation and destruction
PERF_TEST(Pattern, PatternCreateDestroy)
{
    // Create a test image
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 64, 64);

    for (int i = 0; i < 10000; i++) {
        ps_pattern* pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);
        ps_pattern_unref(pattern);
    }

    ps_image_unref(img);
}

// Test 2: Pattern creation with all wrap modes
PERF_TEST_RUN(Pattern, PatternCreateAllWrapModes)
{
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 32, 32);

    auto result = RunBenchmark(Pattern_PatternCreateAllWrapModes, [&]() {
        for (int i = 0; i < 5000; i++) {
            // Test all combinations of wrap modes
            ps_pattern* pattern1 = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);
            ps_pattern* pattern2 = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REFLECT, NULL);
            ps_pattern* pattern3 = ps_pattern_create_image(img, WRAP_TYPE_REFLECT, WRAP_TYPE_REPEAT, NULL);
            ps_pattern* pattern4 = ps_pattern_create_image(img, WRAP_TYPE_REFLECT, WRAP_TYPE_REFLECT, NULL);

            ps_pattern_unref(pattern1);
            ps_pattern_unref(pattern2);
            ps_pattern_unref(pattern3);
            ps_pattern_unref(pattern4);
        }
    });

    CompareToBenchmark(Pattern_PatternCreateAllWrapModes, result);
    ps_image_unref(img);
}

// Test 3: Pattern creation with transform matrix
PERF_TEST_RUN(Pattern, PatternCreateWithTransform)
{
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 64, 64);
    ps_matrix* matrix = ps_matrix_create();
    ps_matrix_scale(matrix, 2.0f, 2.0f);

    auto result = RunBenchmark(Pattern_PatternCreateWithTransform, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_pattern* pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, matrix);
            ps_pattern_unref(pattern);
        }
    });

    CompareToBenchmark(Pattern_PatternCreateWithTransform, result);
    ps_matrix_unref(matrix);
    ps_image_unref(img);
}

// Test 4: Pattern reference counting
PERF_TEST_RUN(Pattern, PatternReferenceOperations)
{
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 32, 32);
    ps_pattern* pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);

    auto result = RunBenchmark(Pattern_PatternReferenceOperations, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_pattern_ref(pattern);
            ps_pattern_unref(pattern);
        }
    });

    CompareToBenchmark(Pattern_PatternReferenceOperations, result);
    ps_pattern_unref(pattern);
    ps_image_unref(img);
}

// Test 5: Pattern transformation operations
PERF_TEST_RUN(Pattern, PatternTransformOperations)
{
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 64, 64);
    ps_pattern* pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);
    ps_matrix* matrix = ps_matrix_create();
    ps_matrix_identity(matrix);
    ps_matrix_translate(matrix, 0.1f, 0.1f);
    ps_matrix_rotate(matrix, 0.01f);
    ps_matrix_scale(matrix, 1.0f + 0.001f, 1.0f + 0.001f);

    auto result = RunBenchmark(Pattern_PatternTransformOperations, [&]() {
        for (int i = 0; i < 100000; i++) {
            // Apply various transformations
            ps_pattern_transform(pattern, matrix);
        }
    });

    CompareToBenchmark(Pattern_PatternTransformOperations, result);
    ps_pattern_unref(pattern);
    ps_matrix_unref(matrix);
    ps_image_unref(img);
}

// Test 6: Pattern with different image formats
PERF_TEST_RUN(Pattern, PatternDifferentImageFormats)
{
    ps_color_format formats[] = {
        COLOR_FORMAT_RGBA, COLOR_FORMAT_ARGB, COLOR_FORMAT_ABGR, COLOR_FORMAT_BGRA,
        COLOR_FORMAT_RGB, COLOR_FORMAT_BGR, COLOR_FORMAT_RGB565, COLOR_FORMAT_A8
    };

    ps_image* images[8];

    // Create images with different formats
    for (int i = 0; i < 8; i++) {
        images[i] = ps_image_create(formats[i], 32, 32);
    }

    auto result = RunBenchmark(Pattern_PatternDifferentImageFormats, [&]() {
        for (int i = 0; i < 10000; i++) {
            for (int j = 0; j < 8; j++) {
                ps_pattern* pattern = ps_pattern_create_image(images[j], WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);
                ps_pattern_unref(pattern);
            }
        }
    });

    CompareToBenchmark(Pattern_PatternDifferentImageFormats, result);

    for (int i = 0; i < 8; i++) {
        ps_image_unref(images[i]);
    }
}

// Test 7: Pattern stress test with many patterns
PERF_TEST_RUN(Pattern, PatternStressTest)
{
    const int num_patterns = 1000;
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 16, 16);
    ps_pattern* patterns[num_patterns];

    auto result = RunBenchmark(Pattern_PatternStressTest, [&]() {
        // Create many patterns
        for (int i = 0; i < num_patterns; i++) {
            patterns[i] = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);
        }

        // Reference operations
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < num_patterns; j++) {
                ps_pattern_ref(patterns[j]);
            }
            for (int j = 0; j < num_patterns; j++) {
                ps_pattern_unref(patterns[j]);
            }
        }

        // Clean up
        for (int i = 0; i < num_patterns; i++) {
            ps_pattern_unref(patterns[i]);
        }
    });

    CompareToBenchmark(Pattern_PatternStressTest, result);
    ps_image_unref(img);
}

// Test 8: Pattern creation from different image sources
PERF_TEST_RUN(Pattern, PatternDifferentImageSources)
{
    // Create different types of images
    ps_image* img1 = ps_image_create(COLOR_FORMAT_RGBA, 32, 32);
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 32, 32);
    ps_image* img2 = ps_image_create_from_canvas(canvas, NULL);

    auto result = RunBenchmark(Pattern_PatternDifferentImageSources, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_pattern* pattern1 = ps_pattern_create_image(img1, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);
            ps_pattern* pattern2 = ps_pattern_create_image(img2, WRAP_TYPE_REFLECT, WRAP_TYPE_REFLECT, NULL);

            ps_pattern_unref(pattern1);
            ps_pattern_unref(pattern2);
        }
    });

    CompareToBenchmark(Pattern_PatternDifferentImageSources, result);
    ps_image_unref(img1);
    ps_image_unref(img2);
    ps_canvas_unref(canvas);
}

// Test 9: Pattern error handling
PERF_TEST_RUN(Pattern, PatternErrorHandling)
{
    auto result = RunBenchmark(Pattern_PatternErrorHandling, [&]() {
        for (int i = 0; i < 10000; i++) {
            // NULL pointer tests
            ps_pattern_create_image(NULL, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);
            ps_pattern_ref(NULL);
            ps_pattern_unref(NULL);
            ps_pattern_transform(NULL, NULL);
        }
    });

    CompareToBenchmark(Pattern_PatternErrorHandling, result);
}

// Test 10: Pattern with large images
PERF_TEST_RUN(Pattern, PatternLargeImages)
{
    ps_image* large_img = ps_image_create(COLOR_FORMAT_RGBA, 1024, 1024);

    auto result = RunBenchmark(Pattern_PatternLargeImages, [&]() {
        for (int i = 0; i < 1000; i++) {
            ps_pattern* pattern = ps_pattern_create_image(large_img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);
            ps_pattern_unref(pattern);
        }
    });

    CompareToBenchmark(Pattern_PatternLargeImages, result);
    ps_image_unref(large_img);
}
