/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
* @file
* @brief MicroEJ MicroVG library low level API: implementation of Path.
* @author MicroEJ Developer Team
* @version 2.0.0
*/

#if !defined MICROVG_PATH_H
#define MICROVG_PATH_H

#if defined __cplusplus
extern "C" {
#endif

#include "microvg_configuration.h"

#ifdef VG_FEATURE_PATH

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <sni.h>

// -----------------------------------------------------------------------------
// Structs
// -----------------------------------------------------------------------------

/*
 * @brief Map a jbyte array that represents a path
 */
typedef struct MICROVG_PATH_HEADER
{
	uint16_t data_size;  /* data size (without header) */
	uint16_t data_offset;
	uint8_t format;
	uint8_t padding1;
	uint8_t padding2;
	uint8_t padding3;
	float bounds_xmin; /* left */
	float bounds_xmax; /* right */
	float bounds_ymin; /* top */
	float bounds_ymax; /* bottom */
} MICROVG_PATH_HEADER_t;

// -----------------------------------------------------------------------------
// Specific path formatting functions [mandatory]
// -----------------------------------------------------------------------------

/*
 * @brief Gets the path's array format used to encode the commands and parameters. This format is stored into the path's
 * header.
 *
 * @return the path's format
 */
uint8_t MICROVG_PATH_get_path_encoder_format(void);

/*
 * @brief Converts the command in destination format.
 *
 * @param[in] command: the command to convert
 *
 * @return the converted command
 */
uint32_t MICROVG_PATH_convert_path_command(jint command);

// -----------------------------------------------------------------------------
// Specific path formatting functions [optional]
// -----------------------------------------------------------------------------

/*
 * @brief Gets the path's array header size.
 *
 * The default implementation returns sizeof(MICROVG_PATH_HEADER_t).
 *
 * @return the size in bytes.
 */
uint32_t MICROVG_PATH_get_path_header_size(void);

/*
 * @brief Gets the size to add in the path array to encode the command and its parameters.
 *
 * The default implementation uses 32-bit fields for the command and for each data.
 *
 * @param[in] command: the command to add.
 * @param[in] nbParams: the available number of command's parameters
 *
 * @return the size to add in the path array.
 */
uint32_t MICROVG_PATH_get_path_command_size(jint command, uint32_t nbParams);

/*
 * @brief Appends the command with zero parameter in the path's array.
 *
 * The caller ensures the path's array is large enough to encode the command and its parameters.
 *
 * The default implementation uses 32-bit fields for the command and for each data.
 *
 * @param[in] path: the path's array
 * @param[in] offset: the offset where store the command
 * @param[in] cmd: the command to add
 *
 * @return the offset after the command and parameters
 *
 * @see #MICROVG_PATH_get_path_command_size(jint, uint32_t)
 */
uint32_t MICROVG_PATH_append_path_command0(jbyte* path, uint32_t offset, jint cmd);

/*
 * @brief Appends the command with 1 point parameter in the path's array.
 *
 * @param[in] path: the path's array
 * @param[in] offset: the offset where store the command
 * @param[in] cmd: the command to add
 * @param[in] x: the command data 1.
 * @param[in] y: the command data 2.
 *
 * @return the offset after the command and parameters
 *
 * @see #MICROVG_PATH_append_path_command0(jbyte*, uint32_t, jint)
 */
uint32_t MICROVG_PATH_append_path_command1(jbyte* path, uint32_t offset, jint cmd, jfloat x, jfloat y);

/*
 * @brief Appends the command with 2 points parameter in the path's array.
 *
 * @param[in] path: the path's array
 * @param[in] offset: the offset where store the command
 * @param[in] cmd: the command to add
 * @param[in] x1: the command data 1.
 * @param[in] y1: the command data 2.
 * @param[in] x2: the command data 3.
 * @param[in] y2: the command data 4.
 *
 * @return the offset after the command and parameters
 *
 * @see #MICROVG_PATH_append_path_command0(jbyte*, uint32_t, jint)
 */
uint32_t MICROVG_PATH_append_path_command2(jbyte* path, uint32_t offset, jint cmd, jfloat x1, jfloat y1, jfloat x2, jfloat y2);

/*
 * @brief Appends the command with 3 points parameter in the path's array.
 *
 * @param[in] path: the path's array
 * @param[in] offset: the offset where store the command
 * @param[in] cmd: the command to add
 * @param[in] x1: the command data 1.
 * @param[in] y1: the command data 2.
 * @param[in] x2: the command data 3.
 * @param[in] y2: the command data 4.
 * @param[in] x3: the command data 5.
 * @param[in] y3: the command data 6.
 *
 * @return the offset after the command and parameters
 *
 * @see #MICROVG_PATH_append_path_command0(jbyte*, uint32_t, jint)
 */
uint32_t MICROVG_PATH_append_path_command3(jbyte* path, uint32_t offset, jint cmd, jfloat x1, jfloat y1, jfloat x2, jfloat y2,
		jfloat x3, jfloat y3);

/*
 * @brief Gets the number of parameters for a specific command.
 *
 * @param[in] command: the command.
 *
 * @return the number of parameters for the given command.
 */
uint32_t MICROVG_PATH_get_command_parameter_number(jint command);

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#endif // VG_FEATURE_PATH

#ifdef __cplusplus
}
#endif

#endif // !defined MICROVG_PATH_H
