/*
 * draw.h -- declarations shared between xboard.c and draw.c
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 * Enhancements Copyright 1992-2016, 2026 Free Software Foundation, Inc.
 *
 * The following terms apply to Digital Equipment Corporation's copyright
 * interest in XBoard:
 * ------------------------------------------------------------------------
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * ------------------------------------------------------------------------
 *
 * The following terms apply to the enhanced version of XBoard
 * distributed by the Free Software Foundation:
 * ------------------------------------------------------------------------
 *
 * GNU XBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU XBoard is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.  *
 *
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

#define DRAWABLE(X) ((cairo_surface_t *)((X)->choice))

/* defined in xboard.c */
void CreateGrid(void);
void CreateGCs(int redo);
void DelayedDrag(void);

/* defined in board.c */
extern int lineGap;
extern int squareSize;

/* defined in draw.c */
void CreateGCs(int redo);
void CreateAnyPieces(int p);
void CreatePNGPieces(char * pieceDir);
void CreateGrid(void);
void DrawSegment(int x, int y, int * lastX, int * lastY, int p);
void DrawRectangle(int left, int top, int right, int bottom, int side, int style);
void DrawEvalText(char * buf, int cbBuf, int y);
void DrawText(char * string, int x, int y, int align);

#if API_USED_FOR_DRAWING_GUI == 2 || API_USED_FOR_DRAWING_GUI == 3
#include <cairo/cairo.h>
cairo_surface_t * CsBoardWindow(Option * opt);
#endif

extern char svgDir[];

/* defined in nevalgraph.c */
extern Option * disp;

/* defined in evaldraw.c */
float Color(char * col, int n);

/* defined in xoptions.c */
void GraphExpose(Option * opt, int x, int y, int w, int h);
