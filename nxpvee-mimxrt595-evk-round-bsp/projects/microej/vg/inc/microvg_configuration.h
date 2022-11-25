/*
 * C
 *
 * Copyright 2020-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
* @file
* @brief MicroEJ MicroVG library low level API: enable some features according to
* the hardware capacities.
* @author MicroEJ Developer Team
* @version 1.0.0
*/

#if !defined MICROVG_CONFIGURATION_H
#define MICROVG_CONFIGURATION_H

#if defined __cplusplus
extern "C" {
#endif

//#error "This header must be customized with platform specific configuration. Remove this #error when done. This file is not modified when a new version of the CCO is installed."

/**
 * @brief Compatibility sanity check value.
 * This define value is checked in the implementation to validate that the version of this configuration
 * is compatible with the implementation.
 *
 * This value must not be changed by the user of the CCO.
 * This value must be incremented by the implementor of the CCO when a configuration define is added, deleted or modified.
 */
#define MICROVG_CONFIGURATION_VERSION (1)

// -----------------------------------------------------------------------------
// MicroVG's LinearGradient Options
// -----------------------------------------------------------------------------

/*
 * @brief Value of "VG_FEATURE_GRADIENT" to use a full implementation of the
 * MicroVG's LinearGradient.
 *
 * This implementation holds an array of all colors and positions set by the
 * application.
 */
#define VG_FEATURE_GRADIENT_FULL (1)

/*
 * @brief Value of "VG_FEATURE_GRADIENT" to use a reduced implementation of the
 * MicroVG's LinearGradient.
 *
 * This implementation keeps only the first color set by the application when
 * inializing a gradient.
 */
#define VG_FEATURE_GRADIENT_FIRST_COLOR (2)

// -----------------------------------------------------------------------------
// MicroVG's VectorFont Options
// -----------------------------------------------------------------------------

/*
 * @brief Value of "VG_FEATURE_FONT" to use FreeTYPE as implementation of the
 * MicroVG's VectorFont.
 *
 * This implementation configures FreeTYPE to manipulate the font's glyphs as a
 * succession of vector paths. It requires a "vector" renderer.
 */
#define VG_FEATURE_FONT_FREETYPE_VECTOR (1)

/*
 * @brief Value of "VG_FEATURE_FONT" to use FreeTYPE as implementation of the
 * MicroVG' VectorFont.
 *
 * This implementation configures FreeTYPE to manipulate the font's glyphs as
 * bitmaps. No renderer is required
 */
#define VG_FEATURE_FONT_FREETYPE_BITMAP (2)


// -----------------------------------------------------------------------------
// MicroVG's Features Implementation
// -----------------------------------------------------------------------------

/*
 * @brief Uncomment this define to embed the implementation of the MicroVG's
 * Path (dynamic path creation and path rendering).
 *
 * This implementation holds an array of path's commands defined by the application.
 *
 * When not set, a stub implementation is used. No error is thrown at runtime when
 * the application uses a path: the dynamic path are not created and the path
 * rendering is not performed.
 */
#define VG_FEATURE_PATH

/*
 * @brief Set this define to specify the implementation of the MicroVG's
 * LinearGradient (dynamic gradient creation and drawings with gradient).
 *
 * @see VG_FEATURE_GRADIENT_FULL
 * @see VG_FEATURE_GRADIENT_FIRST_COLOR
 *
 * When not set, a stub implementation is used. No error is thrown at runtime when
 * the application uses a gradient: the dynamic gradient are not created and the
 * gradient rendering is not performed.
 */
#define VG_FEATURE_GRADIENT VG_FEATURE_GRADIENT_FULL

/*
 * @brief Set this define to specify the implementation of the MicroVG'
 * VectorFont (dynamic font loading and text rendering).
 *
 * @see VG_FEATURE_FONT_FREETYPE_VECTOR
 * @see VG_FEATURE_FONT_FREETYPE_BITMAP
 *
 * When not set, a stub implementation is used. The application cannot load a font
 * (and by consequence cannot draw a text).
 */
#define VG_FEATURE_FONT VG_FEATURE_FONT_FREETYPE_VECTOR

/*
 * @brief Uncomment this define to enable the support of TTF font files'
 *
 */
#define VG_FEATURE_FREETYPE_TTF

/*
 * @brief Uncomment this define to enable the support of OTF font files'
 *
 */
#define VG_FEATURE_FREETYPE_OTF

/*
 * @brief Uncomment this define to enable the support of colored emoji'
 *
 */
#define VG_FEATURE_FREETYPE_COLORED_EMOJI

/*
* @brief Uncomment this define to enable the support of complex layout.
*
* When set, the complex layout feature is disabled by default (the Freetype layout
* manager is used). See functions MICROVG_HELPER_set_complex_layout() and
* MICROVG_HELPER_has_complex_layouter().
*
* Note: the complex layout feature is managed by the Harfbuzz engine.
*       Harfbuzz is used beside Freetype, thus the FT_CONFIG_OPTION_USE_HARFBUZZ define
*       is not needed. This implementation has been chosen to ease the replacement of
*       harfbuzz by an other complex layouter.
*/
#define VG_FEATURE_FONT_COMPLEX_LAYOUT

/*
 * @brief Uncomment this define to allow to load external font files. When a font
 * file is not available in the application classpath, the implementation tries to
 * load it from an external resource system.
 *
 * The Platform must embbed the module "External Resource" and the BSP must implement
 * "LLEXT_RES_impl.h" header file.
 *
 * When not set, only the resources compiled with the application are used.
 */
#define VG_FEATURE_FONT_EXTERNAL

/*
 * @brief Configure this define to set the freetype heap size
 *
 * The freetype heap size depends on the font used by the application
 * @see MICROVG_MONITOR_HEAP in microvg_helper.h to monitor the heap usage evolution.
 */
#define VG_FEATURE_FREETYPE_HEAP_SIZE ( 160 * 1024 )

/*
 * @brief Configure this define to set the complex layouter heap size
 *
 *@see VG_FEATURE_FONT_COMPLEX_LAYOUT
 *
 * The complex layouter heap size depends on the font used by the application
 * @see MICROVG_MONITOR_HEAP in microvg_helper.h to monitor the heap usage evolution.
 */
#ifdef VG_FEATURE_FONT_COMPLEX_LAYOUT
#define VG_FEATURE_FONT_COMPLEX_LAYOUT_HEAP_SIZE ( 80 * 1024 )
#endif

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // !defined MICROVG_CONFIGURATION_H
