/* Picasso - a vector graphics library
 *
 * Copyright (C) 2023 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include <stdio.h>
#include "common.h"
#include "convert.h"
#include "gfx_font_adapter.h"
#include "gfx_rasterizer_scanline.h"
#include "gfx_scanline.h"
#include "gfx_scanline_renderer.h"
#include "gfx_scanline_storage.h"
#include "gfx_trans_affine.h"

#import <CoreText/CTFont.h>
#import <CoreGraphics/CGPath.h>

#include "graphic_path.h"
#include "graphic_helper.h"
#include "graphic_base.h"

namespace gfx {

class font_adapter_impl
{
public:
    font_adapter_impl()
        : font(0)
        , cur_glyph_index(0)
        , cur_data_size(0)
        , flip_y(false)
        , weight(0)
        , height(0)
        , ascent(0)
        , descent(0)
        , leading(0)
        , units_per_em(0)
        , cur_bound_rect(0, 0, 0, 0)
        , cur_advance_x(0)
        , cur_advance_y(0)
    {
    }
    
    ~font_adapter_impl()
    {
        if (font) {
            CFRelease(font);
        }
        font = 0;
    }
    
    CTFontRef font;
    CGAffineTransform matrix;
    CGGlyph cur_glyph_index;
    unsigned int cur_data_size;
    bool flip_y;
    scalar weight;
    scalar height;
    scalar ascent;
    scalar descent;
    scalar leading;
    unsigned int units_per_em;
    
    picasso::graphic_path cur_font_path;
    rect cur_bound_rect;
    scalar cur_advance_x;
    scalar cur_advance_y;
};

gfx_font_adapter::gfx_font_adapter(const char* name, int charset, scalar size, scalar weight,
                                bool italic, bool hint, bool flip, bool a, const abstract_trans_affine* mtx)
    :m_impl(new font_adapter_impl)
{
    CFStringRef fName = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
    m_impl->matrix = CGAffineTransformMake(mtx->sx(), mtx->shy(), mtx->shx(), mtx->sy(), mtx->tx(), mtx->ty());
    CTFontOptions opt = kCTFontOptionsPreferSystemFont;
    m_impl->font = CTFontCreateWithNameAndOptions(fName, (CGFloat)size, &(m_impl->matrix), opt);
    m_impl->flip_y = flip;
    if ((CTFontGetSymbolicTraits(m_impl->font) & kCTFontTraitItalic) == 0 && italic) {
        m_impl->matrix = CGAffineTransformConcat(m_impl->matrix, CGAffineTransformMake(1.0, 0.0, 0.4, 1.0, 0.0, 0.0));
    }
    // FIXME： weight not implement.
    CFRelease(fName);
}

gfx_font_adapter::~gfx_font_adapter()
{
    delete m_impl;
}

static void _cgPathReader(void *info, const CGPathElement *element)
{
    font_adapter_impl* imp = (font_adapter_impl*)info;
    picasso::graphic_path* path = (picasso::graphic_path*)(&(imp->cur_font_path));
    bool flip_y = imp->flip_y;
    
    switch (element->type) {
        case kCGPathElementMoveToPoint:
        {
            CGPoint* p = element->points;
            path->move_to((scalar)p[0].x, flip_y ? -(scalar)p[0].y : (scalar)p[0].y);
            break;
        }
        case kCGPathElementAddLineToPoint:
        {
            CGPoint* p = element->points;
            path->line_to((scalar)p[0].x, flip_y ? -(scalar)p[0].y : (scalar)p[0].y);
            break;
        }
        case kCGPathElementAddCurveToPoint:
        {
            CGPoint* p = element->points;
            path->add_vertex((scalar)p[0].x, flip_y ? -(scalar)p[0].y : (scalar)p[0].y, path_cmd_curve4);
            path->add_vertex((scalar)p[1].x, flip_y ? -(scalar)p[1].y : (scalar)p[1].y, path_cmd_curve4);
            path->add_vertex((scalar)p[2].x, flip_y ? -(scalar)p[2].y : (scalar)p[2].y, path_cmd_curve4);
            break;
        }
        case kCGPathElementAddQuadCurveToPoint:
        {
            CGPoint* p = element->points;
            path->add_vertex((scalar)p[0].x, flip_y ? -(scalar)p[0].y : (scalar)p[0].y, path_cmd_curve3);
            path->add_vertex((scalar)p[1].x, flip_y ? -(scalar)p[1].y : (scalar)p[1].y, path_cmd_curve3);
            break;
        }
        case kCGPathElementCloseSubpath:
        {
            path->close_polygon();
            break;
        }
    }
}

static rect get_bounding_rect(graphic_path& path)
{
    rect_s rc(0,0,0,0);
    bounding_rect(path, 0, &rc.x1, &rc.y1, &rc.x2, &rc.y2);
    return rect((int)Floor(rc.x1), (int)Floor(rc.y1), (int)Ceil(rc.x2), (int)Ceil(rc.y2));
}

bool gfx_font_adapter::prepare_glyph(unsigned int code)
{
    if (m_impl->font) {
        UniChar charCode = (UniChar)code;
        if (!CTFontGetGlyphsForCharacters(m_impl->font, &charCode, &(m_impl->cur_glyph_index), 1)) {
            return false;
        }
        
        CGSize advances;
        CTFontGetAdvancesForGlyphs(m_impl->font, kCTFontOrientationDefault, &(m_impl->cur_glyph_index), &advances, 1);
        m_impl->cur_advance_x = (scalar)advances.width;
        m_impl->cur_advance_y = (scalar)advances.height;
        
        CGPathRef fontPath = CTFontCreatePathForGlyph(m_impl->font, m_impl->cur_glyph_index, &m_impl->matrix);
        m_impl->cur_font_path.remove_all();
        CGPathApply(fontPath, m_impl, _cgPathReader);
        m_impl->cur_bound_rect = get_bounding_rect(m_impl->cur_font_path);
        m_impl->cur_data_size = m_impl->cur_font_path.total_byte_size()+sizeof(unsigned int);//count data
        m_impl->ascent = (scalar)CTFontGetAscent(m_impl->font);
        m_impl->descent = (scalar)CTFontGetDescent(m_impl->font);
        m_impl->leading = (scalar)CTFontGetLeading(m_impl->font);
        m_impl->units_per_em = CTFontGetUnitsPerEm(m_impl->font);
        m_impl->height = m_impl->ascent + m_impl->descent;
        CGPathRelease(fontPath);
        return true;
    }
    return false;
}

void gfx_font_adapter::write_glyph_to(byte* buffer)
{
    if (buffer && m_impl->cur_data_size) {
        unsigned int count = m_impl->cur_font_path.total_vertices();
        mem_copy(buffer, &count, sizeof(unsigned int));
        buffer += sizeof(unsigned int);
        m_impl->cur_font_path.serialize_to(buffer);
    }
}

scalar gfx_font_adapter::ascent(void) const
{
    return m_impl->ascent;
}

scalar gfx_font_adapter::descent(void) const
{
    return m_impl->descent;
}

scalar gfx_font_adapter::leading(void) const
{
    return m_impl->leading;
}

unsigned int gfx_font_adapter::units_per_em(void) const
{
    return m_impl->units_per_em;
}

unsigned int gfx_font_adapter::glyph_index(void) const
{
    return (unsigned int)m_impl->cur_glyph_index;
}

unsigned int gfx_font_adapter::data_size(void) const
{
    return m_impl->cur_data_size;
}

glyph_type gfx_font_adapter::data_type(void) const
{
    return glyph_type_outline;
}

const rect& gfx_font_adapter::bounds(void) const
{
    return m_impl->cur_bound_rect;
}

scalar gfx_font_adapter::height(void) const
{
    return m_impl->height;
}

scalar gfx_font_adapter::advance_x(void) const
{
    return m_impl->cur_advance_x;
}

scalar gfx_font_adapter::advance_y(void) const
{
    return m_impl->cur_advance_y;
}

void gfx_font_adapter::add_kerning(unsigned int first, unsigned int second, scalar* x, scalar* y)
{
    //FIXME：implement
}


void gfx_font_adapter::active(void) { }
void gfx_font_adapter::deactive(void) { }
void* gfx_font_adapter::create_storage(byte* buf, unsigned int len, scalar x, scalar y) { return NULL; }
void gfx_font_adapter::destroy_storage(void*) { }
void gfx_font_adapter::translate_storage(void* storage, scalar x, scalar y) { }
}

bool platform_font_init(void) { return true; }
void platform_font_shutdown(void) { }
