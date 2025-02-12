#include "test.h"
#include "timeuse.h"

#include "psx_array.h"

class PsxArrayTest : public ::testing::Test
{
protected:
    psx_array_t array;

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