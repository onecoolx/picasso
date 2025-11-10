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
#define _DEBUG 1
#include "src/include/data_vector.h"

using namespace picasso;

struct data_test {
    int i;
    float f;
    double d;
};

TEST(Pod_Vector, CreateAndDestroy)
{
    pod_vector<unsigned int> iv;
    EXPECT_EQ(0, (int)iv.size());
    EXPECT_EQ(0, (int)iv.capacity());

    {
        pod_vector<unsigned int> sv = iv;
        EXPECT_EQ(0, (int)sv.size());
        EXPECT_EQ(0, (int)sv.capacity());

        EXPECT_NE(&iv, &sv);
    }

    pod_vector<data_test> dv(10);

    EXPECT_EQ(0, (int)dv.size());
    EXPECT_EQ(10, (int)dv.capacity());
}

TEST(Pod_Vector, PushAndInsert)
{
    pod_vector<unsigned int> iv;
    pod_vector<unsigned int> sv = iv;

    iv.resize(3);
    sv.resize(5);

    EXPECT_EQ(0, (int)iv.size());
    EXPECT_EQ(3, (int)iv.capacity());

    EXPECT_EQ(0, (int)sv.size());
    EXPECT_EQ(5, (int)sv.capacity());

    bool b;
    b = iv.push_back(10);
    EXPECT_EQ(true, b);

    b = iv.push_back(11);
    EXPECT_EQ(true, b);

    b = iv.push_back(12);
    EXPECT_EQ(true, b);

    b = iv.push_back(13);
    EXPECT_EQ(false, b);

    for (unsigned int i = 0; i < iv.size(); i++) {
        printf("iv: integer vector[%d] = %d\n", i, iv[i]);
    }

    sv = iv;

    EXPECT_EQ(3, (int)sv.size());
    EXPECT_EQ(5, (int)sv.capacity());

    EXPECT_EQ(false, sv.is_full());
    EXPECT_EQ(true, iv.is_full());

    for (unsigned int i = 0; i < sv.size(); i++) {
        printf("sv: integer vector[%d] = %d\n", i, sv[i]);
    }

    iv.resize(6);
    for (unsigned int i = 0; i < iv.size(); i++) {
        EXPECT_EQ(sv[i], iv[i]);
    }

    EXPECT_EQ(false, iv.is_full());
    EXPECT_EQ(3, (int)iv.size());
    EXPECT_EQ(6, (int)iv.capacity());

    b = iv.insert_at(2, 15);
    EXPECT_EQ(true, b);
    EXPECT_EQ(4, (int)iv.size());
    for (unsigned int i = 0; i < iv.size(); i++) {
        printf("iv: integer vector[%d] = %d\n", i, iv[i]);
    }

    b = iv.insert_at(15, 15);
    EXPECT_EQ(false, b);
    EXPECT_EQ(4, (int)iv.size());

    EXPECT_NE(iv.data(), sv.data());

    sv.capacity(1);
    EXPECT_EQ(0, (int)sv.size());
    EXPECT_EQ(5, (int)sv.capacity());

    sv.capacity(10);
    EXPECT_EQ(0, (int)sv.size());
    EXPECT_EQ(10, (int)sv.capacity());

    iv.cut_at(2);
    EXPECT_EQ(2, (int)iv.size());
    EXPECT_EQ(6, (int)iv.capacity());
    for (unsigned int i = 0; i < iv.size(); i++) {
        printf("iv: integer vector[%d] = %d\n", i, iv[i]);
    }

    unsigned int data[] = {100, 200, 300};
    iv.set_data(3, data);
    EXPECT_EQ(3, (int)iv.size());
    EXPECT_EQ(6, (int)iv.capacity());
    for (unsigned int i = 0; i < iv.size(); i++) {
        printf("iv: integer vector[%d] = %d\n", i, iv[i]);
    }

    unsigned int data2[] = {50, 150, 250, 350, 450, 550, 650, 750};
    iv.set_data(8, data2);
    EXPECT_EQ(3, (int)iv.size());
    EXPECT_EQ(6, (int)iv.capacity());
    for (unsigned int i = 0; i < iv.size(); i++) {
        printf("iv: integer vector[%d] = %d\n", i, iv[i]);
    }

    iv.resize(8);
    iv.set_data(8, data2);
    EXPECT_EQ(8, (int)iv.size());
    EXPECT_EQ(8, (int)iv.capacity());
    for (unsigned int i = 0; i < iv.size(); i++) {
        printf("iv: integer vector[%d] = %d\n", i, iv[i]);
    }

    iv.clear();

    EXPECT_EQ(0, (int)iv.size());
    EXPECT_EQ(8, (int)iv.capacity());
}

TEST(Pod_BVector, BlockAndAutoSizeVector)
{
    pod_bvector<unsigned int> iv;
    EXPECT_EQ(0, (int)iv.size());
    EXPECT_EQ(0, (int)iv.capacity());

    iv.add(10);
    iv.add(11);
    iv.add(12);

    EXPECT_EQ(3, (int)iv.size());
    EXPECT_EQ(32, (int)iv.capacity());

    iv.add(13);
    iv.add(14);

    EXPECT_EQ(5, (int)iv.size());
    EXPECT_EQ(32, (int)iv.capacity());

    for (unsigned int i = 0; i < iv.size(); i++) {
        printf("iv: integer vector[%d] = %d\n", i, iv[i]);
    }

    iv.remove_last();

    EXPECT_EQ(4, (int)iv.size());
    EXPECT_EQ(32, (int)iv.capacity());
    for (unsigned int i = 0; i < iv.size(); i++) {
        printf("iv: integer vector[%d] = %d\n", i, iv[i]);
    }

    iv.remove_all();

    EXPECT_EQ(0, (int)iv.size());
    EXPECT_EQ(0, (int)iv.capacity());

}

TEST(Block_Allocater, BlockBaseAllocater)
{
    block_allocator alloc(16384 - 16);

    printf("alloc 100 elements aligment 4\n");
    data_test* ds = (data_test*)alloc.allocate(sizeof(data_test) * 256, 4);
    EXPECT_EQ(true, ds != 0);

    printf("block memsize %d \n", alloc.all_mem_used());

    data_test* ss = (data_test*)alloc.allocate(sizeof(data_test), 8);
    EXPECT_EQ(true, ss != 0);
    printf("block memsize %d \n", alloc.all_mem_used());

    printf("free all memory\n");
    alloc.remove_all();

    printf("block memsize %d \n", alloc.all_mem_used());
}

TEST(Pod_Array, CreateAndInitialize)
{
    pod_array<unsigned int> iv;
    pod_array<unsigned int> sv;

    EXPECT_EQ(0, (int)iv.size());
    EXPECT_EQ(0, (int)sv.size());

    EXPECT_NE(&iv, &sv);

    pod_array<data_test> dv(10);
    EXPECT_EQ(10, (int)dv.size());
}
