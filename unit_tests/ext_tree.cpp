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

#include "test.h"
#include "timeuse.h"

#include "psx_tree.h"

class test_tree : public psx_tree_node
{
public:
    test_tree(test_tree* parent)
        : psx_tree_node(parent)
    {
    }

    virtual ~test_tree()
    {
    }
};

class PsxTreeTest : public ::testing::Test
{
protected:
    test_tree* root;

    virtual void SetUp()
    {
        root = new test_tree(nullptr);
    }

    virtual void TearDown()
    {
        delete root;
    }
};

TEST_F(PsxTreeTest, CreateAndDestroy)
{
    EXPECT_EQ(root->child_count(), 0u);
    EXPECT_EQ(root->parent(), nullptr);

    test_tree* child = new test_tree(root);
    EXPECT_EQ(root->child_count(), 1u);
    EXPECT_EQ(root->get_child(0), child);

    delete child;
    EXPECT_EQ(root->child_count(), 1u); // chrrent child is nullptr
}

bool b_work(const psx_tree_node* node, void* data)
{
    psx_tree_node** p = static_cast<psx_tree_node**>(data);
    *p = const_cast<psx_tree_node*>(node);
    return true;
}

bool tree_work(const psx_tree_node* node, void* data)
{
    psx_tree_node** p = static_cast<psx_tree_node**>(data);
    printf("node access : %p  === %p\n", *p, node);
    if (node == *p) {
        return true;
    }
    return false;
}

bool a_work(const psx_tree_node* node, void* data)
{
    psx_tree_node** p = static_cast<psx_tree_node**>(data);
    *p = nullptr;
    return true;
}

bool b_work2(const psx_tree_node* node, void* data)
{
    return true;
}

bool tree_work2(const psx_tree_node* node, void* data)
{
    uint32_t* p = static_cast<uint32_t*>(data);
    (*p)++;
    return true;
}

bool a_work2(const psx_tree_node* node, void* data)
{
    return true;
}

TEST_F(PsxTreeTest, TraversalPreOrder)
{
    test_tree* child1 = new test_tree(root);
    test_tree* child2 = new test_tree(root);

    psx_tree_node* cb_data = nullptr;
    psx_tree_traversal<PSX_TREE_WALK_PRE_ORDER>(root, &cb_data, tree_work, b_work, a_work);
    EXPECT_EQ(cb_data, nullptr);

    delete child2;
    delete child1;
}

TEST_F(PsxTreeTest, TraversalPostOrder)
{
    test_tree* child1 = new test_tree(root);
    test_tree* child2 = new test_tree(root);

    uint32_t cb_data = 0;
    psx_tree_traversal<PSX_TREE_WALK_POST_ORDER>(root, &cb_data, tree_work2, b_work2, a_work2);
    EXPECT_EQ(cb_data, 3u);

    delete child2;
    delete child1;
}

TEST_F(PsxTreeTest, CallbackCombination)
{
    test_tree* child = new test_tree(root);
    new test_tree(root); // remove with parent
    new test_tree(root); // remove with parent
    new test_tree(root); // remove with parent
    new test_tree(root); // remove with parent

    psx_tree_node* cb_data = nullptr;
    psx_tree_traversal<PSX_TREE_WALK_PRE_ORDER>(root, &cb_data, tree_work, b_work, a_work);
    EXPECT_EQ(cb_data, nullptr);

    delete child;
}
