
/*
 * Copyright 2020-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * @file
 * @brief This file implements all "Drawing" (MicroUI extended library) drawing native functions.
 * @see LLDW_PAINTER_impl.h file comment
 * @author MicroEJ Developer Team
 * @version 2.0.0
 * @date 22 July 2022
 * @since MicroEJ UI Pack 13.0.0
 */

// --------------------------------------------------------------------------------
// Includes
// --------------------------------------------------------------------------------

// implements LLDW_PAINTER_impl functions
#include "LLDW_PAINTER_impl.h"

// calls dw_drawing functions
#include "dw_drawing.h"

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
#define LOG_DRAW_drawThickFadedPoint 100
#define LOG_DRAW_drawThickFadedLine 101
#define LOG_DRAW_drawThickFadedCircle 102
#define LOG_DRAW_drawThickFadedCircleArc 103
#define LOG_DRAW_drawThickFadedEllipse 104
#define LOG_DRAW_drawThickLine 105
#define LOG_DRAW_drawThickCircle 106
#define LOG_DRAW_drawThickEllipse 107
#define LOG_DRAW_drawThickCircleArc 108

#define LOG_DRAW_drawFlippedImage 200
#define LOG_DRAW_drawRotatedImageNearestNeighbor 201
#define LOG_DRAW_drawRotatedImageBilinear 202
#define LOG_DRAW_drawScaledImageNearestNeighbor 203
#define LOG_DRAW_drawScaledImageBilinear 204

// --------------------------------------------------------------------------------
// LLDW_PAINTER_impl.h functions
// --------------------------------------------------------------------------------

void LLDW_PAINTER_IMPL_drawThickFadedPoint(MICROUI_GraphicsContext* gc, jint x, jint y, jint thickness, jint fade) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawThickFadedPoint)) {
		LOG_DRAW_START(drawThickFadedPoint);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawThickFadedPoint(gc, x, y, thickness, fade));
		LOG_DRAW_END(drawThickFadedPoint);
	}
}

void LLDW_PAINTER_IMPL_drawThickFadedLine(MICROUI_GraphicsContext* gc, jint startX, jint startY, jint endX, jint endY, jint thickness, jint fade, DRAWING_Cap startCap, DRAWING_Cap endCap) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawThickFadedLine)) {
		LOG_DRAW_START(drawThickFadedLine);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawThickFadedLine(gc, startX, startY, endX, endY, thickness, fade, startCap, endCap));
		LOG_DRAW_END(drawThickFadedLine);
	}
}

void LLDW_PAINTER_IMPL_drawThickFadedCircle(MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter, jint thickness, jint fade) {
	if ((diameter > 0) && LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawThickFadedCircle)) {
		LOG_DRAW_START(drawThickFadedCircle);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawThickFadedCircle(gc, x, y, diameter, thickness, fade));
		LOG_DRAW_END(drawThickFadedCircle);
	}
}

void LLDW_PAINTER_IMPL_drawThickFadedCircleArc(MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter, jfloat startAngle, jfloat arcAngle, jint thickness, jint fade, DRAWING_Cap start, DRAWING_Cap end) {
	if ((diameter > 0) && (int32_t)arcAngle != 0 && LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawThickFadedCircleArc)) {
		LOG_DRAW_START(drawThickFadedCircleArc);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawThickFadedCircleArc(gc, x, y, diameter, startAngle, arcAngle, thickness, fade, start, end));
		LOG_DRAW_END(drawThickFadedCircleArc);
	}
}

void LLDW_PAINTER_IMPL_drawThickFadedEllipse(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height, jint thickness, jint fade) {
	if ((width > 0) && (height > 0) && LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawThickFadedEllipse)) {
		LOG_DRAW_START(drawThickFadedEllipse);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawThickFadedEllipse(gc, x, y, width, height, thickness, fade));
		LOG_DRAW_END(drawThickFadedEllipse);
	}
}

void LLDW_PAINTER_IMPL_drawThickLine(MICROUI_GraphicsContext* gc, jint startX, jint startY, jint endX, jint endY, jint thickness) {
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawThickLine)) {
		LOG_DRAW_START(drawThickLine);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawThickLine(gc, startX, startY, endX, endY, thickness));
		LOG_DRAW_END(drawThickLine);
	}
}

void LLDW_PAINTER_IMPL_drawThickCircle(MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter, jint thickness) {
	if ((diameter > 0) && LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawThickCircle)) {
		LOG_DRAW_START(drawThickCircle);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawThickCircle(gc, x, y, diameter, thickness));
		LOG_DRAW_END(drawThickCircle);
	}
}

void LLDW_PAINTER_IMPL_drawThickEllipse(MICROUI_GraphicsContext* gc, jint x, jint y, jint width, jint height, jint thickness) {
	if ((width > 0) && (height > 0) && LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawThickEllipse)) {
		LOG_DRAW_START(drawThickEllipse);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawThickEllipse(gc, x, y, width, height, thickness));
		LOG_DRAW_END(drawThickEllipse);
	}
}

void LLDW_PAINTER_IMPL_drawThickCircleArc(MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter, jfloat startAngle, jfloat arcAngle, jint thickness) {
	if ((diameter > 0) && ((int32_t)arcAngle != 0) && LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawThickCircleArc)) {
		LOG_DRAW_START(drawThickCircleArc);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawThickCircleArc(gc, x, y, diameter, startAngle, arcAngle, thickness));
		LOG_DRAW_END(drawThickCircleArc);
	}
}

void LLDW_PAINTER_IMPL_drawFlippedImage(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint regionX, jint regionY, jint width, jint height, jint x, jint y, DRAWING_Flip transformation, jint alpha) {
	if (!LLUI_DISPLAY_isClosed(img) && (alpha > 0) && LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawFlippedImage)) {
		LOG_DRAW_START(drawFlippedImage);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawFlippedImage(gc, img, regionX, regionY, width, height, x, y, transformation, alpha));
		LOG_DRAW_END(drawFlippedImage);
	}
}

void LLDW_PAINTER_IMPL_drawRotatedImageNearestNeighbor(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jint rotationX, jint rotationY, jfloat angle, jint alpha) {
	if (!LLUI_DISPLAY_isClosed(img) && (alpha > 0) && LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawRotatedImageNearestNeighbor)) {
		LOG_DRAW_START(drawRotatedImageNearestNeighbor);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawRotatedImageNearestNeighbor(gc, img, x, y, rotationX, rotationY, angle, alpha));
		LOG_DRAW_END(drawRotatedImageNearestNeighbor);
	}
}

void LLDW_PAINTER_IMPL_drawRotatedImageBilinear(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jint rotationX, jint rotationY, jfloat angle, jint alpha) {
	if (!LLUI_DISPLAY_isClosed(img) && (alpha > 0) && LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawRotatedImageBilinear)) {
		LOG_DRAW_START(drawRotatedImageBilinear);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawRotatedImageBilinear(gc, img, x, y, rotationX, rotationY, angle, alpha));
		LOG_DRAW_END(drawRotatedImageBilinear);
	}
}

void LLDW_PAINTER_IMPL_drawScaledImageNearestNeighbor(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jfloat factorX, jfloat factorY, jint alpha) {
	if (!LLUI_DISPLAY_isClosed(img) && (alpha > 0) && (factorX > 0.f) && (factorY > 0.f) && LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawScaledImageNearestNeighbor)) {
		LOG_DRAW_START(drawScaledImageNearestNeighbor);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawScaledImageNearestNeighbor(gc, img, x, y, factorX, factorY, alpha));
		LOG_DRAW_END(drawScaledImageNearestNeighbor);
	}
}

void LLDW_PAINTER_IMPL_drawScaledImageBilinear(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x, jint y, jfloat factorX, jfloat factorY, jint alpha) {
	if (!LLUI_DISPLAY_isClosed(img) && (alpha > 0) && (factorX > 0.f) && (factorY > 0.f) && LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&LLDW_PAINTER_IMPL_drawScaledImageBilinear)) {
		LOG_DRAW_START(drawScaledImageBilinear);
		LLUI_DISPLAY_setDrawingStatus(DW_DRAWING_drawScaledImageBilinear(gc, img, x, y, factorX, factorY, alpha));
		LOG_DRAW_END(drawScaledImageBilinear);
	}
}

// --------------------------------------------------------------------------------
// EOF
// --------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
