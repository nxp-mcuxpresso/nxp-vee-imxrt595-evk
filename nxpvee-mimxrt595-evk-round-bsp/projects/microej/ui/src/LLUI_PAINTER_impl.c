
/*
 * Copyright 2020-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * @file
 * @brief This file implements all MicroUI drawing native functions.
 * @see LLUI_PAINTER_impl.h file comment
 * @author MicroEJ Developer Team
 * @version 2.0.0
 * @date 22 July 2022
 * @since MicroEJ UI Pack 13.0.0
 */

// --------------------------------------------------------------------------------
// Includes
// --------------------------------------------------------------------------------

// implements LLUI_PAINTER_impl functions
#include "LLUI_PAINTER_impl.h"

// calls ui_drawing functions
#include "ui_drawing.h"

// use graphical engine functions to synchronize drawings
#include "LLUI_DISPLAY.h"

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------------
// Macros and Defines
// --------------------------------------------------------------------------------

// macros to log a drawing
#define LOG_DRAW_START(fn) LLUI_DISPLAY_logDrawingStart(CONCAT_DEFINES(LOG_DRAW_, fn))
#define LOG_DRAW_END(fn) LLUI_DISPLAY_logDrawingEnd(CONCAT_DEFINES(LOG_DRAW_, fn))

/*
 * LOG_DRAW_EVENT logs identifiers
 */
#define LOG_DRAW_writePixel 1
#define LOG_DRAW_drawLine 2
#define LOG_DRAW_drawHorizontalLine 3
#define LOG_DRAW_drawVerticalLine 4
#define LOG_DRAW_drawRectangle 5
#define LOG_DRAW_fillRectangle 6
#define LOG_DRAW_drawRoundedRectangle 8
#define LOG_DRAW_fillRoundedRectangle 9
#define LOG_DRAW_drawCircleArc 10
#define LOG_DRAW_fillCircleArc 11
#define LOG_DRAW_drawEllipseArc 12
#define LOG_DRAW_fillEllipseArc 13
#define LOG_DRAW_drawEllipse 14
#define LOG_DRAW_fillEllipse 15
#define LOG_DRAW_drawCircle 16
#define LOG_DRAW_fillCircle 17
#define LOG_DRAW_drawARGB 18
#define LOG_DRAW_drawImage 19

// --------------------------------------------------------------------------------
// Private functions
// --------------------------------------------------------------------------------

/*
 * Checks given bound to fit in bound limits: 0 and max (excluded). Updates size and
 * origin in consequence
 */
static inline void _check_bound(jint max, jint* bound, jint* size, jint* origin) {
	if (*bound < 0) {
		*size += *bound; // decrease size
		*origin -= *bound; // increase origin
		*bound = 0;
	}

	if ((*bound + *size) > max) {
		*size = max - *bound; // decrease size
	}
}

// --------------------------------------------------------------------------------
// LLUI_PAINTER_impl.h functions
// --------------------------------------------------------------------------------

void LLUI_PAINTER_IMPL_writePixel(MICROUI_GraphicsContext* gc, jint x, jint y) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_writePixel)) {
		DRAWING_Status status;
		LOG_DRAW_START(writePixel);
		if (LLUI_DISPLAY_isPixelInClip(gc, x, y)) {
			LLUI_DISPLAY_configureClip(gc, false/* point is in clip */);
			status = UI_DRAWING_writePixel(gc, x, y);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(writePixel);
	}
	// else: pixel out of clip: nothing to do (requestDrawing() has not been called)
}

void LLUI_PAINTER_IMPL_drawLine(MICROUI_GraphicsContext* gc, jint startX, jint startY, jint endX, jint endY) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_drawLine)) {
		LOG_DRAW_START(drawLine);
		// cannot reduce/clip line: may be endX < startX and / or endY < startY
		LLUI_DISPLAY_setDrawingStatus(UI_DRAWING_drawLine(gc, startX, startY, endX, endY));
		LOG_DRAW_END(drawLine);
	}
	// else: refused drawing
}

void LLUI_PAINTER_IMPL_drawHorizontalLine(MICROUI_GraphicsContext* gc, jint x, jint y, jint length) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_drawHorizontalLine)) {
		DRAWING_Status status;
		LOG_DRAW_START(drawHorizontalLine);

		jint x1 = x;
		jint x2 = x + length - 1;

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if ((length > 0) && LLUI_DISPLAY_clipHorizontalLine(gc, &x1, &x2, y)) {
			LLUI_DISPLAY_configureClip(gc, false /* line has been clipped */);
			status = UI_DRAWING_drawHorizontalLine(gc, x1, x2, y);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(drawHorizontalLine);
	}
	// else: line out of clip: nothing to do (requestDrawing() has not been called)
}

void LLUI_PAINTER_IMPL_drawVerticalLine(MICROUI_GraphicsContext* gc, jint x, jint y, jint length) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_drawVerticalLine)) {
		DRAWING_Status status;
		LOG_DRAW_START(drawVerticalLine);

		jint y1 = y;
		jint y2 = y + length - 1;

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if ((length > 0) && LLUI_DISPLAY_clipVerticalLine(gc, &y1, &y2, x)) {
			LLUI_DISPLAY_configureClip(gc, false /* line has been clipped */);
			status = UI_DRAWING_drawVerticalLine(gc, x, y1, y2);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(drawVerticalLine);
	}
	// else: line out of clip: nothing to do (requestDrawing() has not been called)
}

void LLUI_PAINTER_IMPL_drawRectangle(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_drawRectangle)) {
		DRAWING_Status status;
		LOG_DRAW_START(drawRectangle);

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if ((width > 0) && (height > 0)) {
			jint x1 = x;
			jint x2 = x + width - 1;
			jint y1 = y;
			jint y2 = y + height - 1;

			// cannot reduce rectangle; can only check if it is fully in clip
			LLUI_DISPLAY_configureClip(gc, !LLUI_DISPLAY_isRectangleInClip(gc, x1, y1, x2, y2));
			status = UI_DRAWING_drawRectangle(gc, x1, y1, x2, y2);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(drawRectangle);
	}
	// else: refused drawing
}

void LLUI_PAINTER_IMPL_fillRectangle(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_fillRectangle)) {
		DRAWING_Status status;
		LOG_DRAW_START(fillRectangle);

		jint x1 = x;
		jint x2 = x + width - 1;
		jint y1 = y;
		jint y2 = y + height - 1;

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if ((width > 0) && (height) > 0 && LLUI_DISPLAY_clipRectangle(gc, &x1, &y1, &x2, &y2)) {
			LLUI_DISPLAY_configureClip(gc, false /* rectangle has been clipped */);
			status = UI_DRAWING_fillRectangle(gc, x1, y1, x2, y2);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(fillRectangle);
	}
	// else: rectangle out of clip: nothing to do (requestDrawing() has not been called)
}

void LLUI_PAINTER_IMPL_drawRoundedRectangle(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height, jint cornerEllipseWidth, jint cornerEllipseHeight) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_drawRoundedRectangle)) {
		DRAWING_Status status;
		LOG_DRAW_START(drawRoundedRectangle);

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if ((width > 0) && (height > 0)) {
			// cannot reduce rectangle; can only check if it is fully in clip
			LLUI_DISPLAY_configureClip(gc, !LLUI_DISPLAY_isRegionInClip(gc, x, y, width, height));
			status = UI_DRAWING_drawRoundedRectangle(gc, x, y, width, height, cornerEllipseWidth, cornerEllipseHeight);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(drawRoundedRectangle);
	}
	// else: refused drawing
}

void LLUI_PAINTER_IMPL_fillRoundedRectangle(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height, jint cornerEllipseWidth, jint cornerEllipseHeight) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_fillRoundedRectangle)) {
		DRAWING_Status status;
		LOG_DRAW_START(fillRoundedRectangle);

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if ((width > 0) && (height > 0)) {
			// cannot reduce rectangle; can only check if it is fully in clip
			LLUI_DISPLAY_configureClip(gc, !LLUI_DISPLAY_isRegionInClip(gc, x, y, width, height));
			status = UI_DRAWING_fillRoundedRectangle(gc, x, y, width, height, cornerEllipseWidth, cornerEllipseHeight);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(fillRoundedRectangle);
	}
	// else: refused drawing
}

void LLUI_PAINTER_IMPL_drawCircleArc(MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter, jfloat startAngle, jfloat arcAngle) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_drawCircleArc)) {
		DRAWING_Status status;
		LOG_DRAW_START(drawCircleArc);

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if ((diameter > 0) && ((int32_t)arcAngle != 0)) {
			// cannot reduce rectangle; can only check if it is fully in clip
			LLUI_DISPLAY_configureClip(gc, !LLUI_DISPLAY_isRegionInClip(gc, x, y, diameter, diameter));
			status = UI_DRAWING_drawCircleArc(gc, x, y, diameter, startAngle, arcAngle);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(drawCircleArc);
	}
	// else: refused drawing
}

void LLUI_PAINTER_IMPL_drawEllipseArc(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height, jfloat startAngle, jfloat arcAngle) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_drawEllipseArc)) {
		DRAWING_Status status;
		LOG_DRAW_START(drawEllipseArc);

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if ((width > 0) && (height > 0) && ((int32_t)arcAngle != 0)) {
			// cannot reduce rectangle; can only check if it is fully in clip
			LLUI_DISPLAY_configureClip(gc, !LLUI_DISPLAY_isRegionInClip(gc, x, y, width, height));
			status = UI_DRAWING_drawEllipseArc(gc, x, y, width, height, startAngle, arcAngle);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(drawEllipseArc);
	}
	// else: refused drawing
}

void LLUI_PAINTER_IMPL_fillCircleArc(MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter, jfloat startAngle, jfloat arcAngle) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_fillCircleArc)) {
		DRAWING_Status status;
		LOG_DRAW_START(fillCircleArc);

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if ((diameter > 0) && ((int32_t)arcAngle != 0)) {
			// cannot reduce rectangle; can only check if it is fully in clip
			LLUI_DISPLAY_configureClip(gc, !LLUI_DISPLAY_isRegionInClip(gc, x, y, diameter, diameter));
			status = UI_DRAWING_fillCircleArc(gc, x, y, diameter, startAngle, arcAngle);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(fillCircleArc);
	}
	// else: refused drawing
}

void LLUI_PAINTER_IMPL_fillEllipseArc(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height, jfloat startAngle, jfloat arcAngle) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_fillEllipseArc)) {
		DRAWING_Status status;
		LOG_DRAW_START(fillEllipseArc);

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if ((width > 0) && (height > 0) && ((int32_t)arcAngle != 0)) {
			// cannot reduce rectangle; can only check if it is fully in clip
			LLUI_DISPLAY_configureClip(gc, !LLUI_DISPLAY_isRegionInClip(gc, x, y, width, height));
			status = UI_DRAWING_fillEllipseArc(gc, x, y, width, height, startAngle, arcAngle);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(fillEllipseArc);
	}
	// else: refused drawing
}

void LLUI_PAINTER_IMPL_drawEllipse(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_drawEllipse)) {
		DRAWING_Status status;
		LOG_DRAW_START(drawEllipse);

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if ((width > 0) && (height > 0)) {
			// cannot reduce rectangle; can only check if it is fully in clip
			LLUI_DISPLAY_configureClip(gc, !LLUI_DISPLAY_isRegionInClip(gc, x, y, width, height));
			status = UI_DRAWING_drawEllipse(gc, x, y, width, height);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(drawEllipse);
	}
	// else: refused drawing
}

void LLUI_PAINTER_IMPL_fillEllipse(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_fillEllipse)) {
		DRAWING_Status status;
		LOG_DRAW_START(fillEllipse);

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if ((width > 0) && (height > 0)) {
			// cannot reduce rectangle; can only check if it is fully in clip
			LLUI_DISPLAY_configureClip(gc, !LLUI_DISPLAY_isRegionInClip(gc, x, y, width, height));
			status = UI_DRAWING_fillEllipse(gc, x, y, width, height);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(fillEllipse);
	}
	// else: refused drawing
}

void LLUI_PAINTER_IMPL_drawCircle(MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_drawCircle)) {
		DRAWING_Status status;
		LOG_DRAW_START(drawCircle);

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if (diameter > 0) {
			// cannot reduce rectangle; can only check if it is fully in clip
			LLUI_DISPLAY_configureClip(gc, !LLUI_DISPLAY_isRegionInClip(gc, x, y, diameter, diameter));
			status = UI_DRAWING_drawCircle(gc, x, y, diameter);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(drawCircle);
	}
	// else: refused drawing
}

void LLUI_PAINTER_IMPL_fillCircle(MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_fillCircle)) {
		DRAWING_Status status;
		LOG_DRAW_START(fillCircle);

		// tests on size and clip are performed after suspend to prevent to perform it several times
		if (diameter > 0) {
			// cannot reduce rectangle; can only check if it is fully in clip
			LLUI_DISPLAY_configureClip(gc, !LLUI_DISPLAY_isRegionInClip(gc, x, y, diameter, diameter));
			status = UI_DRAWING_fillCircle(gc, x, y, diameter);
		}
		else {
			// requestDrawing() has been called and accepted: notify the end of empty drawing
			status = DRAWING_DONE;
		}
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(fillCircle);
	}
	// else: refused drawing
}

void LLUI_PAINTER_IMPL_drawImage(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint regionX, jint regionY, jint width, jint height, jint x, jint y, jint alpha) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLUI_PAINTER_IMPL_drawImage)) {
		DRAWING_Status status = DRAWING_DONE;
		LOG_DRAW_START(drawImage);

		// tests on parameters and clip are performed after suspend to prevent to perform it several times
		if (!LLUI_DISPLAY_isClosed(img) && (alpha > 0)) {

			// sanity check on opacity
			jint l_alpha = (alpha > 255) ? 255 : alpha;

			// compute inside image bounds
			_check_bound(img->width, &regionX, &width, &x);
			_check_bound(img->height, &regionY, &height, &y);

			// compute inside graphics context bounds
			_check_bound(gc->image.width, &x, &width, &regionX);
			_check_bound(gc->image.height, &y, &height, &regionY);

			if ((width > 0) && (height > 0) && LLUI_DISPLAY_clipRegion(gc, &regionX, &regionY, &width, &height, &x, &y)) {

				LLUI_DISPLAY_configureClip(gc, false /* region has been clipped */);

				if (gc->image.format == img->format) {
					// source & destination have got the same pixels memory representation

					if (0xff /* fully opaque */ == l_alpha && !LLUI_DISPLAY_isTransparent(img)) {
						// copy source on destination without applying an opacity (beware about the overlapping)
						status = UI_DRAWING_copyImage(gc, img, regionX, regionY, width, height, x, y);
					}
					else if (img == &gc->image){
						// blend source on itself applying an opacity (beware about the overlapping)
						status = UI_DRAWING_drawRegion(gc, regionX, regionY, width, height, x, y, l_alpha);
					}
					else {
						// blend source on destination applying an opacity
						status = UI_DRAWING_drawImage(gc, img, regionX, regionY, width, height, x, y, l_alpha);
					}
				}
				else {
					// draw source on destination applying an opacity
					status = UI_DRAWING_drawImage(gc, img, regionX, regionY, width, height, x, y, l_alpha);
				}
			}
			// else: nothing to do
		}
		// else: nothing to do

		// requestDrawing() has been called and accepted: notify the end of empty drawing
		LLUI_DISPLAY_setDrawingStatus(status);
		LOG_DRAW_END(drawImage);
	}
	// else: refused drawing
}

// --------------------------------------------------------------------------------
// EOF
// --------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
