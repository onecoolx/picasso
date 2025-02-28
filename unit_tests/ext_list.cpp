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

#include "psx_list.h"

TEST(ListTest, TestListInit)
{
    list_hdr list;
    list_init(&list);
    EXPECT_EQ(&list, list.next);
    EXPECT_EQ(&list, list.prev);
}

TEST(ListTest, TestListEmpty)
{
    list_hdr list, entry;
    list_init(&list);
    EXPECT_TRUE(list_empty(&list));
    list_add_entry(&list, &list);
    EXPECT_TRUE(list_empty(&list));
    list_add_entry(&list, &entry);
    EXPECT_FALSE(list_empty(&list));
}

TEST(ListTest, TestListAdd)
{
    list_hdr list, entry;
    list_init(&list);
    list_add(&list, &entry);
    EXPECT_FALSE(list_empty(&list));
    EXPECT_EQ(&entry, list.next);
    EXPECT_EQ(&list, entry.prev);
}

TEST(ListTest, TestListAddTail)
{
    list_hdr list, entry;
    list_init(&list);
    list_add_tail(&list, &entry);
    EXPECT_FALSE(list_empty(&list));
    EXPECT_EQ(&entry, list.prev);
    EXPECT_EQ(&list, entry.next);
}

TEST(ListTest, TestListRemove)
{
    list_hdr list, entry;
    list_init(&list);
    list_add(&list, &entry);
    list_remove_entry(&entry);
    EXPECT_TRUE(list_empty(&list));
}

TEST(ListTest, TestListForEach)
{
    list_hdr list, entry1, entry2;
    list_init(&list);
    list_add(&list, &entry1);
    list_add(&list, &entry2);
    list_hdr* iterator;
    list_for_each(&list, iterator) {
        EXPECT_TRUE(&entry1 == iterator || &entry2 == iterator);
    }
}
