/**
 * \file picasso_backport.h
 * \author Zhang Ji Peng <onecoolx@gmail.com>
 * \date 2026/1/17
 *
 * This file includes all interfaces of Picasso
 *
 \verbatim

    Copyright (C) 2008 ~ 2026  Zhang Ji Peng

    All rights reserved.

    Picasso is a vector graphic library.

 \endverbatim
 */

#ifndef _PICASSO_BACKPORT_H_
#define _PICASSO_BACKPORT_H_

#include "picasso.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup backport Backport interface
 * @{
 */

/**
 * \defgroup memory Memory Management
 * @{
 */

/**
 * \brief Memory allocation function pointer type
 * \param size Size in bytes to allocate
 * \return Pointer to allocated memory or NULL on failure
 */
typedef void* (*ps_malloc_func)(size_t size);

/**
 * \brief Memory free function pointer type
 * \param ptr Pointer to memory to free
 */
typedef void (*ps_free_func)(void* ptr);

/**
 * \brief Memory calloc function pointer type
 * \param num Number of elements
 * \param size Size of each element in bytes
 * \return Pointer to allocated memory or NULL on failure
 */
typedef void* (*ps_calloc_func)(size_t num, size_t size);

/**
 * \brief Memory allocator functions structure
 */
typedef struct _ps_memory_funcs {
    ps_malloc_func mem_malloc; /** Memory allocation function */
    ps_free_func mem_free; /** Memory free function */
    ps_calloc_func mem_calloc; /** Memory calloc function */
} ps_memory_funcs;

/** @} end of memory */

/** @} end of backport */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*_PICASSO_BACKPORT_H_*/
