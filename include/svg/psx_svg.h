/**
 * \file psx_svg.h
 * \author Zhang Ji Peng <onecoolx@gmail.com>
 * \date 2025/5/12
 *
 * This file includes all interfaces of picasso's SVG extension function.
 \verbatim

    Copyright (C) 2008 ~ 2025  Zhang Ji Peng

    All rights reserved.

    Picasso is a vector graphic library.

 \endverbatim
 */

#ifndef _PSX_SVG_H_
#define _PSX_SVG_H_

#include "picasso.h"
#include "picasso_ext.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup extension Extensions
 * @{
 */
/**
 * \defgroup extsvg XSvg Extension
 * @{
 */
/**
 * \defgroup svgtypes XSvg Object Types
 * @{
 */

/**
 * \typedef psx_svg
 * \brief An opaque type represents an svg document.
 * \sa psx_svg_render
 */
typedef struct _psx_svg psx_svg;

/**
 * \typedef psx_svg_render
 * \brief An opaque type represents an svg render object.
 * \sa psx_svg
 */
typedef struct _psx_svg_render psx_svg_render;

/** @} end of extsvg svgtypes */

/**
 * \defgroup svgfunctions XSvg Functions
 * @{
 */

/**
 * \fn psx_result psx_svg_init(void)
 * \brief Initialze the library and load resources.
 *
 * \return Result code returned.
 *
 * \sa psx_svg_shutdown
 */
PEXPORT psx_result PICAPI psx_svg_init(void);

/**
 * \fn void psx_svg_shutdown(void)
 * \brief Release resources and shutdoen.
 *
 * \sa psx_svg_init
 */
PEXPORT void PICAPI psx_svg_shutdown(void);

/**
 * \fn psx_svg* psx_svg_load(const char* file_name, int* err_code)
 * \brief Create a new psx_svg object and load from file.
 *
 * \param file_name  The svg file path which will be loaded, which is encoded by utf8.
 * \param err_code   Pointer to a value to receiving the result code. can be NULL.
 *
 * \return If successs, the return value is the pointer to new psx_svg object.
 *         If fails, the return value is NULL, and result will be return by \a err_code.
 *
 * \sa psx_svg_destroy psx_svg_load_from_memory
 */
PEXPORT psx_svg* PICAPI psx_svg_load(const char* file_name, psx_result* err_code);

/**
 * \fn psx_svg* psx_svg_load_from_memory(const ps_byte* data, size_t length, int* err_code)
 * \brief Create a new psx_svg object and load data from memory.
 *
 * \param data       Pointer to data buffer in memeory.
 * \param length     Data length bytes.
 * \param err_code   Pointer to a value to receiving the result code. can be NULL.
 *
 * \return If successs, the return value is the pointer to new psx_svg object.
 *         If fails, the return value is NULL, and result will be return by \a err_code.
 *
 * \sa psx_svg_destroy psx_svg_load
 */
PEXPORT psx_svg* PICAPI psx_svg_load_from_memory(const ps_byte* data, size_t length, psx_result* err_code);

/**
 * \fn void psx_svg_destroy(psx_svg* doc)
 * \brief Destroy the psx_svg object and release resources.
 *
 * \param doc Pointer to an existing psx_svg object.
 *
 * \sa psx_svg_load psx_svg_load_from_memory
 */
PEXPORT void PICAPI psx_svg_destroy(psx_svg* doc);

/** @} end of extsvg svgfunctions */
/** @} end of extsvg */
/** @} end of extensions */

#ifdef __cplusplus
}
#endif

#endif /*_PSX_SVG_H_*/
