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

#ifndef _FONT_ADAPTER_H_
#define _FONT_ADAPTER_H_

#include "common.h"
#include "interfaces.h"
#include "non_copy.h"

namespace picasso {

class font_adapter_impl;

class font_adapter : public non_copyable
{
public:
    font_adapter(const char* name, int32_t charset, scalar height, scalar weight,
                 bool italic, bool hint, bool flip, bool antialias, const trans_affine* mtx);
    ~font_adapter();

    scalar height(void) const;
    scalar ascent(void) const;
    scalar descent(void) const;
    scalar leading(void) const;
    uint32_t units_per_em(void) const;

    void active(void);
    void deactive(void);

    bool prepare_glyph(uint32_t code);
    void write_glyph_to(byte* buffer);
    void add_kerning(uint32_t f, uint32_t s, scalar* x, scalar* y);

    uint32_t glyph_index(void) const;
    uint32_t data_size(void) const;
    glyph_type data_type(void) const;
    const rect& bounds(void) const;
    scalar advance_x(void) const;
    scalar advance_y(void) const;

    void* create_storage(byte* buf, uint32_t buf_len, scalar x, scalar y);
    void destroy_storage(void*);
    void translate_storage(void*, scalar x, scalar y);
private:
    font_adapter_impl* m_impl;
};

}
#endif /*_FONT_ADAPTER_H_*/
