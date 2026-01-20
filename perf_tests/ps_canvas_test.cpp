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

#define WIDTH 10
#define HEIGHT 10

PERF_TEST_DEFINE(Canvas);

// Test 1: Basic canvas creation and destruction
PERF_TEST(Canvas, CanvasCreateDestroy)
{
    for (int i = 0; i < 1000; i++) {
        ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, WIDTH, HEIGHT);
        ps_canvas_unref(canvas);
    }
}

// Test 2: Canvas creation with all color formats
PERF_TEST_RUN(Canvas, CanvasCreateAllFormats)
{
    ps_color_format formats[] = {
        COLOR_FORMAT_RGBA, COLOR_FORMAT_ARGB, COLOR_FORMAT_ABGR, COLOR_FORMAT_BGRA,
        COLOR_FORMAT_RGB, COLOR_FORMAT_BGR, COLOR_FORMAT_RGB565, COLOR_FORMAT_RGB555,
        COLOR_FORMAT_A8
    };

    auto result = RunBenchmark(Canvas_CanvasCreateAllFormats, [&]() {
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 9; j++) {
                ps_canvas* canvas = ps_canvas_create(formats[j], WIDTH, HEIGHT);
                ps_canvas_unref(canvas);
            }
        }
    });

    CompareToBenchmark(Canvas_CanvasCreateAllFormats, result);
}

// Test 3: Canvas creation with data
PERF_TEST_RUN(Canvas, CanvasCreateWithData)
{
    uint8_t* buffers[9];
    int pitches[9];

    // Create buffers for different formats
    buffers[0] = (uint8_t*)malloc(WIDTH * HEIGHT * 4); // RGBA
    buffers[1] = (uint8_t*)malloc(WIDTH * HEIGHT * 4); // ARGB
    buffers[2] = (uint8_t*)malloc(WIDTH * HEIGHT * 4); // ABGR
    buffers[3] = (uint8_t*)malloc(WIDTH * HEIGHT * 4); // BGRA
    buffers[4] = (uint8_t*)malloc(WIDTH * HEIGHT * 3); // RGB
    buffers[5] = (uint8_t*)malloc(WIDTH * HEIGHT * 3); // BGR
    buffers[6] = (uint8_t*)malloc(WIDTH * HEIGHT * 2); // RGB565
    buffers[7] = (uint8_t*)malloc(WIDTH * HEIGHT * 2); // RGB555
    buffers[8] = (uint8_t*)malloc(WIDTH * HEIGHT); // A8

    pitches[0] = pitches[1] = pitches[2] = pitches[3] = WIDTH * 4;
    pitches[4] = pitches[5] = WIDTH * 3;
    pitches[6] = pitches[7] = WIDTH * 2;
    pitches[8] = WIDTH;

    ps_color_format formats[] = {
        COLOR_FORMAT_RGBA, COLOR_FORMAT_ARGB, COLOR_FORMAT_ABGR, COLOR_FORMAT_BGRA,
        COLOR_FORMAT_RGB, COLOR_FORMAT_BGR, COLOR_FORMAT_RGB565, COLOR_FORMAT_RGB555,
        COLOR_FORMAT_A8
    };

    auto result = RunBenchmark(Canvas_CanvasCreateWithData, [&]() {
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 9; j++) {
                ps_canvas* canvas = ps_canvas_create_with_data(buffers[j], formats[j], WIDTH, HEIGHT, pitches[j]);
                ps_canvas_unref(canvas);
            }
        }
    });

    CompareToBenchmark(Canvas_CanvasCreateWithData, result);

    for (int i = 0; i < 9; i++) {
        free(buffers[i]);
    }
}

// Test 4: Canvas create compatible
PERF_TEST_RUN(Canvas, CanvasCreateCompatible)
{
    ps_canvas* reference = ps_canvas_create(COLOR_FORMAT_RGBA, 320, 240);

    auto result = RunBenchmark(Canvas_CanvasCreateCompatible, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_canvas* canvas = ps_canvas_create_compatible(reference, WIDTH, HEIGHT);
            ps_canvas_unref(canvas);
        }
    });

    CompareToBenchmark(Canvas_CanvasCreateCompatible, result);
    ps_canvas_unref(reference);
}

// Test 5: Canvas create from canvas
PERF_TEST_RUN(Canvas, CanvasCreateFromCanvas)
{
    ps_canvas* source = ps_canvas_create(COLOR_FORMAT_RGBA, 320, 240);
    ps_rect rect = {100, 100, WIDTH, HEIGHT};

    auto result = RunBenchmark(Canvas_CanvasCreateFromCanvas, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_canvas* canvas = ps_canvas_create_from_canvas(source, &rect);
            ps_canvas_unref(canvas);
        }
    });

    CompareToBenchmark(Canvas_CanvasCreateFromCanvas, result);
    ps_canvas_unref(source);
}

// Test 6: Canvas reference counting
PERF_TEST_RUN(Canvas, CanvasReferenceOperations)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, WIDTH, HEIGHT);

    auto result = RunBenchmark(Canvas_CanvasReferenceOperations, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_canvas_ref(canvas);
            ps_canvas_unref(canvas);
        }
    });

    CompareToBenchmark(Canvas_CanvasReferenceOperations, result);
    ps_canvas_unref(canvas);
}

// Test 7: Canvas property operations
PERF_TEST_RUN(Canvas, CanvasPropertyOperations)
{
    ps_canvas* canvases[9];
    ps_color_format formats[] = {
        COLOR_FORMAT_RGBA, COLOR_FORMAT_ARGB, COLOR_FORMAT_ABGR, COLOR_FORMAT_BGRA,
        COLOR_FORMAT_RGB, COLOR_FORMAT_BGR, COLOR_FORMAT_RGB565, COLOR_FORMAT_RGB555,
        COLOR_FORMAT_A8
    };

    for (int i = 0; i < 9; i++) {
        canvases[i] = ps_canvas_create(formats[i], WIDTH, HEIGHT);
    }

    auto result = RunBenchmark(Canvas_CanvasPropertyOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            for (int j = 0; j < 9; j++) {
                ps_size size;
                ps_canvas_get_size(canvases[j], &size);
                ps_canvas_get_format(canvases[j]);
            }
        }
    });

    CompareToBenchmark(Canvas_CanvasPropertyOperations, result);

    for (int i = 0; i < 9; i++) {
        ps_canvas_unref(canvases[i]);
    }
}

// Test 8: Canvas mask operations
PERF_TEST_RUN(Canvas, CanvasMaskOperations)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, WIDTH, HEIGHT);
    ps_mask* mask = ps_mask_create(WIDTH, HEIGHT);

    auto result = RunBenchmark(Canvas_CanvasMaskOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_canvas_set_mask(canvas, mask);
            ps_canvas_reset_mask(canvas);
        }
    });

    CompareToBenchmark(Canvas_CanvasMaskOperations, result);
    ps_canvas_unref(canvas);
    ps_mask_unref(mask);
}

// Test 9: Canvas bitblt operations
PERF_TEST_RUN(Canvas, CanvasBitbltOperations)
{
    ps_canvas* src = ps_canvas_create(COLOR_FORMAT_RGBA, 320, 240);
    ps_canvas* dst = ps_canvas_create(COLOR_FORMAT_RGBA, 160, 120);
    ps_rect rect = {0, 0, WIDTH, HEIGHT};
    ps_point location = {100, 100};

    auto result = RunBenchmark(Canvas_CanvasBitbltOperations, [&]() {
        for (int i = 0; i < 1000; i++) {
            ps_canvas_bitblt(src, &rect, dst, &location);
        }
    });

    CompareToBenchmark(Canvas_CanvasBitbltOperations, result);
    ps_canvas_unref(src);
    ps_canvas_unref(dst);
}

// Test 10: Canvas replace data
PERF_TEST_RUN(Canvas, CanvasReplaceData)
{
    uint8_t* buffer1 = (uint8_t*)malloc(WIDTH * HEIGHT * 4);
    uint8_t* buffer2 = (uint8_t*)malloc(WIDTH * 2 * HEIGHT * 2 * 4);

    ps_canvas* canvas = ps_canvas_create_with_data(buffer1, COLOR_FORMAT_RGBA, WIDTH, HEIGHT, WIDTH * 4);

    auto result = RunBenchmark(Canvas_CanvasReplaceData, [&]() {
        for (int i = 0; i < 1000; i++) {
            ps_canvas_replace_data(canvas, buffer2, COLOR_FORMAT_RGBA, WIDTH * 2, HEIGHT * 2, WIDTH * 2 * 4);
            ps_canvas_replace_data(canvas, buffer1, COLOR_FORMAT_RGBA, WIDTH, HEIGHT, WIDTH * 4);
        }
    });

    CompareToBenchmark(Canvas_CanvasReplaceData, result);

    ps_canvas_unref(canvas);
    free(buffer1);
    free(buffer2);
}

// Test 11: Stress test with many canvases
PERF_TEST_RUN(Canvas, CanvasStressTest)
{
    const int num_canvases = 1000;
    ps_canvas* canvases[num_canvases];

    auto result = RunBenchmark(Canvas_CanvasStressTest, [&]() {
        // Create many canvases
        for (int i = 0; i < num_canvases; i++) {
            canvases[i] = ps_canvas_create(COLOR_FORMAT_RGBA, 64, 64);
        }

        // Reference operations
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < num_canvases; j++) {
                ps_canvas_ref(canvases[j]);
            }
            for (int j = 0; j < num_canvases; j++) {
                ps_canvas_unref(canvases[j]);
            }
        }

        // Clean up
        for (int i = 0; i < num_canvases; i++) {
            ps_canvas_unref(canvases[i]);
        }
    });

    CompareToBenchmark(Canvas_CanvasStressTest, result);
}

// Test 12: Error handling with NULL parameters
PERF_TEST_RUN(Canvas, CanvasErrorHandling)
{
    auto result = RunBenchmark(Canvas_CanvasErrorHandling, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_canvas_create(COLOR_FORMAT_UNKNOWN, WIDTH, HEIGHT);
            ps_canvas_create(COLOR_FORMAT_RGBA, -1, HEIGHT);
            ps_canvas_create(COLOR_FORMAT_RGBA, WIDTH, -1);
            ps_canvas_ref(NULL);
            ps_canvas_unref(NULL);
            ps_canvas_get_size(NULL, NULL);
            ps_canvas_get_format(NULL);
            ps_canvas_set_mask(NULL, NULL);
            ps_canvas_reset_mask(NULL);
        }
    });

    CompareToBenchmark(Canvas_CanvasErrorHandling, result);
}

// Test 14: Canvas create from mask
PERF_TEST_RUN(Canvas, CanvasCreateFromMask)
{
    ps_mask* mask = ps_mask_create(WIDTH * 2, HEIGHT * 2);
    ps_rect rect = {10, 10, WIDTH, HEIGHT};

    auto result = RunBenchmark(Canvas_CanvasCreateFromMask, [&]() {
        for (int i = 0; i < 1000; i++) {
            ps_canvas* canvas = ps_canvas_create_from_mask(mask, &rect);
            ps_canvas_unref(canvas);
        }
    });

    CompareToBenchmark(Canvas_CanvasCreateFromMask, result);
    ps_mask_unref(mask);
}
