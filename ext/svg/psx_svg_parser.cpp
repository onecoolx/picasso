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

#include "psx_svg_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAP_LEN(m) sizeof((m)) / sizeof((m[0]))

static const struct {
    const char* name;
    uint32_t name_len;
    uint32_t tag;
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
    uint32_t attr;
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

void psx_svg_parser_init(psx_svg_parser* parser)
{
}

void psx_svg_parser_destroy(psx_svg_parser* parser)
{
}

bool psx_svg_parser_token(psx_svg_parser* parser, const psx_xml_token* token)
{
    return false;
}

bool psx_svg_parser_is_finish(psx_svg_parser* parser)
{
    return false;
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
