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
    DOUBLE_QUOTE = 2,
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

static INLINE void _psx_token_init(psx_xml_token_t* token)
{
    token->flags = 0;
    token->start = NULL;
    token->end = NULL;
    token->type = PSX_XML_CONTENT;
    token->cur_attr = NULL;
    psx_array_init_type(&token->attrs, psx_xml_token_attr_t);
}

static INLINE void _psx_token_clear(psx_xml_token_t* token)
{
    token->flags = 0;
    token->start = NULL;
    token->end = NULL;
    token->type = PSX_XML_CONTENT;
    token->cur_attr = NULL;
    psx_array_clear(&token->attrs);
}

static INLINE psx_xml_token_attr_t* _create_xml_attr(psx_xml_token_t* token)
{
    psx_array_append(&token->attrs, NULL);

    psx_xml_token_attr_t* attr = psx_array_get_last(&token->attrs, psx_xml_token_attr_t);
    memset(attr, 0, sizeof(psx_xml_token_attr_t));
    return attr;
}

static INLINE bool _xml_token_process(psx_xml_token_t* token, xml_token_process cb, void* data)
{
    if (!token->start || TOKEN_LEN(token) == 0) {
        return true;
    }

    bool ret = cb(data, token);
    _psx_token_clear(token);
    return ret;
}

static INLINE void _psx_set_state(xml_token_state_t* state, uint32_t bit)
{
    state->flags |= bit;
}

static INLINE void _psx_clear_state(xml_token_state_t* state, uint32_t bit)
{
    state->flags &= ~bit;
}

static INLINE bool _psx_is_state(xml_token_state_t* state, uint32_t bit)
{
    return state->flags & bit;
}

static INLINE void _psx_set_tag_state(xml_token_state_t* state, uint32_t bit)
{
    state->flags = (state->flags & ~IN_TAG_MASK) | bit;
}

static INLINE void _psx_set_quote_state(xml_token_state_t* state, uint32_t bit)
{
    state->flags = (state->flags & ~IN_QUOTE_MASK) | (bit << 6);
}

static INLINE bool _psx_special_handles(xml_token_state_t* state)
{
    return state->flags & (IN_START_TAG | IN_SEARCH | IN_TAG_MASK | IN_ENTITY_MASK | IN_COMMENT | IN_SERVER_SIDE | IN_DOCTYPE | IN_XMLINST | IN_SCRIPT);
}

static INLINE void _psx_proc_xml_inst(xml_token_state_t* state, psx_xml_token_t* token)
{
    // ignore xml inst
    while (state->cur <= state->end) {
        char ch = *(state->cur);
        if (ch == '>' && (*(state->cur - 1)) == '?') {
            _psx_clear_state(state, IN_XMLINST);
            state->cur++;
            break;
        }
        state->cur++;
    }
}

static INLINE void _psx_proc_comment(xml_token_state_t* state, psx_xml_token_t* token)
{
    // ignore comment
    while (state->cur <= state->end) {
        char ch = *(state->cur);
        if (ch == '>' && (*(state->cur - 1)) == '-' && (*(state->cur - 2)) == '-') {
            _psx_clear_state(state, IN_COMMENT);
            state->cur++;
            break;
        }
        state->cur++;
    }
}

static INLINE void _psx_proc_doctype(xml_token_state_t* state, psx_xml_token_t* token)
{
    // ignore doctype
    while (state->cur <= state->end) {
        char ch = *(state->cur);
        if (ch == '>') {
            _psx_clear_state(state, IN_DOCTYPE);
            state->cur++;
            break;
        }
        state->cur++;
    }
}

static INLINE bool _psx_proc_tag(xml_token_state_t* state, psx_xml_token_t* token, xml_token_process cb, void* data)
{
    while (state->cur <= state->end) {
        switch (state->flags & IN_TAG_MASK) {
            case NO_TAG: {
                    if (!_xml_token_process(token, cb, data)) {
                        return false;
                    }
                    state->cur++;
                }
                return true;
            case TAG_NAME: {
                    char ch = *(state->cur);
                    if (ch == '/') {
                        token->type = PSX_XML_END;
                        state->cur++;
                        if (!token->start) {
                            token->start = state->cur;
                        }
                        continue;
                    } else if (ch == '>' || isspace(ch)) {
                        token->end = state->cur;
                        _psx_set_tag_state(state, ATTR_START);
                        continue;
                    } else {
                        if (!token->start) {
                            token->type = PSX_XML_BEGIN;
                            token->start = state->cur;
                        }
                        state->cur++;
                        continue;
                    }
                }
                break;
            case ATTR_START: {
                    char ch = *(state->cur);
                    if (!isspace(ch) && ch != '\'' && ch != '\"') {
                        if (ch == '/') {
                            token->flags |= PSX_XML_TOKEN_FLAT;
                            state->cur++;
                            continue;
                        }
                        if (ch == '>') {
                            _psx_set_tag_state(state, NO_TAG);
                        } else {
                            token->cur_attr = NULL;
                            _psx_set_tag_state(state, ATTR_NAME);
                        }
                        continue;
                    }
                }
                break;
            case ATTR_NAME: {
                    if (!token->cur_attr) {
                        token->cur_attr = _create_xml_attr(token);
                    }
                    char ch = *(state->cur);
                    if (isspace(ch) || ch == '=' || ch == '/' || ch == '>') {
                        token->cur_attr->name_end = state->cur;
                        _psx_set_tag_state(state, SEARCH_EQUAL);
                        continue;
                    } else {
                        if (!token->cur_attr->name_start) {
                            token->cur_attr->name_start = state->cur;
                        }
                        state->cur++;
                        continue;
                    }
                }
                break;
            case SEARCH_EQUAL: {
                    char ch = *(state->cur);
                    if (!isspace(ch) && ch != '/' && ch != '\'' && ch != '\"') {
                        if (ch == '=') {
                            _psx_set_tag_state(state, SEARCH_VALUE);
                        } else {
                            // attr name has empty value
                            token->cur_attr = NULL;
                            _psx_set_tag_state(state, ATTR_START);
                            continue;
                        }
                    }
                }
                break;
            case SEARCH_VALUE: {
                    char ch = *(state->cur);
                    if (!isspace(ch)) {
                        if (ch == '\'' || ch == '\"') {
                            if (ch == '\'') {
                                _psx_set_quote_state(state, SINGLE_QUOTE);
                            } else {
                                _psx_set_quote_state(state, DOUBLE_QUOTE);
                            }
                            _psx_set_tag_state(state, QUOTE_VALUE);
                        } else {
                            _psx_set_tag_state(state, VALUE);
                            continue;
                        }
                    }
                }
                break;
            case QUOTE_VALUE: {
                    char ch = *(state->cur);
                    if ((ch == '\'' && ((state->flags & IN_QUOTE_MASK) >> 3) == SINGLE_QUOTE)
                        || (ch == '\"' && ((state->flags & IN_QUOTE_MASK) >> 3) == DOUBLE_QUOTE)) {
                        if (!token->cur_attr->value_start) {
                            token->cur_attr->value_start = state->cur;
                        }
                        token->cur_attr->value_end = state->cur;
                        _psx_set_quote_state(state, NO_QUOTE);
                        _psx_set_tag_state(state, ATTR_START);
                        continue;
                    } else {
                        if (!token->cur_attr->value_start) {
                            token->cur_attr->value_start = state->cur;
                        }
                        state->cur++;
                        continue;
                    }
                }
                break;
            case VALUE: {
                    char ch = *(state->cur);
                    if (isspace(ch) || ch == '>' || ch == '/') {
                        if (!token->cur_attr->value_start) {
                            token->cur_attr->value_start = state->cur;
                        }
                        token->cur_attr->value_end = state->cur;
                        _psx_set_quote_state(state, NO_QUOTE);
                        _psx_set_tag_state(state, ATTR_START);
                        continue;
                    } else {
                        if (!token->cur_attr->value_start) {
                            token->cur_attr->value_start = state->cur;
                        }
                        state->cur++;
                        continue;
                    }
                }
                break;
        }
        state->cur++;
    }
    return true;
}

bool psx_xml_tokenizer(const char* xml_data, uint32_t data_len, xml_token_process cb, void* data)
{
    psx_xml_token_t token;
    _psx_token_init(&token);

    xml_token_state_t state;
    state.flags = 0;
    state.cur = xml_data;
    state.end = xml_data + data_len;

    while (state.cur < state.end) {
        char ch = *(state.cur);
        if (ch == '\r' || ch == '\n') { // skip LR character
            state.cur++;
            continue;
        } else if (_psx_special_handles(&state)) {
            if (_psx_is_state(&state, IN_START_TAG)) {
                _psx_clear_state(&state, IN_START_TAG);

                switch (ch) {
                    case '/': // end tag
                        _psx_set_tag_state(&state, TAG_NAME);
                        break;
                    case '!': {
                            // <!-- or <!DOCTYPE
                            _psx_set_state(&state, IN_SEARCH);
                            state.cur++;
                        }
                        break;
                    case '?': {
                            // xml instruction
                            _psx_set_state(&state, IN_XMLINST);
                            state.cur++;
                        }
                        break;
                    default: {
                            if (isalpha(ch)) {
                                _psx_set_tag_state(&state, TAG_NAME);
                            } else {
                                psx_array_destroy(&token.attrs);
                                return false;
                            }
                        }
                }
                // process token
                if (!_xml_token_process(&token, cb, data)) {
                    psx_array_destroy(&token.attrs);
                    return false;
                }
            } else if (_psx_is_state(&state, IN_SEARCH)) {
                if (ch == '-' || isalpha(ch)) {
                    if (!token.start) {
                        token.start = state.cur;
                    }
                    token.end = state.cur;
                } else {
                    // processing as a normal tag name.
                    _psx_clear_state(&state, IN_SEARCH);
                    _psx_set_tag_state(&state, TAG_NAME);
                    continue;
                }

                if (((token.end - token.start) == 1) && (token.start[0] == '-') && (token.start[1] == '-')) {
                    // is <!-- comment start
                    _psx_clear_state(&state, IN_SEARCH);
                    token.start = token.end = NULL;
                    _psx_set_state(&state, IN_COMMENT);
                } else if (((token.end - token.start) == 6) && (strncmp(token.start, "DOCTYPE", 7) == 0)) {
                    _psx_clear_state(&state, IN_SEARCH);
                    token.start = token.end = NULL;
                    _psx_set_state(&state, IN_DOCTYPE);
                }
                state.cur++;
            } else if (_psx_is_state(&state, IN_COMMENT)) {
                _psx_proc_comment(&state, &token);
            } else if (_psx_is_state(&state, IN_DOCTYPE)) {
                _psx_proc_doctype(&state, &token);
            } else if (_psx_is_state(&state, IN_TAG_MASK)) {
                if (!_psx_proc_tag(&state, &token, cb, data)) {
                    psx_array_destroy(&token.attrs);
                    return false;
                }
            } else if (_psx_is_state(&state, IN_XMLINST)) {
                _psx_proc_xml_inst(&state, &token);
            }
        } else {
            switch (ch) {
                case '<': {
                        _psx_set_state(&state, IN_START_TAG); // start a new tag
                        state.cur++;
                    }
                    break;
                default: {
                        if (!token.start) {
                            token.start = state.cur;
                        }
                        if (state.cur == state.end) {
                            goto finish; // make santizer happy
                        }
                        token.end = ++state.cur;
                    }
            }
        }
    }

finish:
    psx_array_destroy(&token.attrs);
    return false;
}

#ifdef __cplusplus
}
#endif
