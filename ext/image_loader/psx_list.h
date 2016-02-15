/* Picasso - a vector graphics library
 *
 * Copyright (C) 2016 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PSX_LIST_H_
#define _PSX_LIST_H_

#if defined(__GNUC__)
#define INLINE inline
#elif defined(_MSC_VER)
#define INLINE __inline
#else
#define INLINE
#endif

struct list_hdr {
    struct list_hdr* next;
    struct list_hdr* prev;
};

/* all function return 0 is success or true, -1 is fail or false. */

static INLINE int list_init(struct list_hdr* head)
{
    head->next = head;
    head->prev = head;
    return 0;
}

static INLINE int list_empty(const struct list_hdr* head)
{
    return head->next == head ? 0 : -1;
}

#define list_add(head, value) \
            list_add_entry((struct list_hdr*)(head), (struct list_hdr*)(value))

static INLINE int list_add_entry(struct list_hdr* head, struct list_hdr* value)
{
    register struct list_hdr* next = head->next;

    next->prev = value;
    value->next = next;
    value->prev = head;
    head->next = value;
    return 0;
}

#define list_add_tail(head, value) \
            list_add_tail_entry((struct list_hdr*)(head), (struct list_hdr*)(value))

static INLINE int list_add_tail_entry(struct list_hdr* head, struct list_hdr* value)
{
    register struct list_hdr* prev = head->prev;

    head->prev = value;
    value->next = head;
    value->prev = prev;
    prev->next = value;
    return 0;
}

#define list_remove(entry) \
            list_remove_entry((struct list_hdr*)(entry))

static INLINE int list_remove_entry(struct list_hdr* entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
    entry->next = entry->prev = NULL;
    return 0;
}

#define list_for_each(head, iterator) \
        for ((iterator) = (head)->next; (iterator) != (head); (iterator) = (iterator)->next)

#define list_for_each_start_with(head, start, iterator) \
        for ((iterator) = ((struct list_hdr*)(start))->next; (iterator) != (head); (iterator) = (iterator)->next)

#endif /*_PSX_LIST_H_*/

