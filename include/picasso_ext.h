/**
 * \file picasso_ext.h
 * \author Zhang Ji Peng <onecoolx@gmail.com>
 * \date 2025/5/12
 *
 * This file includes all common definitions for the picasso extension.
 *
 \verbatim

    Copyright (C) 2008 ~ 2025  Zhang Ji Peng

    All rights reserved.

    Picasso is a vector graphic library.

 \endverbatim
 */

#ifndef _PICASSO_EXT_H_
#define _PICASSO_EXT_H_

#include "picasso.h"

#include <stddef.h>
#include <stdint.h>

/**
 * \defgroup extension Extensions
 * @{
 */

/**
 * \brief Status code return by call APIs.
 */
typedef enum {
    /** Successful no errors. */
    S_OK = 0,
    /** Invalid params input. */
    S_BAD_PARAMS = 1,
    /** Not support. */
    S_NOT_SUPPORT = 2,
    /** Not enough memory. */
    S_OUT_OF_MEMORY = 3,
    /** Not initialize system. */
    S_INIT_FAILURE = 4,
    /** Operation failed by internal errors. */
    S_FAILURE = 5,
} psx_result;

/** @} end of extensions */

#endif /*_PICASSO_EXT_H_*/
