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

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * \fn const char* psx_result_get_string(psx_result result)
 * \brief Return the string of result code.
 *
 * \param result The status code of result.
 * \return The string of result code.
 */
const char* psx_result_get_string(psx_result result);

/** @} end of extensions */

#ifdef __cplusplus
}
#endif

#endif /*_PICASSO_EXT_H_*/
