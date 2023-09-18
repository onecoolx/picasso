/* Picasso - a vector graphics library
 *
 * Copyright (C) 2019 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include <stdio.h>
#include "common.h"
#include "convert.h"
#include "gfx_font_adapter.h"

#if !defined(WIN32) && !ENABLE(FREE_TYPE2) && !defined(__APPLE__)

// dummy font interface
namespace gfx {

gfx_font_adapter::gfx_font_adapter(const char* name, int charset, scalar size, scalar weight,
                                bool italic, bool hint, bool flip, bool a, const trans_affine* mtx) { }
gfx_font_adapter::~gfx_font_adapter() { }
void gfx_font_adapter::active(void) { }
void gfx_font_adapter::deactive(void) { }
void gfx_font_adapter::add_kerning(unsigned int first, unsigned int second, scalar* x, scalar* y) { }
bool gfx_font_adapter::prepare_glyph(unsigned int code) { return false; }
void gfx_font_adapter::write_glyph_to(byte* buffer) { }
void* gfx_font_adapter::create_storage(byte* buf, unsigned int len, scalar x, scalar y) { return NULL; }
void gfx_font_adapter::destroy_storage(void*) { }
void gfx_font_adapter::translate_storage(void* storage, scalar x, scalar y) { }
scalar gfx_font_adapter::height(void) const { return 0; }
scalar gfx_font_adapter::ascent(void) const { return 0; }
scalar gfx_font_adapter::descent(void) const { return 0; }
scalar gfx_font_adapter::leading(void) const { return 0; }
unsigned int gfx_font_adapter::units_per_em(void) const { return 0; }
unsigned int gfx_font_adapter::glyph_index(void) const { return 0; }
unsigned int gfx_font_adapter::data_size(void) const { return 0; }
glyph_type gfx_font_adapter::data_type(void) const { return glyph_type_invalid; }
const rect& gfx_font_adapter::bounds(void) const { static rect r; return r; }
scalar gfx_font_adapter::advance_x(void) const { return 0; }
scalar gfx_font_adapter::advance_y(void) const { return 0; }

}

bool platform_font_init(void)
{
    return true;
}

void platform_font_shutdown(void)
{
}
#endif
