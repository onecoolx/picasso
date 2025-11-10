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

#include "psx_array.h"

class PsxArrayTest : public ::testing::Test
{
protected:
    psx_array array;

    void SetUp() override
    {
        psx_array_init_type(&array, int);
    }

    void TearDown() override
    {
        psx_array_destroy(&array);
    }
};

TEST_F(PsxArrayTest, CreateTest)
{
    EXPECT_EQ(psx_array_size(&array), 0);
    EXPECT_EQ(psx_array_capacity(&array), PSX_ARRAY_DEFAULT_CAPACITY);
    EXPECT_EQ(array.element_size, sizeof(int));
    EXPECT_NE(array.data, nullptr);
}

TEST_F(PsxArrayTest, AppendTest)
{
    int value = 10;
    EXPECT_TRUE(psx_array_append(&array, &value));
    EXPECT_EQ(*psx_array_get(&array, 0, int), value);
    EXPECT_EQ(psx_array_size(&array), 1);

    psx_array_resize(&array, 0);
    EXPECT_TRUE(psx_array_append(&array, nullptr));
}

TEST_F(PsxArrayTest, ExpandTest)
{
    for (int i = 0; i < PSX_ARRAY_DEFAULT_CAPACITY; ++i) {
        int value = i;
        EXPECT_TRUE(psx_array_append(&array, &value));
    }
    EXPECT_EQ(psx_array_capacity(&array), PSX_ARRAY_DEFAULT_CAPACITY);
    EXPECT_EQ(psx_array_size(&array), PSX_ARRAY_DEFAULT_CAPACITY);

    int value = 100;
    EXPECT_TRUE(psx_array_append(&array, &value));
    EXPECT_EQ(psx_array_capacity(&array), PSX_ARRAY_DEFAULT_CAPACITY * 2);
    EXPECT_EQ(psx_array_size(&array), PSX_ARRAY_DEFAULT_CAPACITY + 1);
    EXPECT_EQ(*psx_array_get(&array, PSX_ARRAY_DEFAULT_CAPACITY, int), value);
}

TEST_F(PsxArrayTest, GetValueTest)
{
    int value = 10;
    psx_array_append(&array, &value);
    EXPECT_EQ(*psx_array_get(&array, 0, int), value);

    EXPECT_EQ(psx_array_get(&array, 1, int), nullptr);
    EXPECT_EQ(psx_array_get(&array, array.size, int), nullptr);
}

TEST_F(PsxArrayTest, ShrinkTest)
{
    for (int i = 0; i < PSX_ARRAY_DEFAULT_CAPACITY * 2; ++i) {
        int value = i;
        EXPECT_TRUE(psx_array_append(&array, &value));
    }
    EXPECT_EQ(psx_array_capacity(&array), PSX_ARRAY_DEFAULT_CAPACITY * 2);
    psx_array_shrink(&array);
    EXPECT_EQ(psx_array_capacity(&array), PSX_ARRAY_DEFAULT_CAPACITY * 2);
    array.size = PSX_ARRAY_DEFAULT_CAPACITY;
    psx_array_shrink(&array);
    EXPECT_EQ(psx_array_capacity(&array), PSX_ARRAY_DEFAULT_CAPACITY);
}

TEST_F(PsxArrayTest, ClearTest)
{
    int value = 10;
    psx_array_append(&array, &value);
    EXPECT_EQ(psx_array_size(&array), 1);
    psx_array_clear(&array);
    EXPECT_EQ(psx_array_size(&array), 0);
}

TEST_F(PsxArrayTest, EmptyTest)
{
    EXPECT_TRUE(psx_array_empty(&array));
    int value = 10;
    psx_array_append(&array, &value);
    EXPECT_FALSE(psx_array_empty(&array));
}

TEST_F(PsxArrayTest, RemoveTest)
{
    int value1 = 10, value2 = 20;
    psx_array_append(&array, &value1);
    psx_array_append(&array, &value2);
    EXPECT_TRUE(psx_array_remove(&array, 0));
    EXPECT_EQ(array.size, 1u);
    EXPECT_EQ(*(int*)psx_array_at(&array, 0), value2);

    EXPECT_FALSE(psx_array_remove(&array, 1));
    EXPECT_FALSE(psx_array_remove(&array, array.size));
}

TEST_F(PsxArrayTest, RemoveLastTest)
{
    int value1 = 10, value2 = 20;
    psx_array_append(&array, &value1);
    psx_array_append(&array, &value2);
    psx_array_remove_last(&array);
    EXPECT_EQ(array.size, 1u);
    EXPECT_EQ(*(int*)psx_array_at(&array, 0), value1);
}
