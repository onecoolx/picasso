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

#include "psx_linear_allocator.h"

TEST(LinearAllocatorTest, TestDefaultAlignment)
{
    psx_linear_allocator* allocator;
    allocator = psx_linear_allocator_create(PSX_LINEAR_ALIGN_DEFAULT);
    void* ptr = allocator->alloc(allocator, 100);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr) % PSX_LINEAR_ALIGN_DEFAULT, 0);
    psx_linear_allocator_destroy(allocator);
}

TEST(LinearAllocatorTest, TestAllocationSizes)
{
    psx_linear_allocator* allocator;
    allocator = psx_linear_allocator_create(PSX_LINEAR_ALIGN_DEFAULT);
    void* ptr1 = allocator->alloc(allocator, 1);
    ASSERT_NE(ptr1, nullptr);
    void* ptr4 = allocator->alloc(allocator, 4);
    ASSERT_NE(ptr4, nullptr);
    void* ptr10 = allocator->alloc(allocator, 10);
    ASSERT_NE(ptr10, nullptr);
    void* ptr38 = allocator->alloc(allocator, 38);
    ASSERT_NE(ptr38, nullptr);
    void* ptr122 = allocator->alloc(allocator, 122);
    ASSERT_NE(ptr122, nullptr);
    void* ptr2345 = allocator->alloc(allocator, 2345);
    ASSERT_NE(ptr2345, nullptr);
    psx_linear_allocator_destroy(allocator);
}

TEST(LinearAllocatorTest, TestAllocationOver64KB)
{
    psx_linear_allocator* allocator;
    allocator = psx_linear_allocator_create(PSX_LINEAR_ALIGN_DEFAULT);
    void* ptr = allocator->alloc(allocator, 65540); // > 64kb
    ASSERT_EQ(ptr, nullptr); // fail to allocate
    psx_linear_allocator_destroy(allocator);
}

TEST(LinearAllocatorTest, TestAlignment1Byte)
{
    psx_linear_allocator* allocator;
    allocator = psx_linear_allocator_create(PSX_LINEAR_ALIGN_1);
    void* ptr = allocator->alloc(allocator, 1);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr) % PSX_LINEAR_ALIGN_1, 0);
    psx_linear_allocator_destroy(allocator);
}

TEST(LinearAllocatorTest, TestAlignment4Bytes)
{
    psx_linear_allocator* allocator;
    allocator = psx_linear_allocator_create(PSX_LINEAR_ALIGN_4);
    void* ptr = allocator->alloc(allocator, 4);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr) % PSX_LINEAR_ALIGN_4, 0);
    psx_linear_allocator_destroy(allocator);
}

TEST(LinearAllocatorTest, TestAlignment10Bytes)
{
    psx_linear_allocator* allocator;
    allocator = psx_linear_allocator_create(PSX_LINEAR_ALIGN_1);
    void* ptr = allocator->alloc(allocator, 10);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr) % PSX_LINEAR_ALIGN_1, 0);
    psx_linear_allocator_destroy(allocator);
}

TEST(LinearAllocatorTest, TestAlignment38Bytes)
{
    psx_linear_allocator* allocator;
    allocator = psx_linear_allocator_create(PSX_LINEAR_ALIGN_2);
    void* ptr = allocator->alloc(allocator, 38);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr) % PSX_LINEAR_ALIGN_2, 0);
    psx_linear_allocator_destroy(allocator);
}

TEST(LinearAllocatorTest, TestAlignment122Bytes)
{
    psx_linear_allocator* allocator;
    allocator = psx_linear_allocator_create(PSX_LINEAR_ALIGN_8);
    void* ptr = allocator->alloc(allocator, 122);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr) % PSX_LINEAR_ALIGN_8, 0);
    psx_linear_allocator_destroy(allocator);
}

TEST(LinearAllocatorTest, TestAlignment2345Bytes)
{
    psx_linear_allocator* allocator;
    allocator = psx_linear_allocator_create(PSX_LINEAR_ALIGN_64);
    void* ptr = allocator->alloc(allocator, 2345);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr) % PSX_LINEAR_ALIGN_64, 0);
    psx_linear_allocator_destroy(allocator);
}
