/*
 * xboard.h -- Parameter definitions for X front end
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
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL DIGITAL
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * ------------------------------------------------------------------------
 *
 * The following terms apply to the enhanced version of XBoard distributed by
 * the Free Software Foundation:
 * ------------------------------------------------------------------------
 *
 * GNU XBoard is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * GNU XBoard is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see http://www.gnu.org/licenses/.
 *
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

#include <stdio.h>

#define ICS_LOGON ".icsrc"
#define MANPAGE "xboard.6"

#if API_USED_FOR_DRAWING_GUI == 0
# define CLOCK_FONT_NAME \
     "-*-helvetica-bold-r-normal--*-*-*-*-*-*-*-*," \
     "-misc-fixed-bold-r-normal--*-*-*-*-*-*-*-*," \
     "-*-*-*-*-*-*-*-*-*-*-*-*-*-*"
# define COORD_FONT_NAME \
     "-*-helvetica-bold-r-normal--*-*-*-*-*-*-*-*," \
     "-misc-fixed-bold-r-normal--*-*-*-*-*-*-*-*," \
     "-*-*-*-*-*-*-*-*-*-*-*-*-*-*"
# define DEFAULT_FONT_NAME \
     "-*-helvetica-medium-r-normal--*-*-*-*-*-*-*-*," \
     "-misc-fixed-medium-r-normal--*-*-*-*-*-*-*-*," \
     "-*-*-*-*-*-*-*-*-*-*-*-*-*-*"
#elif API_USED_FOR_DRAWING_GUI == 2 || API_USED_FOR_DRAWING_GUI == 3
# define CLOCK_FONT_NAME "Sans Bold %d"
# define COORD_FONT_NAME "Sans Bold %d"
# define DEFAULT_FONT_NAME "Sans Normal %d"
# define CONSOLE_FONT_NAME "Monospace Normal %d"
# define HISTORY_FONT_NAME "Sans Normal %d"
# define COMMENT_FONT_NAME "Sans Normal %d"
# define TAGS_FONT_NAME "Sans Normal %d"
# define GAMELIST_FONT_NAME "Sans Normal %d"
#else
# error Unexpected API_USED_FOR_DRAWING_GUI value in xboard.h.
#endif

#define COLOR_SHOUT "green"
#define COLOR_SSHOUT "green,black,1"
#define COLOR_CHANNEL1 "cyan"
#define COLOR_CHANNEL "cyan,black,1"
#define COLOR_KIBITZ "magenta,black,1"
#define COLOR_TELL "yellow,black,1"
#define COLOR_CHALLENGE "red,black,1"
#define COLOR_REQUEST "red"
#define COLOR_SEEK "blue"
#define COLOR_NORMAL "default"
#define COLOR_LOWTIMEWARNING "red"

typedef struct {
    char * name;
    int squareSize;
    int lineGap;
    int clockFontPxlSize;
    int coordFontPxlSize;
    int fontPxlSize;
    int smallLayout;
    int tinyLayout;
    int minScreenSize;
} SizeDefaults;


/* These macro definitions are not identical: the tinyLayout column has differences.

   TODO: Perhaps they should be identical, though? */
#if API_USED_FOR_DRAWING_GUI == 0
#define SIZE_DEFAULTS { \
     {"Titanic",  129, 4, 34, 14, 14, 0, 0, 1200}, \
     {"Colossal", 116, 4, 34, 14, 14, 0, 0, 1200}, \
     {"Giant",    108, 3, 34, 14, 14, 0, 0, 1024}, \
     {"Huge",     95,  3, 34, 14, 14, 0, 0, 1024}, \
     {"Big",      87,  3, 34, 14, 14, 0, 0, 864 }, \
     {"Large",    80,  3, 34, 14, 14, 0, 0, 864 }, \
     {"Bulky",    72,  3, 34, 12, 14, 0, 0, 864 }, \
     {"Medium",   64,  3, 34, 12, 14, 1, 0, 768 }, \
     {"Moderate", 58,  3, 34, 12, 14, 1, 0, 768 }, \
     {"Average",  54,  2, 30, 11, 12, 1, 0, 600 }, \
     {"Middling", 49,  2, 24, 10, 12, 1, 0, 600 }, \
     {"Mediocre", 45,  2, 20, 10, 12, 1, 0, 600 }, \
     {"Small",    40,  2, 20, 10, 12, 1, 0, 480 }, \
     {"Slim",     37,  2, 20, 10, 12, 1, 0, 480 }, \
     {"Petite",   33,  1, 15, 9,  11, 1, 0, 480 }, \
     {"Dinky",    29,  1, 15, 9,  11, 1, 0, 480 }, \
     {"Teeny",    25,  1, 12, 8,  11, 1, 1, 480 }, \
     {"Tiny",     21,  1, 12, 8,  11, 1, 1, 0   }, \
     {NULL,       0,   0, 0,  0,  0,  0, 0, 0   } \
}
#elif API_USED_FOR_DRAWING_GUI == 2 || API_USED_FOR_DRAWING_GUI == 3
#define SIZE_DEFAULTS { \
     {"Titanic",  129, 4, 34, 14, 14, 0, 0, 1200}, \
     {"Colossal", 116, 4, 34, 14, 14, 0, 0, 1200}, \
     {"Giant",    108, 3, 34, 14, 14, 0, 0, 1024}, \
     {"Huge",     95,  3, 34, 14, 14, 0, 0, 1024}, \
     {"Big",      87,  3, 34, 14, 14, 0, 0, 864 }, \
     {"Large",    80,  3, 34, 14, 14, 0, 0, 864 }, \
     {"Bulky",    72,  3, 34, 12, 14, 0, 0, 864 }, \
     {"Medium",   64,  3, 34, 12, 14, 1, 0, 768 }, \
     {"Moderate", 58,  3, 34, 12, 14, 1, 0, 768 }, \
     {"Average",  54,  2, 30, 11, 12, 1, 0, 600 }, \
     {"Middling", 49,  2, 24, 10, 12, 1, 0, 600 }, \
     {"Mediocre", 45,  2, 20, 10, 12, 1, 4, 600 }, \
     {"Small",    40,  2, 20, 10, 12, 1, 3, 480 }, \
     {"Slim",     37,  2, 20, 10, 12, 1, 3, 480 }, \
     {"Petite",   33,  1, 15, 9,  11, 1, 2, 480 }, \
     {"Dinky",    29,  1, 15, 9,  11, 1, 1, 480 }, \
     {"Teeny",    25,  1, 12, 8,  11, 1, 1, 480 }, \
     {"Tiny",     21,  1, 12, 8,  11, 1, 1, 0   }, \
     {NULL,       0,   0, 0,  0,  0,  0, 0, 0   } \
}
#else
# error Unexpected API_USED_FOR_DRAWING_GUI value in xboard.h.
#endif

#define BORDER_X_OFFSET 3
#define BORDER_Y_OFFSET 27
#define FIRST_CHESS_PROGRAM "fairymax"
#define SECOND_CHESS_PROGRAM ""
#define FIRST_DIRECTORY "."
#define SECOND_DIRECTORY "."
#define SOUND_BELL ""
#define ICS_NAMES ""
#define FCP_NAMES ""
#define SCP_NAMES ""
#define ICS_TEXT_MENU_DEFAULT ""
#define SETTINGS_FILE XBOARD_SYSTEM_CONFIG_DIR "/xboard.conf"
#define COLOR_BKGD "white"

void NewTagsPopup(char * text, char * msg);
int AppendText(Option * opt, char * s);
void NewCommentPopup(char * title, char * text, int index);

#if API_USED_FOR_DRAWING_GUI == 0
void CatchDeleteWindow(Widget w, String procname);
void GenericPopDown(Widget w, XEvent * event, String * prms, Cardinal * nprms);
#elif API_USED_FOR_DRAWING_GUI == 2 || API_USED_FOR_DRAWING_GUI == 3
# ifdef TODO_GTK
void CatchDeleteWindow(Widget w, String procname);
void GenericPopDown(Widget w, XEvent * event, String * prms, Cardinal * nprms);
# endif
#else
# error Unexpected API_USED_FOR_DRAWING_GUI value in xboard.h.
#endif

#if API_USED_FOR_DRAWING_GUI == 0
void InitDrawingSizes(int i, int j);
void SendToICS(char * buf);
void SendToProgram(char * message, ChessProgramState * cps);
#elif API_USED_FOR_DRAWING_GUI == 2 || API_USED_FOR_DRAWING_GUI == 3
#else
# error Unexpected API_USED_FOR_DRAWING_GUI value in xboard.h.
#endif

// from xoptions.c
#if API_USED_FOR_DRAWING_GUI == 0
void SetFocus(Widget w, XtPointer data, XEvent * event, Boolean * b);
void TypeInProc(Widget w, XEvent * event, String * prms, Cardinal * nprms);
Widget CreateMenuItem(Widget menu, char * msg, XtCallbackProc CB, int n);
void WheelProc(Widget w, XEvent * event, String * prms, Cardinal * nprms);
void TabProc(Widget w, XEvent * event, String * prms, Cardinal * nprms);
#elif API_USED_FOR_DRAWING_GUI == 2 || API_USED_FOR_DRAWING_GUI == 3
# ifdef TODO_GTK
void SetFocus(Widget w, XtPointer data, XEvent * event, Boolean * b);
void TypeInProc(Widget w, XEvent * event, String * prms, Cardinal * nprms);
Widget CreateMenuItem(Widget menu, char * msg, XtCallbackProc CB, int n);
void WheelProc(Widget w, XEvent * event, String * prms, Cardinal * nprms);
void TabProc(Widget w, XEvent * event, String * prms, Cardinal * nprms);
# endif
#else
# error Unexpected API_USED_FOR_DRAWING_GUI value in xboard.h.
#endif

// from xengineoutput.c
#if API_USED_FOR_DRAWING_GUI == 0
void SelectPV(Widget w, XEvent * event, String * params, Cardinal * nParams);
void StopPV(Widget w, XEvent * event, String * params, Cardinal * nParams);
#elif API_USED_FOR_DRAWING_GUI == 2 || API_USED_FOR_DRAWING_GUI == 3
# ifdef TODO_GTK
void SelectPV(Widget w, XEvent * event, String * params, Cardinal * nParams);
void StopPV(Widget w, XEvent * event, String * params, Cardinal * nParams);
# endif
#else
# error Unexpected API_USED_FOR_DRAWING_GUI value in xboard.h.
#endif

#if API_USED_FOR_DRAWING_GUI == 0
#elif API_USED_FOR_DRAWING_GUI == 2 || API_USED_FOR_DRAWING_GUI == 3
GdkPixbuf * LoadIconFile(char * name);
void GetActualPlacement(GtkWidget * shell, WindowPlacement * wp);
void DelayedDrag(void);
#else
# error Unexpected API_USED_FOR_DRAWING_GUI value in xboard.h.
#endif

extern char memoTranslations[];

#if API_USED_FOR_DRAWING_GUI == 0
extern Widget shells[];
extern Widget formWidget;
extern Widget shellWidget;
extern Widget boardWidget;
extern Widget menuBarWidget;
extern Display * xDisplay;
extern Window xBoardWindow;
extern Pixmap xMarkPixmap;
extern Pixmap wIconPixmap;
extern Pixmap bIconPixmap;
extern Pixel timerForegroundPixel;
extern Pixel timerBackgroundPixel;
extern Pixel dialogColor;
extern Pixel buttonColor;
extern Atom wm_delete_window;
extern GC coordGC;
#elif API_USED_FOR_DRAWING_GUI == 2 || API_USED_FOR_DRAWING_GUI == 3
# if TODO_GTK
extern Widget shells[];
extern Widget formWidget;
extern Widget shellWidget;
extern Widget boardWidget;
extern Widget menuBarWidget;
extern Display * xDisplay;
extern Window xBoardWindow;
extern Pixmap xMarkPixmap;
extern Pixmap wIconPixmap;
extern Pixmap bIconPixmap;
extern Pixel timerForegroundPixel;
extern Pixel timerBackgroundPixel;
extern Pixel dialogColor;
extern Pixel buttonColor;
extern Atom wm_delete_window;
extern GC coordGC;
# else
extern GtkWidget * shells[];
# endif
#else
# error Unexpected API_USED_FOR_DRAWING_GUI value in xboard.h.
#endif

/* textHeight is of the message widget within the board window. */
#if API_USED_FOR_DRAWING_GUI == 0
extern Dimension textHeight;
#elif API_USED_FOR_DRAWING_GUI == 2 || API_USED_FOR_DRAWING_GUI == 3
# if TODO_GTK
extern Dimension textHeight;
# endif
#else
# error Unexpected API_USED_FOR_DRAWING_GUI value in xboard.h.
#endif

extern int dialogError;
extern int squareSize;
extern char * layoutName;
extern char * selected_fen_position;
extern char ICSInputTranslations[];
extern int useImages;
extern int useImageSqs;

/* preference item; 1 = make popup windows toplevel */
#define TOPLEVEL 1

/* These last ones seem to be unique to either the Xaw or GTK version. */
#if API_USED_FOR_DRAWING_GUI == 0
extern char * icsTextMenuString;
extern int defaultLineGap;
extern int lineGap;
extern int savedIndex;
extern int searchTime;
extern int startedFromPositionFile;
#elif API_USED_FOR_DRAWING_GUI == 2 || API_USED_FOR_DRAWING_GUI == 3
extern char * chessDir;
extern GtkAccelGroup * GtkAccelerators;
#else
# error Unexpected API_USED_FOR_DRAWING_GUI value in xboard.h.
#endif
