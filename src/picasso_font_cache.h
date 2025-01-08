/*
 * Copyright (c) 2013, Zhang Ji Peng
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

#ifndef _PICASSO_FONT_CACHE_H_
#define _PICASSO_FONT_CACHE_H_

#include "common.h"
#include "data_vector.h"
#include "device.h"
#include "interfaces.h"
#include "non_copy.h"

namespace picasso {

class glyph_cache_manager : public non_copyable
{
    enum {
        block_size = 16384 - 16
    };

public:
    glyph_cache_manager()
        : m_allocator(block_size)
        , m_signature(0)
    {
        memset(m_glyphs, 0, sizeof(m_glyphs));
    }

    ~glyph_cache_manager()
    {
        // block_allocator will free all memory.
    }

    void set_signature(const char* font_signature)
    {
        m_signature = (char*)m_allocator.allocate((uint32_t)strlen(font_signature) + 1, 4);
        strcpy(m_signature, font_signature);
    }

    const char* signature(void) const
    {
        return m_signature;
    }

    const glyph* find_glyph(uint32_t code) const
    {
        uint32_t msb = (code >> 8) & 0xFF;
        if (m_glyphs[msb]) { // find row
            return m_glyphs[msb][code & 0xFF];
        }
        return 0;
    }

    glyph* cache_glyph(uint32_t code, uint32_t index, uint32_t data_size, glyph_type data_type,
                       const rect& bounds, scalar height, scalar advance_x, scalar advance_y)
    {
        uint32_t msb = (code >> 8) & 0xFF;
        if (m_glyphs[msb] == 0) { // cache row is empty.
            // alloc cache row.
            m_glyphs[msb] = (glyph**)m_allocator.allocate(sizeof(glyph*) * 256, sizeof(glyph*));
            memset(m_glyphs[msb], 0, sizeof(glyph*) * 256);
        }

        uint32_t lsb = code & 0xFF;
        if (m_glyphs[msb][lsb]) {
            return 0; // already exists.
        }

        glyph* g = (glyph*)m_allocator.allocate(sizeof(glyph), sizeof(int));

        g->code = code;
        g->index = index;
        g->data = m_allocator.allocate(data_size);
        g->data_size = data_size;
        g->type = data_type;
        g->bounds = bounds;
        g->height = height;
        g->advance_x = advance_x;
        g->advance_y = advance_y;
        return m_glyphs[msb][lsb] = g;
    }

private:
    block_allocator m_allocator;
    glyph** m_glyphs[256];
    char* m_signature;
};

}
#endif /*_PICASSO_FONT_CACHE_H_*/
