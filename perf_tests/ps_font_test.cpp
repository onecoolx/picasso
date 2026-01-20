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

PERF_TEST_DEFINE(Font);

// Test 1: Basic font creation and destruction
PERF_TEST(Font, FontCreateDestroy)
{
    for (int i = 0; i < 10000; i++) {
        ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
        ps_font_unref(font);
    }
}

// Test 2: Font creation with all charsets
PERF_TEST_RUN(Font, FontCreateAllCharsets)
{
    auto result = RunBenchmark(Font_FontCreateAllCharsets, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_font* font1 = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
            ps_font* font2 = ps_font_create("Arial", CHARSET_UNICODE, 12.0f, FONT_WEIGHT_REGULAR, False);
            ps_font_unref(font1);
            ps_font_unref(font2);
        }
    });

    CompareToBenchmark(Font_FontCreateAllCharsets, result);
}

// Test 3: Font creation with all weights
PERF_TEST_RUN(Font, FontCreateAllWeights)
{
    ps_font_weight weights[] = {
        FONT_WEIGHT_REGULAR, FONT_WEIGHT_MEDIUM, FONT_WEIGHT_BOLD, FONT_WEIGHT_HEAVY
    };

    auto result = RunBenchmark(Font_FontCreateAllWeights, [&]() {
        for (int i = 0; i < 2500; i++) {
            for (int j = 0; j < 4; j++) {
                ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, weights[j], False);
                ps_font_unref(font);
            }
        }
    });

    CompareToBenchmark(Font_FontCreateAllWeights, result);
}

// Test 4: Font creation with italic variants
PERF_TEST_RUN(Font, FontCreateItalicVariants)
{
    auto result = RunBenchmark(Font_FontCreateItalicVariants, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_font* regular = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
            ps_font* italic = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, True);
            ps_font_unref(regular);
            ps_font_unref(italic);
        }
    });

    CompareToBenchmark(Font_FontCreateItalicVariants, result);
}

// Test 5: Font creation with different sizes
PERF_TEST_RUN(Font, FontCreateDifferentSizes)
{
    float sizes[] = {8.0f, 12.0f, 16.0f, 24.0f, 36.0f, 48.0f, 72.0f, 96.0f};

    auto result = RunBenchmark(Font_FontCreateDifferentSizes, [&]() {
        for (int i = 0; i < 1250; i++) {
            for (int j = 0; j < 8; j++) {
                ps_font* font = ps_font_create("Arial", CHARSET_ANSI, sizes[j], FONT_WEIGHT_REGULAR, False);
                ps_font_unref(font);
            }
        }
    });

    CompareToBenchmark(Font_FontCreateDifferentSizes, result);
}

// Test 6: Font creation copy
PERF_TEST_RUN(Font, FontCreateCopy)
{
    ps_font* source = ps_font_create("Arial", CHARSET_ANSI, 18.0f, FONT_WEIGHT_BOLD, True);

    auto result = RunBenchmark(Font_FontCreateCopy, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_font* copy = ps_font_create_copy(source);
            ps_font_unref(copy);
        }
    });

    CompareToBenchmark(Font_FontCreateCopy, result);
    ps_font_unref(source);
}

// Test 7: Font reference counting
PERF_TEST_RUN(Font, FontReferenceOperations)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);

    auto result = RunBenchmark(Font_FontReferenceOperations, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_font_ref(font);
            ps_font_unref(font);
        }
    });

    CompareToBenchmark(Font_FontReferenceOperations, result);
    ps_font_unref(font);
}

// Test 8: Multiple font references
PERF_TEST_RUN(Font, FontMultipleReferences)
{
    auto result = RunBenchmark(Font_FontMultipleReferences, [&]() {
        for (int i = 0; i < 1000; i++) {
            ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
            ps_font_ref(font);
            ps_font_ref(font);
            ps_font_unref(font);
            ps_font_unref(font);
            ps_font_unref(font);
        }
    });

    CompareToBenchmark(Font_FontMultipleReferences, result);
}

// Test 9: Font property operations - size
PERF_TEST_RUN(Font, FontSizeOperations)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);

    auto result = RunBenchmark(Font_FontSizeOperations, [&]() {
        for (int i = 0; i < 50000; i++) {
            ps_font_set_size(font, 8.0f + (i % 20) * 2.0f);
        }
    });

    CompareToBenchmark(Font_FontSizeOperations, result);
    ps_font_unref(font);
}

// Test 10: Font property operations - weight
PERF_TEST_RUN(Font, FontWeightOperations)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ps_font_weight weights[] = {
        FONT_WEIGHT_REGULAR, FONT_WEIGHT_MEDIUM, FONT_WEIGHT_BOLD, FONT_WEIGHT_HEAVY
    };

    auto result = RunBenchmark(Font_FontWeightOperations, [&]() {
        for (int i = 0; i < 25000; i++) {
            ps_font_set_weight(font, weights[i % 4]);
        }
    });

    CompareToBenchmark(Font_FontWeightOperations, result);
    ps_font_unref(font);
}

// Test 11: Font property operations - italic
PERF_TEST_RUN(Font, FontItalicOperations)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);

    auto result = RunBenchmark(Font_FontItalicOperations, [&]() {
        for (int i = 0; i < 50000; i++) {
            ps_font_set_italic(font, (i % 2) ? True : False);
        }
    });

    CompareToBenchmark(Font_FontItalicOperations, result);
    ps_font_unref(font);
}

// Test 12: Font property operations - charset
PERF_TEST_RUN(Font, FontCharsetOperations)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);

    auto result = RunBenchmark(Font_FontCharsetOperations, [&]() {
        for (int i = 0; i < 50000; i++) {
            ps_font_set_charset(font, (i % 2) ? CHARSET_ANSI : CHARSET_UNICODE);
        }
    });

    CompareToBenchmark(Font_FontCharsetOperations, result);
    ps_font_unref(font);
}

// Test 13: Font property operations - hint
PERF_TEST_RUN(Font, FontHintOperations)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);

    auto result = RunBenchmark(Font_FontHintOperations, [&]() {
        for (int i = 0; i < 50000; i++) {
            ps_font_set_hint(font, (i % 2) ? True : False);
        }
    });

    CompareToBenchmark(Font_FontHintOperations, result);
    ps_font_unref(font);
}

// Test 14: Font property operations - flip
PERF_TEST_RUN(Font, FontFlipOperations)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);

    auto result = RunBenchmark(Font_FontFlipOperations, [&]() {
        for (int i = 0; i < 50000; i++) {
            ps_font_set_flip(font, (i % 2) ? True : False);
        }
    });

    CompareToBenchmark(Font_FontFlipOperations, result);
    ps_font_unref(font);
}

// Test 15: Complex font property operations
PERF_TEST_RUN(Font, FontComplexPropertyOperations)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ps_font_weight weights[] = {
        FONT_WEIGHT_REGULAR, FONT_WEIGHT_MEDIUM, FONT_WEIGHT_BOLD, FONT_WEIGHT_HEAVY
    };

    auto result = RunBenchmark(Font_FontComplexPropertyOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_font_set_size(font, 8.0f + (i % 10) * 4.0f);
            ps_font_set_weight(font, weights[i % 4]);
            ps_font_set_italic(font, (i % 2) ? True : False);
            ps_font_set_charset(font, (i % 2) ? CHARSET_ANSI : CHARSET_UNICODE);
            ps_font_set_hint(font, (i % 2) ? True : False);
            ps_font_set_flip(font, (i % 2) ? True : False);
        }
    });

    CompareToBenchmark(Font_FontComplexPropertyOperations, result);
    ps_font_unref(font);
}

// Test 16: Context font operations
PERF_TEST_RUN(Font, FontContextOperations)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_font* fonts[4];

    fonts[0] = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    fonts[1] = ps_font_create("Arial", CHARSET_ANSI, 16.0f, FONT_WEIGHT_BOLD, False);
    fonts[2] = ps_font_create("Arial", CHARSET_ANSI, 20.0f, FONT_WEIGHT_MEDIUM, True);
    fonts[3] = ps_font_create("Arial", CHARSET_UNICODE, 24.0f, FONT_WEIGHT_HEAVY, False);

    auto result = RunBenchmark(Font_FontContextOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            for (int j = 0; j < 4; j++) {
                ps_font* oldFont = ps_set_font(ctx, fonts[j]);
                ps_font_info info;
                ps_get_font_info(ctx, &info);
                ps_set_font(ctx, oldFont);
            }
        }
    });

    CompareToBenchmark(Font_FontContextOperations, result);

    for (int i = 0; i < 4; i++) {
        ps_font_unref(fonts[i]);
    }
    ps_context_unref(ctx);
}

// Test 17: Stress test with many fonts
PERF_TEST_RUN(Font, FontStressTest)
{
    const int num_fonts = 1000;
    ps_font* fonts[num_fonts];

    auto result = RunBenchmark(Font_FontStressTest, [&]() {
        // Create many fonts
        for (int i = 0; i < num_fonts; i++) {
            fonts[i] = ps_font_create("Arial", CHARSET_ANSI, 12.0f + i * 0.01f, FONT_WEIGHT_REGULAR, False);
        }

        // Reference operations
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < num_fonts; j++) {
                ps_font_ref(fonts[j]);
            }
            for (int j = 0; j < num_fonts; j++) {
                ps_font_unref(fonts[j]);
            }
        }

        // Clean up
        for (int i = 0; i < num_fonts; i++) {
            ps_font_unref(fonts[i]);
        }
    });

    CompareToBenchmark(Font_FontStressTest, result);
}

// Test 18: Font family variations
PERF_TEST_RUN(Font, FontFamilyVariations)
{
    const char* families[] = {"Arial", "Times New Roman", "Courier New", "Verdana", "Helvetica"};

    auto result = RunBenchmark(Font_FontFamilyVariations, [&]() {
        for (int i = 0; i < 2000; i++) {
            for (int j = 0; j < 5; j++) {
                ps_font* font = ps_font_create(families[j], CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
                ps_font_unref(font);
            }
        }
    });

    CompareToBenchmark(Font_FontFamilyVariations, result);
}

// Test 19: Error handling with NULL parameters
PERF_TEST_RUN(Font, FontErrorHandling)
{
    auto result = RunBenchmark(Font_FontErrorHandling, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_font_create(NULL, CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
            ps_font_create_copy(NULL);
            ps_font_ref(NULL);
            ps_font_unref(NULL);
            ps_font_set_size(NULL, 12.0f);
            ps_font_set_weight(NULL, FONT_WEIGHT_REGULAR);
            ps_font_set_italic(NULL, False);
            ps_font_set_charset(NULL, CHARSET_ANSI);
            ps_font_set_hint(NULL, True);
            ps_font_set_flip(NULL, False);
        }
    });

    CompareToBenchmark(Font_FontErrorHandling, result);
}

// Test 20: Font reuse performance
PERF_TEST_RUN(Font, FontReusePerformance)
{
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);

    auto result = RunBenchmark(Font_FontReusePerformance, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_font_set_size(font, 8.0f + (i % 20) * 2.0f);
            ps_font_set_weight(font, FONT_WEIGHT_REGULAR + (i % 4) * 100);
            ps_font_set_italic(font, (i % 2) ? True : False);
            ps_font_set_charset(font, (i % 2) ? CHARSET_ANSI : CHARSET_UNICODE);
            ps_font_set_hint(font, (i % 2) ? True : False);
            ps_font_set_flip(font, (i % 2) ? True : False);
        }
    });

    CompareToBenchmark(Font_FontReusePerformance, result);
    ps_font_unref(font);
}
