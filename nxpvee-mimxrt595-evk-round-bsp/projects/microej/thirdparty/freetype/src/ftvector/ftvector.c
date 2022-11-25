/*
 * C
 *
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
* @file
* @brief Freetype vglite renderer
* @author MicroEJ Developer Team
* @version 3.0.0
*/

#define FT_MAKE_OPTION_SINGLE_OBJECT

#include "ft2build.h"

#include FT_FREETYPE_H
#include <freetype/internal/ftobjs.h>

#include "ftsmerrs.h"

#include "ft2build.h"
#include <freetype/internal/ftdebug.h>
#include FT_OUTLINE_H
//#include "ftspic.h"

#include "microvg_configuration.h"
#include "ftvector/ftvector.h"

#if defined VG_FEATURE_FONT && (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_VECTOR)

// cppcheck-suppress [misra-c2012-20.5] freetype behavior
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_smooth

#ifndef FT_MEM_SET
#define FT_MEM_SET( d, s, c )  ft_memset( d, s, c )
#endif

#ifndef FT_MEM_ZERO
#define FT_MEM_ZERO( dest, count )  FT_MEM_SET( dest, 0, count )
#endif

#include "trace_vglite.h"
#include "display_vglite.h"
#include "vglite_path.h"
#include "microvg_helper.h"

/*
 * @brief Logs an init gpu start event
 *
 * @param[in] operation: The VGLite operation sent to the GPU
 */
// cppcheck-suppress [misra-c2012-20.10]
#define FT_VGLITE_TRACE_INIT_GPU_START(operation) \
		TRACE_PLATFORM_START_U32( \
				VGLITE, \
				VGLITE_TRACE_INIT_GPU, \
				VGLITE_TRACE_OP_ ## operation \
		);

/*
 * @brief Logs an init gpu end event
 */
#define FT_VGLITE_TRACE_INIT_GPU_END() \
		TRACE_PLATFORM_END_VOID( \
				VGLITE, \
				VGLITE_TRACE_INIT_GPU \
		);

/*
 * @brief INIT_GPU event identifier
 */
#define VGLITE_TRACE_INIT_GPU 1

/*
 * @brief VGLite operation DRAW (draw path) identifier
 * This event is expected to be used to trace a call to vg_lite_draw
 */
#define VGLITE_TRACE_OP_DRAW 2

/*
 * @brief VGLite operation DRAW_GRAD (draw path with gradient) identifier
 * This event is expected to be used to trace a call to vg_lite_draw_grad
 */
#define VGLITE_TRACE_OP_DRAW_GRAD 3


typedef struct  vglite_TWorker_ {
	FT_Outline  outline;

	void* vglite_dest;
	vg_lite_matrix_t* vglite_matrix;
	vg_lite_path_t vglite_path;
	uint32_t vglite_path_pool_size;
	vg_lite_blend_t vg_lite_blend;
	vg_lite_color_t vg_lite_color;
	vg_lite_linear_gradient_t* vg_lite_gradient;

	FT_Glyph_Metrics    *metrics;
} vglite_TWorker, *vglite_PWorker;

static vglite_TWorker  _worker;

typedef struct vglite_TRaster_ {
	void* memory;
} vglite_TRaster, *vglite_PRaster;

static vglite_TRaster _raster;

static FT_Error vglite_extend_path(int extra_length){
	FT_Error error = FT_ERR( Ok );
	// cppcheck-suppress [unreadVariable] "memory" is used by "FT_REALLOC"
	FT_Memory memory = _raster.memory;
	uint32_t new_path_length;

	new_path_length = _worker.vglite_path.path_length + extra_length;

	while ( new_path_length > _worker.vglite_path_pool_size ) {
		if ( FT_ERR( Ok )  != FT_REALLOC( _worker.vglite_path.path, _worker.vglite_path_pool_size, _worker.vglite_path_pool_size + (uint32_t)FT_VGLITE_PATH_POOL_CHUNK ) ) {
			error = FT_ERR( Out_Of_Memory );
			break;
		}

		_worker.vglite_path_pool_size += (uint32_t)FT_VGLITE_PATH_POOL_CHUNK;
	};

	if ( FT_ERR( Ok ) == error ) {
		_worker.vglite_path.path_length = new_path_length;
	}

	return error;
}

static FT_Error vglite_get_path_slot(void **slot, int extra_length) {
	FT_Error    error;
	int32_t     _prev_path_length;

	*slot = NULL;

	_prev_path_length = _worker.vglite_path.path_length;

	error = vglite_extend_path(extra_length);
	if ( FT_ERR( Ok ) == error ) {
		*slot = &(((uint8_t *) _worker.vglite_path.path) [_prev_path_length]);;
	}

	return error;
}

// cppcheck-suppress [unusedFunction] function used through a list
static int vglite_move_to( const FT_Vector*    to,
		vglite_PWorker      worker )
{
	(void)worker;
	FT_Error            error;
	path_move_to_16_t   *path_move_to;
	void                *path_slot;

	FT_TRACE7(("%s: \n", __func__));

#ifdef FT_VGLITE_LOG_OUTLINES
	float to_x = (((float)to->x)*_worker.vglite_matrix->m[0][0]) + (((float)to->y)*_worker.vglite_matrix->m[0][1]) + _worker.vglite_matrix->m[0][2];
	float to_y = (((float)to->x)*_worker.vglite_matrix->m[1][0]) + (((float)to->y)*_worker.vglite_matrix->m[1][1]) + _worker.vglite_matrix->m[1][2];

	MEJ_LOG_PRINTF("M %f %f ", to_x, to_y);
#endif
	error = vglite_get_path_slot(&path_slot, sizeof(*path_move_to));
	if ( FT_ERR( Ok ) == error ) {
		FT_TRACE7(("move to %d, %d\n", to->x, to->y));
		// cppcheck-suppress [misra-c2012-11.5]
		path_move_to = path_slot;
		path_move_to->cmd = VGLITE_CMD_MOVE_TO;
		path_move_to->x = to->x;
		path_move_to->y = -to->y;
	}

	return error;
}

// cppcheck-suppress [unusedFunction] function used through a list
static int vglite_line_to( const FT_Vector*    to,
		vglite_PWorker      worker )
{
	(void)worker;
	FT_Error            error;
	path_line_to_16_t   *path_line_to;
	void                *path_slot;

	FT_TRACE7(("%s: \n", __func__));

#ifdef FT_VGLITE_LOG_OUTLINES
	float to_x = (((float)to->x)*_worker.vglite_matrix->m[0][0]) + (((float)to->y)*_worker.vglite_matrix->m[0][1]) + _worker.vglite_matrix->m[0][2];
	float to_y = (((float)to->x)*_worker.vglite_matrix->m[1][0]) + (((float)to->y)*_worker.vglite_matrix->m[1][1]) + _worker.vglite_matrix->m[1][2];

	MEJ_LOG_PRINTF("L %f %f ", to_x, to_y);
#endif

	error = vglite_get_path_slot(&path_slot, sizeof(*path_line_to));
	if ( FT_ERR( Ok ) == error ) {
		FT_TRACE7(("line to %d, %d\n", to->x, to->y));
		// cppcheck-suppress [misra-c2012-11.5]
		path_line_to = path_slot;
		path_line_to->cmd = VGLITE_CMD_LINE_TO;
		path_line_to->x = to->x;
		path_line_to->y = -to->y;
	}

	return error;
}

// cppcheck-suppress [unusedFunction] function used through a list
static int vglite_conic_to(    const FT_Vector*    control,
		const FT_Vector*    to,
		vglite_PWorker      worker )
{
	(void)worker;
	FT_Error            error;
	path_quad_to_16_t   *path_quad_to;
	void                *path_slot;

	FT_TRACE7(("%s: \n", __func__));

#ifdef FT_VGLITE_LOG_OUTLINES
	float to_x = (((float)to->x)*_worker.vglite_matrix->m[0][0]) + (((float)to->y)*_worker.vglite_matrix->m[0][1]) + _worker.vglite_matrix->m[0][2];
	float to_y = (((float)to->x)*_worker.vglite_matrix->m[1][0]) + (((float)to->y)*_worker.vglite_matrix->m[1][1]) + _worker.vglite_matrix->m[1][2];
	float control_x = (((float)control->x)*_worker.vglite_matrix->m[0][0]) + (((float)control->y)*_worker.vglite_matrix->m[0][1]) + _worker.vglite_matrix->m[0][2];
	float control_y = (((float)control->x)*_worker.vglite_matrix->m[1][0]) + (((float)control->y)*_worker.vglite_matrix->m[1][1]) + _worker.vglite_matrix->m[1][2];

	MEJ_LOG_PRINTF("Q %f %f %f %f ", control_x, control_y, to_x, to_y);
#endif

	error = vglite_get_path_slot(&path_slot, sizeof(*path_quad_to));
	if ( FT_ERR( Ok ) == error ) {
		FT_TRACE7(("quad to %d, %d, %d, %d\n", control->x, control->y, to->x, to->y));
		// cppcheck-suppress [misra-c2012-11.5]
		path_quad_to = path_slot;
		path_quad_to->cmd = VGLITE_CMD_QUAD_TO;
		path_quad_to->cx = control->x;
		path_quad_to->cy = -control->y;
		path_quad_to->x = to->x;
		path_quad_to->y = -to->y;
	}

	return error;
}

// cppcheck-suppress [unusedFunction] function used through a list
static int vglite_cubic_to(    const FT_Vector*    control1,
		const FT_Vector*    control2,
		const FT_Vector*    to,
		vglite_PWorker      worker )
{
	(void)worker;
	FT_Error            error;
	path_cubic_to_16_t  *path_cubic_to;
	void                *path_slot;

	FT_TRACE7(("%s: \n", __func__));

#ifdef FT_VGLITE_LOG_OUTLINES
	float to_x = (((float)to->x)*_worker.vglite_matrix->m[0][0]) + (((float)to->y)*_worker.vglite_matrix->m[0][1]) + _worker.vglite_matrix->m[0][2];
	float to_y = (((float)to->x)*_worker.vglite_matrix->m[1][0]) + (((float)to->y)*_worker.vglite_matrix->m[1][1]) + _worker.vglite_matrix->m[1][2];
	float control1_x = (((float)control1->x)*_worker.vglite_matrix->m[0][0]) + (((float)control1->y)*_worker.vglite_matrix->m[0][1]) + _worker.vglite_matrix->m[0][2];
	float control1_y = (((float)control1->x)*_worker.vglite_matrix->m[1][0]) + (((float)control1->y)*_worker.vglite_matrix->m[1][1]) + _worker.vglite_matrix->m[1][2];
	float control2_x = (((float)control2->x)*_worker.vglite_matrix->m[0][0]) + (((float)control2->y)*_worker.vglite_matrix->m[0][1]) + _worker.vglite_matrix->m[0][2];
	float control2_y = (((float)control2->x)*_worker.vglite_matrix->m[1][0]) + (((float)control2->y)*_worker.vglite_matrix->m[1][1]) + _worker.vglite_matrix->m[1][2];

	MEJ_LOG_PRINTF("C %f %f %f %f %f %f ", control1_x, control1_y, control2_x, control2_y, to_x, to_y);
#endif

	error = vglite_get_path_slot(&path_slot, sizeof(*path_cubic_to));
	if ( FT_ERR( Ok ) == error ) {
		FT_TRACE7(("cubic to %d, %d, %d, %d, %d, %d\n", control1->x, control1->y, control2->x, control2->y, to->x, to->y));
		// cppcheck-suppress [misra-c2012-11.5]
		path_cubic_to = path_slot;
		path_cubic_to->cmd = VGLITE_CMD_CUBIC_TO;
		path_cubic_to->cx1 = control1->x;
		path_cubic_to->cy1 = -control1->y;
		path_cubic_to->cx2 = control2->x;
		path_cubic_to->cy2 = -control2->y;
		path_cubic_to->x = to->x;
		path_cubic_to->y = -to->y;
	}

	return error;
}

static int vglite_end( vglite_PWorker  worker )
{
	(void)worker;
	FT_Error            error;
	path_end_16_t       *path_end;
	void                *path_slot;

	FT_TRACE7(("%s: \n", __func__));

#ifdef FT_VGLITE_LOG_OUTLINES
	MEJ_LOG_PRINTF("Z\n");
#endif
	error = vglite_get_path_slot(&path_slot, sizeof(*path_end));
	if ( FT_ERR( Ok ) == error ) {
		FT_TRACE7(("end\n"));
		// cppcheck-suppress [misra-c2012-11.5]
		path_end = path_slot;
		path_end->cmd = VGLITE_CMD_END;
	}

	return error;
}

FT_DEFINE_OUTLINE_FUNCS(func_interface,
		(FT_Outline_MoveTo_Func) vglite_move_to,
		(FT_Outline_LineTo_Func) vglite_line_to,
		(FT_Outline_ConicTo_Func)vglite_conic_to,
		(FT_Outline_CubicTo_Func)vglite_cubic_to,
		0,
		0
)

static int
vglite_convert_glyph( void )
{
	volatile int  error = 0;

	FT_TRACE7(("%s: \n", __func__));

	// Reinit path.
	_worker.vglite_path.path_length = 0;

	error = FT_Outline_Decompose( &_worker.outline, &func_interface, &_worker );

	if ( FT_ERR( Ok ) == error ) {

		FT_TRACE7(( "Send path to gpu\n" ));
		error = vglite_end( &_worker );

		if ( FT_ERR( Ok ) == error ) {

			_worker.vglite_path.path_changed = 1;

			FT_TRACE7(("width: %d\n", _worker.metrics->width));
			FT_TRACE7(("height: %d\n", _worker.metrics->height));
			FT_TRACE7(("horiBearingX: %d\n", _worker.metrics->horiBearingX));
			FT_TRACE7(("horiBearingY: %d\n", _worker.metrics->horiBearingY));

			// FIXME the commented code break characters alignment on the baseline.
			// I guess they were added to fix wrong positions.
			//    int offX = _worker.metrics->horiBearingX;
			//    int offY = _worker.metrics->horiBearingY - _worker.metrics->height;

			// Get fill rule from outline
			vg_lite_fill_t vg_lite_fill_rule = (_worker.outline.flags & FT_OUTLINE_EVEN_ODD_FILL) ? VG_LITE_FILL_EVEN_ODD : VG_LITE_FILL_NON_ZERO;

			//    vg_lite_translate(offX, offY, _worker.vglite_matrix);
			if(_worker.vg_lite_gradient == MICROVG_HELPER_NULL_GRADIENT){
				FT_VGLITE_TRACE_INIT_GPU_START(DRAW);
				void* target = _worker.vglite_dest;
				VG_DRAWER_draw_path(target, &_worker.vglite_path, vg_lite_fill_rule, _worker.vglite_matrix, _worker.vg_lite_blend, _worker.vg_lite_color);
				FT_VGLITE_TRACE_INIT_GPU_END();
			}
			else {
				FT_VGLITE_TRACE_INIT_GPU_START(DRAW_GRAD);
				void* target = _worker.vglite_dest;
				VG_DRAWER_draw_gradient(target, &_worker.vglite_path, vg_lite_fill_rule, _worker.vglite_matrix, _worker.vg_lite_gradient, _worker.vg_lite_blend);
				FT_VGLITE_TRACE_INIT_GPU_END();
			}
			//    vg_lite_translate(0, offY, _worker.vglite_matrix);
		}
	}

	return error;
}

// cppcheck-suppress [unusedFunction] function used through a list
static int vglite_render(  const vglite_PRaster          raster,
		const FT_Raster_Params* params )
{
	const FT_Outline*  outline    = (const FT_Outline*)params->source;

	FT_TRACE7(("%s: \n", __func__));

	int ret;
	if ( &_raster == raster ) {
		_worker.outline        = *outline;
		ret = vglite_convert_glyph(  );
	}
	else {
		FT_TRACE7(("%s: internal error\n", __func__));
		ret = FT_THROW( Invalid_Argument );
	}

	return ret;
}


/**** RASTER OBJECT CREATION: In stand-alone mode, we simply use *****/
/****                         a static object.                   *****/

static void
vglite_reset(   FT_Raster  raster,
		char*      pool_base,
		long       pool_size )
{
	(void)pool_base;
	(void)pool_size;
	FT_TRACE7(("%s: \n", __func__));

	if ( ((FT_Raster) &_raster) != raster ) {
		FT_TRACE7(("%s: internal error\n", __func__));
	}

	_worker.vglite_dest = NULL;
	_worker.vglite_matrix = NULL;
	_worker.vglite_path_pool_size = 0;

	(void)memset(&_worker.vglite_path, 0, sizeof(_worker.vglite_path));

	_worker.vglite_path.bounding_box[0] = -4000; // Left
	_worker.vglite_path.bounding_box[1] = -4000; // Top
	_worker.vglite_path.bounding_box[2] = 4000; // Right
	_worker.vglite_path.bounding_box[3] = 4000; // Bottom
}


// cppcheck-suppress [unusedFunction] function used through a list
static int vglite_new( FT_Memory   memory,
		FT_Raster*  araster )
{
	FT_TRACE7(("%s: \n", __func__));

	*araster = (FT_Raster) &_raster;
	_raster.memory = memory;

	vglite_reset( *araster, NULL, 0 );

	return FT_ERR( Ok );
}

// cppcheck-suppress [unusedFunction, misra-c2012-2.7] function used through a list and raster used to retrieve "memory"
static void vglite_done( FT_Raster  raster )
{
	FT_TRACE7(("%s: \n", __func__));

	// cppcheck-suppress [unreadVariable] "memory" is used by "FT_FREE"
	FT_Memory  memory = _raster.memory;

	if ( NULL != _worker.vglite_path.path ) {
		FT_FREE( _worker.vglite_path.path );
		_worker.vglite_path_pool_size = 0;
	}
}

FT_DEFINE_RASTER_FUNCS(ft_vglite_raster,
		FT_GLYPH_FORMAT_OUTLINE,

		(FT_Raster_New_Func)     vglite_new,
		(FT_Raster_Reset_Func)   vglite_reset,
		(FT_Raster_Set_Mode_Func)0,
		(FT_Raster_Render_Func)  vglite_render,
		(FT_Raster_Done_Func)    vglite_done
)

/* initialize renderer -- init its raster */
static FT_Error
ft_vglite_init( FT_Renderer  render )
{
	//FT_Library  library = FT_MODULE_LIBRARY( render );

	FT_TRACE7(("%s: \n", __func__));

	render->clazz->raster_class->raster_reset( render->raster, NULL, 0  );

	return 0;
}


/* sets render-specific mode */
static FT_Error ft_vglite_set_mode(FT_Renderer render, FT_ULong  mode_tag, FT_Pointer data ){
	FT_Error ret = FT_ERR(Invalid_Argument);

	FT_TRACE7(("%s: \n", __func__));

	switch (mode_tag){
	case FT_PARAM_TAG_VGLITE_DESTINATION:{
		_worker.vglite_dest = data;
		ret = FT_ERR(Ok);
		break;
	}
	case FT_PARAM_TAG_VGLITE_MATRIX:{
		_worker.vglite_matrix = data;
		ret = FT_ERR(Ok);
		break;
	}
	case FT_PARAM_TAG_VGLITE_QUALITY:{
		_worker.vglite_path.quality = (vg_lite_quality_t) (uint32_t) data;
		ret = FT_ERR(Ok);
		break;
	}
	case FT_PARAM_TAG_VGLITE_FORMAT:{
		_worker.vglite_path.format = (vg_lite_format_t) (uint32_t) data;
		ret = FT_ERR(Ok);
		break;
	}
	case FT_PARAM_TAG_VGLITE_BLEND:{
		_worker.vg_lite_blend = (vg_lite_blend_t) (uint32_t) data;
		ret = FT_ERR(Ok);
		break;
	}
	case FT_PARAM_TAG_VGLITE_COLOR:{
		// convert the color only once, see FT_PARAM_TAG_VGLITE_DESTINATION
		_worker.vg_lite_color = (uint32_t) data;
		VG_DRAWER_update_color(_worker.vglite_dest, &_worker.vg_lite_color, _worker.vg_lite_blend);
		ret = FT_ERR(Ok);
		break;
	}
	case FT_PARAM_TAG_VGLITE_GRADIENT:{
		_worker.vg_lite_gradient = data;
		if (MICROVG_HELPER_NULL_GRADIENT != data) {
			VG_DRAWER_update_gradient(_worker.vglite_dest, _worker.vg_lite_gradient, _worker.vg_lite_blend);
		}
		ret = FT_ERR(Ok);
		break;
	}
	default:{
		/* we simply pass it to the raster */
		ret = render->clazz->raster_class->raster_set_mode(render->raster, mode_tag, data);
		break;
	}
	}

	return ret;
}

/* transform a given glyph image */
static FT_Error
ft_vglite_transform(    FT_Renderer       render,
		FT_GlyphSlot      slot,
		const FT_Matrix*  matrix,
		const FT_Vector*  delta )
{
	FT_Error error;

	FT_TRACE7(("%s: \n", __func__));

	if ( slot->format == render->glyph_format ) {
		if (NULL != matrix ){
			FT_Outline_Transform( &slot->outline, matrix );
		}
		if (NULL != delta ){
			FT_Outline_Translate( &slot->outline, delta->x, delta->y );
		}
		error = FT_Err_Ok;
	}
	else {
		error = FT_THROW( Invalid_Argument );
	}
	return error;
}


/* return the glyph's control box */
static void
ft_vglite_get_cbox( FT_Renderer   render,
		FT_GlyphSlot  slot,
		FT_BBox*      cbox )
{
	FT_MEM_ZERO( cbox, sizeof ( *cbox ) );

	if ( slot->format == render->glyph_format ){
		FT_Outline_Get_CBox( &slot->outline, cbox );
	}
}

/* render a slot's glyph with GPU */
static FT_Error
ft_vglite_render(   FT_Renderer       render,
		FT_GlyphSlot      slot,
		FT_Render_Mode    mode,
		const FT_Vector*  origin )
{
	(void)mode;
	(void)origin;
	FT_Error     error;
	FT_Outline*  outline = NULL;
	FT_BBox      cbox;
	FT_Pos       width;
	FT_Pos       height;

	FT_Raster_Params  params;

	FT_TRACE7(("%s: \n", __func__));

	_worker.metrics = &slot->metrics;

	/* check glyph image format */
	if ( slot->format != render->glyph_format ) {
		error = FT_THROW( Invalid_Argument );
	}
	else{
		outline = &slot->outline;

		/* compute the control box, and grid fit it */
		FT_Outline_Get_CBox( outline, &cbox );

		// Update path bounding_box from cbox before floor and ceil operations
		_worker.vglite_path.bounding_box[0] = (float) cbox.xMin;
		_worker.vglite_path.bounding_box[1] = - (float) cbox.yMax;
		_worker.vglite_path.bounding_box[2] = (float) cbox.xMax;
		_worker.vglite_path.bounding_box[3] = - (float) cbox.yMin;

		cbox.xMin = FT_PIX_FLOOR( cbox.xMin );
		cbox.yMin = FT_PIX_FLOOR( cbox.yMin );
		cbox.xMax = FT_PIX_CEIL( cbox.xMax );
		cbox.yMax = FT_PIX_CEIL( cbox.yMax );

		if ( (cbox.xMin < 0) && (cbox.xMax > (FT_INT_MAX + cbox.xMin)) ) {
			FT_ERROR((  "ft_vglite_render: glyph too large:"
					" xMin = %d, xMax = %d\n",
					cbox.xMin >> 6, cbox.xMax >> 6 ));
			error = FT_THROW( Raster_Overflow );
		}
		else{
			width = ( cbox.xMax - cbox.xMin ) >> 6;

			if ( (cbox.yMin < 0) && (cbox.yMax > (FT_INT_MAX + cbox.yMin)) ) {
				FT_ERROR((  "ft_vglite_render: glyph too large:"
						" yMin = %d, yMax = %d\n",
						cbox.yMin >> 6, cbox.yMax >> 6 ));
				error = FT_THROW( Raster_Overflow );
			}
			else{
				height = ( cbox.yMax - cbox.yMin ) >> 6;

				/* Required check is (pitch * height < FT_ULONG_MAX),        */
				/* but we care realistic cases only.  Always pitch <= width. */
				if ( (width > 0x7FFF) || (height > 0x7FFF) )  {
					FT_ERROR(( "ft_vglite_render: glyph too large: %u x %u\n",
							width, height ));
					error = FT_THROW( Raster_Overflow );
				}
				else {

					/* set up parameters */
					params.source = outline;
					params.flags  = FT_RASTER_FLAG_AA;

					/* render outline into bitmap */
					error = render->raster_render( render->raster, &params );
				}
			}
		}
	}

	return error;
}

FT_DEFINE_RENDERER( ft_vglite_renderer_class,

		FT_MODULE_RENDERER,
		sizeof ( FT_RendererRec ),

		"vglite_renderer",
		0x10000L,   // Version
		0x20000L,   // Requires

		0,    /* module specific interface */

		(FT_Module_Constructor)     ft_vglite_init,
		(FT_Module_Destructor)      0,
		(FT_Module_Requester)       0,

		FT_GLYPH_FORMAT_OUTLINE,

		(FT_Renderer_RenderFunc)    ft_vglite_render,
		(FT_Renderer_TransformFunc) ft_vglite_transform,
		(FT_Renderer_GetCBoxFunc)   ft_vglite_get_cbox,
		(FT_Renderer_SetModeFunc)   ft_vglite_set_mode,

		(FT_Raster_Funcs*)          &ft_vglite_raster
)

#endif // defined VG_FEATURE_FONT && (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_VECTOR)
/* END */
