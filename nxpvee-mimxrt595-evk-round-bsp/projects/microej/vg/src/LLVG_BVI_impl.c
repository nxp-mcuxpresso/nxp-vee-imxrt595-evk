/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief MicroEJ MicroVG library low level API: implementation of buffered
 * vector images (BVI).
 * @author MicroEJ Developer Team
 * @version 3.0.0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "display_configuration.h"

/**
 * The buffered vector image feature is only available when the VGLITE multiple drawer option
 * is enabled.
 */
#if defined VGLITE_USE_MULTIPLE_DRAWERS

#include <string.h>
#include <stdbool.h>

#include <sni.h>
#include <LLUI_DISPLAY.h>
#include <LLUI_DISPLAY_impl.h>
#include <LLVG_MATRIX_impl.h>
#include <BESTFIT_ALLOCATOR.h>

#include "bsp_util.h"
#include "vg_lite.h"
#include "vg_lite_kernel.h"
#include "display_vglite.h"
#include "vglite_path.h"
#include "microvg_vglite_helper.h"
#include "microvg_helper.h"

#include "fsl_debug_console.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief MicroUI format to Buffered vector Image format
 */
#define BVI_FORMAT (MICROUI_IMAGE_FORMAT_CUSTOM_4) // hardcoded in Java lib
#define IS_BVI(image) (BVI_FORMAT == (MICROUI_ImageFormat)((image)->format))
#define MAP_BVI(image) (BVI_resource*)LLUI_DISPLAY_getBufferAddress((image))

/*
 * @brief Allocator management
 */
#define HEAP_SIZE (56 * 1024)
#define HEAP_START (&allocator_heap[0])
#define HEAP_END (&allocator_heap[HEAP_SIZE])
#define MALLOC(s) (BESTFIT_ALLOCATOR_allocate(&allocator_instance, VG_LITE_ALIGN((s), 4)))
#define FREE(s) (BESTFIT_ALLOCATOR_free(&allocator_instance, (s)))

/*
 * @brief The vglite matrix can be mapped on a float array
 */
#define MAP_TO_VGLITE_MATRIX(m) ((vg_lite_matrix_t*)(m))

// --------------------------------------------------------------------------------
// Foundation library's native functions names redefinition
// --------------------------------------------------------------------------------

#define LLVG_BVI_IMPL_draw          Java_ej_microvg_BufferedVectorImageNatives_draw
#define LLVG_BVI_IMPL_clear         Java_ej_microvg_BufferedVectorImageNatives_clear

// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------

/*
 * @brief Types of image's elements.
 */
typedef enum vglite_element_kind {

	VGLITE_DRAW_PATH,
	VGLITE_DRAW_GRADIENT,
	VGLITE_BLIT,
	VGLITE_BLIT_RECT,

} vglite_element_kind_t;

/*
 * @brief Defines a buffered image's element.
 */
typedef struct vglite_element {

	// pointer to the next element in the image
	struct vglite_element* next;

	// element's kind and pointer to specific element's data
	vglite_element_kind_t kind;
	void* operation; 

	// common element's data for all VG drawings
	vg_lite_matrix_t* matrix;
	int32_t* scissor;
	vg_lite_blend_t blend;

} vglite_element_t;

/*
 * @brief Element "draw VG path".
 */
typedef struct vglite_operation_path {

	vg_lite_path_t* path;
	vg_lite_color_t color;
	vg_lite_fill_t fill_rule;

} vglite_operation_path_t;

/*
 * @brief Element "draw VG gradient".
 */
typedef struct vglite_operation_gradient {

	vg_lite_path_t* path;
	vg_lite_linear_gradient_t* gradient;
	vg_lite_fill_t fill_rule;

} vglite_operation_gradient_t;

/*
 * @brief Element "blit image (rect or not)".
 */
typedef struct vglite_operation_blit {

	vg_lite_buffer_t* buffer;
	uint32_t* blit_rect;
	vg_lite_color_t color;
	vg_lite_filter_t filter;

} vglite_operation_blit_t;

/*
 * @brief Structure to load a resource by calling SNIX_get_resource()
 */
typedef struct {

	// pointer on the first commands buffer
	vglite_element_t* vglite_element_first;

	// pointer on the next commands buffer
	vglite_element_t* vglite_element_next;

} BVI_resource;

// -----------------------------------------------------------------------------
// Extern functions & fields
// -----------------------------------------------------------------------------

/*
 * @brief Function to retrieve the current scissor.
 */
extern uint32_t vg_lite_get_scissor(int32_t** scissor);

/*
 * @brief Linker symbols that define the ROM block.
 */
extern uint32_t m_text_start;
extern uint32_t m_text_end;

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

static vg_lite_error_t _add_draw_path(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_blend_t blend,
		vg_lite_color_t color) ;

static vg_lite_error_t _add_draw_gradient(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_linear_gradient_t * grad,
		vg_lite_blend_t blend) ;

static vg_lite_error_t _add_blit_rect(
		void* target,
		vg_lite_buffer_t *source,
		uint32_t         *rect,
		vg_lite_matrix_t *matrix,
		vg_lite_blend_t   blend,
		vg_lite_color_t   color,
		vg_lite_filter_t  filter) ;

static vg_lite_error_t _add_blit(
		void* target,
		vg_lite_buffer_t *source,
		vg_lite_matrix_t *matrix,
		vg_lite_blend_t   blend,
		vg_lite_color_t   color,
		vg_lite_filter_t  filter) ;

static vg_lite_error_t _add_clear(
		void* target,
		vg_lite_rectangle_t *rectangle,
		vg_lite_color_t   color) ;

static void __get_original_color(vg_lite_color_t* color, vg_lite_blend_t blend) ;

static void __get_original_gradient(vg_lite_linear_gradient_t* gradient, vg_lite_blend_t blend) ;

static DRAWING_Status __post_drawing(vg_lite_error_t vg_lite_error) ;

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

static BESTFIT_ALLOCATOR allocator_instance;
static uint8_t allocator_heap[HEAP_SIZE];
static bool initialiazed = false;

/*
 * @brief Matrix used to render a path in order to not alterate the original path's matrix.
 */
static vg_lite_matrix_t render_matrix;

static VG_DRAWER_drawer_t image_drawer = {

		_add_draw_path,
		_add_draw_gradient,
		_add_blit_rect,
		_add_blit,
		_add_clear,

		__get_original_color,
		__get_original_gradient,

		__post_drawing,
};

/*
 * @brief Gradient used for all drawings with gradient (only one image allocation,
 * does not alterate the original gradient's matrix and colors, etc.).
 */
static vg_lite_linear_gradient_t gradient;

// -----------------------------------------------------------------------------
// Private functions to store a VG drawing
// -----------------------------------------------------------------------------

/*
 * @brief Frees the given data from the heap or does nothing
 * when targets RO memory.
 *
 * @param[in] data: pointer on the data to free.
 */
static void _free_data(void* data) {
	if ((NULL != data) && !((data > &m_text_start) && (data < &m_text_end))) {
		if ((data > (void*) HEAP_START) && (data < (void*) HEAP_END) ) {
			FREE(data);
		}
		else {
			MEJ_LOG_ERROR_MICROVG("Invalid memory pointer: 0x%x is not in HEAP section.\n", data);
		}
	}
	// else: data in flash or null
}

/*
 * @brief Allocates the given size in the heap .
 *
 * @param[in] size: the data's size (in bytes).
 *
 * @return the pointer to the stored data.
 */
inline static uint8_t* _alloc_data(uint32_t size) {
	uint8_t* ret = MALLOC(size);
	if (NULL == ret) {
		MEJ_LOG_ERROR_MICROVG("OOM\n");
	}
	return ret;
}

/*
 * @brief Stores the given data in the heap or return the given pointer
 * when targets RO memory (no need to copy).
 *
 * @param[in] data: pointer on the data to store.
 * @param[in] size: the data's size.
 *
 * @return the pointer to the stored data or the ROM address.
 */
static uint8_t* _store_data(void* data, uint32_t size) {
	uint8_t* ret;
	if ((data > &m_text_start) && (data < &m_text_end)) {
		// cppcheck-suppress [misra-c2012-11.5] cast the object in a u8 address
		ret = (uint8_t*)data;
	}
	else {
		ret = _alloc_data(size);
		(void)memcpy((void*)ret, data, size);
	}
	return ret;
}

/*
 * @brief Stores the given rectangle in the heap. A rectangle is an array of 4 integers.
 *
 * @param[in] rect: pointer on the rectangle to store.
 *
 * @return the pointer to the stored rectangle.
 */
static uint32_t* _store_rectangle(uint32_t* rect) {
	// cppcheck-suppress [misra-c2012-11.3] cast is possible (operation has been allocated with 4-byte as alignment)
	return (NULL != rect) ? (uint32_t*)_store_data((void*)rect, (uint32_t)4 * sizeof(uint32_t)) : NULL;
}
static void _free_rectangle(uint32_t* rect) {
	_free_data(rect);
}

/*
 * @brief Stores the given path in the heap.
 *
 * @param[in] path: pointer on the path to store.
 *
 * @return the pointer to the stored path.
 */
static vg_lite_path_t* _store_vglite_path(vg_lite_path_t * path) {
	// store the path's header
	vg_lite_path_t* stored_path = (vg_lite_path_t*)_store_data(path, sizeof(vg_lite_path_t));

	// store the path's RAW data
	stored_path->path = (void*)_store_data(path->path, path->path_length);

	return stored_path;
}
static void _free_vglite_path(vg_lite_path_t * path) {
	_free_data(path->path);
	_free_data(path);
	path->path = NULL;
}

/*
 * @brief Stores the given gradient in the heap.
 *
 * @param[in] grad: pointer on the gradient to store.
 *
 * @return the pointer to the stored gradient.
 */
static vg_lite_linear_gradient_t* _store_vglite_gradient(vg_lite_linear_gradient_t * grad) {
	return (vg_lite_linear_gradient_t*)_store_data(grad, sizeof(vg_lite_linear_gradient_t));
}
static void _free_vglite_gradient(vg_lite_linear_gradient_t * grad)  {
	_free_data(grad);
}

/*
 * @brief Stores the given matrix in the heap.
 *
 * @param[in] matrix: pointer on the matrix to store.
 *
 * @return the pointer to the stored matrix.
 */
static vg_lite_matrix_t* _store_vglite_matrix(vg_lite_matrix_t * matrix) {
	return (vg_lite_matrix_t*)_store_data(matrix, sizeof(vg_lite_matrix_t));
}
static void _free_vglite_matrix(vg_lite_matrix_t * matrix) {
	_free_data(matrix);
}

static int32_t* _get_current_vglite_scissor(void) {
	int32_t* scissor;
	return ((uint32_t)0 != vg_lite_get_scissor(&scissor)) ? scissor : NULL;
}

static int32_t* _store_vglite_scissor(int32_t* scissor) {
	// cppcheck-suppress [misra-c2012-11.3] cast is possible (operation has been allocated with 4-byte as alignment)
	return (NULL != scissor) ? (int32_t*)_store_data(scissor, (uint32_t)4*sizeof(int32_t)) : NULL;
}

static void _free_vglite_scissor(int32_t* scissor) {
	_free_data(scissor);
}

/*
 * @brief Stores the given buffer in the heap. It does not store the buffer's pixel (keep
 * pointing on the original buffer's pixel).
 *
 * @param[in] buffer: pointer on the buffer to store.
 *
 * @return the pointer to the stored buffer.
 */
static vg_lite_buffer_t* _store_vglite_buffer(vg_lite_buffer_t* buffer) {
	return (vg_lite_buffer_t*)_store_data(buffer, sizeof(vg_lite_buffer_t));
}
static void _free_vglite_buffer(vg_lite_buffer_t* buffer) {
	_free_data(buffer);
}

/*
 * @brief Allocates an image element and resets it.
 *
 * @return the pointer to the allocated element.
 */
static vglite_element_t* _malloc_element(void) {
	// cppcheck-suppress [misra-c2012-11.3] cast is possible (operation has been allocated with 4-byte as alignment)
	vglite_element_t* elem = (vglite_element_t*)_alloc_data(sizeof(vglite_element_t));
	elem->next = NULL;
	elem->operation = NULL;
	return elem;
}

inline static vglite_operation_path_t* get_operation_path(void* op) {
	// cppcheck-suppress [misra-c2012-11.5] cast is possible (operation has been allocated with 4-byte as alignment)
	return (vglite_operation_path_t*)op;
}

inline static vglite_operation_gradient_t* get_operation_gradient(void* op) {
	// cppcheck-suppress [misra-c2012-11.5] cast is possible (operation has been allocated with 4-byte as alignment)
	return (vglite_operation_gradient_t*)op;
}

inline static vglite_operation_blit_t* get_operation_blit(void* op) {
	// cppcheck-suppress [misra-c2012-11.5] cast is possible (operation has been allocated with 4-byte as alignment)
	return (vglite_operation_blit_t*)op;
}

inline static BVI_resource* get_target(void* target) {
	// cppcheck-suppress [misra-c2012-11.5] cast is possible (the target stored in drawer is a BVI_resource* for sure)
	return (BVI_resource*)target;
}

/*
 * @brief Stores the given element in the image.
 *
 * @param[in] bvi: pointer on image.
 * @param[in] operation: pointer on the specific element's data.
 * @param[in] kind: type of element.
 * @param[in] matrix: pointer on the matrix to store.
 * @param[in] blend: blending to apply.
 */
static void _store_element(BVI_resource* bvi, void* operation, vglite_element_kind_t kind, vg_lite_matrix_t * matrix, vg_lite_blend_t blend, int32_t* scissor) {

	vglite_element_t* elem = bvi->vglite_element_next;

	// store operation
	elem->kind = kind;
	elem->operation = operation;

	// store operation rules
	elem->matrix = _store_vglite_matrix(matrix);
	elem->scissor = _store_vglite_scissor(scissor);
	elem->blend = blend;

	// prepare next element
	bvi->vglite_element_next = _malloc_element();
	elem->next = bvi->vglite_element_next;
}

static void _free_operation(void* operation, vglite_element_kind_t kind) {

	switch(kind) {
	case VGLITE_DRAW_PATH: {
		vglite_operation_path_t* op = get_operation_path(operation);
		_free_vglite_path(op->path);
		op->path = NULL;
	}
	break;
	case VGLITE_DRAW_GRADIENT: {
		vglite_operation_gradient_t* op = get_operation_gradient(operation);
		_free_vglite_path(op->path);
		_free_vglite_gradient(op->gradient);
		op->path = NULL;
		op->gradient = NULL;

	}
	break;
	case VGLITE_BLIT:
	case VGLITE_BLIT_RECT: {
		vglite_operation_blit_t* op = get_operation_blit(operation);
		_free_vglite_buffer(op->buffer);
		_free_rectangle(op->blit_rect);
		op->buffer = NULL;
		op->blit_rect = NULL;
	}
	break;
	default:
		MEJ_LOG_ERROR_MICROVG("unknown operation: %u\n", kind);
		break;
	}

	_free_data(operation);
}

static vglite_element_t* _free_element(vglite_element_t* elem) {

	vglite_element_t* ret = NULL;

	if (NULL != elem->operation) {

		ret = elem->next;

		_free_vglite_matrix(elem->matrix);
		_free_vglite_scissor(elem->scissor);
		_free_operation(elem->operation, elem->kind);

		elem->matrix = NULL;
		elem->scissor = NULL;
		elem->operation = NULL;
		elem->next = NULL;

	}

	_free_data(elem);

	return ret;
}

// -----------------------------------------------------------------------------
// Private functions to draw into a buffered vector image
// -----------------------------------------------------------------------------

/*
 * @brief Transform a 2D point by a given matrix.
 *
 * Copy from vg_lite.c
 */
static void _transform_point(vg_lite_point_t * result, vg_lite_float_t x, vg_lite_float_t y, vg_lite_matrix_t * matrix) {

	/* Transform w. */
	vg_lite_float_t pt_w = (x * matrix->m[2][0]) + (y * matrix->m[2][1]) + matrix->m[2][2];
	if (pt_w <= 0.0f) {
		result->x = 0;
		result->y = 0;
	}
	else {
		/* Transform x and y. */
		vg_lite_float_t pt_x = (x * matrix->m[0][0]) + (y * matrix->m[0][1]) + matrix->m[0][2];
		vg_lite_float_t pt_y = (x * matrix->m[1][0]) + (y * matrix->m[1][1]) + matrix->m[1][2];

		/* Compute projected x and y. */
		result->x = (int)(pt_x / pt_w);
		result->y = (int)(pt_y / pt_w);
	}
}

/*
 * @brief Gets the top-left and bottom-right points scissor points adjusted with the given matrix.
 *
 * @param[in] scissor: pointer on the scissor (not NULL).
 * @param[in] matrix: pointer on the matrix to apply on the scissor.
 * @param[in] matrix: pointer on the top-left point to fill.
 * @param[in] matrix: pointer on the bottom-right point to fill.
 */
static inline void _get_scissor_points(int32_t* scissor, vg_lite_matrix_t *matrix, vg_lite_point_t* top_left, vg_lite_point_t* bottom_right) {
	_transform_point(top_left, (vg_lite_float_t)scissor[0], (vg_lite_float_t)scissor[1], matrix);
	_transform_point(bottom_right, (vg_lite_float_t)(scissor[0] + scissor[2] - 1), (vg_lite_float_t)(scissor[1] + scissor[3] - 1), matrix);
}

/*
 * @brief Applies the scissor. The scissor is adjusted with the given matrix (image's matrix)
 * and the target's bounds.
 *
 * @param[in] gc: pointer on the target.
 * @param[in] scissor: pointer on the scissor to apply or NULL.
 * @param[in] matrix: pointer on the matrix to apply on the scissor.
 *
 * @return false when the drawing is useless (empty scissor), true otherise.
 */
static bool _apply_scissor(MICROUI_GraphicsContext* gc, int32_t* scissor, vg_lite_matrix_t *matrix) {

	bool draw = true;

	vg_lite_enable_scissor();

	if (NULL != scissor) {
		vg_lite_point_t top_left;
		vg_lite_point_t bottom_right;

		_get_scissor_points(scissor, matrix, &top_left, &bottom_right);

		if (top_left.x < gc->clip_x1) {
			top_left.x = gc->clip_x1;
		}
		if (top_left.y < gc->clip_y1) {
			top_left.y = gc->clip_y1;
		}
		if (bottom_right.x >= gc->clip_x2) {
			bottom_right.x = gc->clip_x2;
		}
		if (bottom_right.y >= gc->clip_y2) {
			bottom_right.y = gc->clip_y2;
		}

		if ((top_left.x <= bottom_right.x) && (top_left.y <= bottom_right.y)) {
			vg_lite_set_scissor(top_left.x, top_left.y, bottom_right.x - top_left.x + 1, bottom_right.y - top_left.y + 1);
		}
		else {
			// empty clip
			draw = false;
		}
	}
	else {
		vg_lite_set_scissor(gc->clip_x1, gc->clip_y1, gc->clip_x2 - gc->clip_x1 + 1, gc->clip_y2 - gc->clip_y1 + 1);
	}

	return draw;
}

/*
 * @brief Derives the scissor. The scissor is adjusted with the given matrix.
 *
 * @param[in] scissor: pointer on the scissor to apply (not NULL).
 * @param[in] matrix: pointer on the matrix to apply on the scissor.
 */
static void _derive_scissor(int32_t* scissor, vg_lite_matrix_t *matrix) {

	vg_lite_point_t top_left;
	vg_lite_point_t bottom_right;
	_get_scissor_points(scissor, matrix, &top_left, &bottom_right);

	scissor[0] = top_left.x;
	scissor[1] = top_left.y;
	scissor[2] = bottom_right.x - top_left.x + 1;
	scissor[3] = bottom_right.y - top_left.y + 1;
}

/*
 * @brief Applies the given matrix (image's matrix) on the element's matrix.
 *
 * Copy from vg_lite.c
 *
 * TODO / IDEAS
 * - flag matrix identity in elem
 * - flag matrix identity in global param
 * - check if matrix param has changed and save result in other struct element (prevent mul)
 *
 * @param[in] result: pointer on the matrix where storing the result.
 * @param[in] elem_matrix: pointer on the element's matrix.
 * @param[in] matrix: pointer on the matrix to apply on the element's matrix.
 */
static void _apply_matrix(vg_lite_matrix_t* result, vg_lite_matrix_t* elem_matrix, vg_lite_matrix_t* matrix) {
	/* Process all rows. */
	for (int row = 0; row < 3; row++) {
		/* Process all columns. */
		for (int column = 0; column < 3; column++) {
			/* Compute matrix entry. */
			result->m[row][column] =
					(matrix->m[row][0] * elem_matrix->m[0][column])
					+ (matrix->m[row][1] * elem_matrix->m[1][column])
					+ (matrix->m[row][2] * elem_matrix->m[2][column]);
		}
	}
}

/*
 * @brief Applies the global opacity on each color of given gradient operation.
 *
 * @param[in] gradient: the gradient operation.
 * @param[in] alpha: the opacity.
 *
 * @return the new color.
 */
static void _apply_alpha_on_gradient(vg_lite_linear_gradient_t* gradient, uint32_t alpha) {
	if (alpha != (uint32_t)0xff) {
		uint32_t *colors = (uint32_t *)gradient->colors;
		for(int i = 0; i < gradient->count; i++) {
			colors[i] = MICROVG_HELPER_apply_alpha(colors[i], alpha);
		}
	}
	// else: no need to apply alpha
}

static void _draw_in_pixel_buffer(MICROUI_GraphicsContext* gc, BVI_resource* source, vg_lite_matrix_t* matrix, uint32_t alpha) {

	VG_DRAWER_drawer_t* drawer = VGLITE_PATH_get_vglite_drawer(gc);
	vglite_element_t* elem = (vglite_element_t*)source->vglite_element_first;
	bool draw_gradient_flushed = true; // no drawing with gradient has been added yet

	while(NULL != elem->operation) {

		if (_apply_scissor(gc, elem->scissor, matrix)) {

			// update the path's matrix
			_apply_matrix(&render_matrix, elem->matrix, matrix);

			switch(elem->kind) {
			case VGLITE_DRAW_PATH: {
				vglite_operation_path_t* op = get_operation_path(elem->operation);
				vg_lite_color_t color = MICROVG_HELPER_apply_alpha(op->color, alpha);
				drawer->update_color(&color, elem->blend);
				VG_DRAWER_draw_path(drawer, op->path, op->fill_rule, &render_matrix, elem->blend, color);
			}
			break;
			case VGLITE_DRAW_GRADIENT: {
				vglite_operation_gradient_t* op = get_operation_gradient(elem->operation);

				// copy the gradient content but not the image that renders the gradient
				size_t copy_size = sizeof(vg_lite_linear_gradient_t) - sizeof(vg_lite_buffer_t);

				// copy op's gradient in a local gradient to apply the opacity
				vg_lite_linear_gradient_t local_gradient;
				(void)memcpy(&local_gradient, op->gradient, copy_size);
				_apply_alpha_on_gradient(&local_gradient, alpha);

				if (0 != memcmp(&gradient, &local_gradient, copy_size)) {
					// not same gradient than previous: have to:

					// 1- flush the previous gradient drawings because we will update the shared gradient's image
					if (!draw_gradient_flushed) {
						DISPLAY_VGLITE_start_operation(false);
					}

					// 2- copy the new gradient data in shared gradient
					(void)memcpy(&gradient, &local_gradient, copy_size);

					// 3- update the shared gradient's image.
					drawer->update_gradient(&gradient, elem->blend);
				}
				// else: same gradient than previous: no need to update gradient's image again

				// update the gradient's matrix
				_apply_matrix(&(gradient.matrix), &(op->gradient->matrix), matrix);

				VG_DRAWER_draw_gradient(drawer, op->path, op->fill_rule, &render_matrix, &gradient, elem->blend);
				draw_gradient_flushed = false;
			}
			break;
			case VGLITE_BLIT: {
				vglite_operation_blit_t* op = get_operation_blit(elem->operation);
				vg_lite_color_t color = MICROVG_HELPER_apply_alpha(op->color, alpha);
				drawer->update_color(&color, elem->blend);
				VG_DRAWER_blit(drawer, op->buffer, &render_matrix, elem->blend, color, op->filter);
			}
			break;
			case VGLITE_BLIT_RECT: {
				vglite_operation_blit_t* op = get_operation_blit(elem->operation);
				vg_lite_color_t color = MICROVG_HELPER_apply_alpha(op->color, alpha);
				drawer->update_color(&color, elem->blend);
				VG_DRAWER_blit_rect(drawer, op->buffer, op->blit_rect, &render_matrix, elem->blend, color, op->filter);
			}
			break;
			default:
				MEJ_LOG_ERROR_MICROVG("unknown operation: %u\n", elem->kind);
				break;
			}
		}
		elem = elem->next;
	}

	// flush all operations
	DISPLAY_VGLITE_start_operation(true);
}

static void _draw_in_command_buffer(BVI_resource* target, BVI_resource* source, vg_lite_matrix_t* matrix, uint32_t alpha) {

	// have to copy all source's commands in target because the source commands list can be cleared

	vglite_element_t* elem = (vglite_element_t*)source->vglite_element_first;

	while(NULL != elem->operation) {

		// update the path's matrix
		_apply_matrix(&render_matrix, elem->matrix, matrix);

		void* operation;


		switch(elem->kind) {
		case VGLITE_DRAW_PATH: {

			vglite_operation_path_t* op_source = get_operation_path(elem->operation);
			vglite_operation_path_t* op_dest = get_operation_path(_alloc_data(sizeof(vglite_operation_path_t)));
			operation = (void*)op_dest;

			op_dest->path = _store_vglite_path(op_source->path);
			op_dest->color = MICROVG_HELPER_apply_alpha(op_source->color, alpha);
			op_dest->fill_rule = op_source->fill_rule;
		}
		break;
		case VGLITE_DRAW_GRADIENT: {
			vglite_operation_gradient_t* op_source = get_operation_gradient(elem->operation);
			vglite_operation_gradient_t* op_dest = get_operation_gradient(_alloc_data(sizeof(vglite_operation_gradient_t)));
			operation = (void*)op_dest;

			op_dest->path = _store_vglite_path(op_source->path);
			op_dest->gradient = _store_vglite_gradient(op_source->gradient);
			op_dest->fill_rule = op_source->fill_rule;

			// update the gradient's matrix and colors
			_apply_matrix(&op_dest->gradient->matrix, &(op_source->gradient->matrix), matrix);
			_apply_alpha_on_gradient(op_dest->gradient, alpha);
		}
		break;
		case VGLITE_BLIT_RECT:
		case VGLITE_BLIT: {
			vglite_operation_blit_t* op_source = get_operation_blit(elem->operation);
			vglite_operation_blit_t* op_dest = get_operation_blit(_alloc_data(sizeof(vglite_operation_blit_t)));
			operation = (void*)op_dest;

			op_dest->buffer = _store_vglite_buffer(op_source->buffer);
			op_dest->blit_rect = _store_rectangle(op_source->blit_rect);
			op_dest->color = MICROVG_HELPER_apply_alpha(op_source->color, alpha);
			op_dest->filter = op_source->filter;
		}
		break;
		default:
			MEJ_LOG_ERROR_MICROVG("unknown operation: %u\n", elem->kind);
			break;
		}

		vglite_element_t* new_elem = target->vglite_element_next;
		_store_element(target, operation, elem->kind, &render_matrix, elem->blend, elem->scissor);
		if (NULL != new_elem->scissor) {
			_derive_scissor(new_elem->scissor, matrix);
		}

		elem = elem->next;
	}
}

// -----------------------------------------------------------------------------
// LLUI_DISPLAY_impl.h functions (FIXME)
// -----------------------------------------------------------------------------

void LLUI_DISPLAY_IMPL_adjustNewImageCharacteristics(jbyte image_format, uint32_t width, uint32_t height, uint32_t* data_size, uint32_t* data_alignment) {
	(void)width;
	(void)height;
	(void)data_alignment;
	*data_size = (BVI_FORMAT == (MICROUI_ImageFormat)image_format) ? (*data_size + sizeof(BVI_resource)) : *data_size /* unknown format */ ;
}

void LLUI_DISPLAY_IMPL_initializeNewImage(MICROUI_Image* image) {
	if (IS_BVI(image)) {

		if (!initialiazed) {

			// prepare the allocator
			BESTFIT_ALLOCATOR_new(&allocator_instance);
			// cppcheck-suppress [misra-c2012-11.4] force the cast to uint32_t
			BESTFIT_ALLOCATOR_initialize(&allocator_instance, (uint32_t) HEAP_START, (uint32_t) HEAP_END);

			// prepare the gradient used by all draw_gradient
			(void)memset(&gradient, 0, sizeof(vg_lite_linear_gradient_t));
			(void)vg_lite_init_grad(&gradient); // always success
			gradient.image.format = VG_LITE_RGBA8888; // fix color format

			initialiazed = true;
		}

		// map a struct on image data area
		BVI_resource* bvi = MAP_BVI(image);

		bvi->vglite_element_first = _malloc_element();
		bvi->vglite_element_next =  bvi->vglite_element_first;
	}
}

// -----------------------------------------------------------------------------
// LLVG_BVI_impl.h functions (FIXME)
// -----------------------------------------------------------------------------

void LLVG_BVI_IMPL_clear(MICROUI_GraphicsContext* gc) {

	// cppcheck-suppress [misra-c2012-14.4] ignore warning
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLVG_BVI_IMPL_clear)) {

		// map a struct on graphics context's pixel area
		BVI_resource* bvi = MAP_BVI(&gc->image);

		vglite_element_t* elem = (vglite_element_t*)bvi->vglite_element_first;

		do {
			elem = _free_element(elem);
		} while(NULL != elem) ;

		bvi->vglite_element_first = _malloc_element();
		bvi->vglite_element_next =  bvi->vglite_element_first;

		LLUI_DISPLAY_setDrawingStatus(DRAWING_DONE);
	}
}

void LLVG_BVI_IMPL_draw(MICROUI_GraphicsContext* gc, MICROUI_GraphicsContext* source, jint x, jint y, jfloat *matrix, uint32_t alpha) {

	if ((alpha > (uint32_t)0) && LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLVG_BVI_IMPL_draw)) {

		vg_lite_matrix_t vg_lite_matrix;
		jfloat* mapped_matrix = MAP_VGLITE_MATRIX(&vg_lite_matrix);

		if((0 != x) || (0 != y)) {
			// Create translate matrix for initial x,y translation from graphicscontext.
			LLVG_MATRIX_IMPL_setTranslate(mapped_matrix, x, y);
			LLVG_MATRIX_IMPL_concatenate(mapped_matrix, matrix);
		}
		else {
			// use original matrix
			LLVG_MATRIX_IMPL_copy(mapped_matrix, matrix);
		}

		BVI_resource* bvi = MAP_BVI(&source->image);

		if (!IS_BVI(&gc->image)) {
			_draw_in_pixel_buffer(gc, bvi, &vg_lite_matrix, alpha);
			LLUI_DISPLAY_setDrawingStatus(DRAWING_RUNNING);
		}
		else {
			_draw_in_command_buffer(MAP_BVI(&gc->image), bvi, &vg_lite_matrix, alpha);
			LLUI_DISPLAY_setDrawingStatus(DRAWING_DONE);
		}
	}
}

// -----------------------------------------------------------------------------
// VGLITE Drawer redirection
// -----------------------------------------------------------------------------

static vg_lite_error_t _add_draw_path(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_blend_t blend,
		vg_lite_color_t color) {

	vglite_operation_path_t* op = get_operation_path(_alloc_data(sizeof(vglite_operation_path_t)));
	op->path = _store_vglite_path(path);
	op->color = color;
	op->fill_rule = fill_rule;

	_store_element(get_target(target), (void*)op, VGLITE_DRAW_PATH, matrix, blend, _get_current_vglite_scissor());
	return VG_LITE_SUCCESS;
}

static vg_lite_error_t _add_draw_gradient(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_linear_gradient_t * grad,
		vg_lite_blend_t blend) {

	vglite_operation_gradient_t* op = get_operation_gradient(_alloc_data(sizeof(vglite_operation_gradient_t)));
	op->path = _store_vglite_path(path);
	op->gradient = _store_vglite_gradient(grad);
	op->fill_rule = fill_rule;

	_store_element(get_target(target), (void*)op, VGLITE_DRAW_GRADIENT, matrix, blend, _get_current_vglite_scissor());
	return VG_LITE_SUCCESS;
}

static vg_lite_error_t _add_blit_rect(
		void* target,
		vg_lite_buffer_t *source,
		uint32_t         *rect,
		vg_lite_matrix_t *matrix,
		vg_lite_blend_t   blend,
		vg_lite_color_t   color,
		vg_lite_filter_t  filter) {

	vglite_operation_blit_t* op = get_operation_blit(_alloc_data(sizeof(vglite_operation_blit_t)));
	op->buffer = _store_vglite_buffer(source);
	op->blit_rect = _store_rectangle(rect);
	op->color = color;
	op->filter = filter;

	_store_element(get_target(target), (void*)op, VGLITE_BLIT_RECT, matrix, blend, _get_current_vglite_scissor());
	return VG_LITE_SUCCESS;
}

static vg_lite_error_t _add_blit(
		void* target,
		vg_lite_buffer_t *source,
		vg_lite_matrix_t *matrix,
		vg_lite_blend_t   blend,
		vg_lite_color_t   color,
		vg_lite_filter_t  filter) {

	vglite_operation_blit_t* op = get_operation_blit(_alloc_data(sizeof(vglite_operation_blit_t)));
	op->buffer = _store_vglite_buffer(source);
	op->blit_rect = NULL;
	op->color = color;
	op->filter = filter;

	_store_element(get_target(target), (void*)op, VGLITE_BLIT, matrix, blend, _get_current_vglite_scissor());
	return VG_LITE_SUCCESS;
}

static vg_lite_error_t _add_clear(
		void* target,
		vg_lite_rectangle_t *rectangle,
		vg_lite_color_t   color) {

	// create a path that represents the rectangle because there is no API to clear a rectangle with a matrix

	int path_offset;
	vg_lite_path_t shape_vg_path;
	vg_lite_matrix_t matrix;
	vg_lite_error_t ret;

	vg_lite_identity(&matrix);

	// Compute the rectangle shape path
	vglite_path_rectangle_t rectangle_path;
	shape_vg_path.path = &rectangle_path;
	shape_vg_path.path_length = sizeof(rectangle_path);
	path_offset = VGLITE_PATH_compute_rectangle(&shape_vg_path, 0, rectangle->x, rectangle->y, rectangle->width, rectangle->height, true);

	if (0 > path_offset) {
		ret = VG_LITE_INVALID_ARGUMENT;
	}
	else {
		ret = _add_draw_path(
				target,
				&shape_vg_path,
				VG_LITE_FILL_EVEN_ODD,
				&matrix,
				VG_LITE_BLEND_SRC_OVER,
				color
		);
	}

	return ret;
}

static void __get_original_color(vg_lite_color_t* color, vg_lite_blend_t blend) {
	(void)color;
	(void)blend;
	// nothing to do
}

static void __get_original_gradient(vg_lite_linear_gradient_t* gradient, vg_lite_blend_t blend) {
	(void)gradient;
	(void)blend;
	// nothing to do
}

static DRAWING_Status __post_drawing(vg_lite_error_t vg_lite_error) {
	(void)vg_lite_error;
	// TODO manage OOM
	return DRAWING_DONE;
}

// -----------------------------------------------------------------------------
// Replace default drawer
// -----------------------------------------------------------------------------

VG_DRAWER_drawer_t* VG_DRAWER_get_drawer(MICROUI_GraphicsContext* gc){

	VG_DRAWER_drawer_t* drawer;

	if (!IS_BVI(&gc->image)) {
		drawer = VGLITE_PATH_get_vglite_drawer(gc);
	}
	else {
		// don't care about caller's drawer configuration
		// see _draw_in_pixel_buffer()
		image_drawer.target = (void*)MAP_BVI(&gc->image);
		drawer = &image_drawer;
	}

	return drawer;
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#endif // VGLITE_USE_MULTIPLE_DRAWERS
