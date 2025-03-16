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

#include "psx_svg_render.h"
#include "psx_linear_allocator.h"

class render_obj_base : public psx_svg_render_obj
{
public:
    render_obj_base(const psx_svg_node* node)
    {
    }

    virtual ~render_obj_base()
    {
    }

    render_obj_base* next(void) const { return m_next; }
protected:
    render_obj_base* m_next;
};

class svg_render_list_impl : public psx_svg_render_list
{
public:
    svg_render_list_impl()
        : m_alloc(NULL)
    {
        m_alloc = psx_linear_allocator_create(PSX_LINEAR_ALIGN_DEFAULT);
    }

    virtual ~svg_render_list_impl()
    {
        psx_linear_allocator_destroy(m_alloc);
    }

    uint32_t get_bytes_size(void) const override
    {
        return (uint32_t)m_alloc->total_memory;
    }
private:
    psx_linear_allocator* m_alloc;
};

struct _svg_list_builder_state {
    const psx_svg_node* doc;
    svg_render_list_impl* list;
    render_obj_base* tail;
};

static bool svg_doc_walk(const psx_tree_node* node, void* data)
{
    return false;
}

static bool svg_doc_walk_before(const psx_tree_node* node, void* data)
{
    return false;
}

static bool svg_doc_walk_after(const psx_tree_node* node, void* data)
{
    return false;
}

#ifdef __cplusplus
extern "C" {
#endif

psx_svg_render_list* psx_svg_render_create(const psx_svg_node* doc)
{
    if (!doc) {
        LOG_ERROR("Invalid parameters!\n");
        return NULL;
    }

    svg_render_list_impl* list = new svg_render_list_impl();
    if (!list) {
        LOG_ERROR("Not enough memory for create render list!\n");
        return NULL;
    }

    _svg_list_builder_state state;
    state.doc = doc;
    state.list = list;
    state.tail = NULL;

    psx_tree_traversal<PSX_TREE_WALK_PRE_ORDER>(doc, &state, svg_doc_walk, svg_doc_walk_before, svg_doc_walk_after);

    return list;
}

void psx_svg_render_destroy(psx_svg_render_list* list)
{
    delete list;
}

#ifdef __cplusplus
}
#endif
