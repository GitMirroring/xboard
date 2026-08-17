/*
 * Engine output (PV)
 *
 * Author: Alessandro Scotti (Dec 2005)
 *
 * Copyright 2005 Alessandro Scotti
 *
 * Enhancements Copyright 2009-2016, 2026 Free Software Foundation, Inc.
 *
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
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 * ------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

#ifdef HAVE_CONFIG_H
/* <> is used to support out-of-source autoconf builds. */
# include <config.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "dialogs.h"
#include "menus.h"
#include "engineoutput.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define _(s) gettext(s)
# define N_(s) gettext_noop(s)
#else
# define _(s) (s)
# define N_(s) s
#endif


int windowMode = 1;

/* mem1 and mem2 are dummies: this dialog can never be okayed. */
char * mem1;
char * mem2;

int highTextStart[2];
int highTextEnd[2];

int MemoProc(Option * opt, int n, int x, int y, char * text, int index);

/* clang-format off */
Option engoutOptions[] = {
 {0,   LL | T2T,             18,  NULL, NULL,          NULL, NULL,              Icon,    " ",               0},
 {0,   L2L | T2T | SAME_ROW, 162, NULL, NULL,          NULL, NULL,              Label,   N_("engine name"), 0},
 {0,   T2T | SAME_ROW,       30,  NULL, NULL,          NULL, NULL,              Icon,    " ",               0},
 /* TRANSLATORS: Here, "move" is used as a noun, e.g., "the move Nf3". */
 {0,   R2R | T2T | SAME_ROW, 188, NULL, NULL,          NULL, NULL,              Label,   N_("move"),        0},
 {0,   RR | T2T | SAME_ROW,  80,  NULL, NULL,          NULL, NULL,              Label,   N_("NPS"),         0},
 {200, T_VSCRL | T_TOP,      500, NULL, (void *)&mem1, NULL, (char **)MemoProc, TextBox, "",                &appData.historyFont},
 {0,   0,                    0,   NULL, NULL,          "",   NULL,              Break,   "",                0},
 {0,   LL | T2T,             18,  NULL, NULL,          NULL, NULL,              Icon,    " ",               0},
 {0,   L2L | T2T | SAME_ROW, 162, NULL, NULL,          NULL, NULL,              Label,   N_("engine name"), 0},
 {0,   T2T | SAME_ROW,       30,  NULL, NULL,          NULL, NULL,              Icon,    " ",               0},
 /* TRANSLATORS: Here, "move" is used as a noun, e.g., "the move Nf3". */
 {0,   R2R | T2T | SAME_ROW, 188, NULL, NULL,          NULL, NULL,              Label,   N_("move"),        0},
 {0,   RR | T2T | SAME_ROW,  80,  NULL, NULL,          NULL, NULL,              Label,   N_("NPS"),         0},
 {200, T_VSCRL | T_TOP,      500, NULL, (void *)&mem2, NULL, (char **)MemoProc, TextBox, "",                &appData.historyFont},
 {0,   NO_OK,                0,   NULL, NULL,          "",   NULL,              EndMark, "",                0}
};
/* clang-format on */

/* user callback for mouse events in memo */
int MemoProc(Option * opt, int n, int x, int y, char * text, int index) {
    /* keep track of button 3 state */
    static int pressed;
    int start, end, currentPV = (opt != &engoutOptions[5]);

    switch (n) {
    case 0:
        /* pointer motion */
        if (!pressed) {
            /* only motion with button 3 down is of interest */
            return FALSE;
        }
        MovePV(x, y, 500 /*desired_board_dimension_in_pixels(BOARD_HEIGHT, squareSize, lineGap)*/);
        break;
    case 3:
        /* press button 3 */
        pressed = 1;
        if (LoadMultiPV(x, y, text, index, &start, &end, currentPV)) {
            highTextStart[currentPV] = start;
            highTextEnd[currentPV] = end;
            HighlightText(&engoutOptions[currentPV ? 12 : 5], start, end, TRUE);
        }
        break;
    case -3:
        /* release button 3 */
        pressed = 0;
        if (highTextStart[currentPV] != highTextEnd[currentPV]) {
            HighlightText(&engoutOptions[currentPV ? 12 : 5], highTextStart[currentPV], highTextEnd[currentPV], FALSE);
        }
        highTextStart[currentPV] = highTextEnd[currentPV] = 0;
        UnLoadPV();
        break;
    default:
        /* not meant for us; do regular event handler */
        return FALSE;
    }
    return TRUE;
}

/* first call into xengineoutput.c to pick up icon pixmap */
void SetIcon(int which, int field, int nIcon) {
    if (nIcon) {
        DrawWidgetIcon(&engoutOptions[STRIDE * which + field - 1], nIcon);
    }
}

void DoSetWindowText(int which, int field, char * s_label) { SetWidgetLabel(&engoutOptions[STRIDE * which + field - 1], s_label); }

void SetEngineOutputTitle(char * title) { SetDialogTitle(EngOutDlg, title); }


void DoClearMemo(int which) { SetWidgetText(&engoutOptions[STRIDE * which + MEMO], "", -1); }

void EngineOutputPopUp(void) {
    static int needInit = TRUE;
    static char * title = N_("Engine output");

    if (GenericPopUp(engoutOptions, _(title), EngOutDlg, BoardWindow, NONMODAL, appData.topLevel)) {
        if (engoutOptions[STRIDE - 1].type != Break) {
            DisplayFatalError(_("Mismatch of STRIDE in nengineoutput.c\nChange and recompile!"), 0, 2);
        }
        AddHandler(&engoutOptions[MEMO], EngOutDlg, 6);
        AddHandler(&engoutOptions[MEMO + STRIDE], EngOutDlg, 6);
        if (needInit) {
            /* make icon bitmaps */
            InitEngineOutput(&engoutOptions[0], &engoutOptions[MEMO]);
            needInit = FALSE;
        }
        SetEngineColorIcon(0);
        SetEngineColorIcon(1);
        SetEngineState(0, STATE_IDLE, "");
        SetEngineState(1, STATE_IDLE, "");
    } else {
        SetIconName(EngOutDlg, _(title));
        SetDialogTitle(EngOutDlg, _(title));
    }

    MarkMenu("View.EngineOutput", EngOutDlg);

    /* [HGM] thinking: might need to prompt engine for thinking output */
    ShowThinkingEvent();
}

int EngineOutputIsUp(void) { return shellUp[EngOutDlg]; }

int EngineOutputDialogExists(void) { return DialogExists(EngOutDlg); }

void EngineOutputProc(void) {
    if (!PopDown(EngOutDlg)) {
        EngineOutputPopUp();
    }
}
