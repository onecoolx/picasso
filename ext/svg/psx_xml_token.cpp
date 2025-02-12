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

#include <ctype.h>
#include <string.h>

#include "psx_xml_token.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TOKEN_LEN(t) ((t)->end - (t)->start)

/*
 *   tag mask   entity mask  quote mask   tag   search  comment  doctype   xmlinst   server side   script
 * |  0 0 0   |   0 0 0    |    0 0     |  0  |   0   |    0   |    0    |    0    |      0     |    0    |
 */
enum {
    IN_TAG_MASK = (1 << 3) - 1, // we need 3 bits to store the state of tag.
    IN_ENTITY_MASK = (1 << 6) - (1 << 3), //we need 3 bits to store the state of entity.
    IN_QUOTE_MASK = (1 << 8) - (1 << 6), //we need 2 bit to store quote state.
    IN_START_TAG = 1 << 8,
    IN_SEARCH = 1 << 9,
    IN_COMMENT = 1 << 10,
    IN_DOCTYPE = 1 << 11,
    IN_XMLINST = 1 << 12,
    IN_SERVER_SIDE = 1 << 13,
    IN_SCRIPT = 1 << 14,
};

enum {
    NO_QUOTE = 0,
    SINGLE_QUOTE = 1,
    SDOUBLE_QUOTE = 2,
};

enum {
    NO_ENTITY = 0,
    START_ENTITY = 1,
    NUMERIC = 2,
    HEX_DECIMAL = 3,
    DECIMAL = 4,
    ENTITY_NAME = 5,
    SEMICOLON = 6,
};

enum {
    NO_TAG = 0,
    TAG_NAME = 1,
    ATTR_START = 2,
    ATTR_NAME = 3,
    SEARCH_EQUAL = 4,
    SEARCH_VALUE = 5,
    QUOTE_VALUE = 6,
    VALUE = 7,
};

typedef struct {
    uint32_t flags;
    const char* cur;
    const char* end;
} xml_token_state_t;

bool psx_xml_tokenizer(const char* xml_data, uint32_t data_len, xml_token_process, void* data)
{
    return false;
}

#ifdef __cplusplus
}
#endif
