/*
 * Copyright (c) 2024, Zhang Ji Peng
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

#include "common.h"
#include "font_adapter.h"

#if !defined(WIN32) && !ENABLE(FREE_TYPE2) && !defined(__APPLE__)

// dummy font interface
namespace picasso {

font_adapter::font_adapter(const char* name, int charset, scalar size, scalar weight,
                           bool italic, bool hint, bool flip, bool a, const trans_affine* mtx) { m_impl = NULL; }
font_adapter::~font_adapter() { }
void font_adapter::active(void) { }
void font_adapter::deactive(void) { }
void font_adapter::add_kerning(uint32_t first, uint32_t second, scalar* x, scalar* y) { }
bool font_adapter::prepare_glyph(uint32_t code) { return false; }
void font_adapter::write_glyph_to(byte* buffer) { }
void* font_adapter::create_storage(byte* buf, uint32_t len, scalar x, scalar y) { return NULL; }
void font_adapter::destroy_storage(void*) { }
void font_adapter::translate_storage(void* storage, scalar x, scalar y) { }
scalar font_adapter::height(void) const { return 0; }
scalar font_adapter::ascent(void) const { return 0; }
scalar font_adapter::descent(void) const { return 0; }
scalar font_adapter::leading(void) const { return 0; }
uint32_t font_adapter::units_per_em(void) const { return 0; }
uint32_t font_adapter::glyph_index(void) const { return 0; }
uint32_t font_adapter::data_size(void) const { return 0; }
glyph_type font_adapter::data_type(void) const { return glyph_type_invalid; }
const rect& font_adapter::bounds(void) const { static rect r; return r; }
scalar font_adapter::advance_x(void) const { return 0; }
scalar font_adapter::advance_y(void) const { return 0; }

}

bool platform_font_init(void) { return true; }
void platform_font_shutdown(void) { }

#endif
