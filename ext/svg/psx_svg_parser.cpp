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

#include <math.h>
#include "psx_svg_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef M_PI
#define M_PI 3.1415926f
#endif

#define MAP_LEN(m) sizeof((m)) / sizeof((m[0]))
#define DEFAULT_DPI  (96)
#define ATTR_VALUE_LEN(attr) ((uint32_t)((attr)->value_end - (attr)->value_start))
#define BUF_LEN(s, e) ((uint32_t)((e) - (s)))

static const struct {
    const char* name;
    uint32_t name_len;
    psx_svg_tag tag;
} _svg_tag_map[] = {
    {"svg", 3, SVG_TAG_SVG},
    {"use", 3, SVG_TAG_USE},
    {"g", 1, SVG_TAG_G},
    {"path", 4, SVG_TAG_PATH},
    {"rect", 4, SVG_TAG_RECT},
    {"circle", 6, SVG_TAG_CIRCLE},
    {"ellipse", 7, SVG_TAG_ELLIPSE},
    {"line", 4, SVG_TAG_LINE},
    {"polyline", 8, SVG_TAG_POLYLINE},
    {"polygon", 7, SVG_TAG_POLYGON},
    {"solidColor", 10, SVG_TAG_SOLID_COLOR},
    {"linearGradient", 14, SVG_TAG_LINEAR_GRADIENT},
    {"radialGradient", 14, SVG_TAG_RADIAL_GRADIENT},
    {"stop", 4, SVG_TAG_STOP},
    {"defs", 4, SVG_TAG_DEFS},
    {"image", 5, SVG_TAG_IMAGE},
    {"mpath", 5, SVG_TAG_MPATH},
    {"set", 3, SVG_TAG_SET},
    {"animate", 7, SVG_TAG_ANIMATE},
    {"animateColor", 12, SVG_TAG_ANIMATE_COLOR},
    {"animateTransform", 16, SVG_TAG_ANIMATE_TRANSFORM},
    {"animateMotion", 13, SVG_TAG_ANIMATE_MOTION},
    {"text", 4, SVG_TAG_TEXT},
    {"tspan", 5, SVG_TAG_TSPAN},
    {"textArea", 8, SVG_TAG_TEXT_AREA},
    {"tbreak", 6, SVG_TAG_TBREAK},
};

static const struct {
    const char* name;
    uint32_t name_len;
    psx_svg_attr_type attr;
} _svg_attr_map[] = {
    {"id", 2, SVG_ATTR_ID},
    {"xml:id", 6, SVG_ATTR_XML_ID},
    {"version", 7, SVG_ATTR_VERSION},
    {"baseProfile", 11, SVG_ATTR_BASE_PROFILE},
    {"viewBox", 7, SVG_ATTR_VIEWBOX},
    {"preserveAspectRatio", 19, SVG_ATTR_PRESERVE_ASPECT_RATIO},
    {"viewport-fill", 13, SVG_ATTR_VIEWPORT_FILL},
    {"viewport-fill-opacity", 21, SVG_ATTR_VIEWPORT_FILL_OPACITY},
    {"display", 7, SVG_ATTR_DISPLAY},
    {"visibility", 10, SVG_ATTR_VISIBILITY},
    {"x", 1, SVG_ATTR_X},
    {"y", 1, SVG_ATTR_Y},
    {"width", 5, SVG_ATTR_WIDTH},
    {"height", 6, SVG_ATTR_HEIGHT},
    {"rx", 2, SVG_ATTR_RX},
    {"ry", 2, SVG_ATTR_RY},
    {"cx", 2, SVG_ATTR_CX},
    {"cy", 2, SVG_ATTR_CY},
    {"r", 1, SVG_ATTR_R},
    {"x1", 2, SVG_ATTR_X1},
    {"y1", 2, SVG_ATTR_Y1},
    {"x2", 2, SVG_ATTR_X2},
    {"y2", 2, SVG_ATTR_Y2},
    {"points", 6, SVG_ATTR_POINTS},
    {"d", 1, SVG_ATTR_D},
    {"pathLength", 10, SVG_ATTR_PATH_LENGTH},
    {"xlink:href", 10, SVG_ATTR_XLINK_HREF},
    {"fill", 4, SVG_ATTR_FILL},
    {"fill-rule", 9, SVG_ATTR_FILL_RULE},
    {"fill-opacity", 12, SVG_ATTR_FILL_OPACITY},
    {"stroke", 6, SVG_ATTR_STROKE},
    {"stroke-width", 12, SVG_ATTR_STROKE_WIDTH},
    {"stroke-linecap", 14, SVG_ATTR_STROKE_LINECAP},
    {"stroke-linejoin", 15, SVG_ATTR_STROKE_LINEJOIN},
    {"stroke-miterlimit", 17, SVG_ATTR_STROKE_MITER_LIMIT},
    {"stroke-dasharray", 16, SVG_ATTR_STROKE_DASH_ARRAY},
    {"stroke-dashoffset", 17, SVG_ATTR_STROKE_DASH_OFFSET},
    {"stroke-opacity", 14, SVG_ATTR_STROKE_OPACITY},
    {"opacity", 7, SVG_ATTR_OPACITY},
    {"solid-color", 11, SVG_ATTR_SOLID_COLOR},
    {"solid-opacity", 13, SVG_ATTR_SOLID_OPACITY},
    {"gradientUnits", 13, SVG_ATTR_GRADIENT_UNITS},
    {"offset", 6, SVG_ATTR_GRADIENT_STOP_OFFSET},
    {"stop-color", 10, SVG_ATTR_GRADIENT_STOP_COLOR},
    {"stop-opacity", 12, SVG_ATTR_GRADIENT_STOP_OPACITY},
    {"font-family", 11, SVG_ATTR_FONT_FAMILY},
    {"font-style", 10, SVG_ATTR_FONT_STYLE},
    {"font-variant", 12, SVG_ATTR_FONT_VARIANT},
    {"font-weight", 11, SVG_ATTR_FONT_WEIGHT},
    {"font-size", 9, SVG_ATTR_FONT_SIZE},
    {"transform", 9, SVG_ATTR_TRANSFORM},
    {"text-anchor", 11, SVG_ATTR_TEXT_ANCHOR},
    {"attributeName", 13, SVG_ATTR_ATTRIBUTE_NAME},
    {"attributeType", 13, SVG_ATTR_ATTRIBUTE_TYPE},
    {"begin", 5, SVG_ATTR_BEGIN},
    {"end", 3, SVG_ATTR_END},
    {"dur", 3, SVG_ATTR_DUR},
    {"min", 3, SVG_ATTR_MIN},
    {"max", 3, SVG_ATTR_MAX},
    {"restart", 7, SVG_ATTR_RESTART},
    {"repeatCount", 11, SVG_ATTR_REPEAT_COUNT},
    {"repeatDur", 9, SVG_ATTR_REPEAT_DUR},
    {"calcMode", 8, SVG_ATTR_CALC_MODE},
    {"values", 6, SVG_ATTR_VALUES},
    {"keyTimes", 8, SVG_ATTR_KEY_TIMES},
    {"keySplines", 10, SVG_ATTR_KEY_SPLINES},
    {"keyPoints", 9, SVG_ATTR_KEY_POINTS},
    {"from", 4, SVG_ATTR_FROM},
    {"to", 2, SVG_ATTR_TO},
    {"by", 2, SVG_ATTR_BY},
    {"additive", 8, SVG_ATTR_ADDITIVE},
    {"accumulate", 10, SVG_ATTR_ACCUMULATE},
    {"path", 4, SVG_ATTR_PATH},
    {"rotate", 6, SVG_ATTR_ROTATE},
    {"type", 4, SVG_ATTR_TRANSFORM_TYPE},
};

static const struct {
    const char* name;
    uint32_t align;
} _svg_attr_aspect_ratio_map[] = {
    {"xMinYMin", SVG_ASPECT_RATIO_XMIN_YMIN},
    {"xMidYMin", SVG_ASPECT_RATIO_XMID_YMIN},
    {"xMaxYMin", SVG_ASPECT_RATIO_XMAX_YMIN},
    {"xMinYMid", SVG_ASPECT_RATIO_XMIN_YMID},
    {"xMidYMid", SVG_ASPECT_RATIO_XMID_YMID},
    {"xMaxYMid", SVG_ASPECT_RATIO_XMAX_YMID},
    {"xMinYMax", SVG_ASPECT_RATIO_XMIN_YMAX},
    {"xMidYMax", SVG_ASPECT_RATIO_XMID_YMAX},
    {"xMaxYMax", SVG_ASPECT_RATIO_XMAX_YMAX},
};

static const struct {
    const char* name;
    uint32_t name_len;
    uint32_t color;
} _svg_color_map[] = {
    {"aliceblue", 9, 0xf0f8ff},
    {"antiquewhite", 12, 0xfaebd7},
    {"aqua", 4, 0x00ffff},
    {"aquamarine", 10, 0x7fffd4},
    {"azure", 5, 0xf0ffff},
    {"beige", 5, 0xf5f5dc},
    {"bisque", 6, 0xffe4c4},
    {"black", 5, 0x000000},
    {"blanchedalmond", 14, 0xffebcd},
    {"blue", 4, 0x0000ff},
    {"blueviolet", 10, 0x8a2be2},
    {"brown", 5, 0xa52a2a},
    {"burlywood", 9, 0xdeb887},
    {"cadetblue", 9, 0x5f9ea0},
    {"chartreuse", 10, 0x7fff00},
    {"chocolate", 9, 0xd2691e},
    {"coral", 5, 0xff7f50},
    {"cornflowerblue", 14, 0x6495ed},
    {"cornsilk", 8, 0xfff8dc},
    {"crimson", 7, 0xdc143c},
    {"cyan", 4, 0x00ffff},
    {"darkblue", 8, 0x00008b},
    {"darkcyan", 8, 0x008b8b},
    {"darkgoldenrod", 13, 0xb8860b},
    {"darkgray", 8, 0xa9a9a9},
    {"darkgrey", 8, 0xa9a9a9},
    {"darkgreen", 9, 0x006400},
    {"darkkhaki", 9, 0xbdb76b},
    {"darkmagenta", 11, 0x8b008b},
    {"darkolivegreen", 14, 0x556b2f},
    {"darkorange", 10, 0xff8c00},
    {"darkorchid", 10, 0x9932cc},
    {"darkred", 7, 0x8b0000},
    {"darksalmon", 10, 0xe9967a},
    {"darkseagreen", 12, 0x8fbc8f},
    {"darkslateblue", 13, 0x483d8b},
    {"darkslategray", 13, 0x2f4f4f},
    {"darkslategrey", 13, 0x2f4f4f},
    {"darkturquoise", 13, 0x00ced1},
    {"darkviolet", 10, 0x9400d3},
    {"deeppink", 8, 0xff1493},
    {"deepskyblue", 11, 0x00bfff},
    {"dimgray", 7, 0x696969},
    {"dimgrey", 7, 0x696969},
    {"dodgerblue", 10, 0x1e90ff},
    {"firebrick", 9, 0xb22222},
    {"floralwhite", 11, 0xfffaf0},
    {"forestgreen", 11, 0x228b22},
    {"fuchsia", 7, 0xff00ff},
    {"gainsboro", 9, 0xdcdcdc},
    {"ghostwhite", 10, 0xf8f8ff},
    {"gold", 4, 0xffd700},
    {"goldenrod", 9, 0xdaa520},
    {"gray", 4, 0x808080},
    {"grey", 4, 0x808080},
    {"green", 5, 0x008000},
    {"greenyellow", 11, 0xadff2f},
    {"honeydew", 8, 0xf0fff0},
    {"hotpink", 7, 0xff69b4},
    {"indianred", 9, 0xcd5c5c},
    {"indigo", 6, 0x4b0082},
    {"ivory", 5, 0xfffff0},
    {"khaki", 5, 0xf0e68c},
    {"lavender", 8, 0xe6e6fa},
    {"lavenderblush", 13, 0xfff0f5},
    {"lawngreen", 9, 0x7cfc00},
    {"lemonchiffon", 12, 0xfffacd},
    {"lightblue", 9, 0xadd8e6},
    {"lightcoral", 10, 0xf08080},
    {"lightcyan", 9, 0xe0ffff},
    {"lightgoldenrodyellow", 20, 0xfafad2},
    {"lightgray", 9, 0xd3d3d3},
    {"lightgrey", 9, 0xd3d3d3},
    {"lightgreen", 10, 0x90ee90},
    {"lightpink", 9, 0xffb6c1},
    {"lightsalmon", 11, 0xffa07a},
    {"lightseagreen", 13, 0x20b2aa},
    {"lightskyblue", 12, 0x87cefa},
    {"lightslategray", 14, 0x778899},
    {"lightslategrey", 14, 0x778899},
    {"lightsteelblue", 14, 0xb0c4de},
    {"lightyellow", 11, 0xffffe0},
    {"lime", 4, 0x00ff00},
    {"limegreen", 9, 0x32cd32},
    {"linen", 5, 0xfaf0e6},
    {"magenta", 7, 0xff00ff},
    {"maroon", 6, 0x800000},
    {"mediumaquamarine", 16, 0x66cdaa},
    {"mediumblue", 10, 0x0000cd},
    {"mediumorchid", 12, 0xba55d3},
    {"mediumpurple", 12, 0x9370d8},
    {"mediumseagreen", 14, 0x3cb371},
    {"mediumslateblue", 15, 0x7b68ee},
    {"mediumspringgreen", 17, 0x00fa9a},
    {"mediumturquoise", 15, 0x48d1cc},
    {"mediumvioletred", 15, 0xc71585},
    {"midnightblue", 12, 0x191970},
    {"mintcream", 9, 0xf5fffa},
    {"mistyrose", 9, 0xffe4e1},
    {"moccasin", 8, 0xffe4b5},
    {"navajowhite", 11, 0xffdead},
    {"navy", 4, 0x000080},
    {"oldlace", 7, 0xfdf5e6},
    {"olive", 5, 0x808000},
    {"olivedrab", 9, 0x6b8e23},
    {"orange", 6, 0xffa500},
    {"orangered", 9, 0xff4500},
    {"orchid", 6, 0xda70d6},
    {"palegoldenrod", 13, 0xeee8aa},
    {"palegreen", 9, 0x98fb98},
    {"paleturquoise", 13, 0xafeeee},
    {"palevioletred", 13, 0xd87093},
    {"papayawhip", 10, 0xffefd5},
    {"peachpuff", 9, 0xffdab9},
    {"peru", 4, 0xcd853f},
    {"pink", 4, 0xffc0cb},
    {"plum", 4, 0xdda0dd},
    {"powderblue", 10, 0xb0e0e6},
    {"purple", 6, 0x800080},
    {"red", 3, 0xff0000},
    {"rosybrown", 9, 0xbc8f8f},
    {"royalblue", 9, 0x4169e1},
    {"saddlebrown", 11, 0x8b4513},
    {"salmon", 6, 0xfa8072},
    {"sandybrown", 10, 0xf4a460},
    {"seagreen", 8, 0x2e8b57},
    {"seashell", 8, 0xfff5ee},
    {"sienna", 6, 0xa0522d},
    {"silver", 6, 0xc0c0c0},
    {"skyblue", 7, 0x87ceeb},
    {"slateblue", 9, 0x6a5acd},
    {"slategray", 9, 0x708090},
    {"slategrey", 9, 0x708090},
    {"snow", 4, 0xfffafa},
    {"springgreen", 11, 0x00ff7f},
    {"steelblue", 9, 0x4682b4},
    {"tan", 3, 0xd2b48c},
    {"teal", 4, 0x008080},
    {"thistle", 7, 0xd8bfd8},
    {"tomato", 6, 0xff6347},
    {"turquoise", 9, 0x40e0d0},
    {"violet", 6, 0xee82ee},
    {"wheat", 5, 0xf5deb3},
    {"white", 5, 0xffffff},
    {"whitesmoke", 10, 0xf5f5f5},
    {"yellow", 6, 0xffff00},
    {"yellowgreen", 11, 0x9acd32},
};

static INLINE const char* _skip_space(const char* str, const char* str_end)
{
    while ((str < str_end) && isspace(*str)) {
        ++str;
    }
    return str;
}

static INLINE bool _is_separators(char c)
{
    return c == ',' || c == '\t' || c == '\n' || c == '\r';
}

static INLINE const char* _skip_space_and_separators(const char* str, const char* str_end)
{
    while ((str < str_end) && (isspace(*str) || _is_separators(*str))) {
        ++str;
    }
    return str;
}

static INLINE bool _is_number_begin(char ch)
{
    return ch != 0 && strchr("0123456789+-.", ch) != NULL;
}

static INLINE const char* _parse_number(const char* str, const char* str_end, float* val)
{
    if (!str) {
        return NULL;
    }
    // skip loading
    while ((str < str_end) && !_is_number_begin(*str)) {
        ++str;
    }

    if (str == str_end) { // parse fail
        return NULL;
    }

    char* end = NULL;
    *val = strtof(str, &end);
    return end;
}

static INLINE const char* _parse_length(const char* str, const char* str_end, int32_t dpi, float* val)
{
    str = _parse_number(str, str_end, val);
    if (str) {
        uint32_t len = BUF_LEN(str, str_end);
        if (len > 0) {
            if (len == 1 && (*str == '%')) {
                // percentage
                *val *= 0.01f;
            } else if (len == 2) {
                if (str[0] == 'p' && str[1] == 't') { // pt
                    *val = *val / 72.0f * (float)dpi;
                } else if (str[0] == 'p' && str[1] == 'c') { // pc
                    *val = *val / 6.0f * (float)dpi;
                } else if (str[0] == 'i' && str[1] == 'n') { // in
                    *val = *val * (float)dpi;
                } else if (str[0] == 'm' && str[1] == 'm') { // mm
                    *val = *val / 25.4f * (float)dpi;
                } else if (str[0] == 'c' && str[1] == 'm') { // cm
                    *val = *val / 2.54f * (float)dpi;
                } else if (str[0] == 'e' && str[1] == 'm') { // em
                    *val = *val * 16.0f; // most web browser default font size
                } else if (str[0] == 'e' && str[1] == 'x') { // ex
                    *val = *val * 16.0f * 0.52f;
                }
            }
        }
        str += len;
    }
    return str;
}

static INLINE const char* _parse_clock_time(const char* str, const char* str_end, float* val)
{
    str = _parse_number(str, str_end, val);
    if (str) {
        uint32_t len = BUF_LEN(str, str_end);
        if (len > 0) {
            if (len >= 2 && str[0] == 'm' && str[1] == 's') {
                *val = roundf(*val);
            } else {
                *val = roundf(*val * 1000.0f);
            }
        } else {
            *val = roundf(*val * 1000.0f);
        }
        str += len;
        return str;
    }
    *val = roundf(*val * 1000.0f);
    return str;
}

static INLINE const char* _parse_color(const char* str, const char* str_end, uint32_t* val)
{
    if (!str) {
        return NULL;
    }

    const char* ptr = str;
    while ((ptr < str_end) && (*ptr != ')')) { // calc letters end
        ++ptr;
    }

    uint32_t len = BUF_LEN(str, ptr);
    uint8_t r = 0, g = 0, b = 0;

    if (*str == '#') {
        if (len == 4) { // three digit hex format '#rgb'
            if (isxdigit(str[1]) && isxdigit(str[2]) && isxdigit(str[3])) {
                char st[3] = {0};
                st[0] = st[1] = str[1];
                r = (uint8_t)strtol(st, NULL, 16);
                st[0] = st[1] = str[2];
                g = (uint8_t)strtol(st, NULL, 16);
                st[0] = st[1] = str[3];
                b = (uint8_t)strtol(st, NULL, 16);
            }
        } else if (len == 7) { // six digit hex format '#rrggbb'
            if (isxdigit(str[1]) && isxdigit(str[2]) && isxdigit(str[3])
                && isxdigit(str[4]) && isxdigit(str[5]) && isxdigit(str[6])) {
                char st[3] = {0};
                st[0] = str[1];
                st[1] = str[2];
                r = (uint8_t)strtol(st, NULL, 16);
                st[0] = str[3];
                st[1] = str[4];
                g = (uint8_t)strtol(st, NULL, 16);
                st[0] = str[5];
                st[1] = str[6];
                b = (uint8_t)strtol(st, NULL, 16);
            }
        }
        // make color
        *val = (r << 16) + (g << 8) + b;
    } else if (len > 5 && strncmp(str, "rgba(", 5) == 0) {
        str += 5;
        bool valid_color = true;
        float vals[3] = {0};
        uint8_t alpha = 255;

        for (int i = 0; i < 3; i++) {
            str = _parse_number(str, ptr, &vals[i]);
            if (!str) { valid_color = false; }

            if (*str == '%') {
                vals[i] *= 2.56f;
            }
        }

        float a = 0.0f;
        str = _parse_number(str, ptr, &a);
        if (str) {
            if (*str == '%') {
                a *= 2.56f;
            } else if (a >= 0.0f && a <= 1.0f) {
                a *= 255.0f;
            }
            alpha = (uint8_t)a;
        }

        if (valid_color) {
            r = (uint8_t)vals[0];
            g = (uint8_t)vals[1];
            b = (uint8_t)vals[2];
        }
        // make color
        *val = (alpha << 24) + (r << 16) + (g << 8) + b;
    } else if (len > 4 && strncmp(str, "rgb(", 4) == 0) {
        str += 4;
        bool valid_color = true;
        float vals[3] = {0};

        for (int i = 0; i < 3; i++) {
            str = _parse_number(str, ptr, &vals[i]);
            if (!str) { valid_color = false; }

            if (*str == '%') {
                vals[i] *= 2.56f;
            }
        }

        if (valid_color) {
            r = (uint8_t)vals[0];
            g = (uint8_t)vals[1];
            b = (uint8_t)vals[2];
        }
        // make color
        *val = (r << 16) + (g << 8) + b;
    } else { // color keyword
        uint32_t map_len = MAP_LEN(_svg_color_map);
        for (uint32_t i = 0; i < map_len; i++) {
            if (len == _svg_color_map[i].name_len && strncmp(_svg_color_map[i].name, str, len) == 0) {
                *val = _svg_color_map[i].color;
            }
        }
    }
    return ++ptr;
}

static INLINE const char* _parse_matrix(const char* str, const char* str_end, uint32_t type,
                                        ps_matrix* matrix)
{
    // skip loading
    while ((str < str_end) && *str != '(') {
        ++str;
    }

    if (str == str_end) { // parse fail
        return str;
    }

    const char* ptr = str;
    switch (type) {
        case SVG_TRANSFORM_TYPE_MATRIX: {
                float vals[6] = {0};
                for (int i = 0; i < 6; i++) {
                    ptr = _parse_number(ptr, str_end, &vals[i]);
                    if (!ptr) {
                        return str;
                    }
                    str = ptr;
                }

                ps_matrix_init(matrix, vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
            }
            break;
        case SVG_TRANSFORM_TYPE_TRANSLATE: {
                float tx = 0.0f, ty = 0.0f;
                ptr = _parse_number(ptr, str_end, &tx);
                if (!ptr) {
                    return str;
                }
                str = ptr;

                ptr = _skip_space(ptr, str_end);
                if (*ptr != ')') {
                    ptr = _parse_number(ptr, str_end, &ty);
                    if (ptr) {
                        str = ptr;
                    }
                }

                ps_matrix_translate(matrix, tx, ty);
            }
            break;
        case SVG_TRANSFORM_TYPE_ROTATE: {
                float degree = 0.0f, cx = 0.0f, cy = 0.0f;
                bool trans = false;

                ptr = _parse_number(ptr, str_end, &degree);
                if (!ptr) {
                    return str;
                }
                str = ptr;

                ptr = _skip_space(ptr, str_end);
                if (*ptr != ')') {
                    ptr = _parse_number(ptr, str_end, &cx);
                    ptr = _parse_number(ptr, str_end, &cy);
                    if (ptr) {
                        trans = true;
                        str = ptr;
                    }
                }

                float radian = degree / 180.0f * (float)M_PI;
                if (!trans) {
                    ps_matrix_rotate(matrix, radian);
                } else {
                    ps_matrix_translate(matrix, -cx, -cy);
                    ps_matrix_rotate(matrix, radian);
                    ps_matrix_translate(matrix, cx, cy);
                }
            }
            break;
        case SVG_TRANSFORM_TYPE_SCALE: {
                float sx = 0.0f, sy = 0.0f;
                ptr = _parse_number(ptr, str_end, &sx);
                if (!ptr) {
                    return str;
                }
                str = ptr;

                sy = sx;

                ptr = _skip_space(ptr, str_end);
                if (*ptr != ')') {
                    ptr = _parse_number(ptr, str_end, &sy);
                    if (ptr) {
                        str = ptr;
                    }
                }
                ps_matrix_scale(matrix, sx, sy);
            }
            break;
        case SVG_TRANSFORM_TYPE_SKEW_X: {
                float degree = 0.0f;
                ptr = _parse_number(ptr, str_end, &degree);
                if (!ptr) {
                    return str;
                }
                str = ptr;

                float radian = degree / 180.0f * (float)M_PI;
                ps_matrix_shear(matrix, radian, 0);
            }
            break;
        case SVG_TRANSFORM_TYPE_SKEW_Y: {
                float degree = 0.0f;
                ptr = _parse_number(ptr, str_end, &degree);
                if (!ptr) {
                    return str;
                }
                str = ptr;

                float radian = degree / 180.0f * (float)M_PI;
                ps_matrix_shear(matrix, 0, radian);
            }
            break;
    }
    return str;
}

static INLINE psx_svg_tag _get_svg_tag_type(const psx_xml_token* token)
{
    uint32_t len = MAP_LEN(_svg_tag_map);
    uint32_t token_len = TOKEN_LEN(token);

    for (uint32_t i = 0; i < len; i++) {
        if (token_len == _svg_tag_map[i].name_len && strncmp(_svg_tag_map[i].name, token->start, token_len) == 0) {
            return _svg_tag_map[i].tag;
        }
    }
    return SVG_TAG_INVALID;
}

static INLINE psx_svg_attr_type _get_svg_attr_type(const char* attr_start, const char* attr_end)
{
    uint32_t len = MAP_LEN(_svg_attr_map);
    uint32_t attr_len = BUF_LEN(attr_start, attr_end);

    for (uint32_t i = 0; i < len; i++) {
        if (attr_len == _svg_attr_map[i].name_len && strncmp(_svg_attr_map[i].name, attr_start, attr_len) == 0) {
            return _svg_attr_map[i].attr;
        }
    }
    return SVG_ATTR_INVALID;
}

static INLINE void _process_string(psx_svg_node* node, psx_svg_attr_type type, const char* val_start, const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_PTR;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t len = BUF_LEN(val_start, val_end);
    char* str = (char*)mem_malloc(len + 1);
    memcpy(str, val_start, len);
    str[len] = '\0';
    attr->value.sval = str;
}

static INLINE void _process_xlink(psx_svg_node* node, psx_svg_attr_type type, const char* val_start, const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_PTR;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    if (*val_start == '#') {
        val_start++;
    }

    uint32_t len = BUF_LEN(val_start, val_end);
    char* str = (char*)mem_malloc(len + 1);
    memcpy(str, val_start, len);
    str[len] = '\0';
    attr->value.sval = str;
}

static INLINE void _process_view_box(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                     const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_PTR;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t len = BUF_LEN(val_start, val_end);
    if (len >= 4 && strncmp(val_start, "none", 4) == 0) {
        attr->val_type = SVG_ATTR_VALUE_DATA;
        attr->class_type = SVG_ATTR_VALUE_NONE;
        attr->value.ival = 0;
        return;
    }

    float* vals = (float*)mem_malloc(sizeof(float) * 4);
    memset(vals, 0, sizeof(float) * 4);
    const char* ptr = val_start;
    for (int i = 0; i < 4; i++) {
        ptr = _parse_number(ptr, val_end, &vals[i]);
        if (!ptr) {
            attr->val_type = SVG_ATTR_VALUE_DATA;
            attr->class_type = SVG_ATTR_VALUE_NONE;
            mem_free(vals);
            attr->value.ival = 0;
            return;
        }
    }
    attr->value.val = vals;
}

static INLINE void _process_preserve_aspect_ratio(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                                  const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t ratio = SVG_ASPECT_RATIO_XMID_YMID;
    uint32_t len = MAP_LEN(_svg_attr_aspect_ratio_map);

    for (uint32_t i = 0; i < len; i++) {
        if (strncmp(_svg_attr_aspect_ratio_map[i].name, val_start, 8) == 0) {
            ratio = _svg_attr_aspect_ratio_map[i].align;
            val_start += 8;
            break;
        } else if (strncmp("none", val_start, 4) == 0) {
            ratio = SVG_ASPECT_RATIO_NONE;
            val_start += 4;
            break;
        }
    }

    if (ratio != SVG_ASPECT_RATIO_NONE) {
        len = BUF_LEN(val_start, val_end);
        if (len > 4) {
            val_start = _skip_space(val_start, val_end);
            if (strncmp(val_start, "meet", 4) == 0) {
                ratio |= SVG_ASPECT_RATIO_OPT_MEET;
            } else if (strncmp(val_start, "slice", 5) == 0) {
                ratio |= SVG_ASPECT_RATIO_OPT_SLICE;
            }
        }
    }
    attr->value.uval = ratio;
}

static INLINE void _process_opacity_value(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                          const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t len = BUF_LEN(val_start, val_end);
    if (len >= 7 && strncmp(val_start, "inherit", 7) == 0) {
        attr->class_type = SVG_ATTR_VALUE_INHERIT;
        return;
    }

    float val = 1.0f;
    val_start = _parse_number(val_start, val_end, &val);

    if (val < 0.0f) {
        val = 0.0f;
    } else if (val > 1.0f) {
        val = 1.0f;
    }
    attr->value.fval = val;
}

static INLINE void _process_length_value(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                         const char* val_end, int32_t dpi)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    float val = 0.0f;
    val_start = _parse_length(val_start, val_end, dpi, &val);
    attr->value.fval = val;
}

static INLINE void _process_points_value(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                         const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_PTR;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t list_cap = 4;
    psx_svg_attr_values_list* list = (psx_svg_attr_values_list*)mem_malloc(sizeof(psx_svg_point) * list_cap + sizeof(uint32_t));

    float val = 0.0f;
    const char* ptr = val_start;
    uint32_t point_cnt = 0;

    while (ptr < val_end) {
        if (point_cnt == list_cap) {
            list_cap = list_cap << 1;
            list = (psx_svg_attr_values_list*)mem_realloc(list, sizeof(psx_svg_point) * list_cap + sizeof(uint32_t));
        }
        psx_svg_point* pt = (psx_svg_point*)(&list->data) + point_cnt;
        val = 0.0f;
        ptr = _parse_number(ptr, val_end, &val);
        pt->x = val;
        val = 0.0f;
        ptr = _parse_number(ptr, val_end, &val);
        pt->y = val;
        if (!ptr) {
            break;
        }
        ++point_cnt;
    }

    list->length = point_cnt;
    attr->value.val = list;
}

static INLINE void _process_gradient_units(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                           const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t len = BUF_LEN(val_start, val_end);
    int32_t val = 0;

    if (len == 14 && strncmp(val_start, "userSpaceOnUse", 14) == 0) {
        val = SVG_GRADIENT_UNITS_USER_SPACE;
    } else {
        val = SVG_GRADIENT_UNITS_OBJECT;
    }
    attr->value.ival = val;
}

static INLINE void _process_clock_time(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                       const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t len = BUF_LEN(val_start, val_end);
    if (len == 10 && strncmp(val_start, "indefinite", 10) == 0) {
        attr->value.fval = 0.0f;
        return;
    }

    float val = 0.0f;
    val_start = _parse_clock_time(val_start, val_end, &val);
    attr->value.fval = val; // ms
}

static INLINE void _process_paint(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                  const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t len = BUF_LEN(val_start, val_end);
    if (len >= 4 && strncmp(val_start, "none", 4) == 0) {
        attr->class_type = SVG_ATTR_VALUE_NONE;
        attr->value.ival = 0;
        return;
    } else if (len >= 7 && strncmp(val_start, "inherit", 7) == 0) {
        attr->class_type = SVG_ATTR_VALUE_INHERIT;
        attr->value.ival = 0;
        return;
    } else if (len > 4 && strncmp(val_start, "url(", 4) == 0) {
        // parse url
        const char* ptr = val_start + 4;
        const char* url_start = NULL;
        const char* url_end = NULL;

        ptr = _skip_space(ptr, val_end);
        if (ptr == val_end) {
            attr->class_type = SVG_ATTR_VALUE_NONE;
            attr->value.ival = 0;
            return;
        }

        if (*ptr == '#') {
            url_start = ptr + 1;
        }

        while ((ptr < val_end) && !isspace(*ptr) && *ptr != ')') {
            ++ptr;
        }

        url_end = ptr;
        if (url_start && url_end) {
            attr->val_type = SVG_ATTR_VALUE_PTR;
            len = BUF_LEN(url_start, url_end);
            char* node_id = (char*)mem_malloc(len + 1);
            memcpy(node_id, url_start, len);
            node_id[len] = '\0';
            attr->value.sval = node_id;
        }
        return;
    } else {
        if (len == 6) {
            if (strncmp(val_start, "freeze", 6) == 0) {
                attr->value.ival = SVG_ANIMATION_FREEZE;
                return;
            } else if (strncmp(val_start, "remove", 6) == 0) {
                attr->value.ival = SVG_ANIMATION_REMOVE;
                return;
            }
        }
        // parse color
        uint32_t color = 0;
        _parse_color(val_start, val_end, &color);
        attr->value.uval = color;
        return;
    }
}

static INLINE void _process_paint_attrs(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                        const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t len = BUF_LEN(val_start, val_end);

    if (len >= 7 && strncmp(val_start, "inherit", 7) == 0) {
        attr->class_type = SVG_ATTR_VALUE_INHERIT;
        return;
    }

    if (type == SVG_ATTR_FILL_RULE) {
        int32_t val = 0;
        if (strncmp(val_start, "evenodd", 7) == 0) {
            val = FILL_RULE_EVEN_ODD;
        } else {
            val = FILL_RULE_WINDING;
        }
        attr->value.ival = val;
    } else if (type == SVG_ATTR_STROKE_LINECAP) {
        int32_t val = 0;
        if (strncmp(val_start, "round", 5) == 0) {
            val = LINE_CAP_ROUND;
        } else if (strncmp(val_start, "square", 6) == 0) {
            val = LINE_CAP_SQUARE;
        } else {
            val = LINE_CAP_BUTT;
        }
        attr->value.ival = val;
    } else if (type == SVG_ATTR_STROKE_LINEJOIN) {
        int32_t val = 0;
        if (strncmp(val_start, "round", 5) == 0) {
            val = LINE_JOIN_ROUND;
        } else if (strncmp(val_start, "bevel", 5) == 0) {
            val = LINE_JOIN_BEVEL;
        } else {
            val = LINE_JOIN_MITER;
        }
        attr->value.ival = val;
    } else if (type == SVG_ATTR_STROKE_WIDTH) {
        float val = 1.0f;
        val_start = _parse_number(val_start, val_end, &val);
        if (val < 0.0f) {
            val = 0.0f;
        }
        attr->value.fval = val;
    } else if (type == SVG_ATTR_STROKE_MITER_LIMIT) {
        float val = 4.0f;
        val_start = _parse_number(val_start, val_end, &val);
        if (val < 1.0f) {
            val = 1.0f;
        }
        attr->value.ival = (int32_t)val;
    } else if (type == SVG_ATTR_STROKE_DASH_OFFSET) {
        float val = 0.0f;
        val_start = _parse_number(val_start, val_end, &val);
        attr->value.fval = val;
    } else if (type == SVG_ATTR_GRADIENT_STOP_OFFSET) {
        float val = 0.0f;
        val_start = _parse_number(val_start, val_end, &val);
        attr->value.fval = val;
    }
}

static INLINE void _process_paint_dasharray(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                            const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t len = BUF_LEN(val_start, val_end);

    if (len >= 4 && strncmp(val_start, "none", 4) == 0) {
        attr->class_type = SVG_ATTR_VALUE_NONE;
        attr->value.ival = 0;
        return;
    } else if (len >= 7 && strncmp(val_start, "inherit", 7) == 0) {
        attr->class_type = SVG_ATTR_VALUE_INHERIT;
        attr->value.ival = 0;
        return;
    } else {
        attr->val_type = SVG_ATTR_VALUE_PTR;

        uint32_t list_cap = 4;
        psx_svg_attr_values_list* list = (psx_svg_attr_values_list*)mem_malloc(sizeof(float) * list_cap + sizeof(uint32_t));

        uint32_t count = 0;
        const char* ptr = val_start;

        while (ptr < val_end) {
            if (count == list_cap) {
                list_cap = list_cap << 1;
                list = (psx_svg_attr_values_list*)mem_realloc(list, sizeof(float) * list_cap + sizeof(uint32_t));
            }
            float* val = (float*)(&list->data) + count;
            ptr = _parse_number(ptr, val_end, val);
            if (!ptr) {
                break;
            }
            ++count;
        }

        list->length = count;
        attr->value.val = list;
    }
}

static INLINE void _process_font_attrs(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                       const char* val_end, int32_t dpi)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t len = BUF_LEN(val_start, val_end);

    if (len >= 7 && strncmp(val_start, "inherit", 7) == 0) {
        attr->class_type = SVG_ATTR_VALUE_INHERIT;
        attr->value.ival = 0;
        return;
    }

    if (type == SVG_ATTR_FONT_SIZE && _is_number_begin(*val_start)) {
        float val = 0.0f;
        val_start = _parse_length(val_start, val_end, dpi, &val);
        attr->value.fval = val;
    } else {
        attr->val_type = SVG_ATTR_VALUE_PTR;

        char* str = (char*)mem_malloc(len + 1);
        memcpy(str, val_start, len);
        str[len] = '\0';
        attr->value.sval = str;
    }
}

static INLINE void _process_transform(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                      const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_MATRIX_PTR;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t len = BUF_LEN(val_start, val_end);
    if (len >= 4 && strncmp(val_start, "none", 4) == 0) {
        attr->val_type = SVG_ATTR_VALUE_DATA;
        attr->class_type = SVG_ATTR_VALUE_NONE;
        attr->value.ival = 0;
        return;
    }

    ps_matrix* matrix = ps_matrix_create();
    const char* ptr = val_start;
    while (ptr < val_end) {
        ptr = _skip_space(ptr, val_end);
        if (ptr == val_end) { break; }

        len = BUF_LEN(ptr, val_end);

        if (len >= 9 && strncmp(ptr, "translate", 9) == 0) {
            ptr = _parse_matrix(ptr, val_end, SVG_TRANSFORM_TYPE_TRANSLATE, matrix);
        } else if (len >= 6 && strncmp(ptr, "matrix", 6) == 0) {
            ptr = _parse_matrix(ptr, val_end, SVG_TRANSFORM_TYPE_MATRIX, matrix);
        } else if (len >= 6 && strncmp(ptr, "rotate", 6) == 0) {
            ptr = _parse_matrix(ptr, val_end, SVG_TRANSFORM_TYPE_ROTATE, matrix);
        } else if (len >= 5 && strncmp(ptr, "scale", 5) == 0) {
            ptr = _parse_matrix(ptr, val_end, SVG_TRANSFORM_TYPE_SCALE, matrix);
        } else if (len >= 5 && strncmp(ptr, "skewX", 5) == 0) {
            ptr = _parse_matrix(ptr, val_end, SVG_TRANSFORM_TYPE_SKEW_X, matrix);
        } else if (len >= 5 && strncmp(ptr, "skewY", 5) == 0) {
            ptr = _parse_matrix(ptr, val_end, SVG_TRANSFORM_TYPE_SKEW_Y, matrix);
        }

        ++ptr;
    }
    attr->value.val = matrix;
}

static INLINE bool _is_relative_cmd(char cmd)
{
    switch (cmd) {
        case 'm':
        case 'l':
        case 'h':
        case 'v':
        case 'c':
        case 's':
        case 'q':
        case 't':
        case 'z':
            return true;
        case 'M':
        case 'L':
        case 'H':
        case 'V':
        case 'C':
        case 'S':
        case 'Q':
        case 'T':
        case 'Z':
        default:
            return false;
    }
}

static INLINE bool _is_path_cmd(char ch)
{
    return ch != 0 && strchr("MLHVCSQTZmlhvcsqtz", ch) != NULL;
}

static INLINE void _process_path_value(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                       const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_PATH_PTR;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    ps_path* path = ps_path_create();

    char cur_cmd = 0;
    psx_svg_point cur_point = {0, 0};
    psx_svg_point cur_ctrlPoint = {0, 0};
    psx_svg_point first_point = {0, 0};

    const char* ptr = val_start;

    while (ptr < val_end) {
        ptr = _skip_space_and_separators(ptr, val_end);
        if (ptr == val_end) { break; }

        char ch = *ptr;
        if (_is_number_begin(ch)) {
            if (cur_cmd != 0) {
                if (cur_cmd == 'M') {
                    ch = 'L';
                } else if (cur_cmd == 'm') {
                    ch = 'l';
                } else {
                    ch = cur_cmd;
                }
            } else {
                break;
            }
        } else if (_is_path_cmd(ch)) {
            ++ptr;
        } else {
            break;
        }

        bool relative = _is_relative_cmd(ch);

        switch (ch) {
            case 'm':
            case 'M': {
                    float xval = 0.0f;
                    ptr = _parse_number(ptr, val_end, &xval);
                    float yval = 0.0f;
                    ptr = _parse_number(ptr, val_end, &yval);
                    if (relative) {
                        xval += cur_point.x;
                        yval += cur_point.y;
                    }
                    ps_point pt;
                    pt.x = xval;
                    pt.y = yval;
                    ps_path_move_to(path, &pt);
                    cur_point.x = xval;
                    cur_point.y = yval;
                    first_point.x = xval;
                    first_point.y = yval;
                }
                break;
            case 'L':
            case 'l': {
                    float xval = 0.0f;
                    ptr = _parse_number(ptr, val_end, &xval);
                    float yval = 0.0f;
                    ptr = _parse_number(ptr, val_end, &yval);
                    if (relative) {
                        xval += cur_point.x;
                        yval += cur_point.y;
                    }
                    ps_point pt;
                    pt.x = xval;
                    pt.y = yval;
                    ps_path_line_to(path, &pt);
                    cur_point.x = xval;
                    cur_point.y = yval;
                }
                break;
            case 'H':
            case 'h': {
                    float xval = 0.0f;
                    ptr = _parse_number(ptr, val_end, &xval);
                    if (relative) {
                        xval += cur_point.x;
                    }
                    ps_point pt;
                    pt.x = xval;
                    pt.y = cur_point.y;
                    ps_path_line_to(path, &pt);
                    cur_point.x = xval;
                }
                break;
            case 'V':
            case 'v': {
                    float yval = 0.0f;
                    ptr = _parse_number(ptr, val_end, &yval);
                    if (relative) {
                        yval += cur_point.y;
                    }
                    ps_point pt;
                    pt.x = cur_point.x;
                    pt.y = yval;
                    ps_path_line_to(path, &pt);
                    cur_point.y = yval;
                }
                break;
            case 'C':
            case 'c': {
                    ps_point point[3];
                    for (int i = 0; i < 3; i++) {
                        float xval = 0.0f;
                        ptr = _parse_number(ptr, val_end, &xval);
                        float yval = 0.0f;
                        ptr = _parse_number(ptr, val_end, &yval);
                        if (relative) {
                            xval += cur_point.x;
                            yval += cur_point.y;
                        }
                        point[i].x = xval;
                        point[i].y = yval;
                    }
                    ps_path_bezier_to(path, &point[0], &point[1], &point[2]);

                    cur_ctrlPoint.x = point[1].x;
                    cur_ctrlPoint.y = point[1].y;
                    cur_point.x = point[2].x;
                    cur_point.y = point[2].y;
                }
                break;
            case 'S':
            case 's': {
                    ps_point point[3];
                    if (cur_cmd == 'C' || cur_cmd == 'c' || cur_cmd == 'S' || cur_cmd == 's') {
                        point[0].x = cur_point.x * 2 - cur_ctrlPoint.x;
                        point[0].y = cur_point.y * 2 - cur_ctrlPoint.y;
                    } else {
                        point[0].x = cur_point.x;
                        point[0].y = cur_point.y;
                    }

                    for (int i = 1; i < 3; i++) {
                        float xval = 0.0f;
                        ptr = _parse_number(ptr, val_end, &xval);
                        float yval = 0.0f;
                        ptr = _parse_number(ptr, val_end, &yval);
                        if (relative) {
                            xval += cur_point.x;
                            yval += cur_point.y;
                        }
                        point[i].x = xval;
                        point[i].y = yval;
                    }
                    ps_path_bezier_to(path, &point[0], &point[1], &point[2]);

                    cur_ctrlPoint.x = point[1].x;
                    cur_ctrlPoint.y = point[1].y;
                    cur_point.x = point[2].x;
                    cur_point.y = point[2].y;
                }
                break;
            case 'Q':
            case 'q': {
                    ps_point point[2];
                    for (int i = 0; i < 2; i++) {
                        float xval = 0.0f;
                        ptr = _parse_number(ptr, val_end, &xval);
                        float yval = 0.0f;
                        ptr = _parse_number(ptr, val_end, &yval);
                        if (relative) {
                            xval += cur_point.x;
                            yval += cur_point.y;
                        }
                        point[i].x = xval;
                        point[i].y = yval;
                    }
                    ps_path_quad_to(path, &point[0], &point[1]);

                    cur_ctrlPoint.x = point[0].x;
                    cur_ctrlPoint.y = point[0].y;
                    cur_point.x = point[1].x;
                    cur_point.y = point[1].y;
                }
                break;
            case 'T':
            case 't': {
                    ps_point point[2];
                    if (cur_cmd == 'Q' || cur_cmd == 'q' || cur_cmd == 'T' || cur_cmd == 't') {
                        point[0].x = cur_point.x * 2 - cur_ctrlPoint.x;
                        point[0].y = cur_point.y * 2 - cur_ctrlPoint.y;
                    } else {
                        point[0].x = cur_point.x;
                        point[0].y = cur_point.y;
                    }

                    for (int i = 1; i < 2; i++) {
                        float xval = 0.0f;
                        ptr = _parse_number(ptr, val_end, &xval);
                        float yval = 0.0f;
                        ptr = _parse_number(ptr, val_end, &yval);
                        if (relative) {
                            xval += cur_point.x;
                            yval += cur_point.y;
                        }
                        point[i].x = xval;
                        point[i].y = yval;
                    }
                    ps_path_quad_to(path, &point[0], &point[1]);

                    cur_ctrlPoint.x = point[0].x;
                    cur_ctrlPoint.y = point[0].y;
                    cur_point.x = point[1].x;
                    cur_point.y = point[1].y;
                }
                break;
            case 'Z':
            case 'z': {
                    ps_path_sub_close(path);
                    cur_point.x = first_point.x;
                    cur_point.y = first_point.y;
                }
                break;
        }

        if (!ptr) {
            break;
        }
        cur_cmd = ch;
    }
    attr->value.val = path;
}

static INLINE void _process_animation_attr_names(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                                 const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;
    attr->value.ival = _get_svg_attr_type(val_start, val_end);
}

static INLINE void _process_animation_attr_options(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                                   const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    uint32_t len = BUF_LEN(val_start, val_end);
    switch (type) {
        case SVG_ATTR_RESTART: {
                if (len == 6 && strncmp(val_start, "always", 6) == 0) {
                    attr->value.ival = SVG_ANIMATION_RESTART_ALWAYS;
                    return;
                } else if (len == 13 && strncmp(val_start, "whenNotActive", 13) == 0) {
                    attr->value.ival = SVG_ANIMATION_RESTART_WHEN_NOT_ACTIVE;
                    return;
                } else if (len == 5 && strncmp(val_start, "never", 5) == 0) {
                    attr->value.ival = SVG_ANIMATION_RESTART_NEVER;
                    return;
                }
            }
            break;
        case SVG_ATTR_CALC_MODE: {
                if (len == 6 && strncmp(val_start, "linear", 6) == 0) {
                    attr->value.ival = SVG_ANIMATION_CALC_MODE_LINEAR;
                    return;
                } else if (len == 5 && strncmp(val_start, "paced", 5) == 0) {
                    attr->value.ival = SVG_ANIMATION_CALC_MODE_PACED;
                    return;
                } else if (len == 6 && strncmp(val_start, "spline", 6) == 0) {
                    attr->value.ival = SVG_ANIMATION_CALC_MODE_SPLINE;
                    return;
                } else if (len == 8 && strncmp(val_start, "discrete", 8) == 0) {
                    attr->value.ival = SVG_ANIMATION_CALC_MODE_DISCRETE;
                    return;
                }
            }
            break;
        case SVG_ATTR_ADDITIVE: {
                if (len == 7 && strncmp(val_start, "replace", 7) == 0) {
                    attr->value.ival = SVG_ANIMATION_ADDITIVE_REPLACE;
                    return;
                } else if (len == 3 && strncmp(val_start, "sum", 3) == 0) {
                    attr->value.ival = SVG_ANIMATION_ADDITIVE_SUM;
                    return;
                }
            }
            break;
        case SVG_ATTR_ACCUMULATE: {
                if (len == 4 && strncmp(val_start, "none", 4) == 0) {
                    attr->value.ival = SVG_ANIMATION_ACCUMULATE_NONE;
                    return;
                } else if (len == 3 && strncmp(val_start, "sum", 3) == 0) {
                    attr->value.ival = SVG_ANIMATION_ACCUMULATE_SUM;
                    return;
                }
            }
            break;
        case SVG_ATTR_TRANSFORM_TYPE: {
                if (len == 9 && strncmp(val_start, "translate", 9) == 0) {
                    attr->value.ival = SVG_TRANSFORM_TYPE_TRANSLATE;
                    return;
                } else if (len == 5 && strncmp(val_start, "scale", 5) == 0) {
                    attr->value.ival = SVG_TRANSFORM_TYPE_SCALE;
                    return;
                } else if (len == 6 && strncmp(val_start, "rotate", 6) == 0) {
                    attr->value.ival = SVG_TRANSFORM_TYPE_ROTATE;
                    return;
                } else if (len == 5 && strncmp(val_start, "skewX", 5) == 0) {
                    attr->value.ival = SVG_TRANSFORM_TYPE_SKEW_X;
                    return;
                } else if (len == 5 && strncmp(val_start, "skewY", 5) == 0) {
                    attr->value.ival = SVG_TRANSFORM_TYPE_SKEW_Y;
                    return;
                }
            }
            break;
        default:
            // make compiler happy
            break;
    }
    attr->value.ival = 0;
}

static INLINE void _parse_animation_value(psx_svg_node* node, psx_svg_attr* attr, const char* val_start, const char* val_end,
                                          int32_t dpi)
{
    if (node->type() == SVG_TAG_ANIMATE || node->type() == SVG_TAG_SET) {
        float val_number = 0.0f;
        val_start = _parse_length(val_start, val_end, dpi, &val_number);
        attr->value.fval = val_number;
    } else if (node->type() == SVG_TAG_ANIMATE_COLOR) {
        uint32_t color = 0;
        val_start = _parse_color(val_start, val_end, &color);
        attr->value.uval = color;
    } else if (node->type() == SVG_TAG_ANIMATE_TRANSFORM) {
        attr->val_type = SVG_ATTR_VALUE_PTR;
        psx_svg_attr_values_list* list = (psx_svg_attr_values_list*)mem_malloc(sizeof(float) * 4 + sizeof(uint32_t));

        float val_number = 0.0f;
        uint32_t cnt = 0;
        const char* ptr = val_start;

        while ((ptr < val_end) && (cnt < 3)) {
            float* val = (float*)(&list->data) + cnt;

            val_number = 0.0f;
            ptr = _parse_number(ptr, val_end, &val_number);
            *val = val_number;

            if (!ptr) {
                break;
            }
            ++cnt;
        }

        list->length = cnt;
        attr->value.val = list;
    } else if (node->type() == SVG_TAG_ANIMATE_MOTION) {
        attr->val_type = SVG_ATTR_VALUE_PTR;
        psx_svg_attr_values_list* list = (psx_svg_attr_values_list*)mem_malloc(sizeof(psx_svg_point) + sizeof(uint32_t));
        psx_svg_point* pt = (psx_svg_point*)(&list->data);

        float val = 0.0f;
        val_start = _parse_number(val_start, val_end, &val);
        pt->x = val;
        val = 0.0f;
        val_start = _parse_number(val_start, val_end, &val);
        pt->y = val;

        list->length = 1;
        attr->value.val = list;
    }
}

typedef void(*_parse_list_callback)(psx_svg_node* node, psx_svg_attr* attr, const char* val_start, const char* val_end,
                                    int32_t dpi, void* data);

static INLINE uint32_t _parse_animation_value_list(psx_svg_node* node, psx_svg_attr* attr, const char* val_start,
                                                   const char* val_end, int32_t dpi, _parse_list_callback cb, void* data)
{
    uint32_t count = 0;
    val_start = _skip_space(val_start, val_end);
    const char* ptr = val_start;

    while (ptr != val_end) {
        if (*ptr == ';') {
            cb(node, attr, val_start, ptr, dpi, data);
            val_start = ++ptr;
            val_start = _skip_space(val_start, val_end);
            count++;
        } else {
            ++ptr;
        }
    }
    if (val_start < val_end) {
        cb(node, attr, val_start, ptr, dpi, data);
        count++;
    }
    return count;
}

struct _parse_value_list_context {
    uint32_t mem_size;
    uint32_t list_count;
    psx_svg_attr_values_list* list;
};

struct _transform_values_list {
    uint32_t length;
    float data[4];
};

#define GET_NEXT_VALUE_PTR(ptr, ctx, type) \
    do { \
        psx_svg_attr_values_list * list = ctx->list; \
        if(!list) { \
            ctx->mem_size = sizeof(type) * 4 + sizeof(uint32_t);\
            ctx->list = (psx_svg_attr_values_list *)mem_malloc(ctx->mem_size); \
            memset(ctx->list, 0, ctx->mem_size); \
            ptr = (type *)(&(ctx->list->data)); \
            ctx->list_count = 1; \
        } else { \
            uint32_t mem = sizeof(type) * (ctx->list_count + 1) + sizeof(uint32_t); \
            if(ctx->mem_size < mem) { \
                ctx->mem_size = (ctx->list_count << 1) * sizeof(type) + sizeof(uint32_t); \
                ctx->list = (psx_svg_attr_values_list *)mem_realloc(ctx->list, ctx->mem_size); \
            } \
            ptr = (type *)(&(ctx->list->data)) + ctx->list_count; \
            ctx->list_count++; \
        } \
    } while(0)

static void _animation_values_cb(psx_svg_node* node, psx_svg_attr* attr, const char* val_start, const char* val_end,
                                 int32_t dpi, void* data)
{
    struct _parse_value_list_context* ctx = (struct _parse_value_list_context*)data;

    if (node->type() == SVG_TAG_ANIMATE || node->type() == SVG_TAG_SET) {
        float* val = NULL;
        GET_NEXT_VALUE_PTR(val, ctx, float);
        val_start = _parse_length(val_start, val_end, dpi, val);
    } else if (node->type() == SVG_TAG_ANIMATE_COLOR) {
        uint32_t* color = NULL;
        GET_NEXT_VALUE_PTR(color, ctx, uint32_t);
        val_start = _parse_color(val_start, val_end, color);
    } else if (node->type() == SVG_TAG_ANIMATE_TRANSFORM) {
        struct _transform_values_list* trans_vals = NULL;
        GET_NEXT_VALUE_PTR(trans_vals, ctx, struct _transform_values_list);

        uint32_t cnt = 0;
        const char* ptr = val_start;

        while ((ptr < val_end) && (cnt < 3)) {
            float* val = &(trans_vals->data[cnt]);
            ptr = _parse_number(ptr, val_end, val);
            if (!ptr) {
                break;
            }
            ++cnt;
        }

        trans_vals->length = cnt;
    } else if (node->type() == SVG_TAG_ANIMATE_MOTION) {
        psx_svg_point* point = NULL;
        GET_NEXT_VALUE_PTR(point, ctx, psx_svg_point);
        val_start = _parse_number(val_start, val_end, &point->x);
        val_start = _parse_number(val_start, val_end, &point->y);
    }
    ctx->list->length = ctx->list_count;
}

static void _animation_keys_cb(psx_svg_node* node, psx_svg_attr* attr, const char* val_start, const char* val_end,
                               int32_t dpi, void* data)
{
    struct _parse_value_list_context* ctx = (struct _parse_value_list_context*)data;

    float* val = NULL;
    GET_NEXT_VALUE_PTR(val, ctx, float);
    val_start = _parse_number(val_start, val_end, val);

    ctx->list->length = ctx->list_count;
}

static void _animation_key_splines_cb(psx_svg_node* node, psx_svg_attr* attr, const char* val_start,
                                      const char* val_end, int32_t dpi, void* data)
{
    struct _parse_value_list_context* ctx = (struct _parse_value_list_context*)data;

    psx_svg_point* point = NULL;
    GET_NEXT_VALUE_PTR(point, ctx, psx_svg_point);
    val_start = _parse_number(val_start, val_end, &point->x);
    val_start = _parse_number(val_start, val_end, &point->y);

    GET_NEXT_VALUE_PTR(point, ctx, psx_svg_point);
    val_start = _parse_number(val_start, val_end, &point->x);
    val_start = _parse_number(val_start, val_end, &point->y);

    ctx->list->length = ctx->list_count;
}

static void _animation_begin_end_cb(psx_svg_node* node, psx_svg_attr* attr, const char* val_start,
                                    const char* val_end, int32_t dpi, void* data)
{
    struct _parse_value_list_context* ctx = (struct _parse_value_list_context*)data;

    // offset-value
    float* val = NULL;
    GET_NEXT_VALUE_PTR(val, ctx, float);
    val_start = _parse_clock_time(val_start, val_end, val);

    //FIXME: not support begin-end type
    // syncbase-value
    // event-value
    // repeat-value
    // accessKey-value
    // indefinite

    ctx->list->length = ctx->list_count;
}

static INLINE void _process_animation_attr_values(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                                  const char* val_end, int32_t dpi)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    if (type == SVG_ATTR_VALUES) {
        attr->val_type = SVG_ATTR_VALUE_PTR;
        struct _parse_value_list_context ctx;
        ctx.mem_size = 0;
        ctx.list_count = 0;
        ctx.list = NULL;
        _parse_animation_value_list(node, attr, val_start, val_end, dpi, _animation_values_cb, &ctx);
        attr->value.val = ctx.list;
    } else if (type == SVG_ATTR_KEY_TIMES || type == SVG_ATTR_KEY_POINTS) {
        attr->val_type = SVG_ATTR_VALUE_PTR;
        struct _parse_value_list_context ctx;
        ctx.mem_size = 0;
        ctx.list_count = 0;
        ctx.list = NULL;
        _parse_animation_value_list(node, attr, val_start, val_end, dpi, _animation_keys_cb, &ctx);
        attr->value.val = ctx.list;
    } else if (type == SVG_ATTR_KEY_SPLINES) {
        attr->val_type = SVG_ATTR_VALUE_PTR;
        struct _parse_value_list_context ctx;
        ctx.mem_size = 0;
        ctx.list_count = 0;
        ctx.list = NULL;
        _parse_animation_value_list(node, attr, val_start, val_end, dpi, _animation_key_splines_cb, &ctx);
        attr->value.val = ctx.list;
    } else if (type == SVG_ATTR_BEGIN || type == SVG_ATTR_END) {
        attr->val_type = SVG_ATTR_VALUE_PTR;
        struct _parse_value_list_context ctx;
        ctx.mem_size = 0;
        ctx.list_count = 0;
        ctx.list = NULL;
        _parse_animation_value_list(node, attr, val_start, val_end, dpi, _animation_begin_end_cb, &ctx);
        attr->value.val = ctx.list;
    } else {
        _parse_animation_value(node, attr, val_start, val_end, dpi);
    }
}

static INLINE void _process_animation_attr_number(psx_svg_node* node, psx_svg_attr_type type, const char* val_start,
                                                  const char* val_end)
{
    psx_array_append(node->attrs(), NULL);
    psx_svg_attr* attr = psx_array_get_last(node->attrs(), psx_svg_attr);

    attr->attr_id = type;
    attr->val_type = SVG_ATTR_VALUE_DATA;
    attr->class_type = SVG_ATTR_VALUE_INITIAL;

    if (type == SVG_ATTR_REPEAT_COUNT) {
        uint32_t len = BUF_LEN(val_start, val_end);
        if (len == 10 && strncmp(val_start, "indefinite", 10) == 0) {
            attr->value.uval = 0;
            return;
        }

        float val = 0.0f;
        val_start = _parse_number(val_start, val_end, &val);
        attr->value.uval = (uint32_t)val;
    } else { // SVG_ATTR_ROTATE
        uint32_t len = BUF_LEN(val_start, val_end);
        if (len == 4 && strncmp(val_start, "auto", 4) == 0) {
            // rotated over time by the angle of the direction (i.e., directional tangent vector) of the motion path
            attr->class_type = SVG_ATTR_VALUE_INHERIT;
            attr->value.fval = 0.0f;
            return;
        } else if (len == 12 && strncmp(val_start, "auto-reverse", 12) == 0) {
            // rotated over time by the angle of the direction (i.e., directional tangent vector) of the motion path plus 180 degrees.
            attr->class_type = SVG_ATTR_VALUE_INHERIT;
            attr->value.fval = 180.0f;
            return;
        }

        float val_number = 0.0f;
        val_start = _parse_number(val_start, val_end, &val_number);
        attr->value.fval = val_number;
    }
}

static INLINE void _process_attrs_tag(psx_svg_parser* parser, psx_svg_node* node, const psx_xml_token* token)
{
    uint32_t len = psx_array_size(&token->attrs);
    for (uint32_t i = 0; i < len; i++) {
        psx_xml_token_attr* tok_attr = psx_array_get(&token->attrs, i, psx_xml_token_attr);
        psx_svg_attr_type type = _get_svg_attr_type(tok_attr->name_start, tok_attr->name_end);
        if (type == SVG_ATTR_INVALID) {
            continue; // skip invalid attribute
        }

        tok_attr->value_start = _skip_space(tok_attr->value_start, tok_attr->value_end);
        uint32_t value_len = ATTR_VALUE_LEN(tok_attr);
        if (value_len == 0) {
            continue; // skip empty value attribute
        }

        switch (type) {
            case SVG_ATTR_ID:
            case SVG_ATTR_XML_ID: {
                    node->set_content(tok_attr->value_start, value_len);
                    continue;
                }
                break;
            case SVG_ATTR_VERSION:
            case SVG_ATTR_BASE_PROFILE:
                _process_string(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_VIEWBOX:
                _process_view_box(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_PRESERVE_ASPECT_RATIO:
                _process_preserve_aspect_ratio(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_X:
            case SVG_ATTR_Y:
            case SVG_ATTR_WIDTH:
            case SVG_ATTR_HEIGHT:
            case SVG_ATTR_RX:
            case SVG_ATTR_RY:
            case SVG_ATTR_CX:
            case SVG_ATTR_CY:
            case SVG_ATTR_R:
            case SVG_ATTR_X1:
            case SVG_ATTR_Y1:
            case SVG_ATTR_X2:
            case SVG_ATTR_Y2:
            case SVG_ATTR_PATH_LENGTH:
                _process_length_value(node, type, tok_attr->value_start, tok_attr->value_end, parser->dpi);
                break;
            case SVG_ATTR_OPACITY:
            case SVG_ATTR_FILL_OPACITY:
            case SVG_ATTR_STROKE_OPACITY:
            case SVG_ATTR_SOLID_OPACITY:
            case SVG_ATTR_VIEWPORT_FILL_OPACITY:
            case SVG_ATTR_GRADIENT_STOP_OPACITY:
                _process_opacity_value(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_POINTS:
                _process_points_value(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_D:
            case SVG_ATTR_PATH:
                _process_path_value(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_TRANSFORM:
                _process_transform(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_FILL:
            case SVG_ATTR_STROKE:
            case SVG_ATTR_VIEWPORT_FILL:
            case SVG_ATTR_SOLID_COLOR:
            case SVG_ATTR_GRADIENT_STOP_COLOR:
                _process_paint(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_FILL_RULE:
            case SVG_ATTR_STROKE_LINECAP:
            case SVG_ATTR_STROKE_LINEJOIN:
            case SVG_ATTR_STROKE_WIDTH:
            case SVG_ATTR_STROKE_MITER_LIMIT:
            case SVG_ATTR_STROKE_DASH_OFFSET:
            case SVG_ATTR_GRADIENT_STOP_OFFSET:
                _process_paint_attrs(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_STROKE_DASH_ARRAY:
                _process_paint_dasharray(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_GRADIENT_UNITS:
                _process_gradient_units(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_FONT_FAMILY:
            case SVG_ATTR_FONT_STYLE:
            case SVG_ATTR_FONT_VARIANT:
            case SVG_ATTR_FONT_WEIGHT:
            case SVG_ATTR_FONT_SIZE:
                _process_font_attrs(node, type, tok_attr->value_start, tok_attr->value_end, parser->dpi);
                break;
            case SVG_ATTR_XLINK_HREF:
                _process_xlink(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_DUR:
            case SVG_ATTR_MIN:
            case SVG_ATTR_MAX:
            case SVG_ATTR_REPEAT_DUR:
                _process_clock_time(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_ATTRIBUTE_NAME:
                _process_animation_attr_names(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_FROM:
            case SVG_ATTR_TO:
            case SVG_ATTR_BY:
            case SVG_ATTR_VALUES:
            case SVG_ATTR_KEY_TIMES:
            case SVG_ATTR_KEY_POINTS:
            case SVG_ATTR_KEY_SPLINES:
            case SVG_ATTR_BEGIN:
            case SVG_ATTR_END:
                _process_animation_attr_values(node, type, tok_attr->value_start, tok_attr->value_end, parser->dpi);
                break;
            case SVG_ATTR_ROTATE:
            case SVG_ATTR_REPEAT_COUNT:
                _process_animation_attr_number(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_RESTART:
            case SVG_ATTR_CALC_MODE:
            case SVG_ATTR_ADDITIVE:
            case SVG_ATTR_ACCUMULATE:
            case SVG_ATTR_TRANSFORM_TYPE:
                _process_animation_attr_options(node, type, tok_attr->value_start, tok_attr->value_end);
                break;
            case SVG_ATTR_ATTRIBUTE_TYPE:
            case SVG_ATTR_DISPLAY:
            case SVG_ATTR_VISIBILITY:
            case SVG_ATTR_TEXT_ANCHOR:
            default:
                // not support yet
                break;
        }
    }
}

static INLINE bool _process_end_tag(psx_svg_parser* parser, psx_svg_tag tag, const psx_xml_token* token)
{
    if (parser->state == SVG_PARSER_IGNORE) {
        uint32_t len = TOKEN_LEN(token);
        if ((parser->ignore_len == len) && strncmp(parser->ignore_name, token->start, len) == 0) {
            parser->state = SVG_PARSER_PROCESS;
            mem_free(parser->ignore_name);
            parser->ignore_name = NULL;
            parser->ignore_len = 0;
        }
        return true;
    }

    if (parser->cur_node->type() != tag) {
        LOG_ERROR("Svg tag does not match in pairs!\n");
        return false;
    }

    if (parser->cur_node != parser->doc_root) {
        parser->cur_node = (psx_svg_node*)parser->cur_node->parent();
    }
    return true;
}

static INLINE bool _process_begin_tag(psx_svg_parser* parser, psx_svg_tag tag, const psx_xml_token* token)
{
    if (parser->state == SVG_PARSER_IGNORE) {
        return true;
    }

    if (token->type == PSX_XML_CONTENT) {
        uint32_t len = TOKEN_LEN(token);
        psx_svg_node* node = psx_svg_node_create(parser->cur_node);
        node->set_content(token->start, len);
        node->set_type(SVG_TAG_CONTENT);
        return true;
    }

    //FIXME: SVG_TAG_ENTITY

    // begin invalid tag
    if (tag == SVG_TAG_INVALID) {
        if (!(token->flags & PSX_XML_TOKEN_FLAT)) {
            parser->state = SVG_PARSER_IGNORE;
            uint32_t len = TOKEN_LEN(token);
            parser->ignore_name = (char*)mem_malloc(len + 1);
            parser->ignore_len = len;
            memcpy(parser->ignore_name, token->start, len);
            parser->ignore_name[len] = '\0';
        }
        return true;
    }

    // create new node
    psx_svg_node* node = psx_svg_node_create(parser->cur_node);
    node->set_type(tag);

    _process_attrs_tag(parser, node, token);

    if (!parser->doc_root) { // root node
        parser->doc_root = node;
    }
    if (!(token->flags & PSX_XML_TOKEN_FLAT)) {
        parser->cur_node = node;
    }
    return true;
}

void psx_svg_parser_init(psx_svg_parser* parser)
{
    memset(parser, 0, sizeof(psx_svg_parser));
    parser->ignore_name = NULL;
    parser->ignore_len = 0;
    parser->dpi = DEFAULT_DPI;
    parser->doc_root = NULL;
    parser->cur_node = NULL;
}

void psx_svg_parser_destroy(psx_svg_parser* parser)
{
    if (parser->ignore_name) {
        mem_free(parser->ignore_name);
        parser->ignore_name = NULL;
        parser->ignore_len = 0;
    }

    if (parser->doc_root) {
        psx_svg_node_destroy(parser->doc_root);
    }

    parser->doc_root = NULL;
    parser->cur_node = NULL;
}

bool psx_svg_parser_is_finish(psx_svg_parser* parser)
{
    return (parser->doc_root != NULL)
           && (parser->cur_node == parser->doc_root)
           && (parser->state != SVG_PARSER_IGNORE);
}

bool psx_svg_parser_token(psx_svg_parser* parser, const psx_xml_token* token)
{
    psx_svg_tag tag = _get_svg_tag_type(token);

    if (parser->doc_root == NULL) {
        if (!(tag == SVG_TAG_SVG && token->type == PSX_XML_BEGIN)) {
            LOG_ERROR("Root element in svg document must be <svg>!\n");
            return false;
        }
    }

    if (token->type == PSX_XML_END) {
        return _process_end_tag(parser, tag, token);
    }

    return _process_begin_tag(parser, tag, token);
}

#ifdef _DEBUG
#include <stdio.h>
void psx_svg_dump_tree(psx_svg_node* root, int depth)
{
    if (!root) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        fprintf(stderr, "  ");
    }
    if (root->type() == SVG_TAG_CONTENT) {
        fprintf(stderr, "content: [%s]\n", root->content());
    } else {
        fprintf(stderr, "tag <%s>", _svg_tag_map[root->type() - 2].name);
        if (root->content()) {
            fprintf(stderr, " - id [%s]", root->content());
        }
        fprintf(stderr, "\n");
    }

    uint32_t len = root->attr_count();
    for (uint32_t i = 0; i < len; i++) {
        for (int j = 0; j < depth; j++) {
            fprintf(stderr, "  ");
        }
        psx_svg_attr* attr = root->attr_at(i);
        fprintf(stderr, "   attr <%s>\n", _svg_attr_map[attr->attr_id].name);
    }

    psx_tree_node* tree_root = (psx_tree_node*)root;

    for (uint32_t i = 0; i < tree_root->child_count(); i++) {
        ++depth;
        psx_svg_dump_tree((psx_svg_node*)tree_root->get_child(i), depth);
        --depth;
    }
}
#endif

#ifdef __cplusplus
}
#endif
