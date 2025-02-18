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

#ifndef _PSX_XML_TOKEN_H_
#define _PSX_XML_TOKEN_H_

#include "psx_common.h"
#include "psx_array.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PSX_XML_BEGIN = 1,
    PSX_XML_END = 2,
    PSX_XML_CONTENT = 3,
    PSX_XML_ENTITY = 4,
} psx_xml_token_type_t;

typedef struct {
    const char* name_start;
    const char* name_end;
    const char* value_start;
    const char* value_end;
} psx_xml_token_attr_t;

enum {
    PSX_XML_TOKEN_FLAT = 1,
};

typedef struct {
    uint32_t flags;
    const char* start;
    const char* end;
    psx_xml_token_type_t type;
    psx_xml_token_attr_t* cur_attr;
    psx_array_t attrs;
} psx_xml_token_t;

typedef bool (*xml_token_process)(void* context, const psx_xml_token_t* token);

bool psx_xml_tokenizer(const char* xml_data, uint32_t data_len, xml_token_process cb, void* data);

#ifdef __cplusplus
}
#endif

#endif /* _PSX_XML_TOKEN_H_ */
