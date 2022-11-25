/*
 * C
 *
 * Copyright 2020-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief MicroEJ MicroVG library low level API: implementation over FreeType
 * @author MicroEJ Developer Team
 * @version 2.0.0
 */

#include "microvg_configuration.h"

#if defined VG_FEATURE_FONT && \
	(defined VG_FEATURE_FONT_FREETYPE_VECTOR || defined VG_FEATURE_FONT_FREETYPE_BITMAP) && \
	(VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_VECTOR || VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_BITMAP)

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <math.h>

#include <aftypes.h>

#include <LLVG_impl.h>
#include <LLVG_FONT_impl.h>

#if defined (VG_FEATURE_FONT_EXTERNAL)
#include <LLEXT_RES_impl.h>
#include <freetype/internal/ftmemory.h>
#endif

#include "microvg_font_freetype.h"
#include "microvg_helper.h"
#include "mej_math.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Computes the scale to apply to the font.
 */
#define GET_SCALE(s,f) ((s) / (f)->units_per_EM)

// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------

/*
 * @brief Structure to load a resource by calling SNIX_get_resource()
 */
typedef struct {
	void* data;
	uint32_t size;
} SNIX_resource;

// -----------------------------------------------------------------------------
// Extern functions
// -----------------------------------------------------------------------------

/*
 * @brief SNIX_get_resource() available since MicroEJ Architecture version 7.13
 */
extern int32_t SNIX_get_resource(jchar* path, SNIX_resource* resource);

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

/*
 * @brief Disposes Freetype font when the associated Java object VectorFont is
 * garbaged collected.
 */
static void _dispose_registered_font(void* faceHandle);

/*
 * @brief Opens a font that has been loaded into memory.
 *
 * @see FT_New_Memory_Face in freetype.h
 *
 * @param[in] face: a handle to a new face object.
 * @param[in] data: a pointer to the beginning of the font data.
 * @param[in] length: the size of the memory chunk used by the font data.
 *
 * @return FreeType error code.
 */
static FT_Error __load_memory_font(FT_Face* face, void* data, int length);

/*
 * @brief Opens a font that has been compiled with the application.
 *
 * @see FT_New_Memory_Face in freetype.h
 *
 * @param[in] face: a handle to a new face object.
 * @param[in] font_name: a path to the font file.
 *
 * @return FreeType error code.
 */
static FT_Error __load_internal_font(FT_Face* face, jchar* font_name);


#if defined (VG_FEATURE_FONT_EXTERNAL)
/*
 * @brief Opens a font that has not been compiled with the application.
 *
 * @see FT_Open_Face in freetype.h
 *
 * @param[in] face: a handle to a new face object.
 * @param[in] font_name: a path to the font file.
 *
 * @return FreeType error code.
 */
static FT_Error __load_external_font(FT_Face* face, jchar* font_name);



/*
 * @brief Reads a chunk from an external resource.
 *
 * @see FT_Stream_IoFunc in ftsystem.h
 *
 * @param[in] stream: a handle to the source stream.
 * @param[in] offset: the offset of read in stream.
 * @param[in] buffer: the address of the read buffer.
 * @param[in] count: the number of bytes to read from the stream.
 *
 * @return the number of bytes effectively read by the stream.
 */
static unsigned long __read_external_resource(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count);

/*
 * @brief Closes a given input stream.
 *
 * @see FT_Stream_IoFunc in ftsystem.h
 *
 * @param[in] stream: a handle to the target stream.
 */
static void __close_external_resource(FT_Stream stream);

#endif // VG_FEATURE_FONT_EXTERNAL

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

FT_Library library;
FT_Renderer renderer;

#if defined VG_FEATURE_FONT && defined VG_FEATURE_FONT_FREETYPE_VECTOR && (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_VECTOR)
const char* renderer_name = "vglite_renderer";
#endif
#if defined VG_FEATURE_FONT && defined VG_FEATURE_FONT_FREETYPE_BITMAP && (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_BITMAP)
const char* renderer_name = "smooth";
#endif

// -----------------------------------------------------------------------------
// microvg_font_freetype.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
void MICROVG_FONT_FREETYPE_initialize(void) {

	FT_Error error = FT_Init_FreeType(&library);
	if(FT_ERR(Ok) == error) {
		renderer = FT_RENDERER(FT_Get_Module(library, renderer_name));
		if(0 != renderer) {
			MEJ_LOG_INFO_MICROVG("Freetype renderer: %s\n", FT_MODULE_CLASS(renderer)->module_name);
		}
		else {
			MEJ_LOG_ERROR_MICROVG("No renderer found with name %s\n", renderer_name);
		}
	}
	else {
		MEJ_LOG_ERROR_MICROVG("Internal freetype error initializing library, ID = %d\n",error);
	}
}

// -----------------------------------------------------------------------------
// LLVG_FONT_impl.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
jint LLVG_FONT_IMPL_load_font(jchar* font_name, jboolean complex_layout) {

	FT_Face face = 0;
	jint ret = LLVG_FONT_UNLOADED;

	// Check that complex layouter is available for complex text layout
	if(complex_layout && !LLVG_FONT_IMPL_has_complex_layouter()){
		ret = LLVG_FONT_UNLOADED;
	}
	else {
		// try to load an internal resource
		FT_Error error = __load_internal_font(&face, font_name);

#if defined (VG_FEATURE_FONT_EXTERNAL)
		if (FT_ERR(Cannot_Open_Resource) == error) {
			// try to load an external resource
			error = __load_external_font(&face, font_name);
		}
#endif // VG_FEATURE_FONT_EXTERNAL

		if (FT_ERR(Ok) == error) {
			// Use Unicode
			FT_Select_Charmap(face , ft_encoding_unicode);
			MEJ_LOG_INFO_MICROVG("Freetype font loaded: %s\n", (const char*)font_name);

			SNI_registerResource((void*)face, (SNI_closeFunction)&_dispose_registered_font, JNULL);

#if defined (VG_FEATURE_FONT_COMPLEX_LAYOUT)
			if(JTRUE == complex_layout){
				face->face_flags |= FT_FACE_FLAG_COMPLEX_LAYOUT;
			}
#endif // VG_FEATURE_FONT_COMPLEX_LAYOUT

			ret = (jint) face;
		}
		else if (FT_ERR(Cannot_Open_Resource) != error){
			MEJ_LOG_ERROR_MICROVG("An error occurred during font loading: 0x%x, refer to fterrdef.h\n", error);
		}
		else {
			MEJ_LOG_ERROR_MICROVG("Resource not found: %s\n", (const char*)font_name);
		}

	}
	return ret;
}

// See the header file for the function documentation
jfloat LLVG_FONT_IMPL_string_width(jchar* text, jint faceHandle, jfloat size, jfloat letterSpacing) {

	jfloat ret;

	if (LLVG_FONT_UNLOADED == faceHandle) {
		ret = (jfloat)LLVG_RESOURCE_CLOSED;
	}
	else {

		FT_Face face = (FT_Face) faceHandle;
		float scaled_width = 0.f;

		if(size >= 0) {
			float scale = GET_SCALE(size, face);

			int nb_chars = 0;
			long unscaled_width = 0;

			// Layout variables
			int glyph_index ; // current glyph index
			int previous_glyph_index = 0; // previous glyph index for kerning

			int advance_x;
			int previous_advance_x;
			int advance_y;
			int offset_x;
			int offset_y;

			int length = (int)SNI_getArrayLength(text);
			MICROVG_HELPER_layout_configure(faceHandle, text, length);

			while(0 != MICROVG_HELPER_layout_load_glyph(&glyph_index, &advance_x, &advance_y, &offset_x, &offset_y)) {
				// At that point the current glyph has been loaded by Freetype
				if (0 == previous_glyph_index){
					// first glyph: remove the first blank line
					if(0 != face->glyph->metrics.width) {
						unscaled_width -= face->glyph->metrics.horiBearingX;
					}
					else {
						unscaled_width -= face->glyph->advance.x;
					}
				}

				unscaled_width += advance_x;
				previous_glyph_index = glyph_index;
				nb_chars ++;
				// Last call to MICROVG_HELPER_layout_load_glyph clear advance_x, we need to keep.
				previous_advance_x = advance_x;
			}

			// last glyph: remove the last blank line
			if(0 != face->glyph->metrics.width) {
				unscaled_width -= previous_advance_x;
				unscaled_width += face->glyph->metrics.horiBearingX; // glyph's left blank line
				unscaled_width += face->glyph->metrics.width; // glyph's width
			}
			else {
				if(0 != unscaled_width){
					unscaled_width -= previous_advance_x;
				}
			}
			scaled_width = ((scale * (float) unscaled_width) + (((float) nb_chars - 1) * letterSpacing));
		}

		ret = scaled_width;
	}

	return ret;
}

// See the header file for the function documentation
jfloat LLVG_FONT_IMPL_string_height(jchar* text, jint faceHandle, jfloat size) {

	jfloat ret;

	if (LLVG_FONT_UNLOADED == faceHandle) {
		ret = (jfloat)LLVG_RESOURCE_CLOSED;
	}
	else {

		FT_Face face = (FT_Face) faceHandle;
		float scaled_height = 0.f;

		if(size >= 0) {
			float scale = GET_SCALE(size, face);

			FT_Pos horiBearingYTop = 0;
			FT_Pos horiBearingYBottom = 0;

			// Layout variables
			int glyph_index ; // current glyph index
			int previous_glyph_index = 0; // previous glyph index for kerning

			int advance_x;
			int advance_y;
			int offset_x;
			int offset_y;

			int length = (int)SNI_getArrayLength(text);
			MICROVG_HELPER_layout_configure(faceHandle, text, length);

			while(0 != MICROVG_HELPER_layout_load_glyph(&glyph_index, &advance_x, &advance_y, &offset_x, &offset_y)) {
				// At that point the current glyph has been loaded by Freetype

				FT_Pos yBottom = face->glyph->metrics.horiBearingY - face->glyph->metrics.height;
				horiBearingYBottom = (0 == previous_glyph_index) ? yBottom : MEJ_MIN(yBottom, horiBearingYBottom);
				horiBearingYTop = MEJ_MAX(face->glyph->metrics.horiBearingY, horiBearingYTop);

				previous_glyph_index = glyph_index;
			}

			scaled_height = scale * (float) (horiBearingYTop - horiBearingYBottom);
		}

		ret = scaled_height;
	}

	return ret;
}

// See the header file for the function documentation
jfloat LLVG_FONT_IMPL_get_baseline_position(jint faceHandle, jfloat size) {

	jfloat ret;

	if (LLVG_FONT_UNLOADED == faceHandle) {
		ret = (jfloat)LLVG_RESOURCE_CLOSED;
	}
	else {

		FT_Face face = (FT_Face) faceHandle;
		float advance_y = 0.f;

		if(size >= 0) {
			float scale = GET_SCALE(size, face);
			advance_y = (face->ascender * scale);
		}
		ret = advance_y;
	}

	return ret;
}

// See the header file for the function documentation
jfloat LLVG_FONT_IMPL_get_height(jint faceHandle, jfloat size) {

	jfloat ret;

	if (LLVG_FONT_UNLOADED == faceHandle) {
		ret = (jfloat)LLVG_RESOURCE_CLOSED;
	}
	else {
		FT_Face face = (FT_Face) faceHandle;
		float scale = GET_SCALE(size, face);
		ret = face->height * scale;
	}

	return ret;
}

// See the header file for the function documentation
void LLVG_FONT_IMPL_dispose(jint faceHandle) {
	_dispose_registered_font((void*)faceHandle);
}

// See the header file for the function documentation
bool LLVG_FONT_IMPL_has_complex_layouter(void){
#ifdef VG_FEATURE_FONT_COMPLEX_LAYOUT
	return true;
#else
	return false;
#endif
}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

static void _dispose_registered_font(void* faceHandle) {
	FT_Face face = (FT_Face) faceHandle;

#if defined (VG_FEATURE_FONT_EXTERNAL)
	// FT_Done_Face() sets the stream to NULL: have to save it to close the 
	// external resource
	FT_Stream stream = face->stream;
#endif // VG_FEATURE_FONT_EXTERNAL

	FT_Done_Face(face);

#if defined (VG_FEATURE_FONT_EXTERNAL)
	// frees the stream when the font is external (Freetype doesn't recommend
	// to read the flag FT_FACE_FLAG_EXTERNAL_STREAM)
	if (&__close_external_resource == stream->close) {
		ft_mem_free(library->memory, stream);
	}
#endif // VG_FEATURE_FONT_EXTERNAL
}

static FT_Error __load_memory_font(FT_Face* face, void* data, int length) {
	return FT_New_Memory_Face(library, data, length,  0, face);
}

static FT_Error __load_internal_font(FT_Face* face, jchar* font_name) {
	SNIX_resource font_resource;
	FT_Error error;

	if (0 == SNIX_get_resource(font_name, &font_resource)) {
		error = __load_memory_font(face, font_resource.data, font_resource.size);
	}
	else {
		error = FT_ERR(Cannot_Open_Resource);
	}
	return error;
}

#if defined (VG_FEATURE_FONT_EXTERNAL)

static FT_Error __load_external_font(FT_Face* face, jchar* font_name) {

	FT_Error error;

	// try to load an external resource (respect LLEXT_RES_open path naming convention)
	char const* ext_path = &(((char const*)(font_name))[1]) /* first '/' */;
	RES_ID resource_id = LLEXT_RES_open(ext_path);

	if (0 <= resource_id) {

		int32_t resource_size = LLEXT_RES_available(resource_id);

		// check if the external resource is byte addressable
		int32_t base_address = LLEXT_RES_getBaseAddress(resource_id);

		if (-1 != base_address) {

			// load the font as "memory" font
			// cppcheck-suppress [misra-c2012-11.6] base_address is an address
			error = __load_memory_font(face, (void*)base_address, resource_size);

			// we can close the resource because we know its address
			LLEXT_RES_close(resource_id);
		}
		else {
			// load the font as external font

			FT_Open_Args args;
			args.flags = FT_OPEN_STREAM;
			args.driver = NULL;
			args.stream = (FT_StreamRec*)ft_mem_alloc(library->memory, sizeof(FT_StreamRec), &error);

			if (FT_ERR(Ok) == error) {

				args.stream->base = NULL;
				args.stream->size = resource_size;
				args.stream->pos = 0;
				args.stream->descriptor.value = resource_id;
				// args.stream->pathname = NULL; // not used
				args.stream->read = &__read_external_resource;
				args.stream->close = &__close_external_resource;

				error = FT_Open_Face(library, &args, 0, face);
			}
			else {
				error = FT_ERR(Out_Of_Memory);
			}
		}
	}
	else {
		error = FT_ERR(Cannot_Open_Resource);
	}

	return error;
}

static unsigned long __read_external_resource(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count) {
	int32_t size = count;
	RES_ID resource_id = (RES_ID)(stream->descriptor.value);
	LLEXT_RES_seek(resource_id, offset);
	LLEXT_RES_read(resource_id, buffer, &size);
	return size;
}

static void __close_external_resource(FT_Stream stream) {
	RES_ID resource_id = (RES_ID)(stream->descriptor.value);
	LLEXT_RES_close(resource_id);
}

#endif // VG_FEATURE_FONT_EXTERNAL
// cppcheck-suppress [misra-c2012-3.2]
#endif // defined VG_FEATURE_FONT && \
	   // (defined VG_FEATURE_FONT_FREETYPE_VECTOR || defined VG_FEATURE_FONT_FREETYPE_BITMAP) && \
	   // (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_VECTOR || VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_BITMAP)
// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

