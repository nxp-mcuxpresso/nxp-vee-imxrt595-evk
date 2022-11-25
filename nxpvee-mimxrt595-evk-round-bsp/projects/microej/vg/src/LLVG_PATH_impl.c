/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief MicroVG library low level API: implementation of path.
 *
 * This implementation uses a 32-bit "integer" value to store a path command and
 * a 32-bit "float" value to store each command parameter.
 *
 * The encoding can be overrided, see "[optional]: weak functions" in "microvg_path.h"
 *
 * @author MicroEJ Developer Team
 * @version 2.0.0
 */

#include "microvg_configuration.h"

#ifdef VG_FEATURE_PATH

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <math.h>
#include <string.h>

#include <LLVG_PATH_impl.h>

#include "microvg_path.h"
#include "microvg_helper.h"
#include "bsp_util.h"

#include "mej_math.h"

// -----------------------------------------------------------------------------
// Private functions
// -----------------------------------------------------------------------------

/*
 * @brief Extends the path to be able to store the command and its parameters.
 *
 * @return the offset in path buffer where the command will be stored. If the path
 * buffer is not large enough to contain the requested command, returns a negative
 * number corresponding to size the buffer must be enlarged for this command.
 */
static int32_t _extend_path(MICROVG_PATH_HEADER_t* path, jint length, jint cmd, uint32_t nb_fields) {
	uint32_t index = path->data_offset + path->data_size;
	uint32_t extra_size = MICROVG_PATH_get_path_command_size(cmd, nb_fields);
	int32_t ret;

	if (length >= (index + extra_size)) {
		path->data_size += extra_size;

		// return next free space (return a positive value)
		ret = index;
	}
	else {
		// too small buffer, ret is the required extra size
		// (return a negative value)
		ret = -extra_size;
	}

	return ret;
}

static int32_t _close_path(MICROVG_PATH_HEADER_t* path, jint length, jfloat x1, jfloat y1, jfloat x2, jfloat y2) {
	int32_t index = _extend_path(path, length, LLVG_PATH_CMD_CLOSE, 0);
	int32_t ret = LLVG_SUCCESS;
	if (index > 0) {
		// finalizes the path by storing the path's bounds
		path->bounds_xmin = x1;
		path->bounds_xmax = x2;
		path->bounds_ymin = y1;
		path->bounds_ymax = y2;
		(void)MICROVG_PATH_append_path_command0((jbyte*)path, (uint32_t)index, LLVG_PATH_CMD_CLOSE);
	}
	else {
		// too small buffer, ret is the required extra size * -1
		ret = -index;
	}
	return ret;
}

// -----------------------------------------------------------------------------
// Specific path format functions [optional]: weak functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
BSP_DECLARE_WEAK_FCNT uint32_t MICROVG_PATH_get_path_header_size(void) {
	return sizeof(MICROVG_PATH_HEADER_t);
}

// See the header file for the function documentation
BSP_DECLARE_WEAK_FCNT uint32_t MICROVG_PATH_get_path_command_size(jint command, uint32_t nbParams) {
	(void)command;
	return (nbParams + (uint32_t)1 /* command */) * sizeof(uint32_t);
}

// See the header file for the function documentation
BSP_DECLARE_WEAK_FCNT uint32_t MICROVG_PATH_append_path_command0(jbyte* path, uint32_t offset, jint cmd) {
	uint32_t* data = (uint32_t*)(path + offset);
	*data = MICROVG_PATH_convert_path_command(cmd);
	return sizeof(uint32_t);
}

// See the header file for the function documentation
BSP_DECLARE_WEAK_FCNT uint32_t MICROVG_PATH_append_path_command1(jbyte* path, uint32_t offset, jint cmd, jfloat x, jfloat y) {
	uint32_t* data = (uint32_t*)(path + offset);
	*data = MICROVG_PATH_convert_path_command(cmd);
	++data;
	*data = JFLOAT_TO_UINT32_t(x);
	++data;
	*data = JFLOAT_TO_UINT32_t(y);
	return (uint32_t)3 * sizeof(uint32_t);
}

// See the header file for the function documentation
BSP_DECLARE_WEAK_FCNT uint32_t MICROVG_PATH_append_path_command2(jbyte* path, uint32_t offset, jint cmd, jfloat x1, jfloat y1, jfloat x2, jfloat y2) {
	uint32_t* data = (uint32_t*)(path + offset);
	*data = MICROVG_PATH_convert_path_command(cmd);
	++data;
	*data = JFLOAT_TO_UINT32_t(x1);
	++data;
	*data = JFLOAT_TO_UINT32_t(y1);
	++data;
	*data = JFLOAT_TO_UINT32_t(x2);
	++data;
	*data = JFLOAT_TO_UINT32_t(y2);
	return (uint32_t)5 * sizeof(uint32_t);
}

// See the header file for the function documentation
BSP_DECLARE_WEAK_FCNT uint32_t MICROVG_PATH_append_path_command3(jbyte* path, uint32_t offset, jint cmd, jfloat x1, jfloat y1, jfloat x2, jfloat y2,
		jfloat x3, jfloat y3) {
	uint32_t* data = (uint32_t*)(path + offset);
	*data = MICROVG_PATH_convert_path_command(cmd);
	++data;
	*data = JFLOAT_TO_UINT32_t(x1);
	++data;
	*data = JFLOAT_TO_UINT32_t(y1);
	++data;
	*data = JFLOAT_TO_UINT32_t(x2);
	++data;
	*data = JFLOAT_TO_UINT32_t(y2);
	++data;
	*data = JFLOAT_TO_UINT32_t(x3);
	++data;
	*data = JFLOAT_TO_UINT32_t(y3);
	return (uint32_t)7 * sizeof(uint32_t);
}

// -----------------------------------------------------------------------------
// LLVG_PATH_impl.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
jint LLVG_PATH_IMPL_initializePath(jbyte* jpath, jint length) {

	MICROVG_PATH_HEADER_t* path = (MICROVG_PATH_HEADER_t*)jpath;
	uint32_t header_size = MICROVG_PATH_get_path_header_size();
	jint ret = LLVG_SUCCESS;

	if (length >= header_size) {
		path->data_size = 0;
		path->data_offset = header_size;
		path->format = MICROVG_PATH_get_path_encoder_format();
	}
	else {
		// the given byte array is too small
		ret = header_size;
	}

	return ret;
}

// See the header file for the function documentation
jint LLVG_PATH_IMPL_appendPathCommand1(jbyte* jpath, jint length, jint cmd, jfloat x, jfloat y) {

	MICROVG_PATH_HEADER_t* path = (MICROVG_PATH_HEADER_t*)jpath;
	jint ret = LLVG_SUCCESS;

	int32_t index = _extend_path(path, length, cmd, 2);
	if (index > 0) {
		(void)MICROVG_PATH_append_path_command1((jbyte*)path, (uint32_t)index, cmd, x, y);
	}
	else {
		// too small buffer, ret is the required extra size * -1
		ret = -index;
	}

	return ret;
}

// See the header file for the function documentation
jint LLVG_PATH_IMPL_appendPathCommand2(jbyte* jpath, jint length, jint cmd, jfloat x1, jfloat y1, jfloat x2,
		jfloat y2) {

	MICROVG_PATH_HEADER_t* path = (MICROVG_PATH_HEADER_t*)jpath;
	jint ret = LLVG_SUCCESS;

	if (LLVG_PATH_CMD_CLOSE == cmd)	{
		// parameters are path's bounds
		ret = _close_path(path, length, x1, y1, x2, y2);
	}
	else {
		int32_t index = _extend_path(path, length, cmd, 4);
		if (index > 0) {
			(void)MICROVG_PATH_append_path_command2((jbyte*)path, (uint32_t)index, cmd, x1, y1, x2, y2);
		}
		else {
			// too small buffer, ret is the required extra size * -1
			ret = -index;
		}
	}

	return ret;
}

// See the header file for the function documentation
jint LLVG_PATH_IMPL_appendPathCommand3(jbyte* jpath, jint length, jint cmd, jfloat x1, jfloat y1, jfloat x2,
		jfloat y2, jfloat x3, jfloat y3) {

	MICROVG_PATH_HEADER_t* path = (MICROVG_PATH_HEADER_t*)jpath;
	jint ret = LLVG_SUCCESS;

	int32_t index = _extend_path(path, length, cmd, 6);
	if (index > 0) {
		(void)MICROVG_PATH_append_path_command3((jbyte*)path, (uint32_t)index, cmd, x1, y1, x2, y2, x3, y3);
	}
	else {
		// too small buffer, ret is the required extra size * -1
		ret = -index;
	}

	return ret;
}

// See the header file for the function documentation
void LLVG_PATH_IMPL_reopenPath(jbyte* jpath) {
	MICROVG_PATH_HEADER_t* path = (MICROVG_PATH_HEADER_t*)jpath;
	path->data_size -= MICROVG_PATH_get_path_command_size(LLVG_PATH_CMD_CLOSE, 0);
}


// See the header file for the function documentation
jint LLVG_PATH_IMPL_mergePaths(jbyte* jpathDest, jbyte* jpathSrc1, jbyte* jpathSrc2, jfloat ratio){

	jint ret = LLVG_SUCCESS;
	float remaining = (1-ratio);

	MICROVG_PATH_HEADER_t* pathDest = (MICROVG_PATH_HEADER_t*)jpathDest;
	MICROVG_PATH_HEADER_t* pathSrc1 = (MICROVG_PATH_HEADER_t*)jpathSrc1;
	MICROVG_PATH_HEADER_t* pathSrc2 = (MICROVG_PATH_HEADER_t*)jpathSrc2;

	// Copy header from pathSrc1
	pathDest->data_size = pathSrc1->data_size;
	pathDest->data_offset = pathSrc1->data_offset;
	pathDest->format = pathSrc1->format;

	// Compute bounds
	float fSrc1 = pathSrc1->bounds_xmin;
	float fSrc2 = pathSrc2->bounds_xmin;
	float fDest = MEJ_MIN(fSrc1, fSrc2);
	pathDest->bounds_xmin = fDest;

	fSrc1 = pathSrc1->bounds_ymin;
	fSrc2 = pathSrc2->bounds_ymin;
	fDest = MEJ_MIN(fSrc1, fSrc2);
	pathDest->bounds_ymin = fDest;

	fSrc1 = pathSrc1->bounds_xmax;
	fSrc2 = pathSrc2->bounds_xmax;
	fDest = MEJ_MAX(fSrc1, fSrc2);
	pathDest->bounds_xmax = fDest;

	fSrc1 = pathSrc1->bounds_ymax;
	fSrc2 = pathSrc2->bounds_ymax;
	fDest = MEJ_MAX(fSrc1, fSrc2);
	pathDest->bounds_ymax = fDest;

	// Compute commands
	uint32_t* dataDest = (uint32_t*)(jpathDest + pathDest->data_offset);
	uint32_t* dataSrc1 = (uint32_t*)(jpathSrc1 + pathSrc1->data_offset);
	uint32_t* dataSrc2 = (uint32_t*)(jpathSrc2 + pathSrc2->data_offset);

	for(uint16_t i=0; i < pathDest->data_size;){

		uint32_t cmdSrc1 = *dataSrc1;

		uint32_t nb_parameters = MICROVG_PATH_get_command_parameter_number(cmdSrc1);

		*dataDest = cmdSrc1;
		dataSrc1++;
		dataSrc2++;
		dataDest++;
		i += (uint16_t) 4;

		for(uint32_t j=0; j<nb_parameters;j++ ){

			// cppcheck-suppress [invalidPointerCast,redundantPointerOp,misra-c2012-11.3] accepted casting macro
			fSrc1 = UINT32_t_TO_JFLOAT(*dataSrc1);
			// cppcheck-suppress [invalidPointerCast,redundantPointerOp,misra-c2012-11.3] accepted casting macro
			fSrc2 = UINT32_t_TO_JFLOAT(*dataSrc2);
			fDest = (remaining * fSrc1) + (ratio * fSrc2);

			// cppcheck-suppress [invalidPointerCast,misra-c2012-11.3] accepted casting macro
			*dataDest = JFLOAT_TO_UINT32_t(fDest);

			dataSrc1++;
			dataSrc2++;
			dataDest++;
			i += (uint16_t)4;
		}
	}
    return ret;
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#endif // VG_FEATURE_PATH
