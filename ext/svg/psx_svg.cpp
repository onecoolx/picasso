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

#include "psx_svg.h"
#include "psx_svg_parser.h"

#include <math.h>

static INLINE void _free_svg_attr_value(psx_svg_attr* attr)
{
    if (attr->val_type == SVG_ATTR_VALUE_PTR) {
        mem_free(attr->value.val);
    } else if (attr->val_type == SVG_ATTR_VALUE_PATH_PTR) {
        ps_path_unref((ps_path*)attr->value.val);
    }
    // FIXME: release more type
}

psx_svg_node::psx_svg_node(psx_svg_node* parent)
    : psx_tree_node(parent)
    , m_data(NULL)
    , m_len(0)
    , m_tag(SVG_TAG_INVALID)
    , m_render_obj(NULL)
{
    psx_array_init_type(&m_attrs, psx_svg_attr);
}

psx_svg_node::~psx_svg_node()
{
    if (m_data) {
        mem_free(m_data);
    }

    for (uint32_t i = 0; i < psx_array_size(&m_attrs); i++) {
        psx_svg_attr* attr = psx_array_get(&m_attrs, i, psx_svg_attr);
        _free_svg_attr_value(attr);
    }
    psx_array_destroy(&m_attrs);
}

void psx_svg_node::set_content(const char* data, uint32_t len)
{
    if (m_data) {
        mem_free(m_data);
    }

    m_data = (char*)mem_malloc(len + 1);
    mem_copy(m_data, data, len);
    m_data[len] = '\0';
    m_len = len;
}

static bool svg_token_process(void* context, const psx_xml_token* token)
{
    psx_svg_parser* parser = (psx_svg_parser*)context;
    return psx_svg_parser_token(parser, token);
}

#ifdef __cplusplus
extern "C" {
#endif

psx_svg_node* psx_svg_load_data(const char* svg_data, uint32_t len)
{
    if (!svg_data || !len) {
        LOG_ERROR( "Bad arguments for svg data or length!\n");
        return NULL;
    }

    psx_svg_parser parser;
    psx_svg_parser_init(&parser);

    if (psx_xml_tokenizer(svg_data, len, svg_token_process, &parser)) {
        if (psx_svg_parser_is_finish(&parser)) {
            psx_svg_node* doc = parser.doc_root;
            parser.doc_root = NULL;
            psx_svg_parser_destroy(&parser);
#ifdef _DEBUG
            psx_svg_dump_tree(doc, 0);
#endif
            return doc;
        } else {
            psx_svg_parser_destroy(&parser);
            LOG_ERROR( "SVG document parser raise errors!\n");
            return NULL;
        }
    } else {
        psx_svg_parser_destroy(&parser);
        LOG_ERROR( "SVG document tokenizer raise errors!\n");
        return NULL;
    }
}

psx_svg_node* psx_svg_node_create(psx_svg_node* parent)
{
    return new psx_svg_node(parent);
}

void psx_svg_node_destroy(psx_svg_node* node)
{
    delete node;
}

// svg matrix
static INLINE bool _matrix_translation_only(const psx_svg_matrix* matrix)
{
    return (matrix->m[0][0] == 1.0f &&
            matrix->m[0][1] == 0.0f &&
            matrix->m[1][0] == 0.0f &&
            matrix->m[1][1] == 1.0f &&
            matrix->m[2][0] == 0.0f &&
            matrix->m[2][1] == 0.0f &&
            matrix->m[2][2] == 1.0f);
}

static INLINE void _matrix_multiply(psx_svg_matrix* matrix, const psx_svg_matrix* matrix2)
{
    psx_svg_matrix result;
    result.m[0][0] = matrix->m[0][0] * matrix2->m[0][0] + matrix->m[0][1] * matrix2->m[1][0] + matrix->m[0][2] *
                     matrix2->m[2][0];
    result.m[0][1] = matrix->m[0][0] * matrix2->m[0][1] + matrix->m[0][1] * matrix2->m[1][1] + matrix->m[0][2] *
                     matrix2->m[2][1];
    result.m[0][2] = matrix->m[0][0] * matrix2->m[0][2] + matrix->m[0][1] * matrix2->m[1][2] + matrix->m[0][2] *
                     matrix2->m[2][2];

    result.m[1][0] = matrix->m[1][0] * matrix2->m[0][0] + matrix->m[1][1] * matrix2->m[1][0] + matrix->m[1][2] *
                     matrix2->m[2][0];
    result.m[1][1] = matrix->m[1][0] * matrix2->m[0][1] + matrix->m[1][1] * matrix2->m[1][1] + matrix->m[1][2] *
                     matrix2->m[2][1];
    result.m[1][2] = matrix->m[1][0] * matrix2->m[0][2] + matrix->m[1][1] * matrix2->m[1][2] + matrix->m[1][2] *
                     matrix2->m[2][2];

    result.m[2][0] = matrix->m[2][0] * matrix2->m[0][0] + matrix->m[2][1] * matrix2->m[1][0] + matrix->m[2][2] *
                     matrix2->m[2][0];
    result.m[2][1] = matrix->m[2][0] * matrix2->m[0][1] + matrix->m[2][1] * matrix2->m[1][1] + matrix->m[2][2] *
                     matrix2->m[2][1];
    result.m[2][2] = matrix->m[2][0] * matrix2->m[0][2] + matrix->m[2][1] * matrix2->m[1][2] + matrix->m[2][2] *
                     matrix2->m[2][2];

    *matrix = result;
}

void psx_svg_matrix_identity(psx_svg_matrix* matrix)
{
    if (!matrix) {
        return;
    }
    memset(matrix, 0, sizeof(psx_svg_matrix));
    matrix->m[0][0] = matrix->m[1][1] = matrix->m[2][2] = 1.0f; // identity
}

void psx_svg_matrix_init(psx_svg_matrix* matrix, float a, float b, float c, float d, float e, float f)
{
    if (!matrix) {
        return;
    }

    matrix->m[0][0] = a;
    matrix->m[0][1] = c;
    matrix->m[0][2] = e;
    matrix->m[1][0] = b;
    matrix->m[1][1] = d;
    matrix->m[1][2] = f;
    matrix->m[2][0] = 0.0f;
    matrix->m[2][1] = 0.0f;
    matrix->m[2][2] = 1.0f;
}

void psx_svg_matrix_translate(psx_svg_matrix* matrix, float tx, float ty)
{
    if (!matrix) {
        return;
    }

    if (_matrix_translation_only(matrix)) {
        matrix->m[0][2] += tx;
        matrix->m[1][2] += ty;
        return;
    }

    psx_svg_matrix m = {{
            {1.0f, 0.0f, tx},
            {0.0f, 1.0f, ty},
            {0.0f, 0.0f, 1.0f},
        }
    };

    _matrix_multiply(matrix, &m);
}

void psx_svg_matrix_scale(psx_svg_matrix* matrix, float sx, float sy)
{
    if (!matrix) {
        return;
    }

    psx_svg_matrix m = {{
            {sx, 0.0f, 0.0f},
            {0.0f, sy, 0.0f},
            {0.0f, 0.0f, 1.0f},
        }
    };

    _matrix_multiply(matrix, &m);
}

void psx_svg_matrix_rotate(psx_svg_matrix* matrix, float rad)
{
    if (!matrix) {
        return;
    }

    float cosr = cosf(rad);
    float sinr = sinf(rad);

    psx_svg_matrix m = {{
            {cosr, -sinr, 0.0f},
            {sinr, cosr, 0.0f},
            {0.0f, 0.0f, 1.0f},
        }
    };

    _matrix_multiply(matrix, &m);
}

void psx_svg_matrix_skew(psx_svg_matrix* matrix, float shx, float shy)
{
    if (!matrix) {
        return;
    }

    float tanx = tanf(shx);
    float tany = tanf(shy);

    psx_svg_matrix m = {{
            {1.0f, tanx, 0.0f},
            {tany, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
        }
    };

    _matrix_multiply(matrix, &m);
}

#ifdef __cplusplus
}
#endif
