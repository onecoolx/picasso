#include "test.h"
#include "timeuse.h"

#include "psx_tree.h"

class test_tree : public tree_node {
public:
    test_tree(test_tree * parent)
        : tree_node(parent)
    {
        printf("create tree node [%p] : size [%zd]\n", this, sizeof(test_tree));
    }
    
    virtual ~test_tree()
    {
        printf("delete tree node [%p]\n", this);
    }
};

TEST(ExtTree, CreateAndDestroy)
{
    test_tree * p1 = new test_tree(0);
    test_tree * p2 = new test_tree(p1);
    test_tree * p3 = new test_tree(p1);
    test_tree * p4 = new test_tree(p2);

    EXPECT_EQ(p1->child_count(), 2);
    EXPECT_EQ(p2->child_count(), 1);

    EXPECT_EQ(p1->get_child(1), p3);
    EXPECT_EQ(p2->get_child(0), p4);

    delete p3;
    
    // p3 remove from parent 
    EXPECT_EQ(p1->get_child(1), nullptr);
    EXPECT_EQ(p1->child_count(), 2);

    delete p2;
    delete p1;
}


static inline bool b_work(const tree_node* node, void * data)
{
    tree_node ** p = (tree_node**)data;
    *p = (tree_node*)node;
    return true;
}


static inline bool tree_work(const tree_node* node, void * data)
{
    tree_node ** p = (tree_node**)data;
    if (node == *p)
        return true;
    return false;
}

static inline bool a_work(const tree_node* node, void * data)
{
    tree_node ** p = (tree_node**)data;
    *p = nullptr;
    return true;
}


TEST(ExtTree, TreeWalk)
{
    test_tree * com_ptr = nullptr;

    test_tree * p1 = new test_tree(0);
    test_tree * p2 = new test_tree(p1);
    test_tree * p3 = new test_tree(p1);
    test_tree * p4 = new test_tree(p2);
    test_tree * p5 = new test_tree(p1);
    test_tree * p6 = new test_tree(p1);
    test_tree * p7 = new test_tree(p1);

    psx_tree_traversal<PSX_TREE_WALK_PRE_ORDER>(p1, (void*)&com_ptr, tree_work, b_work, a_work);

    EXPECT_EQ(com_ptr, nullptr);

    EXPECT_EQ(p1->get_child(0), p2);
    EXPECT_EQ(p1->get_child(1), p3);
    EXPECT_EQ(p1->get_child(2), p5);
    EXPECT_EQ(p1->get_child(3), p6);
    EXPECT_EQ(p1->get_child(4), p7);
    EXPECT_EQ(p2->get_child(0), p4);

    delete p1;
}
