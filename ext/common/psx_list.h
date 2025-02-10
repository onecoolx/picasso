/*
 * Copyright (c) 2016, Zhang Ji Peng
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

#ifndef _PSX_LIST_H_
#define _PSX_LIST_H_

#include "psx_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Doubly linked list implementation.
 *  Must be the first field of a data struct
 */
typedef struct list_hdr {
    struct list_hdr* next;
    struct list_hdr* prev;
} list_hdr_t;

static INLINE void list_init(struct list_hdr* head)
{
    head->next = head;
    head->prev = head;
}

static INLINE bool list_empty(const struct list_hdr* head)
{
    return head->next == head;
}

#define list_add(head, value) \
    list_add_entry((struct list_hdr*)(head), (struct list_hdr*)(value))

static INLINE void list_add_entry(struct list_hdr* head, struct list_hdr* value)
{
    REGISTER struct list_hdr* next = head->next;

    next->prev = value;
    value->next = next;
    value->prev = head;
    head->next = value;
}

#define list_add_tail(head, value) \
    list_add_tail_entry((struct list_hdr*)(head), (struct list_hdr*)(value))

static INLINE void list_add_tail_entry(struct list_hdr* head, struct list_hdr* value)
{
    REGISTER struct list_hdr* prev = head->prev;

    head->prev = value;
    value->next = head;
    value->prev = prev;
    prev->next = value;
}

#define list_remove(entry) \
    list_remove_entry((struct list_hdr*)(entry))

static INLINE void list_remove_entry(struct list_hdr* entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
    entry->next = entry->prev = NULL;
}

#define list_for_each(head, iterator) \
    for ((iterator) = (head)->next; (iterator) != (head); (iterator) = (iterator)->next)

#define list_for_each_start_with(head, start, iterator) \
    for ((iterator) = ((struct list_hdr*)(start))->next; (iterator) != (head); (iterator) = (iterator)->next)

#ifdef __cplusplus
}
#endif

#endif /*_PSX_LIST_H_*/
