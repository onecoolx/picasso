#include "test.h"
#include "timeuse.h"

#include "psx_tree.h"

class test_tree : public tree_node {
public:
    test_tree(test_tree * parent)
        : tree_node(parent)
    {
        printf("create tree node [%p]\n", this);
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
