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

PERF_TEST_DEFINE(Paint);

PERF_TEST_RUN(Paint, SetSourceColor)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};

    auto result = RunBenchmark(Paint_SetSourceColor, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_source_color(ctx, &color);
        }
    });

    CompareToBenchmark(Paint_SetSourceColor, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetSourceImage)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 50, 50);

    auto result = RunBenchmark(Paint_SetSourceImage, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_source_image(ctx, img);
        }
    });

    CompareToBenchmark(Paint_SetSourceImage, result);
    ps_context_unref(ctx);
    ps_image_unref(img);
}

PERF_TEST_RUN(Paint, SetSourcePattern)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 50, 50);
    ps_pattern* pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);

    auto result = RunBenchmark(Paint_SetSourcePattern, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_source_pattern(ctx, pattern);
        }
    });

    CompareToBenchmark(Paint_SetSourcePattern, result);
    ps_context_unref(ctx);
    ps_pattern_unref(pattern);
    ps_image_unref(img);
}

PERF_TEST_RUN(Paint, SetSourceGradient)
{
    ps_point s = {0, 0};
    ps_point e = {100, 100};
    ps_color c = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &s, &e);
    ps_gradient_add_color_stop(gradient, 0.0f, &c);

    auto result = RunBenchmark(Paint_SetSourceGradient, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_source_gradient(ctx, gradient);
        }
    });

    CompareToBenchmark(Paint_SetSourceGradient, result);
    ps_context_unref(ctx);
    ps_gradient_unref(gradient);
}

PERF_TEST_RUN(Paint, SetSourceCanvas)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 50, 50);

    auto result = RunBenchmark(Paint_SetSourceCanvas, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_source_canvas(ctx, canvas);
        }
    });

    CompareToBenchmark(Paint_SetSourceCanvas, result);
    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
}

PERF_TEST_RUN(Paint, SetLineCap)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetLineCap, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_line_cap(ctx, LINE_CAP_BUTT);
            ps_set_line_cap(ctx, LINE_CAP_ROUND);
            ps_set_line_cap(ctx, LINE_CAP_SQUARE);
        }
    });

    CompareToBenchmark(Paint_SetLineCap, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetLineJoin)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetLineJoin, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_line_join(ctx, LINE_JOIN_MITER);
            ps_set_line_join(ctx, LINE_JOIN_ROUND);
            ps_set_line_join(ctx, LINE_JOIN_BEVEL);
        }
    });

    CompareToBenchmark(Paint_SetLineJoin, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetLineInnerJoin)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetLineInnerJoin, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_line_inner_join(ctx, LINE_INNER_MITER);
            ps_set_line_inner_join(ctx, LINE_INNER_BEVEL);
        }
    });

    CompareToBenchmark(Paint_SetLineInnerJoin, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetLineWidth)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetLineWidth, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_line_width(ctx, 1.0f);
            ps_set_line_width(ctx, 2.0f);
            ps_set_line_width(ctx, 5.0f);
        }
    });

    CompareToBenchmark(Paint_SetLineWidth, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetMiterLimit)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetMiterLimit, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_miter_limit(ctx, 4.0f);
            ps_set_miter_limit(ctx, 10.0f);
        }
    });

    CompareToBenchmark(Paint_SetMiterLimit, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetStrokeColor)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};

    auto result = RunBenchmark(Paint_SetStrokeColor, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_stroke_color(ctx, &color);
        }
    });

    CompareToBenchmark(Paint_SetStrokeColor, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetStrokeImage)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 50, 50);

    auto result = RunBenchmark(Paint_SetStrokeImage, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_stroke_image(ctx, img);
        }
    });

    CompareToBenchmark(Paint_SetStrokeImage, result);
    ps_context_unref(ctx);
    ps_image_unref(img);
}

PERF_TEST_RUN(Paint, SetStrokePattern)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 50, 50);
    ps_pattern* pattern = ps_pattern_create_image(img, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, NULL);

    auto result = RunBenchmark(Paint_SetStrokePattern, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_stroke_pattern(ctx, pattern);
        }
    });

    CompareToBenchmark(Paint_SetStrokePattern, result);
    ps_context_unref(ctx);
    ps_pattern_unref(pattern);
    ps_image_unref(img);
}

PERF_TEST_RUN(Paint, SetStrokeGradient)
{
    ps_point s = {0, 0};
    ps_point e = {100, 100};
    ps_color c = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_gradient* gradient = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &s, &e);
    ps_gradient_add_color_stop(gradient, 0.0f, &c);

    auto result = RunBenchmark(Paint_SetStrokeGradient, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_stroke_gradient(ctx, gradient);
        }
    });

    CompareToBenchmark(Paint_SetStrokeGradient, result);
    ps_context_unref(ctx);
    ps_gradient_unref(gradient);
}

PERF_TEST_RUN(Paint, SetStrokeCanvas)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 50, 50);

    auto result = RunBenchmark(Paint_SetStrokeCanvas, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_stroke_canvas(ctx, canvas);
        }
    });

    CompareToBenchmark(Paint_SetStrokeCanvas, result);
    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
}

PERF_TEST_RUN(Paint, SetLineDash)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    float dashes[] = {5.0f, 2.0f, 1.0f, 2.0f};

    auto result = RunBenchmark(Paint_SetLineDash, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_line_dash(ctx, 0.0f, dashes, 4);
        }
    });

    CompareToBenchmark(Paint_SetLineDash, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, ResetLineDash)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_ResetLineDash, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_reset_line_dash(ctx);
        }
    });

    CompareToBenchmark(Paint_ResetLineDash, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetShadow)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetShadow, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_shadow(ctx, 5.0f, 5.0f, 10.0f);
        }
    });

    CompareToBenchmark(Paint_SetShadow, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetShadowColor)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_color color = {0.0f, 0.0f, 0.0f, 0.5f};

    auto result = RunBenchmark(Paint_SetShadowColor, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_shadow_color(ctx, &color);
        }
    });

    CompareToBenchmark(Paint_SetShadowColor, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, ResetShadow)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_ResetShadow, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_reset_shadow(ctx);
        }
    });

    CompareToBenchmark(Paint_ResetShadow, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetCompositeOperator)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetCompositeOperator, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_composite_operator(ctx, COMPOSITE_SRC_OVER);
            ps_set_composite_operator(ctx, COMPOSITE_SRC);
            ps_set_composite_operator(ctx, COMPOSITE_DST_OVER);
        }
    });

    CompareToBenchmark(Paint_SetCompositeOperator, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetFilter)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetFilter, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_filter(ctx, FILTER_NEAREST);
            ps_set_filter(ctx, FILTER_BILINEAR);
            ps_set_filter(ctx, FILTER_BICUBIC);
        }
    });

    CompareToBenchmark(Paint_SetFilter, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetFillRule)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetFillRule, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_fill_rule(ctx, FILL_RULE_WINDING);
            ps_set_fill_rule(ctx, FILL_RULE_EVEN_ODD);
        }
    });

    CompareToBenchmark(Paint_SetFillRule, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetAlpha)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetAlpha, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_alpha(ctx, 1.0f);
            ps_set_alpha(ctx, 0.5f);
            ps_set_alpha(ctx, 0.0f);
        }
    });

    CompareToBenchmark(Paint_SetAlpha, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetGamma)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetGamma, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_gamma(ctx, 1.0f);
            ps_set_gamma(ctx, 1.5f);
            ps_set_gamma(ctx, 2.2f);
        }
    });

    CompareToBenchmark(Paint_SetGamma, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetAntialias)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetAntialias, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_antialias(ctx, True);
            ps_set_antialias(ctx, False);
        }
    });

    CompareToBenchmark(Paint_SetAntialias, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetBlur)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SetBlur, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_blur(ctx, 0.0f);
            ps_set_blur(ctx, 2.0f);
            ps_set_blur(ctx, 5.0f);
        }
    });

    CompareToBenchmark(Paint_SetBlur, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, Stroke)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_path* path = ps_path_create();
    ps_rect rect = {10, 10, 80, 80};
    ps_path_add_rect(path, &rect);

    auto result = RunBenchmark(Paint_Stroke, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_stroke(ctx);
        }
    });

    CompareToBenchmark(Paint_Stroke, result);
    ps_path_unref(path);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, Fill)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_path* path = ps_path_create();
    ps_rect rect = {10, 10, 80, 80};
    ps_path_add_rect(path, &rect);

    auto result = RunBenchmark(Paint_Fill, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_fill(ctx);
        }
    });

    CompareToBenchmark(Paint_Fill, result);
    ps_path_unref(path);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, Paint)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_path* path = ps_path_create();
    ps_rect rect = {10, 10, 80, 80};
    ps_path_add_rect(path, &rect);

    auto result = RunBenchmark(Paint_Paint, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_paint(ctx);
        }
    });

    CompareToBenchmark(Paint_Paint, result);
    ps_path_unref(path);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, Clear)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_Clear, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_clear(ctx);
        }
    });

    CompareToBenchmark(Paint_Clear, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, Clip)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_path* path = ps_path_create();
    ps_rect rect = {10, 10, 80, 80};
    ps_path_add_rect(path, &rect);

    auto result = RunBenchmark(Paint_Clip, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_clip(ctx);
        }
    });

    CompareToBenchmark(Paint_Clip, result);
    ps_path_unref(path);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, ClipPath)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_path* path = ps_path_create();
    ps_rect rect = {10, 10, 80, 80};
    ps_path_add_rect(path, &rect);

    auto result = RunBenchmark(Paint_ClipPath, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_clip_path(ctx, path, FILL_RULE_WINDING);
        }
    });

    CompareToBenchmark(Paint_ClipPath, result);
    ps_path_unref(path);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, ClipRect)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_rect rect = {10, 10, 80, 80};

    auto result = RunBenchmark(Paint_ClipRect, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_clip_rect(ctx, &rect);
        }
    });

    CompareToBenchmark(Paint_ClipRect, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, ClipRects)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_rect rects[] = {{0, 0, 50, 50}, {50, 50, 50, 50}};

    auto result = RunBenchmark(Paint_ClipRects, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_clip_rects(ctx, rects, 2);
        }
    });

    CompareToBenchmark(Paint_ClipRects, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, ResetClip)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_ResetClip, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_reset_clip(ctx);
        }
    });

    CompareToBenchmark(Paint_ResetClip, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, ScissorRect)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_rect rect = {10, 10, 80, 80};

    auto result = RunBenchmark(Paint_ScissorRect, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_scissor_rect(ctx, &rect);
        }
    });

    CompareToBenchmark(Paint_ScissorRect, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SaveRestore)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_SaveRestore, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_save(ctx);
            ps_restore(ctx);
        }
    });

    CompareToBenchmark(Paint_SaveRestore, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, GetFontInfo)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ps_set_font(ctx, font);

    auto result = RunBenchmark(Paint_GetFontInfo, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_font_info info;
            ps_get_font_info(ctx, &info);
        }
    });

    CompareToBenchmark(Paint_GetFontInfo, result);
    ps_context_unref(ctx);
    ps_font_unref(font);
}

PERF_TEST_RUN(Paint, SetFont)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);

    auto result = RunBenchmark(Paint_SetFont, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_font* old = ps_set_font(ctx, font);
            ps_set_font(ctx, old);
        }
    });

    CompareToBenchmark(Paint_SetFont, result);
    ps_context_unref(ctx);
    ps_font_unref(font);
}

PERF_TEST_RUN(Paint, GetTextExtent)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ps_set_font(ctx, font);

    auto result = RunBenchmark(Paint_GetTextExtent, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_size extent;
            ps_get_text_extent(ctx, "Hello World", 11, &extent);
        }
    });

    CompareToBenchmark(Paint_GetTextExtent, result);
    ps_context_unref(ctx);
    ps_font_unref(font);
}

PERF_TEST_RUN(Paint, SetTextColor)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_color strokeColor = {0.0f, 1.0f, 0.0f, 1.0f};

    auto result = RunBenchmark(Paint_SetTextColor, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_text_color(ctx, &color);
            ps_set_text_stroke_color(ctx, &strokeColor);
        }
    });

    CompareToBenchmark(Paint_SetTextColor, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, TextTransform)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_matrix* matrix = ps_matrix_create();
    ps_matrix_translate(matrix, 10.0f, 10.0f);

    auto result = RunBenchmark(Paint_TextTransform, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_text_transform(ctx, matrix);
            ps_set_text_matrix(ctx, matrix);
        }
    });

    CompareToBenchmark(Paint_TextTransform, result);
    ps_context_unref(ctx);
    ps_matrix_unref(matrix);
}

PERF_TEST_RUN(Paint, TextRenderingOptions)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_TextRenderingOptions, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_text_render_type(ctx, TEXT_TYPE_SMOOTH);
            ps_set_text_render_type(ctx, TEXT_TYPE_MONO);
            ps_set_text_antialias(ctx, True);
            ps_set_text_antialias(ctx, False);
            ps_set_text_kerning(ctx, True);
            ps_set_text_kerning(ctx, False);
        }
    });

    CompareToBenchmark(Paint_TextRenderingOptions, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, TextOutLength)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ps_set_font(ctx, font);

    auto result = RunBenchmark(Paint_TextOutLength, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_text_out_length(ctx, 10.0f, 10.0f, "Hello World", 11);
        }
    });

    CompareToBenchmark(Paint_TextOutLength, result);
    ps_context_unref(ctx);
    ps_font_unref(font);
}

PERF_TEST_RUN(Paint, WideTextOutLength)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ps_set_font(ctx, font);
    const ps_uchar16 text[] = {0x4F60, 0x597D, 0x4E16, 0x754C};

    auto result = RunBenchmark(Paint_WideTextOutLength, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_wide_text_out_length(ctx, 10.0f, 10.0f, text, 4);
        }
    });

    CompareToBenchmark(Paint_WideTextOutLength, result);
    ps_context_unref(ctx);
    ps_font_unref(font);
}

PERF_TEST_RUN(Paint, DrawText)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ps_set_font(ctx, font);
    ps_rect area = {0, 0, 100, 50};

    auto result = RunBenchmark(Paint_DrawText, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_draw_text(ctx, &area, "Hello World", 11, DRAW_TEXT_FILL, TEXT_ALIGN_LEFT);
        }
    });

    CompareToBenchmark(Paint_DrawText, result);
    ps_context_unref(ctx);
    ps_font_unref(font);
}

PERF_TEST_RUN(Paint, GetGlyph)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ps_set_font(ctx, font);

    auto result = RunBenchmark(Paint_GetGlyph, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_glyph glyph;
            ps_get_glyph(ctx, 'A', &glyph);
        }
    });

    CompareToBenchmark(Paint_GetGlyph, result);
    ps_context_unref(ctx);
    ps_font_unref(font);
}

PERF_TEST_RUN(Paint, ShowGlyphs)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ps_set_font(ctx, font);

    ps_glyph glyphs[5];
    for (int i = 0; i < 5; i++) {
        ps_get_glyph(ctx, "HELLO"[i], &glyphs[i]);
    }

    auto result = RunBenchmark(Paint_ShowGlyphs, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_show_glyphs(ctx, 10.0f, 10.0f, glyphs, 5);
        }
    });

    CompareToBenchmark(Paint_ShowGlyphs, result);
    ps_context_unref(ctx);
    ps_font_unref(font);
}

PERF_TEST_RUN(Paint, GetPathFromGlyph)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ps_set_font(ctx, font);
    ps_glyph glyph;
    ps_get_glyph(ctx, 'A', &glyph);

    auto result = RunBenchmark(Paint_GetPathFromGlyph, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_path* path = ps_path_create();
            ps_get_path_from_glyph(ctx, &glyph, path);
            ps_path_unref(path);
        }
    });

    CompareToBenchmark(Paint_GetPathFromGlyph, result);
    ps_context_unref(ctx);
    ps_font_unref(font);
}

PERF_TEST_RUN(Paint, GlyphGetExtent)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_font* font = ps_font_create("Arial", CHARSET_ANSI, 12.0f, FONT_WEIGHT_REGULAR, False);
    ps_set_font(ctx, font);
    ps_glyph glyph;
    ps_get_glyph(ctx, 'A', &glyph);

    auto result = RunBenchmark(Paint_GlyphGetExtent, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_size extent;
            ps_glyph_get_extent(&glyph, &extent);
        }
    });

    CompareToBenchmark(Paint_GlyphGetExtent, result);
    ps_context_unref(ctx);
    ps_font_unref(font);
}

PERF_TEST_RUN(Paint, Translate)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_Translate, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_translate(ctx, 10.0f, 20.0f);
            ps_translate(ctx, -10.0f, -20.0f);
        }
    });

    CompareToBenchmark(Paint_Translate, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, Scale)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_Scale, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_scale(ctx, 2.0f, 2.0f);
            ps_scale(ctx, 0.5f, 0.5f);
        }
    });

    CompareToBenchmark(Paint_Scale, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, Shear)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Paint_Shear, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_shear(ctx, 0.5f, 0.3f);
            ps_shear(ctx, -0.5f, -0.3f);
        }
    });

    CompareToBenchmark(Paint_Shear, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Paint, SetPath)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    ps_path* path1 = ps_path_create();
    ps_path* path2 = ps_path_create();
    ps_path* path3 = ps_path_create();

    ps_rect rect1 = {10, 10, 50, 50};
    ps_rect rect2 = {20, 20, 60, 60};
    ps_rect rect3 = {30, 30, 70, 70};

    ps_path_add_rect(path1, &rect1);
    ps_path_add_rect(path2, &rect2);
    ps_path_add_rect(path3, &rect3);

    auto result = RunBenchmark(Paint_SetPath, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_set_path(ctx, path1);
            ps_set_path(ctx, path2);
            ps_set_path(ctx, path3);
        }
    });

    CompareToBenchmark(Paint_SetPath, result);

    ps_path_unref(path1);
    ps_path_unref(path2);
    ps_path_unref(path3);
    ps_context_unref(ctx);
}
