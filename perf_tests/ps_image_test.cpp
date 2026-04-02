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

PERF_TEST_DEFINE(Image);

// Test 1: Basic image creation and destruction
PERF_TEST(Image, ImageCreateDestroy)
{
    for (int i = 0; i < 10000; i++) {
        ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 64, 64);
        ps_image_unref(img);
    }
}

// Test 2: Image creation with all color formats
PERF_TEST_RUN(Image, ImageCreateAllFormats)
{
    ps_color_format formats[] = {
        COLOR_FORMAT_RGBA, COLOR_FORMAT_ARGB, COLOR_FORMAT_ABGR, COLOR_FORMAT_BGRA,
        COLOR_FORMAT_RGB, COLOR_FORMAT_BGR, COLOR_FORMAT_RGB565, COLOR_FORMAT_RGB555,
        COLOR_FORMAT_A8
    };

    auto result = RunBenchmark(Image_ImageCreateAllFormats, [&]() {
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 9; j++) {
                ps_image* img = ps_image_create(formats[j], 32, 32);
                ps_image_unref(img);
            }
        }
    });

    CompareToBenchmark(Image_ImageCreateAllFormats, result);
}

// Test 3: Image creation with data (wrap existing buffer)
PERF_TEST_RUN(Image, ImageCreateWithData)
{
    uint8_t* buffers[9];
    int pitches[9];

    // Create buffers for different formats
    buffers[0] = (uint8_t*)malloc(64 * 64 * 4); // RGBA
    buffers[1] = (uint8_t*)malloc(64 * 64 * 4); // ARGB
    buffers[2] = (uint8_t*)malloc(64 * 64 * 4); // ABGR
    buffers[3] = (uint8_t*)malloc(64 * 64 * 4); // BGRA
    buffers[4] = (uint8_t*)malloc(64 * 64 * 3); // RGB
    buffers[5] = (uint8_t*)malloc(64 * 64 * 3); // BGR
    buffers[6] = (uint8_t*)malloc(64 * 64 * 2); // RGB565
    buffers[7] = (uint8_t*)malloc(64 * 64 * 2); // RGB555
    buffers[8] = (uint8_t*)malloc(64 * 64); // A8

    pitches[0] = pitches[1] = pitches[2] = pitches[3] = 64 * 4;
    pitches[4] = pitches[5] = 64 * 3;
    pitches[6] = pitches[7] = 64 * 2;
    pitches[8] = 64;

    ps_color_format formats[] = {
        COLOR_FORMAT_RGBA, COLOR_FORMAT_ARGB, COLOR_FORMAT_ABGR, COLOR_FORMAT_BGRA,
        COLOR_FORMAT_RGB, COLOR_FORMAT_BGR, COLOR_FORMAT_RGB565, COLOR_FORMAT_RGB555,
        COLOR_FORMAT_A8
    };

    auto result = RunBenchmark(Image_ImageCreateWithData, [&]() {
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 9; j++) {
                ps_image* img = ps_image_create_with_data(buffers[j], formats[j], 64, 64, pitches[j]);
                ps_image_unref(img);
            }
        }
    });

    CompareToBenchmark(Image_ImageCreateWithData, result);

    for (int i = 0; i < 9; i++) {
        free(buffers[i]);
    }
}

// Test 4: Image creation from data (copy buffer)
PERF_TEST_RUN(Image, ImageCreateFromData)
{
    uint8_t* buffer = (uint8_t*)malloc(64 * 64 * 4);

    auto result = RunBenchmark(Image_ImageCreateFromData, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_image* img = ps_image_create_from_data(buffer, COLOR_FORMAT_RGBA, 64, 64, 64 * 4);
            ps_image_unref(img);
        }
    });

    CompareToBenchmark(Image_ImageCreateFromData, result);
    free(buffer);
}

// Test 5: Image create compatible
PERF_TEST_RUN(Image, ImageCreateCompatible)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 128, 128);

    auto result = RunBenchmark(Image_ImageCreateCompatible, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_image* img = ps_image_create_compatible(canvas, 64, 64);
            ps_image_unref(img);
        }
    });

    CompareToBenchmark(Image_ImageCreateCompatible, result);
    ps_canvas_unref(canvas);
}

// Test 6: Image create from canvas
PERF_TEST_RUN(Image, ImageCreateFromCanvas)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 128, 128);
    ps_rect rect = {32, 32, 64, 64};

    auto result = RunBenchmark(Image_ImageCreateFromCanvas, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_image* img = ps_image_create_from_canvas(canvas, &rect);
            ps_image_unref(img);
        }
    });

    CompareToBenchmark(Image_ImageCreateFromCanvas, result);
    ps_canvas_unref(canvas);
}

// Test 7: Image create from image
PERF_TEST_RUN(Image, ImageCreateFromImage)
{
    ps_image* source = ps_image_create(COLOR_FORMAT_RGBA, 128, 128);
    ps_rect rect = {32, 32, 64, 64};

    auto result = RunBenchmark(Image_ImageCreateFromImage, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_image* img = ps_image_create_from_image(source, &rect);
            ps_image_unref(img);
        }
    });

    CompareToBenchmark(Image_ImageCreateFromImage, result);
    ps_image_unref(source);
}

// Test 8: Image reference counting
PERF_TEST_RUN(Image, ImageReferenceOperations)
{
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 64, 64);

    auto result = RunBenchmark(Image_ImageReferenceOperations, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_image_ref(img);
            ps_image_unref(img);
        }
    });

    CompareToBenchmark(Image_ImageReferenceOperations, result);
    ps_image_unref(img);
}

// Test 9: Image property operations
PERF_TEST_RUN(Image, ImagePropertyOperations)
{
    ps_image* images[9];
    ps_color_format formats[] = {
        COLOR_FORMAT_RGBA, COLOR_FORMAT_ARGB, COLOR_FORMAT_ABGR, COLOR_FORMAT_BGRA,
        COLOR_FORMAT_RGB, COLOR_FORMAT_BGR, COLOR_FORMAT_RGB565, COLOR_FORMAT_RGB555,
        COLOR_FORMAT_A8
    };

    for (int i = 0; i < 9; i++) {
        images[i] = ps_image_create(formats[i], 64, 64);
    }

    auto result = RunBenchmark(Image_ImagePropertyOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            for (int j = 0; j < 9; j++) {
                ps_size size;
                ps_image_get_size(images[j], &size);
                ps_color_format format = ps_image_get_format(images[j]);
                UNUSED(format);
            }
        }
    });

    CompareToBenchmark(Image_ImagePropertyOperations, result);

    for (int i = 0; i < 9; i++) {
        ps_image_unref(images[i]);
    }
}

// Test 10: Image transparency operations
PERF_TEST_RUN(Image, ImageTransparencyOperations)
{
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 64, 64);
    ps_color transparent_color = {1.0f, 0.0f, 1.0f, 1.0f};

    auto result = RunBenchmark(Image_ImageTransparencyOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_image_set_allow_transparent(img, True);
            ps_image_set_transparent_color(img, &transparent_color);
            ps_image_set_allow_transparent(img, False);
            ps_image_set_transparent_color(img, NULL);
        }
    });

    CompareToBenchmark(Image_ImageTransparencyOperations, result);
    ps_image_unref(img);
}

// Test 11: Stress test with many images
PERF_TEST_RUN(Image, ImageStressTest)
{
    const int num_images = 1000;
    ps_image* images[num_images];

    auto result = RunBenchmark(Image_ImageStressTest, [&]() {
        // Create many images
        for (int i = 0; i < num_images; i++) {
            images[i] = ps_image_create(COLOR_FORMAT_RGBA, 16, 16);
        }

        // Reference operations
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < num_images; j++) {
                ps_image_ref(images[j]);
            }
            for (int j = 0; j < num_images; j++) {
                ps_image_unref(images[j]);
            }
        }

        // Clean up
        for (int i = 0; i < num_images; i++) {
            ps_image_unref(images[i]);
        }
    });

    CompareToBenchmark(Image_ImageStressTest, result);
}

// Test 12: Error handling with NULL parameters
PERF_TEST_RUN(Image, ImageErrorHandling)
{
    auto result = RunBenchmark(Image_ImageErrorHandling, [&]() {
        for (int i = 0; i < 10000; i++) {
            // NULL pointer tests
            ps_image_create(COLOR_FORMAT_UNKNOWN, 64, 64);
            ps_image_create(COLOR_FORMAT_RGBA, -1, 64);
            ps_image_create(COLOR_FORMAT_RGBA, 64, -1);
            ps_image_create_with_data(NULL, COLOR_FORMAT_RGBA, 64, 64, 64 * 4);
            ps_image_create_from_data(NULL, COLOR_FORMAT_RGBA, 64, 64, 64 * 4);
            ps_image_create_compatible(NULL, 64, 64);
            ps_image_create_from_canvas(NULL, NULL);
            ps_image_create_from_image(NULL, NULL);
            ps_image_ref(NULL);
            ps_image_unref(NULL);
            ps_image_set_allow_transparent(NULL, True);
            ps_image_set_transparent_color(NULL, NULL);
            ps_image_get_size(NULL, NULL);
            ps_image_get_format(NULL);
        }
    });

    CompareToBenchmark(Image_ImageErrorHandling, result);
}

// Test 13: Image creation with different dimensions
PERF_TEST_RUN(Image, ImageDifferentDimensions)
{
    auto result = RunBenchmark(Image_ImageDifferentDimensions, [&]() {
        for (int i = 0; i < 1000; i++) {
            // Test various dimensions
            ps_image* img1 = ps_image_create(COLOR_FORMAT_RGBA, 1, 1);
            ps_image* img2 = ps_image_create(COLOR_FORMAT_RGBA, 8, 8);
            ps_image* img3 = ps_image_create(COLOR_FORMAT_RGBA, 32, 32);
            ps_image* img4 = ps_image_create(COLOR_FORMAT_RGBA, 256, 256);

            ps_image_unref(img1);
            ps_image_unref(img2);
            ps_image_unref(img3);
            ps_image_unref(img4);
        }
    });

    CompareToBenchmark(Image_ImageDifferentDimensions, result);
}

// Test 14: Image reuse performance
PERF_TEST_RUN(Image, ImageReusePerformance)
{
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 64, 64);
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};

    auto result = RunBenchmark(Image_ImageReusePerformance, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_image_set_allow_transparent(img, True);
            ps_image_set_transparent_color(img, &color);
            ps_size size;
            ps_image_get_size(img, &size);
            ps_color_format format = ps_image_get_format(img);
            UNUSED(format);
        }
    });

    CompareToBenchmark(Image_ImageReusePerformance, result);
    ps_image_unref(img);
}
