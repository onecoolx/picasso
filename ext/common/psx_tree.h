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

#ifndef _PSX_TREE_H_
#define _PSX_TREE_H_

#include "psx_common.h"

#define INIT_CAPACITY 4

class tree_node
{
public:
    tree_node(tree_node* parent)
        : m_parent(parent)
        , m_children(NULL)
        , m_count(0)
        , m_capacity(INIT_CAPACITY)
        , m_index(0)
    {
        m_children = (tree_node**)calloc(m_capacity, sizeof(tree_node*));
        if (m_parent) {
            m_parent->m_count++;
            if (m_parent->m_count == m_parent->m_capacity) {
                m_parent->m_capacity <<= 1;
                m_parent->m_children = (tree_node**)realloc(m_parent->m_children, sizeof(tree_node*) * m_parent->m_capacity);
            }
            m_parent->m_children[m_parent->m_count - 1] = this;
            m_index = m_parent->m_count;
        }
    }

    virtual ~tree_node()
    {
        if (m_parent) {
            if (m_parent->m_children[m_index - 1] == this) {
                m_parent->m_children[m_index - 1] = NULL;
            }
        }
        for (uint32_t i = 0; i < m_count; i++) {
            if (m_children[i]) {
                delete m_children[i];
            }
        }
        free(m_children);
    }

    uint32_t child_count(void) const { return m_count; }

    tree_node* parent(void) const { return m_parent; }

    tree_node* get_child(uint32_t idx) const
    {
        if (idx >= m_count) {
            return NULL;
        }
        return m_children[idx];
    }

private:
    tree_node* m_parent;
    tree_node** m_children;
    uint32_t m_count : 16;
    uint32_t m_capacity : 16;
    uint32_t m_index : 16;
};

typedef enum {
    PSX_TREE_WALK_PRE_ORDER,
    PSX_TREE_WALK_POST_ORDER,
} PSX_TREE_WALK_MODE;

typedef bool (*psx_tree_traversal_callback)(const tree_node* node, void* ctx);

template <PSX_TREE_WALK_MODE mode>
static bool psx_tree_traversal(const tree_node* tree_head, void* ctx, psx_tree_traversal_callback cb,
                               psx_tree_traversal_callback before = NULL,
                               psx_tree_traversal_callback after = NULL)
{
    if (tree_head && cb) {
        if (mode == PSX_TREE_WALK_PRE_ORDER) {
            if (before && !before(tree_head, ctx)) {
                return false;
            }
            if (!cb(tree_head, ctx)) {
                return false;
            }
        }

        for (uint32_t i = 0; i < tree_head->child_count(); i++) {
            if (!psx_tree_traversal<mode>(tree_head->get_child(i), ctx, cb, before, after)) {
                return false;
            }
        }

        if (mode == PSX_TREE_WALK_PRE_ORDER) {
            if (after && !after(tree_head, ctx)) {
                return false;
            }
        }

        if (mode == PSX_TREE_WALK_POST_ORDER) {
            if (before && !before(tree_head, ctx)) {
                return false;
            }
            if (!cb(tree_head, ctx)) {
                return false;
            }
            if (after && !after(tree_head, ctx)) {
                return false;
            }
        }
    }
    return true;
}

#endif /*_PSX_TREE_H_*/
