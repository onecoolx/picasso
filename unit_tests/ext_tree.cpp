#include "test.h"
#include "timeuse.h"

#include "psx_tree.h"

class test_tree : public psx_tree_node {
public:
    test_tree(test_tree * parent)
        : psx_tree_node(parent)
    {
    }
    
    virtual ~test_tree()
    {
    }
};

class PsxTreeTest : public ::testing::Test {
protected:
    test_tree* root;

    virtual void SetUp() {
        root = new test_tree(nullptr);
    }

    virtual void TearDown() {
        delete root;
    }
};

TEST_F(PsxTreeTest, CreateAndDestroy) {
    EXPECT_EQ(root->child_count(), 0u);
    EXPECT_EQ(root->parent(), nullptr);

    test_tree* child = new test_tree(root);
    EXPECT_EQ(root->child_count(), 1u);
    EXPECT_EQ(root->get_child(0), child);

    delete child;
    EXPECT_EQ(root->child_count(), 1u); // chrrent child is nullptr
}

bool b_work(const psx_tree_node* node, void* data) {
    psx_tree_node** p = static_cast<psx_tree_node**>(data);
    *p = const_cast<psx_tree_node*>(node);
    return true;
}

bool tree_work(const psx_tree_node* node, void* data) {
    psx_tree_node** p = static_cast<psx_tree_node**>(data);
    printf("node access : %p  === %p\n", *p, node);
    if (node == *p) {
        return true;
    }
    return false;
}

bool a_work(const psx_tree_node* node, void* data) {
    psx_tree_node** p = static_cast<psx_tree_node**>(data);
    *p = nullptr;
    return true;
}

bool b_work2(const psx_tree_node* node, void* data) {
    return true;
}

bool tree_work2(const psx_tree_node* node, void* data) {
    uint32_t* p = static_cast<uint32_t*>(data);
    (*p)++;
    return true;
}

bool a_work2(const psx_tree_node* node, void* data) {
    return true;
}

TEST_F(PsxTreeTest, TraversalPreOrder) {
    test_tree* child1 = new test_tree(root);
    test_tree* child2 = new test_tree(root);

    psx_tree_node* cb_data = nullptr;
    psx_tree_traversal<PSX_TREE_WALK_PRE_ORDER>(root, &cb_data, tree_work, b_work, a_work);
    EXPECT_EQ(cb_data, nullptr);

    delete child2;
    delete child1;
}

TEST_F(PsxTreeTest, TraversalPostOrder) {
    test_tree* child1 = new test_tree(root);
    test_tree* child2 = new test_tree(root);

    uint32_t cb_data = 0;
    psx_tree_traversal<PSX_TREE_WALK_POST_ORDER>(root, &cb_data, tree_work2, b_work2, a_work2);
    EXPECT_EQ(cb_data, 3u);

    delete child2;
    delete child1;
}

TEST_F(PsxTreeTest, CallbackCombination) {
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

