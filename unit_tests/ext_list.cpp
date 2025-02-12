#include "test.h"
#include "timeuse.h"

#include "psx_list.h"

TEST(ListTest, TestListInit)
{
    list_hdr_t list;
    list_init(&list);
    EXPECT_EQ(&list, list.next);
    EXPECT_EQ(&list, list.prev);
}

TEST(ListTest, TestListEmpty)
{
    list_hdr_t list, entry;
    list_init(&list);
    EXPECT_TRUE(list_empty(&list));
    list_add_entry(&list, &list);
    EXPECT_TRUE(list_empty(&list));
    list_add_entry(&list, &entry);
    EXPECT_FALSE(list_empty(&list));
}

TEST(ListTest, TestListAdd)
{
    list_hdr_t list, entry;
    list_init(&list);
    list_add(&list, &entry);
    EXPECT_FALSE(list_empty(&list));
    EXPECT_EQ(&entry, list.next);
    EXPECT_EQ(&list, entry.prev);
}

TEST(ListTest, TestListAddTail)
{
    list_hdr_t list, entry;
    list_init(&list);
    list_add_tail(&list, &entry);
    EXPECT_FALSE(list_empty(&list));
    EXPECT_EQ(&entry, list.prev);
    EXPECT_EQ(&list, entry.next);
}

TEST(ListTest, TestListRemove)
{
    list_hdr_t list, entry;
    list_init(&list);
    list_add(&list, &entry);
    list_remove_entry(&entry);
    EXPECT_TRUE(list_empty(&list));
}

TEST(ListTest, TestListForEach)
{
    list_hdr_t list, entry1, entry2;
    list_init(&list);
    list_add(&list, &entry1);
    list_add(&list, &entry2);
    list_hdr_t* iterator;
    list_for_each(&list, iterator) {
        EXPECT_TRUE(&entry1 == iterator || &entry2 == iterator);
    }
}