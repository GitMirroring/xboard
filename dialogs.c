/*
 * dialogs.c -- platform-independent code for dialogs of XBoard
 *
 * Copyright 2000, 2009-2016, 2026 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
/* <> is used to support out-of-source autoconf builds. */
# include <config.h>
#endif

/* [HGM] this file is the counterpart of woptions.c, containing xboard popup menus similar to those of WinBoard, to set the most
   common options interactively. */

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
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
#include "xboard2.h"
#include "menus.h"
#include "dialogs.h"
#include "gettext.h"

#ifdef ENABLE_NLS
# define _(s) gettext(s)
# define N_(s) gettext_noop(s)
#else
# define _(s) (s)
# define N_(s) s
#endif


int initialSquareSize;
int values[MAX_OPTIONS];
ChessProgramState * currentCps;
char manDir[MSG_SIZ] = XBOARD_MAN_DIR;

/*----------------------------Generic dialog -------------------------------------------- */

/* cloned from Engine Settings dialog (and later merged with it) */

char * marked[NrOfDialogs];
Boolean shellUp[NrOfDialogs];

void MarkMenu(char * item, int dlgNr) { MarkMenuItem(marked[dlgNr] = item, TRUE); }

void AddLine(Option * opt, char * s) {
    AppendText(opt, s);
    AppendText(opt, "\n");
}

/*---------------------------------------------- Update dialog controls ------------------------------------ */

int SetCurrentComboSelection(Option * opt) {
    int j;
    if (currentCps)
        ;
    else if (!opt->textValue) {
        /* numeric */
        opt->value = *(int *)opt->target;
    } else {
        /* look up actual value in list of possible values, to get selection nr */
        for (j = 0; opt->choice[j]; j++) {
            if (*(char **)opt->target && !strcmp(*(char **)opt->target, ((char **)opt->textValue)[j])) {
                break;
            }
        }
        opt->value = j + (opt->choice[j] == NULL);
    }
    SetComboChoice(opt, opt->value);
    return opt->value;
}

void GenericUpdate(Option * opts, int selected) {
    int i;
    char buf[MSG_SIZ];

    for (i = 0;; i++) {
        if (selected >= 0) {
            if (i < selected) {
                continue;
            } else if (i > selected) {
                break;
            }
        }
        switch (opts[i].type) {
        case TextBox:
        case FileName:
        case PathName:
            SetWidgetText(&opts[i], *(char **)opts[i].target, -1);
            break;
        case Spin:
            sprintf(buf, "%d", *(int *)opts[i].target);
            SetWidgetText(&opts[i], buf, -1);
            break;
        case Fractional:
            sprintf(buf, "%4.2f", *(float *)opts[i].target);
            SetWidgetText(&opts[i], buf, -1);
            break;
        case CheckBox:
            SetWidgetState(&opts[i], *(Boolean *)opts[i].target);
            break;
        case ComboBox:
            if (opts[i].min & COMBO_CALLBACK) {
                break;
            }
            SetCurrentComboSelection(opts + i);
            /* TODO: actually display this (but it is never used that way...) */
            break;
        case EndMark:
            return;
        default:
            printf("GenericUpdate: unexpected case in switch.\n");
        case ListBox:
        case Button:
        case SaveButton:
        case Label:
        case Break:
            break;
        }
    }
}

/*------------------------------------------- Read out dialog controls ------------------------------------ */

int GenericReadout(Option * opts, int selected) {
    int i, j, res = 1;
    char * val;
    char buf[MSG_SIZ], **dest;
    float x;
    /* send all options that had to be OK-ed to engine */
    for (i = 0;; i++) {
        if (selected >= 0) {
            if (i < selected) {
                continue;
            } else if (i > selected) {
                break;
            }
        }
        switch (opts[i].type) {
        case TextBox:
        case FileName:
        case PathName:
            GetWidgetText(&opts[i], &val);
            dest = currentCps ? &(opts[i].textValue) : (char **)opts[i].target;
            if (*dest == NULL || strcmp(*dest, val)) {
                if (currentCps) {
                    snprintf(buf, MSG_SIZ, "option %s=%s\n", opts[i].name, val);
                    SendToProgram(buf, currentCps);
                } else {
                    if (*dest) {
                        free(*dest);
                    }
                    *dest = malloc(strlen(val) + 1);
                }
                /* copy text there */
                safeStrCpy(*dest, val, MSG_SIZ - (*dest - opts[i].name));
            }
            break;
        case Spin:
        case Fractional:
            GetWidgetText(&opts[i], &val);
            /* Initialise, because sscanf() will fail if non-numeric text is entered. */
            x = 0.0;
            sscanf(val, "%f", &x);
            if (x > opts[i].max) {
                x = opts[i].max;
            }
            if (x < opts[i].min) {
                x = opts[i].min;
            }
            if (opts[i].type == Fractional) {
                /* engines never have float options! */
                *(float *)opts[i].target = x;
            } else {
                if (currentCps) {
                    /* Only send the option to the engine if its value changed. */
                    if (opts[i].value != x) {
                        snprintf(buf, MSG_SIZ, "option %s=%.0f\n", opts[i].name, x);
                        SendToProgram(buf, currentCps);
                    }
                } else {
                    *(int *)opts[i].target = x;
                }
                opts[i].value = x;
            }
            break;
        case CheckBox:
            j = 0;
            GetWidgetState(&opts[i], &j);
            if (opts[i].value != j) {
                opts[i].value = j;
                if (currentCps) {
                    snprintf(buf, MSG_SIZ, "option %s=%d\n", opts[i].name, j);
                    SendToProgram(buf, currentCps);
                } else {
                    *(Boolean *)opts[i].target = j;
                }
            }
            break;
        case ComboBox:
            if (opts[i].min & COMBO_CALLBACK) {
                break;
            }
            if (!opts[i].textValue) {
                /* numeric */
                *(int *)opts[i].target = values[i];
                break;
            }
            val = ((char **)opts[i].textValue)[values[i]];
            if (currentCps) {
                if (opts[i].value == values[i]) {
                    break;
                }
                opts[i].value = values[i];
                snprintf(buf, MSG_SIZ, "option %s=%s\n", opts[i].name, opts[i].choice[values[i]]);
                SendToProgram(buf, currentCps);
            } else if (val && (*(char **)opts[i].target == NULL || strcmp(*(char **)opts[i].target, val))) {
                if (*(char **)opts[i].target) {
                    free(*(char **)opts[i].target);
                }
                *(char **)opts[i].target = strdup(val);
            }
            break;
        case EndMark:
            if (opts[i].target && selected != -2) {
                /* callback for implementing necessary actions on OK (like redraw) */
                res = ((OKCallback *)opts[i].target)(i);
            }
            break;
        default:
            printf("GenericReadout: unexpected case in switch.\n");
        case ListBox:
        case Button:
        case SaveButton:
        case Label:
        case Break:
        case Skip:
            break;
        }
        if (opts[i].type == EndMark) {
            break;
        }
    }
    return res;
}

/*------------------------------------------- Match Options ------------------------------------------------------ */

char *engineName, *engineChoice, *tfName;
char *engineList[MAXENGINES] = {" "}, *engineMnemonic[MAXENGINES];

static void AddToTourney(int n, int sel);
static void CloneTourney(void);
static void ReplaceParticipant(void);
static void UpgradeParticipant(void);
static void PseudoOK(void);

static int MatchOK(int n) {
    free_then_strdup(&appData.participants, engineName);
    if (!CreateTourney(tfName) || matchMode) {
        return matchMode || !appData.participants[0];
    }
    /* Early popdown to prevent FreezeUI called through MatchEvent from causing XtGrab warning. */
    PopDown(MasterDlg);
    /* Start tourney. */
    MatchEvent(2);
    /* Avoid a double popdown. */
    return FALSE;
}

static void DoTimeControl(int n) { TimeControlProc(); }

static void DoCommonEngine(int n) { UciMenuProc(); }

static void DoGeneral(int n) { OptionsProc(); }

#define PARTICIPANTS 6 /* This MUST be the number of the Option for &engineName!*/

static Option matchOptions[] = {
 {0,   0,                         0,          NULL, (void *)&tfName,                    ".trn",                NULL, FileName, N_("Tournament file:          ")                              },
 {0,   0,                         0,          NULL, NULL,                               NULL,                  NULL, Label,    N_("For concurrent playing of tourney with multiple XBoards:")},
 {0,   0,                         0,          NULL, (void *)&appData.roundSync,         "",                    NULL, CheckBox, N_("Sync after round")                                        },
 {0,   0,                         0,          NULL, (void *)&appData.cycleSync,         "",                    NULL, CheckBox, N_("Sync after cycle")                                        },
 {0,   LR,                        175,        NULL, NULL,                               NULL,                  NULL, Label,    N_("Tourney participants:")                                   },
 {0,   SAME_ROW | RR,             175,        NULL, NULL,                               NULL,                  NULL, Label,    N_("Select Engine:")                                          },
 {200, T_VSCRL | T_FILL | T_WRAP, 175,        NULL, (void *)&engineName,                NULL,                  NULL, TextBox,  ""                                                            },
 {200, SAME_ROW | RR,             175,        NULL, (void *)engineMnemonic,             (char *)&AddToTourney, NULL, ListBox,  ""                                                            },
 /* to decouple alignment above and below boxes */
 {0,   SAME_ROW,                  0,          NULL, NULL,                               NULL,                  NULL, Break,    ""                                                            },
 /* { 0,  COMBO_CALLBACK | NO_GETTEXT, 0, NULL, (void*) &AddToTourney, (char*)(engineMnemonic+1), (engineMnemonic+1), ComboBox, N_("Select Engine:") }, */
 {0,   0,                         10,         NULL, (void *)&appData.tourneyType,       "",                    NULL, Spin,     N_("Tourney type (0 = round-robin, 1 = gauntlet):")           },
 {0,   1,                         1000000000, NULL, (void *)&appData.tourneyCycles,     "",                    NULL, Spin,     N_("Number of tourney cycles (or Swiss rounds):")             },
 {0,   1,                         1000000000, NULL, (void *)&appData.defaultMatchGames, "",                    NULL, Spin,     N_("Default Number of Games in Match (or Pairing):")          },
 {0,   0,                         1000000000, NULL, (void *)&appData.matchPause,        "",                    NULL, Spin,     N_("Pause between Match Games (msec):")                       },
 {0,   0,                         0,          NULL, (void *)&appData.saveGameFile,      ".pgn .game",          NULL, FileName, N_("Save Tourney Games on:")                                  },
 {0,   0,                         0,          NULL, (void *)&appData.loadGameFile,      ".pgn .game",          NULL, FileName, N_("Game File with Opening Lines:")                           },
 {0,   -2,                        1000000000, NULL, (void *)&appData.loadGameIndex,     "",                    NULL, Spin,     N_("Game Number (-1 or -2 = Auto-Increment):")                },
 {0,   0,                         0,          NULL, (void *)&appData.loadPositionFile,  ".fen .epd .pos",      NULL, FileName, N_("File with Start Positions:")                              },
 {0,   -2,                        1000000000, NULL, (void *)&appData.loadPositionIndex, "",                    NULL, Spin,     N_("Position Number (-1 or -2 = Auto-Increment):")            },
 {0,   0,                         1000000000, NULL, (void *)&appData.rewindIndex,       "",                    NULL, Spin,     N_("Rewind Index after this many Games (0 = never):")         },
 {0,   0,                         0,          NULL, (void *)&appData.defNoBook,         "",                    NULL, CheckBox, N_("Disable own engine books by default")                     },
 {0,   0,                         0,          NULL, (void *)&DoTimeControl,             NULL,                  NULL, Button,   N_("Time Control")                                            },
 {0,   SAME_ROW,                  0,          NULL, (void *)&DoCommonEngine,            NULL,                  NULL, Button,   N_("Common Engine")                                           },
 {0,   SAME_ROW,                  0,          NULL, (void *)&DoGeneral,                 NULL,                  NULL, Button,   N_("General Options")                                         },
 {0,   SAME_ROW,                  0,          NULL, (void *)&PseudoOK,                  NULL,                  NULL, Button,   N_("Continue Later")                                          },
 {0,   0,                         0,          NULL, (void *)&ReplaceParticipant,        NULL,                  NULL, Button,   N_("Replace Engine")                                          },
 {0,   SAME_ROW,                  0,          NULL, (void *)&UpgradeParticipant,        NULL,                  NULL, Button,   N_("Upgrade Engine")                                          },
 {0,   SAME_ROW,                  0,          NULL, (void *)&CloneTourney,              NULL,                  NULL, Button,   N_("Clone Tourney")                                           },
 {0,   SAME_ROW,                  0,          NULL, (void *)&MatchOK,                   "",                    NULL, EndMark,  ""                                                            }
};

static void ReplaceParticipant(void) {
    GenericReadout(matchOptions, PARTICIPANTS);
    Substitute(strdup(engineName), TRUE);
}

static void UpgradeParticipant(void) {
    GenericReadout(matchOptions, PARTICIPANTS);
    Substitute(strdup(engineName), FALSE);
}

static void PseudoOK(void) {
    if (matchMode) {
        return;
    }
    /* read all, but suppress calling of MatchOK */
    GenericReadout(matchOptions, -2);
    free_then_strdup(&appData.participants, engineName);
    free_then_strdup(&appData.tourneyFile, tfName);
    /* early popdown to prevent FreezeUI called through MatchEvent from causing XtGrab warning */
    PopDown(MasterDlg);
}

static void CloneTourney(void) {
    FILE * f;
    char * name;
    GetWidgetText(matchOptions, &name);
    if (name && name[0] && (f = fopen(name, "r"))) {
        char * saveSaveFile;
        saveSaveFile = appData.saveGameFile;
        /* this is a persistent option, protect from change */
        appData.saveGameFile = NULL;
        ParseArgsFromFile(f);
        engineName = appData.participants;
        GenericUpdate(matchOptions, -1);
        free(appData.saveGameFile);
        appData.saveGameFile = saveSaveFile;
    } else {
        DisplayError(_("First you must specify an existing tourney file to clone"), 0);
    }
}

static void AddToTourney(int n, int sel) {
    int nr;
    char buf[MSG_SIZ];
    if (sel < 1) {
        /* back to top level */
        buf[0] = NULLCHAR;
    } else if (engineList[sel][0] == '#') {
        /* group header, open group */
        safeStrCpy(buf, engineList[sel], MSG_SIZ);
    } else {
        /* normal line, select engine */
        AddLine(&matchOptions[PARTICIPANTS], engineMnemonic[sel]);
        return;
    }
    /* replace list by only the group contents */
    nr = NamesToList(firstChessProgramNames, engineList, engineMnemonic, buf);
    free_then_strdup(&engineMnemonic[0], buf);
    LoadListBox(&matchOptions[PARTICIPANTS + 1], _("# no engines are installed"), -1, -1);
    HighlightWithScroll(&matchOptions[PARTICIPANTS + 1], 0, nr);
}

void MatchOptionsProc(void) {
    if (matchOptions[PARTICIPANTS + 1].type != ListBox) {
        DisplayError(_("Internal error: PARTICIPANTS set wrong"), 0);
        return;
    }
    NamesToList(firstChessProgramNames, engineList, engineMnemonic, "");
    /* with pairing engine, allow Swiss */
    matchOptions[9].min = -(appData.pairingEngine[0] != NULLCHAR);
    free_then_strdup(&tfName, appData.tourneyFile[0] ? appData.tourneyFile : MakeName(appData.defName));
    free_then_strdup(&engineName, appData.participants);
    free_then_strdup(&engineMnemonic[0], "");
    GenericPopUp(matchOptions, _("Tournament Options"), MasterDlg, BoardWindow, MODAL, 0);
}

/* ------------------------------------------- General Options -------------------------------------------------- */

static int oldShow, oldBlind, oldPonder;

static int GeneralOptionsOK(int n) {
    int newPonder = appData.ponderNextMove;
    appData.ponderNextMove = oldPonder;
    PonderNextMoveEvent(newPonder);
    if (!appData.highlightLastMove) {
        ClearHighlights(), ClearPremoveHighlights();
    }
    if (oldShow != appData.showCoords || oldBlind != appData.blindfold) {
        DrawPosition(TRUE, NULL);
    }
    return 1;
}

static Option generalOptions[] = {
 {0, 0,        0,   NULL, (void *)&appData.whitePOV,               "",   NULL, CheckBox, N_("Absolute Analysis Scores")                  },
 {0, 0,        0,   NULL, (void *)&appData.sweepSelect,            "",   NULL, CheckBox, N_("Almost Always Queen (Detour Under-Promote)")},
 {0, 0,        0,   NULL, (void *)&appData.animateDragging,        "",   NULL, CheckBox, N_("Animate Dragging")                          },
 {0, 0,        0,   NULL, (void *)&appData.animate,                "",   NULL, CheckBox, N_("Animate Moving")                            },
 {0, 0,        0,   NULL, (void *)&appData.autoCallFlag,           "",   NULL, CheckBox, N_("Auto Flag")                                 },
 {0, 0,        0,   NULL, (void *)&appData.autoFlipView,           "",   NULL, CheckBox, N_("Auto Flip View")                            },
 {0, 0,        0,   NULL, (void *)&appData.blindfold,              "",   NULL, CheckBox, N_("Blindfold")                                 },
 /* TRANSLATORS: The drop menu is used to drop a piece onto the board, e.g., while playing bughouse chess or editing a position. */
 {0, 0,        0,   NULL, (void *)&appData.dropMenu,               "",   NULL, CheckBox, N_("Drop Menu")                                 },
 {0, 0,        0,   NULL, (void *)&appData.variations,             "",   NULL, CheckBox, N_("Enable Variation Trees")                    },
 {0, 0,        0,   NULL, (void *)&appData.headers,                "",   NULL, CheckBox, N_("Headers in Engine Output Window")           },
 {0, 0,        0,   NULL, (void *)&appData.hideThinkingFromHuman,  "",   NULL, CheckBox, N_("Hide Thinking from Human")                  },
 {0, 0,        0,   NULL, (void *)&appData.highlightLastMove,      "",   NULL, CheckBox, N_("Highlight Last Move")                       },
 {0, 0,        0,   NULL, (void *)&appData.highlightMoveWithArrow, "",   NULL, CheckBox, N_("Highlight with Arrow")                      },
 {0, 0,        0,   NULL, (void *)&appData.oneClick,               "",   NULL, CheckBox, N_("One-Click Moving")                          },
 {0, 0,        0,   NULL, (void *)&appData.periodicUpdates,        "",   NULL, CheckBox, N_("Periodic Updates (in Analysis Mode)")       },
 {0, SAME_ROW, 0,   NULL, NULL,                                    NULL, NULL, Break,    ""                                              },
 {0, 0,        0,   NULL, (void *)&appData.autoExtend,             "",   NULL, CheckBox, N_("Play Move(s) of Clicked PV (Analysis)")     },
 {0, 0,        0,   NULL, (void *)&appData.ponderNextMove,         "",   NULL, CheckBox, N_("Ponder Next Move")                          },
 {0, 0,        0,   NULL, (void *)&appData.popupExitMessage,       "",   NULL, CheckBox, N_("Popup Exit Messages")                       },
 {0, 0,        0,   NULL, (void *)&appData.popupMoveErrors,        "",   NULL, CheckBox, N_("Popup Move Errors")                         },
 {0, 0,        0,   NULL, (void *)&appData.showEvalInMoveHistory,  "",   NULL, CheckBox, N_("Scores in Move List")                       },
 {0, 0,        0,   NULL, (void *)&appData.showCoords,             "",   NULL, CheckBox, N_("Show Coordinates")                          },
 {0, 0,        0,   NULL, (void *)&appData.markers,                "",   NULL, CheckBox, N_("Show Target Squares")                       },
 {0, 0,        0,   NULL, (void *)&appData.useStickyWindows,       "",   NULL, CheckBox, N_("Sticky Windows")                            },
 {0, 0,        0,   NULL, (void *)&appData.testLegality,           "",   NULL, CheckBox, N_("Test Legality")                             },
 {0, 0,        0,   NULL, (void *)&appData.topLevel,               "",   NULL, CheckBox, N_("Top-Level Dialogs")                         },
 {0, 0,        10,  NULL, (void *)&appData.flashCount,             "",   NULL, Spin,     N_("Flash Moves (0 = no flashing):")            },
 {0, 1,        10,  NULL, (void *)&appData.flashRate,              "",   NULL, Spin,     N_("Flash Rate (high = fast):")                 },
 {0, 5,        100, NULL, (void *)&appData.animSpeed,              "",   NULL, Spin,     N_("Animation Speed (high = slow):")            },
 {0, 1,        5,   NULL, (void *)&appData.zoom,                   "",   NULL, Spin,     N_("Zoom factor in Evaluation Graph:")          },
 {0, 0,        0,   NULL, (void *)&GeneralOptionsOK,               "",   NULL, EndMark,  ""                                              }
};

void OptionsProc(void) {
    oldPonder = appData.ponderNextMove;
    oldShow = appData.showCoords;
    oldBlind = appData.blindfold;
    GenericPopUp(generalOptions, _("General Options"), TransientDlg, BoardWindow, MODAL, 0);
}

/*---------------------------------------------- New Variant ------------------------------------------------ */

static void Pick(int n);

static char warning[MSG_SIZ];
static int ranksTmp, filesTmp, sizeTmp;

static Option variantDescriptors[] = {
 {VariantNormal,       0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("Normal")                                                               },
 {VariantMakruk,       SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("Makruk")                                                               },
 {VariantFischeRandom, 0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("FRC")                                                                  },
 {VariantShatranj,     SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("Shatranj")                                                             },
 {VariantWildCastle,   0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("Wild castle")                                                          },
 {VariantKnightmate,   SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("Knightmate")                                                           },
 {VariantNoCastle,     0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("No castle")                                                            },
 {VariantCylinder,     SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("Cylinder *")                                                           },
 {Variant3Check,       0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("3-checks")                                                             },
 {VariantBerolina,     SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("berolina *")                                                           },
 {VariantAtomic,       0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("atomic")                                                               },
 {VariantTwoKings,     SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("two kings")                                                            },
 /* To improve alignment */
 {-1,                  0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_(" ")                                                                    },
 {VariantSpartan,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#FF0000", NULL, Button,  N_("Spartan")                                                              },
 {0,                   0,        0,               NULL, NULL,              NULL,      NULL, Label,   N_("Board size ( -1 = default for selected variant):")                     },
 {0,                   -1,       BOARD_RANKS - 1, NULL, (void *)&ranksTmp, "",        NULL, Spin,    N_("Number of Board Ranks:")                                               },
 {0,                   -1,       BOARD_FILES,     NULL, (void *)&filesTmp, "",        NULL, Spin,    N_("Number of Board Files:")                                               },
 {0,                   -1,       BOARD_RANKS - 1, NULL, (void *)&sizeTmp,  "",        NULL, Spin,    N_("Holdings Size:")                                                       },
 {0,                   0,        275,             NULL, NULL,              NULL,      NULL, Label,   warning                                                                    },
 {0,                   0,        275,             NULL, NULL,              NULL,      NULL, Label,   N_("Variants marked with * can only be played\nwith legality testing off.")},
 {0,                   SAME_ROW, 0,               NULL, NULL,              NULL,      NULL, Break,   ""                                                                         },
 {VariantASEAN,        0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("ASEAN")                                                                },
 {VariantGreat,        SAME_ROW, 135,             NULL, (void *)&Pick,     "#BFBFFF", NULL, Button,  N_("Great Shatranj (10x8)")                                                },
 {VariantSChess,       0,        135,             NULL, (void *)&Pick,     "#FFBFBF", NULL, Button,  N_("Seirawan")                                                             },
 {VariantFalcon,       SAME_ROW, 135,             NULL, (void *)&Pick,     "#BFBFFF", NULL, Button,  N_("Falcon (10x8)")                                                        },
 {VariantSuper,        0,        135,             NULL, (void *)&Pick,     "#FFBFBF", NULL, Button,  N_("Superchess")                                                           },
 {VariantCapablanca,   SAME_ROW, 135,             NULL, (void *)&Pick,     "#BFBFFF", NULL, Button,  N_("Capablanca (10x8)")                                                    },
 {VariantCrazyhouse,   0,        135,             NULL, (void *)&Pick,     "#FFBFBF", NULL, Button,  N_("Crazyhouse")                                                           },
 {VariantGothic,       SAME_ROW, 135,             NULL, (void *)&Pick,     "#BFBFFF", NULL, Button,  N_("Gothic (10x8)")                                                        },
 {VariantBughouse,     0,        135,             NULL, (void *)&Pick,     "#FFBFBF", NULL, Button,  N_("Bughouse")                                                             },
 {VariantJanus,        SAME_ROW, 135,             NULL, (void *)&Pick,     "#BFBFFF", NULL, Button,  N_("Janus (10x8)")                                                         },
 {VariantSuicide,      0,        135,             NULL, (void *)&Pick,     "#FFFFBF", NULL, Button,  N_("Suicide")                                                              },
 {VariantCapaRandom,   SAME_ROW, 135,             NULL, (void *)&Pick,     "#BFBFFF", NULL, Button,  N_("CRC (10x8)")                                                           },
 {VariantGiveaway,     0,        135,             NULL, (void *)&Pick,     "#FFFFBF", NULL, Button,  N_("give-away")                                                            },
 {VariantGrand,        SAME_ROW, 135,             NULL, (void *)&Pick,     "#5070FF", NULL, Button,  N_("grand (10x10)")                                                        },
 {VariantLosers,       0,        135,             NULL, (void *)&Pick,     "#FFFFBF", NULL, Button,  N_("losers")                                                               },
 {VariantShogi,        SAME_ROW, 135,             NULL, (void *)&Pick,     "#BFFFFF", NULL, Button,  N_("shogi (9x9)")                                                          },
 {VariantFairy,        0,        135,             NULL, (void *)&Pick,     "#BFBFBF", NULL, Button,  N_("fairy")                                                                },
 {VariantXiangqi,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#BFFFFF", NULL, Button,  N_("xiangqi (9x10)")                                                       },
 {VariantLion,         0,        135,             NULL, (void *)&Pick,     "#BFBFBF", NULL, Button,  N_("mighty lion")                                                          },
 {VariantJanggi,       SAME_ROW, 135,             NULL, (void *)&Pick,     "#BFFFFF", NULL, Button,  N_("Janggi (9x10)")
                                            },
 {VariantChuChess,     0,        135,             NULL, (void *)&Pick,     "#BFBFBF", NULL, Button,  N_("elven chess (10x10)")                                                  },
 {VariantCourier,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#BFFFBF", NULL, Button,  N_("courier (12x8)")                                                       },
 {VariantDuck,         0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Button,  N_("Duck Chess")                                                           },
 {VariantChu,          SAME_ROW, 135,             NULL, (void *)&Pick,     "#BFFFBF", NULL, Button,  N_("chu shogi (12x12)")                                                    },
 /* optional buttons for engine-defined variants */
 {0,                   NO_OK,    0,               NULL, NULL,              "",        NULL, EndMark, ""                                                                         },
 {0,                   SAME_ROW, 0,               NULL, NULL,              NULL,      NULL, Skip,    ""                                                                         },
 {VariantUnknown,      0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      0,        135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {VariantUnknown,      SAME_ROW, 135,             NULL, (void *)&Pick,     "#FFFFFF", NULL, Skip,    NULL                                                                       },
 {0,                   NO_OK,    0,               NULL, NULL,              "",        NULL, EndMark, ""                                                                         }
};

static void Pick(int n) {
    VariantClass v = variantDescriptors[n].value;
    if (v == VariantUnknown) {
        safeStrCpy(engineVariant, variantDescriptors[n].name, MSG_SIZ);
    } else {
        *engineVariant = NULLCHAR;
    }
    /* Read new ranks and file settings. */
    GenericReadout(variantDescriptors, -1);
    if (!appData.noChessProgram) {
        char buf[MSG_SIZ];
        if (!SupportedVariant(first.variants, v, filesTmp, ranksTmp, sizeTmp, first.protocolVersion, first.tidy)) {
            /* Ignore OK if first engine does not support it. */
            DisplayError(variantError, 0);
            return;
        } else if (second.initDone &&
         !SupportedVariant(second.variants, v, filesTmp, ranksTmp, sizeTmp, second.protocolVersion, second.tidy)) {
            snprintf(buf, MSG_SIZ, _("Warning: second engine (%s) does not support this!"), second.tidy);
            /* Use of second engine is optional, so only warn the user. */
            DisplayError(buf, 0);
        }
    }

    gameInfo.variant = v;
    free_then_strdup(&appData.variant, VariantName(v));

    /* [HGM] shuffle: possible shuffle reset when we switch. */
    shuffleOpenings = FALSE;
    /* [HGM] loadPos: no longer valid in new variant. */
    startedFromPositionFile = FALSE;
    /* [HGM] fischer: no longer valid in new variant. */
    appData.fischerCastling = FALSE;
    appData.NrRanks = ranksTmp;
    appData.NrFiles = filesTmp;
    appData.holdingsSize = sizeTmp;
    appData.pieceToCharTable = NULL;
    free_then_strdup(&appData.pieceNickNames, "");
    free_then_strdup(&appData.colorNickNames, "");
    free_then_strdup(&appData.men, "");
    PopDown(TransientDlg);
    Reset(TRUE, TRUE);
    return;
}

void NewVariantProc(void) {
    static int start;
    int i, last;
    /* prefer defaults over actual settings */
    sizeTmp = -1;
    filesTmp = -1;
    ranksTmp = -1;
    if (appData.noChessProgram) {
        sprintf(warning, _("Only bughouse is not available in viewer mode."));
    } else {
        sprintf(warning, _("All variants not supported by the first engine\n(currently %s) are disabled."), first.tidy);
    }
    if (!start) {
        while (variantDescriptors[start].type != EndMark) {
            /* locate spares */
            start++;
        }
        /* conditional EndMark and Break */
        start += 2;
    }
    last = -1;
    /* create buttons for engine-defined variants */
    for (i = 0; variantDescriptors[start + i].type != EndMark; i++) {
        char * v = EngineDefinedVariant(&first, i);
        if (v) {
            last = i;
            free_then_strdup(&variantDescriptors[start + i].name, v);
            variantDescriptors[start + i].type = Button;
        } else {
            variantDescriptors[start + i].type = Skip;
        }
    }
    /* odd number, add filler */
    if (!(last & 1)) {
        free_then_strdup(&variantDescriptors[start + last + 1].name, " ");
        variantDescriptors[start + last + 1].type = Button;
        variantDescriptors[start + last + 1].value = Skip;
    }
    variantDescriptors[start - 2].type = (last < 0 ? EndMark : Skip);
    variantDescriptors[start - 1].type = (last < 6 ? Skip : Break);
    safeStrCpy(engineVariant + 100, engineVariant, 100);
    *engineVariant = NULLCHAR;
    GenericPopUp(variantDescriptors, _("New Variant"), TransientDlg, BoardWindow, MODAL, 0);
    /* must temporarily clear to avoid enabling all variant buttons */
    safeStrCpy(engineVariant, engineVariant + 100, MSG_SIZ);
}

/*------------------------------------------- Common Engine Options ------------------------------------- */

static int oldCores;
static char * egtPath;

static int CommonOptionsOK(int n) {
    int newPonder = appData.ponderNextMove;
    if (*egtPath != '/' && strchr(egtPath, ':')) {
        free_then_strdup(&appData.egtFormats, egtPath);
    } else {
        free_then_strdup(&appData.defaultPathEGTB, egtPath);
    }
    /* Ensure that changes are sent to the first engine by re-initializing it if it was already started pre-emptively at the end of
       the previous game. */
    if (gameMode == BeginningOfGame) {
        Reset(TRUE, TRUE);
    } else {
        /* Some changed setting need immediate sending always. */
        if (oldCores != appData.smpCores) {
            NewSettingEvent(FALSE, &(first.maxCores), "cores", appData.smpCores);
        }
        appData.ponderNextMove = oldPonder;
        PonderNextMoveEvent(newPonder);
    }
    return 1;
}

static Option commonEngineOptions[] = {
 {0, 0,        0,     NULL, (void *)&appData.ponderNextMove,       "",     NULL, CheckBox, N_("Ponder Next Move")                    },
 {0, 0,        1000,  NULL, (void *)&appData.smpCores,             "",     NULL, Spin,     N_("Maximum Number of CPUs per Engine:")  },
 {0, 0,        0,     NULL, (void *)&appData.polyglotDir,          NULL,   NULL, PathName, N_("Polygot Directory:")                  },
 {0, 0,        16000, NULL, (void *)&appData.defaultHashSize,      "",     NULL, Spin,     N_("Hash-Table Size (MB):")               },
 {0, 0,        0,     NULL, (void *)&egtPath,                      NULL,   NULL, PathName, N_("EGTB Path:")                          },
 {0, 0,        1000,  NULL, (void *)&appData.defaultCacheSizeEGTB, "",     NULL, Spin,     N_("EGTB Cache Size (MB):")               },
 {0, 0,        0,     NULL, (void *)&appData.usePolyglotBook,      "",     NULL, CheckBox, N_("Use GUI Book")                        },
 {0, 0,        0,     NULL, (void *)&appData.polyglotBook,         ".bin", NULL, FileName, N_("Opening-Book Filename:")              },
 {0, 0,        100,   NULL, (void *)&appData.bookDepth,            "",     NULL, Spin,     N_("Book Depth (moves):")                 },
 {0, 0,        100,   NULL, (void *)&appData.bookStrength,         "",     NULL, Spin,     N_("Book Variety (0) vs. Strength (100):")},
 {0, 0,        0,     NULL, (void *)&appData.firstHasOwnBookUCI,   "",     NULL, CheckBox, N_("Engine #1 Has Own Book")              },
 {0, 0,        0,     NULL, (void *)&appData.secondHasOwnBookUCI,  "",     NULL, CheckBox, N_("Engine #2 Has Own Book          ")    },
 {0, SAME_ROW, 0,     NULL, (void *)&CommonOptionsOK,              "",     NULL, EndMark,  ""                                        }
};

void UciMenuProc(void) {
    oldCores = appData.smpCores;
    oldPonder = appData.ponderNextMove;
    if (appData.egtFormats && *appData.egtFormats) {
        free_then_strdup(&egtPath, appData.egtFormats);
    } else {
        free_then_strdup(&egtPath, appData.defaultPathEGTB);
    }
    GenericPopUp(commonEngineOptions, _("Common Engine Settings"), TransientDlg, BoardWindow, MODAL, 0);
}

/*------------------------------------------ Adjudication Options -------------------------------------- */

static Option adjudicationOptions[] = {
 {0, 0,        0,    NULL, (void *)&appData.checkMates,              "", NULL, CheckBox, N_("Detect all Mates")                       },
 {0, 0,        0,    NULL, (void *)&appData.testClaims,              "", NULL, CheckBox, N_("Verify Engine Result Claims")            },
 {0, 0,        0,    NULL, (void *)&appData.materialDraws,           "", NULL, CheckBox, N_("Draw if Insufficient Mating Material")   },
 {0, 0,        0,    NULL, (void *)&appData.trivialDraws,            "", NULL, CheckBox, N_("Adjudicate Trivial Draws (3-Move Delay)")},
 {0, 0,        100,  NULL, (void *)&appData.ruleMoves,               "", NULL, Spin,     N_("N-Move Rule:")                           },
 {0, 0,        6,    NULL, (void *)&appData.drawRepeats,             "", NULL, Spin,     N_("N-fold Repeats:")                        },
 {0, 0,        1000, NULL, (void *)&appData.adjudicateDrawMoves,     "", NULL, Spin,     N_("Draw after N Moves Total:")              },
 {0, -5000,    0,    NULL, (void *)&appData.adjudicateLossThreshold, "", NULL, Spin,     N_("Win / Loss Threshold:")                  },
 {0, 0,        0,    NULL, (void *)&first.scoreIsAbsolute,           "", NULL, CheckBox, N_("Negate Score of Engine #1")              },
 {0, 0,        0,    NULL, (void *)&second.scoreIsAbsolute,          "", NULL, CheckBox, N_("Negate Score of Engine #2")              },
 {0, SAME_ROW, 0,    NULL, NULL,                                     "", NULL, EndMark,  ""                                           }
};

void EngineMenuProc(void) { GenericPopUp(adjudicationOptions, _("Adjudicate non-ICS Games"), TransientDlg, BoardWindow, MODAL, 0); }

/*--------------------------------------------- ICS Options --------------------------------------------- */

static int IcsOptionsOK(int n) {
    ParseIcsTextColors();
    return 1;
}

Option icsOptions[] = {
 {0, 0,        0,         NULL, (void *)&appData.autoKibitz,       "",   NULL, CheckBox, N_("Auto-Kibitz")                            },
 {0, 0,        0,         NULL, (void *)&appData.autoComment,      "",   NULL, CheckBox, N_("Auto-Comment")                           },
 {0, 0,        0,         NULL, (void *)&appData.autoObserve,      "",   NULL, CheckBox, N_("Auto-Observe")                           },
 {0, 0,        0,         NULL, (void *)&appData.autoRaiseBoard,   "",   NULL, CheckBox, N_("Auto-Raise Board")                       },
 {0, 0,        0,         NULL, (void *)&appData.autoCreateLogon,  "",   NULL, CheckBox, N_("Auto-Create Logon Script")               },
 {0, 0,        0,         NULL, (void *)&appData.bgObserve,        "",   NULL, CheckBox, N_("Background Observe while Playing")       },
 {0, 0,        0,         NULL, (void *)&appData.dualBoard,        "",   NULL, CheckBox, N_("Dual Board for Background-Observed Game")},
 {0, 0,        0,         NULL, (void *)&appData.getMoveList,      "",   NULL, CheckBox, N_("Get Move List")                          },
 {0, 0,        0,         NULL, (void *)&appData.quietPlay,        "",   NULL, CheckBox, N_("Quiet Play")                             },
 {0, 0,        0,         NULL, (void *)&appData.seekGraph,        "",   NULL, CheckBox, N_("Seek Graph")                             },
 {0, 0,        0,         NULL, (void *)&appData.autoRefresh,      "",   NULL, CheckBox, N_("Auto-Refresh Seek Graph")                },
 {0, 0,        0,         NULL, (void *)&appData.autoBox,          "",   NULL, CheckBox, N_("Auto-InputBox PopUp")                    },
 {0, 0,        0,         NULL, (void *)&appData.quitNext,         "",   NULL, CheckBox, N_("Quit after game")                        },
 {0, 0,        0,         NULL, (void *)&appData.premove,          "",   NULL, CheckBox, N_("Premove")                                },
 {0, 0,        0,         NULL, (void *)&appData.premoveWhite,     "",   NULL, CheckBox, N_("Premove for White")                      },
 {0, 0,        0,         NULL, (void *)&appData.premoveWhiteText, "",   NULL, TextBox,  N_("First White Move:")                      },
 {0, 0,        0,         NULL, (void *)&appData.premoveBlack,     "",   NULL, CheckBox, N_("Premove for Black")                      },
 {0, 0,        0,         NULL, (void *)&appData.premoveBlackText, "",   NULL, TextBox,  N_("First Black Move:")                      },
 {0, SAME_ROW, 0,         NULL, NULL,                              NULL, NULL, Break,    ""                                           },
 {0, 0,        0,         NULL, (void *)&appData.icsAlarm,         "",   NULL, CheckBox, N_("Alarm")                                  },
 {0, 0,        100000000, NULL, (void *)&appData.icsAlarmTime,     "",   NULL, Spin,     N_("Alarm Time (msec):")                     },
 /*{ 0, 0,     0,         NULL, (void*) &appData.chatBoxes,        "",   NULL, TextBox,  N_("Startup Chat Boxes:")                  },*/
 {0, 0,        0,         NULL, (void *)&appData.colorize,         "",   NULL, CheckBox, N_("Colorize Messages")                      },
 {0, 0,        0,         NULL, (void *)&appData.colorShout,       "",   NULL, TextBox,  N_("Shout Text Colors:")                     },
 {0, 0,        0,         NULL, (void *)&appData.colorSShout,      "",   NULL, TextBox,  N_("S-Shout Text Colors:")                   },
 {0, 0,        0,         NULL, (void *)&appData.colorChannel1,    "",   NULL, TextBox,  N_("Channel #1 Text Colors:")                },
 {0, 0,        0,         NULL, (void *)&appData.colorChannel,     "",   NULL, TextBox,  N_("Other Channel Text Colors:")             },
 {0, 0,        0,         NULL, (void *)&appData.colorKibitz,      "",   NULL, TextBox,  N_("Kibitz Text Colors:")                    },
 {0, 0,        0,         NULL, (void *)&appData.colorTell,        "",   NULL, TextBox,  N_("Tell Text Colors:")                      },
 {0, 0,        0,         NULL, (void *)&appData.colorChallenge,   "",   NULL, TextBox,  N_("Challenge Text Colors:")                 },
 {0, 0,        0,         NULL, (void *)&appData.colorRequest,     "",   NULL, TextBox,  N_("Request Text Colors:")                   },
 {0, 0,        0,         NULL, (void *)&appData.colorSeek,        "",   NULL, TextBox,  N_("Seek Text Colors:")                      },
 {0, 0,        0,         NULL, (void *)&appData.colorNormal,      "",   NULL, TextBox,  N_("Other Text Colors:")                     },
 {0, 0,        0,         NULL, (void *)&IcsOptionsOK,             "",   NULL, EndMark,  ""                                           }
};

void IcsOptionsProc(void) { GenericPopUp(icsOptions, _("ICS Options"), TransientDlg, BoardWindow, MODAL, 0); }

/*-------------------------------------------- Load Game Options --------------------------------- */

static char * modeNames[] = {N_("Exact position match"), N_("Shown position is subset"),
 N_("Same material with exactly same Pawn chain"), N_("Same material"), N_("Material range (top board half optional)"),
 N_("Material difference (optional stuff balanced)"), NULL};
static char * modeValues[] = {"1", "2", "3", "4", "5", "6"};
static char *searchMode, *countRange;

static int LoadOptionsOK(void) {
    appData.minPieces = appData.maxPieces = 0;
    sscanf(countRange, "%d-%d", &appData.minPieces, &appData.maxPieces);
    if (appData.maxPieces < appData.minPieces) {
        appData.maxPieces = appData.minPieces;
    }
    appData.searchMode = atoi(searchMode);
    return 1;
}

static Option loadOptions[] = {
 {0, 0,  0,        NULL, (void *)&appData.autoDisplayTags,    "",                 NULL,      CheckBox,   N_("Auto-Display Tags")                                        },
 {0, 0,  0,        NULL, (void *)&appData.autoDisplayComment, "",                 NULL,      CheckBox,   N_("Auto-Display Comment")                                     },
 {0, LR, 0,        NULL, NULL,                                NULL,               NULL,      Label,      N_("Auto-Play speed of loaded games\n(0 = instant, -1 = off):")},
 {0, -1, 10000000, NULL, (void *)&appData.timeDelay,          "",                 NULL,      Fractional, N_("Seconds per Move:")                                        },
 {0, LR, 0,        NULL, NULL,                                NULL,               NULL,      Label,      N_("\noptions to use in game-viewer mode:")                    },
 {0, 0,  300,      NULL, (void *)&appData.viewerOptions,      "",                 NULL,      TextBox,    ""                                                             },
 {0, LR, 0,        NULL, NULL,                                NULL,               NULL,      Label,      N_("\nThresholds for position filtering in game list:")        },
 {0, 0,  5000,     NULL, (void *)&appData.eloThreshold1,      "",                 NULL,      Spin,       N_("Elo of strongest player at least:")                        },
 {0, 0,  5000,     NULL, (void *)&appData.eloThreshold2,      "",                 NULL,      Spin,       N_("Elo of weakest player at least:")                          },
 {0, 0,  5000,     NULL, (void *)&appData.dateThreshold,      "",                 NULL,      Spin,       N_("No games before year:")                                    },
 {0, 1,  50,       NULL, (void *)&appData.stretch,            "",                 NULL,      Spin,       N_("Minimum nr consecutive positions:")                        },
 {0, 0,  197,      NULL, (void *)&countRange,                 "",                 NULL,      TextBox,    "Final nr of pieces"                                           },
 {0, 0,  205,      NULL, (void *)&searchMode,                 (char *)modeValues, modeNames, ComboBox,   N_("Search mode:")                                             },
 {0, 0,  0,        NULL, (void *)&appData.ignoreColors,       "",                 NULL,      CheckBox,   N_("Also match reversed colors")                               },
 {0, 0,  0,        NULL, (void *)&appData.findMirror,         "",                 NULL,      CheckBox,   N_("Also match left-right flipped position")                   },
 {0, 0,  0,        NULL, (void *)&LoadOptionsOK,              "",                 NULL,      EndMark,    ""                                                             }
};

void LoadOptionsPopUp(DialogClass parent) {
    free_then_strdup(&countRange, "");
    free_then_strdup(&searchMode, modeValues[appData.searchMode - 1]);
    GenericPopUp(loadOptions, _("Load Game Options"), TransientDlg, parent, MODAL, 0);
}

/* called from menu */
void LoadOptionsProc(void) {
    LoadOptionsPopUp(BoardWindow);
}

/*------------------------------------------- Save Game Options -------------------------------------------- */

static Option saveOptions[] = {
 {0, 0,        0, NULL, (void *)&appData.autoSaveGames,         "",     NULL, CheckBox, N_("Auto-Save Games")                        },
 {0, 0,        0, NULL, (void *)&appData.onlyOwn,               "",     NULL, CheckBox, N_("Own Games Only")                         },
 {0, 0,        0, NULL, (void *)&appData.saveGameFile,          ".pgn", NULL, FileName, N_("Save Games on File:")                    },
 {0, 0,        0, NULL, (void *)&appData.savePositionFile,      ".fen", NULL, FileName, N_("Save Final Positions on File:")          },
 {0, 0,        0, NULL, (void *)&appData.pgnEventHeader,        "",     NULL, TextBox,  N_("PGN Event Header:")                      },
 {0, 0,        0, NULL, (void *)&appData.oldSaveStyle,          "",     NULL, CheckBox, N_("Old Save Style (as opposed to PGN)")     },
 {0, 0,        0, NULL, (void *)&appData.numberTag,             "",     NULL, CheckBox, N_("Include Number Tag in tourney PGN")      },
 {0, 0,        0, NULL, (void *)&appData.saveExtendedInfoInPGN, "",     NULL, CheckBox, N_("Save Score/Depth Info in PGN")           },
 {0, 0,        0, NULL, (void *)&appData.saveOutOfBookInfo,     "",     NULL, CheckBox, N_("Save Out-of-Book Info in PGN           ")},
 {0, SAME_ROW, 0, NULL, NULL,                                   "",     NULL, EndMark,  ""                                           }
};

void SaveOptionsProc(void) { GenericPopUp(saveOptions, _("Save Game Options"), TransientDlg, BoardWindow, MODAL, 0); }

/*----------------------------------------------- Sound Options --------------------------------------------- */

static void Test(int n);
static char * trialSound;

static char * soundNames[] = {N_("No Sound"), N_("Default Beep"), N_("Above WAV File"), N_("Car Horn"), N_("Cymbal"), N_("Ding"),
 N_("Gong"), N_("Laser"), N_("Penalty"), N_("Phone"), N_("Pop"), N_("Roar"), N_("Slap"), N_("Wood Thunk"), NULL, N_("User File")};

/* sound files corresponding to above names */
static char * soundFiles[] = {
 "",
 "$",
 /* kludge alert: as first thing in the dialog readout this is replaced with the user-given .WAV filename */
 NULL,
 "honkhonk.wav",
 "cymbal.wav",
 "ding1.wav",
 "gong.wav",
 "laser.wav",
 "penalty.wav",
 "phone.wav",
 "pop2.wav",
 "roar.wav",
 "slap.wav",
 "woodthunk.wav",
 NULL,
 NULL};

static Option soundOptions[] = {
 {0, 0,        0, NULL, (void *)(soundFiles + 2) /* kludge! */, ".wav",             NULL,       FileName, N_("User WAV File:")   },
 {0, 0,        0, NULL, (void *)&appData.soundProgram,          "",                 NULL,       TextBox,  N_("Sound Program:")   },
 {0, 0,        0, NULL, (void *)&trialSound,                    (char *)soundFiles, soundNames, ComboBox, N_("Try-Out Sound:")   },
 {0, SAME_ROW, 0, NULL, (void *)&Test,                          NULL,               NULL,       Button,   N_("Play")             },
 {0, 0,        0, NULL, (void *)&appData.soundMove,             (char *)soundFiles, soundNames, ComboBox, N_("Move:")            },
 {0, 0,        0, NULL, (void *)&appData.soundIcsWin,           (char *)soundFiles, soundNames, ComboBox, N_("Win:")             },
 {0, 0,        0, NULL, (void *)&appData.soundIcsLoss,          (char *)soundFiles, soundNames, ComboBox, N_("Lose:")            },
 {0, 0,        0, NULL, (void *)&appData.soundIcsDraw,          (char *)soundFiles, soundNames, ComboBox, N_("Draw:")            },
 {0, 0,        0, NULL, (void *)&appData.soundIcsUnfinished,    (char *)soundFiles, soundNames, ComboBox, N_("Unfinished:")      },
 {0, 0,        0, NULL, (void *)&appData.soundIcsAlarm,         (char *)soundFiles, soundNames, ComboBox, N_("Alarm:")           },
 {0, 0,        0, NULL, (void *)&appData.soundChallenge,        (char *)soundFiles, soundNames, ComboBox, N_("Challenge:")       },
 {0, SAME_ROW, 0, NULL, NULL,                                   NULL,               NULL,       Break,    ""                     },
 {0, 0,        0, NULL, (void *)&appData.soundDirectory,        NULL,               NULL,       PathName, N_("Sounds Directory:")},
 {0, 0,        0, NULL, (void *)&appData.soundShout,            (char *)soundFiles, soundNames, ComboBox, N_("Shout:")           },
 {0, 0,        0, NULL, (void *)&appData.soundSShout,           (char *)soundFiles, soundNames, ComboBox, N_("S-Shout:")         },
 {0, 0,        0, NULL, (void *)&appData.soundChannel,          (char *)soundFiles, soundNames, ComboBox, N_("Channel:")         },
 {0, 0,        0, NULL, (void *)&appData.soundChannel1,         (char *)soundFiles, soundNames, ComboBox, N_("Channel 1:")       },
 {0, 0,        0, NULL, (void *)&appData.soundTell,             (char *)soundFiles, soundNames, ComboBox, N_("Tell:")            },
 {0, 0,        0, NULL, (void *)&appData.soundKibitz,           (char *)soundFiles, soundNames, ComboBox, N_("Kibitz:")          },
 {0, 0,        0, NULL, (void *)&appData.soundRequest,          (char *)soundFiles, soundNames, ComboBox, N_("Request:")         },
 {0, 0,        0, NULL, (void *)&appData.soundRoar,             (char *)soundFiles, soundNames, ComboBox, N_("Lion roar:")       },
 {0, 0,        0, NULL, (void *)&appData.soundSeek,             (char *)soundFiles, soundNames, ComboBox, N_("Seek:")            },
 {0, SAME_ROW, 0, NULL, NULL,                                   "",                 NULL,       EndMark,  ""                     }
};

static void Test(int n) {
    GenericReadout(soundOptions, 1);
    /* temporarily enable */
    mute <<= 1;
    if (soundFiles[values[2]]) {
        PlaySoundFile(soundFiles[values[2]]);
    }
    mute >>= 1;
}

void SoundOptionsProc(void) {
    free(soundFiles[2]);
    soundFiles[2] = strdup("*");
    GenericPopUp(soundOptions, _("Sound Options"), TransientDlg, BoardWindow, MODAL, 0);
}

/*--------------------------------------------- Board Options -------------------------------------- */

static void DefColor(int n);
static void AdjustColor(int i);
static void ThemeSel(int n, int sel);
static int BoardOptionsOK(int n);

static char oldPieceDir[MSG_SIZ];
extern char *engineLine, *nickName;

#define THEMELIST 1

static Option boardOptions[] = {
 {0,    LR | T2T, 0,   NULL, NULL,                                   NULL,              NULL,                 Label,    N_("Selectable themes:")                              },
 {300,  LR | TB,  200, NULL, (void *)engineMnemonic,                 (char *)&ThemeSel, NULL,                 ListBox,  ""                                                    },
 {0,    LR | T2T, 0,   NULL, NULL,                                   NULL,              NULL,                 Label,    N_("New name for current theme:")                     },
 {0,    0,        0,   NULL, (void *)&nickName,                      "",                NULL,                 TextBox,  ""                                                    },
 {0,    SAME_ROW, 0,   NULL, NULL,                                   NULL,              NULL,                 Break,    NULL                                                  },
 {0,    0,        70,  NULL, (void *)&appData.whitePieceColor,       "",                NULL,                 TextBox,  N_("White Piece Color:")                              },
 {1000, SAME_ROW, 0,   NULL, (void *)&DefColor,                      NULL,              (char **)"#FFFFCC",   Button,   "      "                                              },
 /* TRANSLATORS: The role of R here is to be a single letter that represents the colour red. */
 {1,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("R")                                               },
 /* TRANSLATORS: The role of G here is to be a single letter that represents the colour green. */
 {2,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("G")                                               },
 /* TRANSLATORS: The role of B here is to be a single letter that represents the colour blue. */
 {3,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("B")                                               },
 /* TRANSLATORS: The role of D here is to be a single letter that represents making a colour darker. */
 {4,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("D")                                               },
 {0,    0,        70,  NULL, (void *)&appData.blackPieceColor,       "",                NULL,                 TextBox,  N_("Black Piece Color:")                              },
 {1000, SAME_ROW, 0,   NULL, (void *)&DefColor,                      NULL,              (char **)"#202020",   Button,   "      "                                              },
 {1,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("R")                                               },
 {2,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("G")                                               },
 {3,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("B")                                               },
 {4,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("D")                                               },
 {0,    0,        70,  NULL, (void *)&appData.lightSquareColor,      "",                NULL,                 TextBox,  N_("Light Square Color:")                             },
 {1000, SAME_ROW, 0,   NULL, (void *)&DefColor,                      NULL,              (char **)"#C8C365",   Button,   "      "                                              },
 {1,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("R")                                               },
 {2,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("G")                                               },
 {3,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("B")                                               },
 {4,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("D")                                               },
 {0,    0,        70,  NULL, (void *)&appData.darkSquareColor,       "",                NULL,                 TextBox,  N_("Dark Square Color:")                              },
 {1000, SAME_ROW, 0,   NULL, (void *)&DefColor,                      NULL,              (char **)"#77A26D",   Button,   "      "                                              },
 {1,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("R")                                               },
 {2,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("G")                                               },
 {3,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("B")                                               },
 {4,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("D")                                               },
 {0,    0,        70,  NULL, (void *)&appData.highlightSquareColor,  "",                NULL,                 TextBox,  N_("Highlight Color:")                                },
 {1000, SAME_ROW, 0,   NULL, (void *)&DefColor,                      NULL,              (char **)"#FFFF00",   Button,   "      "                                              },
 {1,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("R")                                               },
 {2,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("G")                                               },
 {3,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("B")                                               },
 {4,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("D")                                               },
 {0,    0,        70,  NULL, (void *)&appData.premoveHighlightColor, "",                NULL,                 TextBox,  N_("Premove Highlight Color:")                        },
 {1000, SAME_ROW, 0,   NULL, (void *)&DefColor,                      NULL,              (char **)"#FF0000",   Button,   "      "                                              },
 {1,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("R")                                               },
 {2,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("G")                                               },
 {3,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("B")                                               },
 {4,    SAME_ROW, 0,   NULL, (void *)&AdjustColor,                   NULL,              NULL,                 Button,   N_("D")                                               },
 {0,    0,        0,   NULL, (void *)&appData.upsideDown,            "",                NULL,                 CheckBox,
  N_("Flip Pieces Shogi Style        (Colored buttons restore default)")                                                                                                      },
/*{ 0,  0,        0,   NULL, (void *)&appData.allWhite,              "",                NULL,                 CheckBox, N_("Use Outline Pieces for Black")                    },*/
 {0,    0,        0,   NULL, (void *)&appData.monoMode,              "",                NULL,                 CheckBox, N_("Mono Mode")                                       },
 {0,    0,        200, NULL, (void *)&appData.logoSize,              "",                NULL,                 Spin,     N_("Logo Size (0=off, requires restart):")            },
 {0,    -1,       5,   NULL, (void *)&appData.overrideLineGap,       "",                NULL,                 Spin,     N_("Line Gap (-1 = default for board size):")         },
 {0,    0,        0,   NULL, (void *)&appData.useBitmaps,            "",                NULL,                 CheckBox, N_("Use Board Textures")                              },
 {0,    0,        0,   NULL, (void *)&appData.darkBackTextureFile,   ".png",            (char **)(intptr_t)1, FileName, N_("Dark-Squares Texture File:")                      },
 {0,    0,        0,   NULL, (void *)&appData.liteBackTextureFile,   ".png",            (char **)(intptr_t)2, FileName, N_("Light-Squares Texture File:")                     },
 {0,    0,        0,   NULL, (void *)&appData.trueColors,            "",                NULL,                 CheckBox, N_("Use external piece bitmaps with their own colors")},
 {0,    0,        0,   NULL, (void *)&appData.pieceDirectory,        NULL,              (char **)(intptr_t)3, PathName, N_("Directory with Pieces Images:")                   },
 {0,    0,        0,   NULL, (void *)&BoardOptionsOK,                "",                NULL,                 EndMark,  ""                                                    }
};

static int BoardOptionsOK(int n) {
    /* called by pressing OK, and theme selected */
    if (n && (n = SelectedListBoxItem(&boardOptions[THEMELIST])) > 0 && *engineList[n] != '#') {
        free_then_strdup(&engineLine, engineList[n]);
    }
    LoadTheme();
    return 1;
}

static void SetColorText(int n, char * buf) {
    SetWidgetText(&boardOptions[n - 1], buf, TransientDlg);
    SetColor(buf, &boardOptions[n]);
}

static void DefColor(int n) { SetColorText(n, (char *)boardOptions[n].choice); }

void RefreshColor(int source, int n) {
    int col, j, r, g, b, step = 10;
    char *s, buf[MSG_SIZ];  /* color string */
    GetWidgetText(&boardOptions[source], &s);
    if (sscanf(s, "#%x", &col) != 1) {
        /* malformed */
        return;
    }
    b = col & 0xff;
    g = col & 0xff00;
    r = col & 0xff0000;
    switch (n) {
    case 1:
        r += 0x10000 * step;
        break;
    case 2:
        g += 0x100 * step;
        break;
    case 3:
        b += step;
        break;
    case 4:
        r -= 0x10000 * step;
        g -= 0x100 * step;
        b -= step;
        break;
    }
    if (r < 0) {
        r = 0;
    }
    if (g < 0) {
        g = 0;
    }
    if (b < 0) {
        b = 0;
    }
    if (r > 0xff0000) {
        r = 0xff0000;
    }
    if (g > 0xff00) {
        g = 0xff00;
    }
    if (b > 0xff) {
        b = 0xff;
    }
    col = r | g | b;
    snprintf(buf, MSG_SIZ, "#%06x", col);
    for (j = 1; j < 7; j++) {
        if (buf[j] >= 'a') {
            /* capitalize */
            buf[j] -= 32;
        }
    }
    SetColorText(source + 1, buf);
}

static void AdjustColor(int i) {
    int n = boardOptions[i].value;
    RefreshColor(i - n - 1, n);
}

void ThemeSel(int n, int sel) {
    int nr;
    char buf[MSG_SIZ];
    if (sel < 1) {
        /* back to top level */
        buf[0] = NULLCHAR;
    } else if (engineList[sel][0] == '#') {
        /* group header, open group */
        safeStrCpy(buf, engineList[sel], MSG_SIZ);
    } else {
        /* normal line, select engine */
        free_then_strdup(&engineLine, engineList[sel]);
        LoadTheme();
        PopDown(TransientDlg);
        return;
    }
    /* replace list by only the group contents */
    nr = NamesToList(appData.themeNames, engineList, engineMnemonic, buf);
    free_then_strdup(&engineMnemonic[0], buf);
    LoadListBox(&boardOptions[THEMELIST], _("# no themes are defined"), -1, -1);
    HighlightWithScroll(&boardOptions[THEMELIST], 0, nr);
}

void BoardOptionsProc(void) {
    /* to see if it changed */
    strncpy(oldPieceDir, appData.pieceDirectory, MSG_SIZ - 1);
    free_then_strdup(&engineLine, "");
    free_then_strdup(&nickName, "");
    free_then_strdup(&engineMnemonic[0], "");
    NamesToList(appData.themeNames, engineList, engineMnemonic, "");
    GenericPopUp(boardOptions, _("Board Options"), TransientDlg, BoardWindow, MODAL, 0);
}

/*-------------------------------------------- ICS Text Menu Options ------------------------------ */

Option textOptions[100];
static void PutText(char * text, int pos);
static void NewChat(char * name);
static char clickedWord[MSG_SIZ], click;

void SendString(char * p) {
    char buf[MSG_SIZ], buf2[MSG_SIZ], *q;

    if (q = strstr(p, "$name")) {
        /* in Xaw this is already intercepted */
        if (!shellUp[TextMenuDlg] || !clickedWord[0]) {
            return;
        }
        strncpy(buf2, p, MSG_SIZ);
        snprintf(buf2 + (q - p), MSG_SIZ - (q - p), "%s%s", clickedWord, q + 5);
        p = buf2;
    }
    if (!strcmp(p, "$copy")) {
        /* special case for copy selection */
        CopySomething(clickedWord);
    } else if (!strcmp(p, "$chat")) {
        /* special case for opening chat */
        NewChat(clickedWord);
    } else if (q = strstr(p, "$input")) {
        if (!shellUp[TextMenuDlg]) {
            return;
        }
        strncpy(buf, p, MSG_SIZ);
        strncpy(buf + (q - p), q + 6, MSG_SIZ - (q - p));
        PutText(buf, q - p);
    } else {
        snprintf(buf, MSG_SIZ, "%s\n", p);
        SendToICS(buf);
    }
    if (click) {
        /* popped up by memo click */
        click = clickedWord[0] = 0;
        PopDown(TextMenuDlg);
    }
}

void IcsTextPopUp(void) {
    int i = 0, j;
    char *p, *q, *r;
    if ((p = icsTextMenuString) == NULL) {
        return;
    }
    do {
        q = r = p;
        while (*p && *p != ';') {
            p++;
        }
        if (textOptions[i].name == NULL) {
            textOptions[i].name = (char *)malloc(MSG_SIZ);
        }
        for (j = 0; j < p - q; j++) {
            textOptions[i].name[j] = *r++;
        }
        textOptions[i].name[j++] = 0;
        if (!*p) {
            break;
        }
        if (*++p == '\n') {
            /* optional linefeed after button-text terminating semicolon */
            p++;
        }
        q = p;
        textOptions[i].choice = (char **)(r = textOptions[i].name + j);
        while (*p && (*p != ';' || p[1] != '\n')) {
            textOptions[i].name[j++] = *p++;
        }
        textOptions[i].name[j++] = 0;
        if (*p) {
            p += 2;
        }
        textOptions[i].max = 135;
        textOptions[i].min = i & 1;
        textOptions[i].handle = NULL;
        textOptions[i].target = &SendText;
        textOptions[i].textValue = strstr(r, "$input") ? "#80FF80" : strstr(r, "$name") ? "#FF8080" : "#FFFFFF";
        textOptions[i].type = Button;
    } while (++i < 99 && *p);
    if (i == 0) {
        return;
    }
    textOptions[i].type = EndMark;
    textOptions[i].target = NULL;
    textOptions[i].min = 2;
    MarkMenu("View.ICStextmenu", TextMenuDlg);
    GenericPopUp(textOptions, _("ICS text menu"), TextMenuDlg, BoardWindow, NONMODAL, appData.topLevel);
}

void IcsTextProc(void) {
    if (shellUp[TextMenuDlg]) {
        PopDown(TextMenuDlg);
    } else {
        IcsTextPopUp();
    }
}

/*---------------------------------------------------- Edit Comment ----------------------------------- */

static char * commentText;
static int commentIndex;
static void ClearComment(int n);
static void SaveChanges(int n);
int savedIndex; /* gross that this is global (and even across files...) */

static int CommentClick(Option * opt, int n, int x, int y, char * val, int index);

static int NewComCallback(int n) {
    ReplaceComment(commentIndex, commentText);
    return 1;
}

Option commentOptions[] = {
 {200, T_VSCRL | T_FILL | T_WRAP | T_TOP, 250, NULL, (void *)&commentText, NULL, (char **)&CommentClick, TextBox, "",
  &appData.commentFont},
 {0, 0, 50, NULL, (void *)&ClearComment, NULL, NULL, Button, N_("clear")},
 {0, SAME_ROW, 100, NULL, (void *)&SaveChanges, NULL, NULL, Button, N_("save changes")},
 {0, SAME_ROW, 0, NULL, (void *)&NewComCallback, "", NULL, EndMark, ""}
};

static int CommentClick(Option * opt, int n, int x, int y, char * val, int index) {
    if (n != 3) {
        /* Only a button-3 press is of interest. */
        return FALSE;
    }
    ReplaceComment(savedIndex, val);
    if (savedIndex != currentMove) {
        ToNrEvent(savedIndex);
    }
    /* [HGM] Also does the actual moving to it, now. */
    LoadVariation(index, val);
    return TRUE;
}

static void SaveChanges(int n) {
    GenericReadout(commentOptions, 0);
    ReplaceComment(commentIndex, commentText);
}

static void ClearComment(int n) { SetWidgetText(&commentOptions[0], "", CommentDlg); }

void NewCommentPopup(char * title, char * text, int index) {
    if (DialogExists(CommentDlg)) {
        /* if already exists, alter title and content */
        SetDialogTitle(CommentDlg, title);
        SetWidgetText(&commentOptions[0], text, CommentDlg);
    }
    if (commentText) {
        free(commentText);
    }
    commentText = strdup(text);
    commentIndex = index;
    MarkMenu("View.Comments", CommentDlg);
    if (GenericPopUp(commentOptions, title, CommentDlg, BoardWindow, NONMODAL, appData.topLevel)) {
        AddHandler(&commentOptions[0], CommentDlg, 1);
    }
}

void EditCommentPopUp(int index, char * title, char * text) {
    savedIndex = index;
    if (text == NULL) {
        text = "";
    }
    NewCommentPopup(title, text, index);
}

void CommentPopUp(char * title, char * text) {
    /* [HGM] vari */
    savedIndex = currentMove;
    NewCommentPopup(title, text, currentMove);
}

void CommentPopDown(void) { PopDown(CommentDlg); }


void EditCommentProc(void) {
    if (PopDown(CommentDlg)) {
        /* Pop-down was succesful. */
        /* MarkMenuItem("Edit.EditComment", FALSE); */
        /* MarkMenuItem("View.Comments", FALSE); */
    } else {
        /* Wasn't up. */
        EditCommentEvent();
    }
}

/*------------------------------------------------------ Edit Tags ---------------------------------- */

static void changeTags(int n);
static char *tagsText, **resPtr;

static int TagsClick(Option * opt, int n, int x, int y, char * val, int index);

static int NewTagsCallback(int n) {
    if (bookUp) {
        SaveToBook(tagsText), DisplayBook(currentMove);
    } else if (resPtr) {
        free_then_strdup(resPtr, tagsText);
        if (resPtr == &firstChessProgramNames) {
            SaveEngineList();
        }
    } else {
        ReplaceTags(tagsText, &gameInfo);
    }
    return 1;
}

static void NewMove(void) { addToBookFlag = !addToBookFlag; }

Option tagsOptions[] = {
 {0, 0, 0, NULL, NULL, NULL, NULL, Label, NULL},
 {200, T_VSCRL | T_FILL | T_TOP, 200, NULL, (void *)&tagsText, NULL, (char **)&TagsClick, TextBox, "", &appData.tagsFont},
 {0, 0, 100, NULL, (void *)&NewMove, NULL, NULL, Button, N_("add next move")},
 {0, SAME_ROW, 100, NULL, (void *)&changeTags, NULL, NULL, Button, N_("commit changes")},
 {0, SAME_ROW, 0, NULL, (void *)&NewTagsCallback, "", NULL, EndMark, ""}
};

static int TagsClick(Option * opt, int n, int x, int y, char * val, int index) {
    if (!bookUp || n != 3) {
        /* Only a button-3 press in Edit Book is of interest. */
        return FALSE;
    }
    PlayBookMove(val, index);
    return TRUE;
}

static void changeTags(int n) {
    GenericReadout(tagsOptions, 1);
    NewTagsCallback(0);
}

void NewTagsPopup(char * text, char * msg, char * ttl) {
    char * title = bookUp ? _("Edit book") : ttl;

    tagsOptions[2].type = bookUp ? Button : Skip;
    tagsOptions[3].min = bookUp ? SAME_ROW : 0;
    if (DialogExists(TagsDlg)) {
        /* if already exists, alter title and content */
        SetWidgetText(&tagsOptions[1], text, TagsDlg);
        SetDialogTitle(TagsDlg, title);
    }
    if (tagsText) {
        free(tagsText);
    }
    tagsText = strdup(text);
    tagsOptions[0].name = msg;
    MarkMenu("View.Tags", TagsDlg);
    GenericPopUp(tagsOptions + (msg == NULL), title, TagsDlg, BoardWindow, NONMODAL, appData.topLevel);
}

void TagsPopUp(char * tags, char * msg) { NewTagsPopup(tags, cmailMsgLoaded ? msg : NULL, _("Tags")); }

/* Just a wrapping function to preserve old name used in back-end */
void EditTagsPopUp(char * tags, char ** dest) {
    resPtr = dest;
    NewTagsPopup(tags, NULL, _("Tags"));
}

/* Just a wrapping function to preserve old name used in back-end */
void EditAnyPopUp(char * tags, char ** dest, char * title) {
    TagsPopDown();
    resPtr = dest;
    NewTagsPopup(tags, NULL, title);
}

void TagsPopDown(void) {
    PopDown(TagsDlg);
    bookUp = FALSE;
}

void EditTagsProc(void) {
    if (bookUp || !PopDown(TagsDlg)) {
        EditTagsEvent();
    }
}

void AddBookMove(char * text) { AppendText(&tagsOptions[1], text); }

/*---------------------------------------------- ICS Input Box ---------------------------------- */

char * icsText;

/* [HGM] code borrowed from winboard.c (which should thus go to backend.c!) */
#define HISTORY_SIZE 64
static char * history[HISTORY_SIZE];
static int histIn = 0, histP = 0;
static Boolean noEcho;

static void SaveInHistory(char * cmd) {
    if (noEcho) {
        /* do not save password! */
        return;
    }
    if (history[histIn] != NULL) {
        free(history[histIn]);
        history[histIn] = NULL;
    }
    if (*cmd == NULLCHAR) {
        return;
    }
    history[histIn] = StrSave(cmd);
    histIn = (histIn + 1) % HISTORY_SIZE;
    if (history[histIn] != NULL) {
        free(history[histIn]);
        history[histIn] = NULL;
    }
    histP = histIn;
}

static char * PrevInHistory(char * cmd) {
    int newhp;
    if (histP == histIn) {
        if (history[histIn] != NULL) {
            free(history[histIn]);
        }
        history[histIn] = StrSave(cmd);
    }
    newhp = (histP - 1 + HISTORY_SIZE) % HISTORY_SIZE;
    if (newhp == histIn || history[newhp] == NULL) {
        return NULL;
    }
    histP = newhp;
    return history[histP];
}

static char * NextInHistory(void) {
    if (histP == histIn) {
        return NULL;
    }
    histP = (histP + 1) % HISTORY_SIZE;
    return history[histP];
}
/* end of code borrowed from winboard.c */

#define INPUT 0

Option boxOptions[] = {
 {30, T_TOP, 400, NULL, (void *)&icsText, NULL, NULL, TextBox, ""},
 {0,  NO_OK, 0,   NULL, NULL,             "",   NULL, EndMark, ""}
};

void ICSInputSendText(void) {
    char * val;

    GetWidgetText(&boxOptions[INPUT], &val);
    SaveInHistory(val);
    SendMultiLineToICS(val);
    SetWidgetText(&boxOptions[INPUT], "", InputBoxDlg);
}

void IcsKey(int n) {
    /* [HGM] input: let up-arrow recall previous line from history */
    char * val = NULL;

    if (!shellUp[InputBoxDlg]) {
        return;
    }
    switch (n) {
    case 0:
        ICSInputSendText();
        return;
    case 1:
        GetWidgetText(&boxOptions[INPUT], &val);
        val = PrevInHistory(val);
        break;
    case -1:
        val = NextInHistory();
    }
    SetWidgetText(&boxOptions[INPUT], val = val ? val : "", InputBoxDlg);
    SetInsertPos(&boxOptions[INPUT], strlen(val));
}

void ICSInputBoxPopUp(void) {
    MarkMenu("View.ICSInputBox", InputBoxDlg);
    if (GenericPopUp(boxOptions, _("ICS input box"), InputBoxDlg, BoardWindow, NONMODAL, 0)) {
        AddHandler(&boxOptions[INPUT], InputBoxDlg, 3);
    }
    CursorAtEnd(&boxOptions[INPUT]);
}

void IcsInputBoxProc(void) {
    if (!PopDown(InputBoxDlg)) {
        ICSInputBoxPopUp();
    }
}

/*--------------------------------------------- Move Type In ------------------------------------------ */

static int TypeInOK(int n);

Option typeOptions[] = {
 {30, T_TOP, 400, NULL, (void *)&icsText,  NULL, NULL, TextBox, ""},
 {0,  NO_OK, 0,   NULL, (void *)&TypeInOK, "",   NULL, EndMark, ""}
};

static int TypeInOK(int n) {
    TypeInDoneEvent(icsText);
    return TRUE;
}

void PopUpMoveDialog(char firstchar) {
    static char buf[2];
    buf[0] = firstchar;
    free_then_strdup(&icsText, buf);
    if (GenericPopUp(typeOptions, _("Type a move"), TransientDlg, BoardWindow, MODAL, 0)) {
        AddHandler(&typeOptions[0], TransientDlg, 2);
    }
    CursorAtEnd(&typeOptions[0]);
}

/* only used in Xaw. GTK calls ConsoleAutoPopUp in stead (when we type to board) */
void BoxAutoPopUp(char * buf) {
    if (!appData.autoBox) {
        return;
    }
    if (appData.icsActive) {
        /* text typed to board in ICS mode: divert to ICS input box */
        if (DialogExists(InputBoxDlg)) {
            /* box already exists: append to current contents */
            char *p, newText[MSG_SIZ];
            GetWidgetText(&boxOptions[INPUT], &p);
            snprintf(newText, MSG_SIZ, "%s%c", p, *buf);
            SetWidgetText(&boxOptions[INPUT], newText, InputBoxDlg);
            if (shellUp[InputBoxDlg]) {
                /* TODO: why??? */
                HardSetFocus(&boxOptions[INPUT], InputBoxDlg);
            }
        } else {
            /* box did not exist: make sure it pops up with char in it */
            icsText = buf;
        }
        ICSInputBoxPopUp();
    } else {
        PopUpMoveDialog(*buf);
    }
}

/*------------------------------------------ Engine Settings ------------------------------------ */

void SettingsPopUp(ChessProgramState * cps) {
    if (!cps->nrOptions) {
        DisplayNote(_("Engine has no options"));
        return;
    }
    currentCps = cps;
    GenericPopUp(cps->option, _("Engine Settings"), TransientDlg, BoardWindow, MODAL, 0);
}

void FirstSettingsProc(void) { SettingsPopUp(&first); }

void SecondSettingsProc(void) {
    if (WaitForEngine(&second, SettingsMenuIfReady)) {
        return;
    }
    SettingsPopUp(&second);
}

void RefreshSettingsDialog(ChessProgramState * cps, int val) {
    if (val == 1) {
        /* option values changed */
        if (shellUp[TransientDlg] && cps == currentCps) {
            /* normally update values when dialog is up */
            GenericUpdate(cps->option, -1);
        }
        return;
    }
    if (val == 2) {
        /* option list changed */
        if (!shellUp[TransientDlg] || cps != currentCps) {
            /* our dialog is not up, so nothing to do */
            return;
        }
    }
    /* Make sure any other dialog closes first. */
    PopDown(TransientDlg);
    /* Then, pop up the new one. */
    SettingsPopUp(cps);
}

/*----------------------------------------------- Load Engine -------------------------------------- */

char *engineDir, *engineLine, *nickName, *params, *protocolChoice;
Boolean isUCI, isUSI, hasBook, storeVariant, v1, addToList, useNick, secondEng;

static void EngSel(int n, int sel);
static int InstallOK(int n);

static char * protocols[] = {"autodetect", "WB", "UCI", "USI/UCCI", "WB v1", NULL};

static Option installOptions[] = {
 {0,   LR | T2T, 0,   NULL, NULL,                    NULL,              NULL,      Label,    N_("Select engine from list:")                                        },
 {300, LR | TB,  200, NULL, (void *)engineMnemonic,  (char *)&EngSel,   NULL,      ListBox,  ""                                                                    },
 {0,   SAME_ROW, 0,   NULL, NULL,                    NULL,              NULL,      Break,    NULL                                                                  },
 {0,   LR,       0,   NULL, NULL,                    NULL,              NULL,      Label,    N_("or specify one below:")                                           },
 {0,   0,        0,   NULL, (void *)&engineName,     NULL,              NULL,      FileName, N_("Engine Command:")                                                 },
 {0,   LR,       0,   NULL, NULL,                    NULL,              NULL,      Label,    N_("------------- User preferences (optional) ---------------")       },
 {0,   0,        0,   NULL, (void *)&nickName,       NULL,              NULL,      TextBox,  N_("Nickname (optional):")                                            },
 {0,   0,        0,   NULL, (void *)&useNick,        NULL,              NULL,      CheckBox, N_("Use nickname in PGN player tags of engine-engine games")          },
 {0,   0,        0,   NULL, (void *)&storeVariant,   NULL,              NULL,      CheckBox, N_("Force current variant with this engine")                          },
 {0,   0,        0,   NULL, (void *)&hasBook,        NULL,              NULL,      CheckBox, N_("Must not use GUI book")                                           },
 {0,   0,        0,   NULL, (void *)&addToList,      NULL,              NULL,      CheckBox, N_("Add this engine to the list")                                     },
 {0,   LR,       0,   NULL, NULL,                    NULL,              NULL,      Label,    N_("--------- Advanced (only change in exceptional cases) ----------")},
 {0,   0,        5,   NULL, (void *)&protocolChoice, (char *)protocols, protocols, ComboBox, N_("Engine Protocol:")                                                },
 {0,   0,        0,   NULL, (void *)&engineDir,      NULL,              NULL,      PathName, N_("Engine Directory:")                                               },
 {0,   LR,       0,   NULL, NULL,                    NULL,              NULL,      Label,    N_("(Directory will be derived from engine path when empty)")         },
/*{0,  0,        0,   NULL, (void *)&isUCI,          NULL,              NULL,      CheckBox, N_("UCI")                                                             },*/
/*{0,  0,        0,   NULL, (void *)&isUSI,          NULL,              NULL,      CheckBox, N_("USI/UCCI (uses specified -uxiAdapter)")                           },*/
/*{0,  0,        0,   NULL, (void *)&v1,             NULL,              NULL,      CheckBox, N_("WB protocol v1 (do not wait for engine features)")                },*/
 {0,   0,        0,   NULL, (void *)&InstallOK,      "",                NULL,      EndMark,  ""                                                                    }
};

static int InstallOK(int n) {
    if (n && (n = SelectedListBoxItem(&installOptions[1])) > 0) {
        /* called by pressing OK, and engine selected */
        free_then_strdup(&engineLine, engineList[n]);
    } else {
        switch (values[12]) {
        case 0:
            isUCI = 3;
            break;
        case 2:
            isUCI = 1;
            break;
        case 3:
            isUSI = 1;
            break;
        case 4:
            v1 = 1;
        case 1:
            break;
        }
    }
    /* Early popdown, to allow FreezeUI to instate grab. */
    PopDown(TransientDlg);
    if (isUSI) {
        /* Kludge to pass isUSI to Load(). */
        isUCI = 2;
        /* TODO: Consider whether we should ensure that -uxiAdapter is defined. */
        if (!*appData.ucciAdapter) {
            free_then_strdup(&appData.ucciAdapter, "usi2wb -%variant \"%fcp\"\"%fd\"");
        }
    }
    if (!secondEng) {
        Load(&first, 0);
    } else {
        Load(&second, 1);
    }
    /* No double pop-down! */
    return FALSE;
}

static void EngSel(int n, int sel) {
    int nr;
    char buf[MSG_SIZ];
    if (sel < 1) {
        /* back to top level */
        buf[0] = NULLCHAR;
    } else if (engineList[sel][0] == '#') {
        /* group header, open group */
        safeStrCpy(buf, engineList[sel], MSG_SIZ);
    } else {
        /* normal line, select engine */
        free_then_strdup(&engineLine, engineList[sel]);
        InstallOK(0);
        return;
    }
    /* replace list by only the group contents */
    nr = NamesToList(firstChessProgramNames, engineList, engineMnemonic, buf);
    free_then_strdup(&engineMnemonic[0], buf);
    LoadListBox(&installOptions[1], _("# no engines are installed"), -1, -1);
    HighlightWithScroll(&installOptions[1], 0, nr);
}

static void LoadEngineProc(int engineNr, char * title) {
    int p = appData.defProtocol;
    if (*engineListFile) {
        /* Contains engine list. */
        ParseSettingsFile(engineListFile, &engineListFile);
    }
    if (p >= 0 && p < 5) {
        protocolChoice = protocols[p];
    }

    isUCI = FALSE;
    isUSI = FALSE;
    storeVariant = FALSE;
    v1 = FALSE;
    useNick = FALSE;
    addToList = TRUE;
    hasBook = TRUE;

    secondEng = engineNr;
    if (engineLine) {
        free(engineLine);
    }
    engineLine = strdup("");
    if (engineDir) {
        free(engineDir);
    }
    engineDir = strdup(appData.defEngDir);
    if (nickName) {
        free(nickName);
    }
    nickName = strdup("");
    if (params) {
        free(params);
    }
    params = strdup("");
    free_then_strdup(&engineMnemonic[0], "");
    NamesToList(firstChessProgramNames, engineList, engineMnemonic, "");
    GenericPopUp(installOptions, title, TransientDlg, BoardWindow, MODAL, 0);
}

void LoadEngine1Proc(void) { LoadEngineProc(0, _("Load first engine")); }

void LoadEngine2Proc(void) { LoadEngineProc(1, _("Load second engine")); }

/* ----------------------------------------------------- Edit Book ----------------------------------------- */

void EditBookProc(void) { EditBookEvent(); }

/* --------------------------------------------------- New Shuffle Game ------------------------------ */

static void SetRandom(int n);

static int ShuffleOK(int n) {
    ResetGameEvent();
    return 1;
}

static Option shuffleOptions[] = {
 {0, 0,        0,          NULL, (void *)&shuffleOpenings,            NULL, NULL, CheckBox, N_("shuffle")               },
 {0, 0,        0,          NULL, (void *)&appData.fischerCastling,    NULL, NULL, CheckBox, N_("Fischer castling")      },
 {0, -1,       2000000000, NULL, (void *)&appData.defaultFrcPosition, "",   NULL, Spin,     N_("Start-position number:")},
 {0, 0,        0,          NULL, (void *)&SetRandom,                  NULL, NULL, Button,   N_("randomize")             },
 {0, SAME_ROW, 0,          NULL, (void *)&SetRandom,                  NULL, NULL, Button,   N_("pick fixed")            },
 {0, SAME_ROW, 0,          NULL, (void *)&ShuffleOK,                  "",   NULL, EndMark,  ""                          }
};

static void SetRandom(int n) {
    int r = n == 3 ? -1 : random() & (1 << 30) - 1;
    char buf[MSG_SIZ];
    snprintf(buf, MSG_SIZ, "%d", r);
    SetWidgetText(&shuffleOptions[2], buf, TransientDlg);
    SetWidgetState(&shuffleOptions[0], TRUE);
}

void ShuffleMenuProc(void) { GenericPopUp(shuffleOptions, _("New Shuffle Game"), TransientDlg, BoardWindow, MODAL, 0); }

/*--------------------------------------------------- Fonts ------------------------------ */

static void AdjustFont(int n);

static char * oldFont[7];

/* Figures out if the font changed, and if so, stores it in the fonts table as a side effect. */
static int NewFont(int n, int fnr, char * font) {
    int fontChanged = strcmp(oldFont[n], font) ? 1 : 0;
    if (fontChanged) {
        free_then_strdup(&fontTable[fnr][initialSquareSize], font);
        fontIsSet[fnr] = fontValid[fnr][initialSquareSize] = TRUE;
    }
    return fontChanged;
}

static int FontsOK(int n) {
    int i;
    /* Early popdown to prevent expose events from masking each other. */
    PopDown(TransientDlg);
    LockBoardSize(0);
    if (NewFont(0, CLOCK_FONT, appData.clockFont)) {
        DisplayBothClocks();
    }
    if (NewFont(1, MESSAGE_FONT, appData.font)) {
        ApplyFont(&mainOptions[W_MESSG], NULL);
        for (i = 1; i < 6; i++) {
            ApplyFont(&mainOptions[W_BUTTON + i], NULL);
        }
    }
    /* Unlock. */
    LockBoardSize(1);
    if (NewFont(3, EDITTAGS_FONT, appData.tagsFont)) {
        ApplyFont(&tagsOptions[1], NULL);
    }
    if (NewFont(4, COMMENT_FONT, appData.commentFont)) {
        ApplyFont(&commentOptions[0], NULL);
    }
    if (NewFont(5, MOVEHISTORY_FONT, appData.historyFont)) {
        ApplyFont(&historyOptions[0], NULL);
        ApplyFont(&engoutOptions[5], NULL);
        ApplyFont(&engoutOptions[12], NULL);
    }
    if (NewFont(6, GAMELIST_FONT, appData.gameListFont)) {
        ApplyFont(&gamesOptions[0], NULL);
    }
    if (NewFont(2, CONSOLE_FONT, appData.icsFont)) {
        ApplyFont(&chatOptions[11], appData.icsFont);
        /* Kludge to replace the font tag. */
        AppendColorized(&chatOptions[6], NULL, 0);
    }
    /* For coord font. */
    DrawPosition(TRUE, NULL);
    /* Suppress the normal popdown, because it is already done. */
    return 0;
}

static Option fontOptions[] = {
 {0,   60,       200, NULL, (void *)&appData.clockFont,    NULL, NULL, TextBox, N_("Clocks (requires restart):")                                  },
 {1,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("+")                                                           },
 {2,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("-")                                                           },
 {3,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("B")                                                           },
 {4,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("I")                                                           },
 {666, SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("*")                                                           },
 {0,   60,       70,  NULL, (void *)&appData.font,         NULL, NULL, TextBox, N_("Message (above board):")                                      },
 {1,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("+")                                                           },
 {2,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("-")                                                           },
 {3,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("B")                                                           },
 {4,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("I")                                                           },
 {666, SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("*")                                                           },
 {0,   60,       70,  NULL, (void *)&appData.icsFont,      NULL, NULL, TextBox, N_("ICS Chat/Console:")                                           },
 {1,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("+")                                                           },
 {2,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("-")                                                           },
 {3,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("B")                                                           },
 {4,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("I")                                                           },
 {666, SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("*")                                                           },
 {0,   60,       70,  NULL, (void *)&appData.tagsFont,     NULL, NULL, TextBox, N_("Edit tags / book / engine list:")                             },
 {1,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("+")                                                           },
 {2,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("-")                                                           },
 {3,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("B")                                                           },
 {4,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("I")                                                           },
 {666, SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("*")                                                           },
 {0,   60,       70,  NULL, (void *)&appData.commentFont,  NULL, NULL, TextBox, N_("Edit comments:")                                              },
 {1,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("+")                                                           },
 {2,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("-")                                                           },
 {3,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("B")                                                           },
 {4,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("I")                                                           },
 {666, SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("*")                                                           },
 {0,   60,       70,  NULL, (void *)&appData.historyFont,  NULL, NULL, TextBox, N_("Move history / Engine Output:")                               },
 {1,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("+")                                                           },
 {2,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("-")                                                           },
 {3,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("B")                                                           },
 {4,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("I")                                                           },
 {666, SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("*")                                                           },
 {0,   60,       70,  NULL, (void *)&appData.gameListFont, NULL, NULL, TextBox, N_("Game list:")                                                  },
 {1,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("+")                                                           },
 {2,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("-")                                                           },
 {3,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("B")                                                           },
 {4,   SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("I")                                                           },
 {666, SAME_ROW, 0,   NULL, (void *)&AdjustFont,           NULL, NULL, Button,  N_("*")                                                           },
 {0,   0,        0,   NULL, NULL,                          NULL, NULL, Label,   N_("\nThe * buttons will set the font to the one selected below:")},
 {0,   0,        0,   NULL, NULL,                          NULL, NULL, Button,  "fontsel"                                                         },
 {0,   0,        0,   NULL, (void *)&FontsOK,              "",   NULL, EndMark, ""                                                                }
};

static char name[MSG_SIZ], *bold, *ital, points;

static void BreakUp(char * font) {
    char *p = name, *norm;
    safeStrCpy(name, font, MSG_SIZ);
    bold = StrCaseStr(name, "bold");
    ital = StrCaseStr(name, "ital");
    norm = StrCaseStr(name, "normal");
    points = 0;
    while (p && *p && !(points = atoi(p))) {
        p = strchr(p + 1, ' ');
    }
    if (points) {
        p[*p == ' '] = 0;
    }
    if (bold) {
        *bold = 0;
    }
    if (ital) {
        *ital = 0;
    }
    if (norm) {
        *norm = 0;
    }
}

static void Collect(void) {
    if (bold) {
        strcat(name, "Bold ");
    }
    if (ital) {
        strcat(name, "Italic ");
    }
    if (!ital && !bold && strlen(name) < 2) {
        strncpy(name, "Normal ", MSG_SIZ);
    }
    if (points) {
        sprintf(name + strlen(name), "%d", points);
    } else {
        strcat(name, "%d");
    }
}

static void AdjustFont(int n) {
    int button = fontOptions[n].value, base = n - button;
    char * oldFont;
    GetWidgetText(&fontOptions[base], &oldFont);
    /* take apart old font name */
    BreakUp(oldFont);
    switch (button) {
    case 1:
        points++;
        break;
    case 2:
        points--;
        break;
    case 3:
        if (bold) {
            bold = NULL;
        } else {
            bold = name;
        }
        break;
    case 4:
        if (ital) {
            ital = NULL;
        } else {
            ital = name;
        }
        break;
    }
    Collect();
    SetWidgetText(&fontOptions[base], name, TransientDlg);
    ApplyFont(&fontOptions[base], name);
}

void FontsProc(void) {
    int i;
    if (strstr(appData.font, "-*-")) {
        DisplayNote(_("This only works in the GTK build"));
        return;
    }
    GenericPopUp(fontOptions, _("Fonts"), TransientDlg, BoardWindow, MODAL, 0);
    for (i = 0; i < 7; i++) {
        ApplyFont(&fontOptions[6 * i], *(char **)fontOptions[6 * i].target);
        free_then_strdup(&oldFont[i], *(char **)fontOptions[6 * i].target);
    }
}

/*------------------------------------------------------ Time Control -----------------------------------*/

static int TcOK(int n);
int tmpMoves, tmpTc, tmpInc, tmpOdds1, tmpOdds2, tcType, by60;

static void SetTcType(int n);

static char * Value(int n) {
    static char buf[MSG_SIZ];
    snprintf(buf, MSG_SIZ, "%d", n);
    return buf;
}

static Option tcOptions[] = {
 {0, 0,        0,     NULL, (void *)&SetTcType, NULL, NULL, Button,   N_("classical")                   },
 {0, SAME_ROW, 0,     NULL, (void *)&SetTcType, NULL, NULL, Button,   N_("incremental")                 },
 {0, SAME_ROW, 0,     NULL, (void *)&SetTcType, NULL, NULL, Button,   N_("fixed max")                   },
 {0, 0,        0,     NULL, (void *)&by60,      "",   NULL, CheckBox, N_("Divide entered times by 60")  },
 {0, 0,        200,   NULL, (void *)&tmpMoves,  NULL, NULL, Spin,     N_("Moves per session:")          },
 {0, 0,        10000, NULL, (void *)&tmpTc,     NULL, NULL, Spin,     N_("Initial time (min):")         },
 {0, 0,        10000, NULL, (void *)&tmpInc,    NULL, NULL, Spin,     N_("Increment or max (sec/move):")},
 {0, 0,        0,     NULL, NULL,               NULL, NULL, Label,    N_("Time-Odds factors:")          },
 {0, 1,        1000,  NULL, (void *)&tmpOdds1,  NULL, NULL, Spin,     N_("Engine #1")                   },
 {0, 1,        1000,  NULL, (void *)&tmpOdds2,  NULL, NULL, Spin,     N_("Engine #2 / Human")           },
 {0, 0,        0,     NULL, (void *)&TcOK,      "",   NULL, EndMark,  ""                                }
};

static int TcOK(int n) {
    char *tc, buf[MSG_SIZ];
    if (tcType == 0 && tmpMoves <= 0) {
        return 0;
    }
    if (tcType == 2 && tmpInc <= 0) {
        return 0;
    }
    /* get original text, in case it is min:sec */
    GetWidgetText(&tcOptions[5], &tc);
    if (by60) {
        snprintf(buf, MSG_SIZ, "%d:%02d", tmpTc / 60, tmpTc % 60), tc = buf;
    }
    searchTime = 0;
    switch (tcType) {
    case 0:
        if (!ParseTimeControl(tc, -1, tmpMoves)) {
            return 0;
        }
        appData.movesPerSession = tmpMoves;
        free_then_strdup(&appData.timeControl, tc);
        appData.timeIncrement = -1;
        break;
    case 1:
        if (!ParseTimeControl(tc, tmpInc, 0)) {
            return 0;
        }
        free_then_strdup(&appData.timeControl, tc);
        appData.timeIncrement = (by60 ? tmpInc / 60. : tmpInc);
        break;
    case 2:
        searchTime = (by60 ? tmpInc / 60 : tmpInc);
    }
    appData.firstTimeOdds = first.timeOdds = tmpOdds1;
    appData.secondTimeOdds = second.timeOdds = tmpOdds2;
    Reset(TRUE, TRUE);
    return 1;
}

static void SetTcType(int n) {
    switch (tcType = n) {
    case 0:
        SetWidgetText(&tcOptions[4], Value(tmpMoves), TransientDlg);
        SetWidgetText(&tcOptions[5], Value(tmpTc), TransientDlg);
        SetWidgetText(&tcOptions[6], _("Unused"), TransientDlg);
        break;
    case 1:
        SetWidgetText(&tcOptions[4], _("Unused"), TransientDlg);
        SetWidgetText(&tcOptions[5], Value(tmpTc), TransientDlg);
        SetWidgetText(&tcOptions[6], Value(tmpInc), TransientDlg);
        break;
    case 2:
        SetWidgetText(&tcOptions[4], _("Unused"), TransientDlg);
        SetWidgetText(&tcOptions[5], _("Unused"), TransientDlg);
        SetWidgetText(&tcOptions[6], Value(tmpInc), TransientDlg);
    }
}

void TimeControlProc(void) {
    if (gameMode != BeginningOfGame) {
        DisplayError(_("Changing time control during a game is not implemented"), 0);
        return;
    }
    tmpMoves = appData.movesPerSession;
    tmpInc = appData.timeIncrement;
    if (tmpInc < 0) {
        tmpInc = 0;
    }
    tmpOdds1 = tmpOdds2 = 1;
    tcType = 0;
    tmpTc = atoi(appData.timeControl);
    by60 = 0;
    GenericPopUp(tcOptions, _("Time Control"), TransientDlg, BoardWindow, MODAL, 0);
    SetTcType(searchTime ? 2 : appData.timeIncrement < 0 ? 0 : 1);
}

/*------------------------------- Ask Question -----------------------------------------*/

int SendReply(int n);
char pendingReplyPrefix[MSG_SIZ];
ProcRef pendingReplyPR;
char * answer;

Option askOptions[] = {
 {0, 0, 0, NULL, NULL,               NULL, NULL, Label,   NULL},
 {0, 0, 0, NULL, (void *)&answer,    "",   NULL, TextBox, ""  },
 {0, 0, 0, NULL, (void *)&SendReply, "",   NULL, EndMark, ""  }
};

int SendReply(int n) {
    char buf[MSG_SIZ];
    int err;
    char * reply = answer;
    /*GetWidgetText(&askOptions[1], &reply);*/
    safeStrCpy(buf, pendingReplyPrefix, sizeof(buf) / sizeof(buf[0]));
    if (*buf) {
        strncat(buf, " ", MSG_SIZ - strlen(buf) - 1);
    }
    strncat(buf, reply, MSG_SIZ - strlen(buf) - 1);
    strncat(buf, "\n", MSG_SIZ - strlen(buf) - 1);
    /* does not go into debug file??? => bug */
    OutputToProcess(pendingReplyPR, buf, strlen(buf), &err);
    if (err) {
        DisplayFatalError(_("Error writing to chess program"), err, 0);
    }
    return TRUE;
}

void AskQuestion(char * title, char * question, char * replyPrefix, ProcRef pr) {
    safeStrCpy(pendingReplyPrefix, replyPrefix, sizeof(pendingReplyPrefix) / sizeof(pendingReplyPrefix[0]));
    pendingReplyPR = pr;
    free_then_strdup(&answer, "");
    askOptions[0].name = question;
    if (GenericPopUp(askOptions, title, AskDlg, BoardWindow, MODAL, 0)) {
        AddHandler(&askOptions[1], AskDlg, 2);
    }
}

/*---------------------------- Promotion Popup --------------------------------------*/

static int count;

static void PromoPick(int n);

static Option promoOptions[] = {
 {0, 0,                0, NULL, (void *)&PromoPick, NULL, NULL, Button,  NULL},
 {0, SAME_ROW,         0, NULL, (void *)&PromoPick, NULL, NULL, Button,  NULL},
 {0, SAME_ROW,         0, NULL, (void *)&PromoPick, NULL, NULL, Button,  NULL},
 {0, SAME_ROW,         0, NULL, (void *)&PromoPick, NULL, NULL, Button,  NULL},
 {0, SAME_ROW,         0, NULL, (void *)&PromoPick, NULL, NULL, Button,  NULL},
 {0, SAME_ROW,         0, NULL, (void *)&PromoPick, NULL, NULL, Button,  NULL},
 {0, SAME_ROW,         0, NULL, (void *)&PromoPick, NULL, NULL, Button,  NULL},
 {0, SAME_ROW,         0, NULL, (void *)&PromoPick, NULL, NULL, Button,  NULL},
 {0, SAME_ROW | NO_OK, 0, NULL, NULL,               "",   NULL, EndMark, ""  }
};

static void PromoPick(int n) {
    int promoChar = promoOptions[n + count].value;

    PopDown(PromoDlg);

    if (promoChar == 0) {
        fromX = -1;
    }
    if (fromX == -1) {
        return;
    }

    if (!promoChar) {
        fromX = fromY = -1;
        ClearHighlights();
        return;
    }
    if (promoChar == '=' && !IS_SHOGI(gameInfo.variant)) {
        promoChar = NULLCHAR;
    }
    UserMoveEvent(fromX, fromY, toX, toY, promoChar);

    if (!appData.highlightLastMove || gotPremove) {
        ClearHighlights();
    }
    if (gotPremove) {
        SetPremoveHighlights(fromX, fromY, toX, toY);
    }
    fromX = fromY = -1;
}

static void SetPromo(char * name, int nr, char promoChar) {
    free_then_strdup(&promoOptions[nr].name, name);
    promoOptions[nr].value = promoChar;
    promoOptions[nr].min = SAME_ROW;
}

void PromotionPopUp(char choice) {
    /* choice depends on variant: prepare dialog acordingly */
    count = 8;
    /* Beware: GenericPopUp cannot handle user buttons named "cancel" (lowe case)! */
    SetPromo(_("Cancel"), --count, -1);
    if (choice != '+' && !IS_SHOGI(gameInfo.variant)) {
        if (!appData.testLegality || gameInfo.variant == VariantSuicide ||
         gameInfo.variant == VariantSpartan && !WhiteOnMove(currentMove) || gameInfo.variant == VariantGiveaway) {
            SetPromo(_("King"), --count, 'k');
        }
        if (gameInfo.variant == VariantSpartan && !WhiteOnMove(currentMove)) {
            SetPromo(_("Captain"), --count, 'c');
            SetPromo(_("Lieutenant"), --count, 'l');
            SetPromo(_("General"), --count, 'g');
            SetPromo(_("Warlord"), --count, 'w');
        } else {
            SetPromo(_("Knight"), --count, 'n');
            SetPromo(_("Bishop"), --count, 'b');
            SetPromo(_("Rook"), --count, 'r');
            if (gameInfo.variant == VariantCapablanca || gameInfo.variant == VariantGothic ||
             gameInfo.variant == VariantCapaRandom) {
                SetPromo(_("Archbishop"), --count, 'a');
                SetPromo(_("Chancellor"), --count, 'c');
            }
            SetPromo(_("Queen"), --count, 'q');
            if (gameInfo.variant == VariantChuChess) {
                SetPromo(_("Lion"), --count, 'l');
            }
        }
    } else /* [HGM] shogi */ {
        SetPromo(_("Defer"), --count, '=');
        SetPromo(_("Promote"), --count, '+');
    }
    promoOptions[count].min = 0;
    GenericPopUp(promoOptions + count, "Promotion", PromoDlg, BoardWindow, NONMODAL, 0);
}

/*---------------------------- Chat Windows ----------------------------------------------*/

static char *line, *memo, *chatMemo, *partner, *texts[MAX_CHAT], dirty[MAX_CHAT], *inputs[MAX_CHAT], *icsLine, *tmpLine;
static int activePartner;
int hidden = 1;

void ChatSwitch(int n);
int ChatOK(int n);

#define CHAT_ICS 6
#define CHAT_PARTNER 8
#define CHAT_OUT 11
#define CHAT_PANE 12
#define CHAT_IN 13

void PaneSwitch(void);
void ClearChat(void);

WindowPlacement wpTextMenu;

/* Callback for ICS-output clicks: handles button 3, passes on other events. */
int ContextMenu(Option * opt, int button, int x, int y, char * text, int index) {
    int h;
    if (button == -3) {
        /* Suppress the default GTK context menu on up-click. */
        return TRUE;
    }
    if (button != 3) {
        return FALSE;
    }
    /* pre-existing selection in memo? */
    if (index == -1) {
        strncpy(clickedWord, text, MSG_SIZ);
    } else {
        /* Figure out what word was clicked. */
        char *start, *end;
        start = end = text + index;
        while (isalnum(*end)) {
            end++;
        }
        while (start > text && isalnum(start[-1])) {
            start--;
        }
        clickedWord[0] = NULLCHAR;
        if (end - start >= 80) {
            /* Intended for small words and numbers. */
            end = start + 80;
        }
        strncpy(clickedWord, start, end - start);
        clickedWord[end - start] = NULLCHAR;
    }
    /* Request auto-popdown of textmenu when we popped it up. */
    click = !shellUp[TextMenuDlg];
    /* Use the remembered height of text menu. */
    h = wpTextMenu.height;
    if (h <= 0) {
        /* When unavailable, position with respect to the top. */
        h = 65;
    }
    GetPlacement(ChatDlg, &wpTextMenu);
    if (opt->target == (void *)&chatMemo) {
        /* Click in chat. */
        wpTextMenu.y += (wpTextMenu.height - 30) / 2;
    }
    wpTextMenu.x += x - 50;
    wpTextMenu.y += y - h + 50;
    if (wpTextMenu.x < 0) {
        wpTextMenu.x = 0;
    }
    if (wpTextMenu.y < 0) {
        wpTextMenu.y = 0;
    }
    wpTextMenu.width = wpTextMenu.height = -1;
    IcsTextPopUp();
    return TRUE;
}

Option chatOptions[] = {
 {0,   0,                                 0,   NULL, NULL,                NULL, NULL,                 Label,   N_("Chats:")       },
 {1,   SAME_ROW | TT,                     75,  NULL, (void *)&ChatSwitch, NULL, NULL,                 Button,  N_("New Chat")     },
 {2,   SAME_ROW | TT,                     75,  NULL, (void *)&ChatSwitch, NULL, NULL,                 Button,  N_("New Chat")     },
 {3,   SAME_ROW | TT,                     75,  NULL, (void *)&ChatSwitch, NULL, NULL,                 Button,  N_("New Chat")     },
 {4,   SAME_ROW | TT,                     75,  NULL, (void *)&ChatSwitch, NULL, NULL,                 Button,  N_("New Chat")     },
 {5,   SAME_ROW | TT,                     75,  NULL, (void *)&ChatSwitch, NULL, NULL,                 Button,  N_("New Chat")     },
 {250, T_VSCRL | T_FILL | T_WRAP | T_TOP, 510, NULL, (void *)&memo,       NULL, (void *)&ContextMenu, TextBox, ""                 },
 {0,   0,                                 0,   NULL, NULL,                "",   NULL,                 Break,   ""                 },
 {0,   T_TOP,                             100, NULL, (void *)&partner,    NULL, NULL,                 TextBox, N_("Chat partner:")},
 {0,   SAME_ROW,                          0,   NULL, (void *)&ClearChat,  NULL, NULL,                 Button,  N_("End Chat")     },
 {0,   SAME_ROW,                          0,   NULL, (void *)&PaneSwitch, NULL, NULL,                 Button,  N_("Hide")         },
 {250, T_VSCRL | T_FILL | T_WRAP | T_TOP, 510, NULL, (void *)&chatMemo,   NULL, (void *)&ContextMenu, TextBox, ""                 },
 {0,   0,                                 0,   NULL, NULL,                "",   NULL,                 Break,   ""                 },
 {0,   0,                                 510, NULL, (void *)&line,       NULL, NULL,                 TextBox, ""                 },
 {0,   NO_OK | SAME_ROW,                  0,   NULL, (void *)&ChatOK,     NULL, NULL,                 EndMark, ""                 }
};

static void PutText(char * text, int pos) {
    char buf[MSG_SIZ], *p;
    DialogClass dlg = ChatDlg;
    Option * opt = &chatOptions[CHAT_IN];

    if (strstr(text, "$add ") == text) {
        GetWidgetText(&boxOptions[INPUT], &p);
        snprintf(buf, MSG_SIZ, "%s%s", p, text + 5);
        text = buf;
        pos += strlen(p) - 5;
    }
    if (shellUp[InputBoxDlg]) {
        /* For the benefit of Xaw, give priority to ICS Input Box. */
        opt = &boxOptions[INPUT], dlg = InputBoxDlg;
    }
    SetWidgetText(opt, text, dlg);
    SetInsertPos(opt, pos);
    HardSetFocus(opt, dlg);
    CursorAtEnd(opt);
}

/* [HGM] input: let up-arrow recall previous line from history */
int IcsHist(int n, Option * opt, DialogClass dlg) {
    char * val = NULL;
    int chat, start;

    if (opt != &chatOptions[CHAT_IN] && !(opt == &chatOptions[CHAT_PARTNER] && n == 33)) {
        return 0;
    }
    switch (n) {
    case 5:
        if (!hidden) {
            ClearChat();
        }
        break;
    case 8:
        if (!hidden) {
            PaneSwitch();
        }
        break;
    /* <Esc> */
    case 33:
        if (1) {
            BoardToTop();
        } else if (hidden) {
            BoardToTop();
        } else {
            PaneSwitch();
        }
        break;
    case 15:
        NewChat(lastTalker);
        break;
    case 14:
        for (chat = 0; chat < MAX_CHAT; chat++) {
            if (!chatPartner[chat][0]) {
                break;
            }
        }
        if (chat < MAX_CHAT) {
            ChatSwitch(chat + 1);
        }
        break;
    /* <Tab> */
    case 10:
        chat = start = (activePartner - hidden + MAX_CHAT) % MAX_CHAT;
        while (!dirty[chat = (chat + 1) % MAX_CHAT]) {
            if (chat == start) {
                break;
            }
        }
        if (!dirty[chat]) {
            while (!chatPartner[chat = (chat + 1) % MAX_CHAT][0]) {
                if (chat == start) {
                    break;
                }
            }
        }
        if (!chatPartner[chat][0]) {
            /* if all unused, ignore */
            break;
        }
        ChatSwitch(chat + 1);
        break;
    case 1:
        GetWidgetText(opt, &val);
        val = PrevInHistory(val);
        break;
    case -1:
        val = NextInHistory();
    }
    SetWidgetText(opt, val = val ? val : "", dlg);
    SetInsertPos(opt, strlen(val));
    return 1;
}

void OutputChatMessage(int partner, char * mess) {
    char * p = texts[partner];
    int len = strlen(mess) + 1;

    if (!DialogExists(ChatDlg)) {
        return;
    }
    if (p) {
        len += strlen(p);
    }
    texts[partner] = (char *)malloc(len);
    snprintf(texts[partner], len, "%s%s", p ? p : "", mess);
    free(p);
    if (partner == activePartner && !hidden) {
        AppendText(&chatOptions[CHAT_OUT], mess);
        SetInsertPos(&chatOptions[CHAT_OUT], len - 2);
    } else {
        SetColor("#FFC000", &chatOptions[partner + 1]);
        dirty[partner] = 1;
    }
}

int ChatOK(int n) {
    /* Can only be called through <Enter> in chat-partner text-edit, as there is no OK button. */
    char buf[MSG_SIZ];

    if (!hidden && (!partner || strcmp(partner, chatPartner[activePartner]) || !*partner)) {
        safeStrCpy(chatPartner[activePartner], partner, MSG_SIZ);
        /* Clear text if we alter partner. */
        SetWidgetText(&chatOptions[CHAT_OUT], "", -1);
        SetWidgetText(&chatOptions[CHAT_IN], "", ChatDlg);
        SetWidgetLabel(&chatOptions[activePartner + 1], chatPartner[activePartner][0] ? chatPartner[activePartner] : _("New Chat"));
        if (!*partner) {
            PaneSwitch();
        }
        HardSetFocus(&chatOptions[CHAT_IN], 0);
    }
    if (line[0] || hidden) {
        /* something was typed (for ICS commands we also allow empty line!) */
        SetWidgetText(&chatOptions[CHAT_IN], "", ChatDlg);
        /* From here on, it could be back-end. */
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = NULLCHAR;
        }
        SaveInHistory(line);
        /* Command for ICS. */
        if (hidden || !*chatPartner[activePartner]) {
            snprintf(buf, MSG_SIZ, "%s\n", line);
            if (!remoteEchoOption) {
                ConsoleWrite(buf, strlen(buf));
            }
        } else if (!strcmp("whispers", chatPartner[activePartner])) {
            /* WHISPER box uses "whisper" to send. */
            snprintf(buf, MSG_SIZ, "whisper %s\n", line);
        } else if (!strcmp("shouts", chatPartner[activePartner])) {
            /* SHOUT box uses "shout" to send. */
            snprintf(buf, MSG_SIZ, "shout %s\n", line);
        } else if (!strcmp("c-shouts", chatPartner[activePartner])) {
            /* C-SHOUT box uses "cshout" to send. */
            snprintf(buf, MSG_SIZ, "cshout %s\n", line);
        } else if (!strcmp("kibitzes", chatPartner[activePartner])) {
            /* KIBITZ box uses "kibitz" to send. */
            snprintf(buf, MSG_SIZ, "kibitz %s\n", line);
        } else {
            if (!atoi(chatPartner[activePartner])) {
                /* Echo only tells to handle, not channel. */
                snprintf(buf, MSG_SIZ, "> %s\n", line);
                OutputChatMessage(activePartner, buf);
                snprintf(buf, MSG_SIZ, "xtell %s %s\n", chatPartner[activePartner], line);
            } else {
                snprintf(buf, MSG_SIZ, "tell %s %s\n", chatPartner[activePartner], line);
            }
        }
        SendToICS(buf);
    }
    /* Never pop down. */
    return FALSE;
}

void DelayedSetText(void) {
    /* leave focus on chat-partner field! */
    SetWidgetText(&chatOptions[CHAT_IN], tmpLine, -1);
    SetInsertPos(&chatOptions[CHAT_IN], strlen(tmpLine));
}

/* If we do this immediately it does it before shrinking the memo, so the lower half remains hidden (Ughh!) */
void DelayedScroll(void) {
    SetInsertPos(&chatOptions[CHAT_ICS], 999999);
    SetWidgetText(&chatOptions[CHAT_IN], tmpLine, ChatDlg);
    SetInsertPos(&chatOptions[CHAT_IN], strlen(tmpLine));
}

void ChatSwitch(int n) {
    int i;
    int j;
    char * v;
    if (chatOptions[CHAT_ICS].type == Skip) {
        /* In Xaw there is no ICS pane we can hide behind */
        hidden = 0;
    }
    Show(&chatOptions[CHAT_PANE], 0);
    if (hidden) {
        /* Awful! */
        ScheduleDelayedEvent(DelayedScroll, 50);
    } else {
        ScheduleDelayedEvent(DelayedSetText, 50);
    }
    GetWidgetText(&chatOptions[CHAT_IN], &v);
    if (hidden) {
        free_then_strdup(&icsLine, v);
    } else {
        free_then_strdup(&inputs[activePartner], v);
    }
    hidden = 0;
    activePartner = --n;
    if (!texts[n]) {
        texts[n] = strdup("");
    }
    dirty[n] = 0;
    SetWidgetText(&chatOptions[CHAT_OUT], texts[n], ChatDlg);
    SetInsertPos(&chatOptions[CHAT_OUT], strlen(texts[n]));
    SetWidgetText(&chatOptions[CHAT_PARTNER], chatPartner[n], ChatDlg);
    for (i = j = 0; i < MAX_CHAT; i++) {
        SetWidgetLabel(&chatOptions[++j], *chatPartner[i] ? chatPartner[i] : _("New Chat"));
        SetColor(dirty[i] ? "#FFC000" : "#FFFFFF", &chatOptions[j]);
    }
    if (!inputs[n]) {
        free_then_strdup(&inputs[n], "");
    }
    /*SetWidgetText(&chatOptions[CHAT_IN], inputs[n], ChatDlg); /* does not work (in this widget only) */
    /*SetInsertPos(&chatOptions[CHAT_IN], strlen(inputs[n]));*/
    /* for the delayed event */
    tmpLine = inputs[n];
    HardSetFocus(&chatOptions[strcmp(chatPartner[n], "") ? CHAT_IN : CHAT_PARTNER], 0);
}

void PaneSwitch(void) {
    char * v;
    Show(&chatOptions[CHAT_PANE], hidden = 1);  /* hide */
    GetWidgetText(&chatOptions[CHAT_IN], &v);
    free_then_strdup(&inputs[activePartner], v);
    if (!icsLine) {
        free_then_strdup(&icsLine, "");
    }
    tmpLine = icsLine;
    ScheduleDelayedEvent(DelayedSetText, 50);
    /*SetWidgetText(&chatOptions[CHAT_IN], icsLine, ChatDlg); /* does not work (in this widget only) */
    /*SetInsertPos(&chatOptions[CHAT_IN], strlen(icsLine));*/
}

/* clear the chat to make it free for other use */
void ClearChat(void) {
    chatPartner[activePartner][0] = NULLCHAR;
    free_then_strdup(&texts[activePartner], "");
    free_then_strdup(&inputs[activePartner], "");
    SetWidgetText(&chatOptions[CHAT_PARTNER], "", ChatDlg);
    SetWidgetText(&chatOptions[CHAT_OUT], "", ChatDlg);
    SetWidgetText(&chatOptions[CHAT_IN], "", ChatDlg);
    SetWidgetLabel(&chatOptions[activePartner + 1], _("New Chat"));
    HardSetFocus(&chatOptions[CHAT_PARTNER], 0);
}

/* open a chat on program request. If no empty one available, use last */
static void NewChat(char * name) {
    int i;
    for (i = 0; i < MAX_CHAT - 1; i++) {
        if (!chatPartner[i][0]) {
            break;
        }
    }
    safeStrCpy(chatPartner[i], name, MSG_SIZ);
    ChatSwitch(i + 1);
}

void ConsoleWrite(char * message, int count) {
    /* in Xaw this is a no-op */
    if (shellUp[ChatDlg] && chatOptions[CHAT_ICS].type != Skip) {
        if (*message == 7) {
            /* remove bell */
            message++;
            if (strcmp(message, "\n")) {
                return;
            }
        }
        AppendColorized(&chatOptions[CHAT_ICS], message, count);
        SetInsertPos(&chatOptions[CHAT_ICS], 999999);
    }
}

void ChatPopUp(void) {
    if (GenericPopUp(chatOptions, _("ICS Interaction"), ChatDlg, BoardWindow, NONMODAL, appData.topLevel)) {
        /* treats return as OK */
        AddHandler(&chatOptions[CHAT_PARTNER], ChatDlg, 2), AddHandler(&chatOptions[CHAT_IN], ChatDlg, 2);
    }
    /* hide */
    Show(&chatOptions[CHAT_PANE], hidden = 1);
    /*HardSetFocus(&chatOptions[CHAT_IN], 0);*/
    MarkMenu("View.OpenChatWindow", ChatDlg);
    CursorAtEnd(&chatOptions[CHAT_IN]);
}

void ChatProc(void) {
    if (shellUp[ChatDlg]) {
        PopDown(ChatDlg);
    } else {
        ChatPopUp();
    }
}

void ConsoleAutoPopUp(char * buf) {
    if (*buf == 27) {
        if (appData.icsActive && DialogExists(ChatDlg)) {
            HardSetFocus(&chatOptions[CHAT_IN], ChatDlg);
        }
        return;
    }
    if (!appData.autoBox) {
        return;
    }
    if (appData.icsActive) {
        /* text typed to board in ICS mode: divert to ICS input box */
        if (DialogExists(ChatDlg)) {
            /* box already exists: append to current contents */
            char * p;
            char newText[MSG_SIZ];
            GetWidgetText(&chatOptions[CHAT_IN], &p);
            snprintf(newText, MSG_SIZ, "%s%c", p, *buf);
            SetWidgetText(&chatOptions[CHAT_IN], newText, ChatDlg);
            if (shellUp[ChatDlg]) {
                /* TODO: why is this done here? */
                HardSetFocus(&chatOptions[CHAT_IN], ChatDlg);
            }
        } else {
            /* box did not exist: make sure it pops up with char in it */
            free_then_strdup(&line, buf);
        }
        ChatPopUp();
    } else {
        PopUpMoveDialog(*buf);
    }
}

void EchoOn(void) {
    if (!noEcho) {
        return;
    }
    system("stty echo");
    WidgetEcho(&chatOptions[CHAT_IN], 1);
    noEcho = FALSE;
}

void EchoOff(void) {
    system("stty -echo");
    WidgetEcho(&chatOptions[CHAT_IN], 0);
    noEcho = TRUE;
}

/*--------------------------------- Game-List options dialog ------------------------------------------*/

char * strings[LPUSERGLT_SIZE];
int stringPtr;

void GLT_ClearList(void) {
    strings[0] = NULL;
    stringPtr = 0;
}

void GLT_AddToList(char * name) {
    strings[stringPtr++] = name;
    strings[stringPtr] = NULL;
}

Boolean GLT_GetFromList(int index, char * name) {
    safeStrCpy(name, strings[index], MSG_SIZ);
    return TRUE;
}

void GLT_DeSelectList(void) {}

static void GLT_Button(int n);
static int GLT_OK(int n);

static Option listOptions[] = {
 /* For GTK we need to specify a height, as default would just show 3 lines */
 {300, LR | TB,  200, NULL, (void *)strings,     NULL, NULL, ListBox, ""           },
 {0,   0,        0,   NULL, (void *)&GLT_Button, NULL, NULL, Button,  N_("factory")},
 {0,   SAME_ROW, 0,   NULL, (void *)&GLT_Button, NULL, NULL, Button,  N_("up")     },
 {0,   SAME_ROW, 0,   NULL, (void *)&GLT_Button, NULL, NULL, Button,  N_("down")   },
 {0,   SAME_ROW, 0,   NULL, (void *)&GLT_OK,     "",   NULL, EndMark, ""           }
};

static int GLT_OK(int n) {
    GLT_ParseList();
    appData.gameListTags = strdup(lpUserGLT);
    GameListUpdate();
    return 1;
}

static void GLT_Button(int n) {
    int index = SelectedListBoxItem(&listOptions[0]);
    char * p;
    if (index < 0) {
        DisplayError(_("No tag selected"), 0);
        return;
    }
    p = strings[index];
    if (n == 3) {
        if (index >= strlen(GLT_ALL_TAGS)) {
            return;
        }
        strings[index] = strings[index + 1];
        strings[++index] = p;
        /* only change the two specified entries */
        LoadListBox(&listOptions[0], "?", index, index - 1);
    } else if (n == 2) {
        if (index == 0) {
            return;
        }
        strings[index] = strings[index - 1];
        strings[--index] = p;
        LoadListBox(&listOptions[0], "?", index, index + 1);
    } else if (n == 1) {
        safeStrCpy(lpUserGLT, GLT_DEFAULT_TAGS, LPUSERGLT_SIZE);
        GLT_TagsToList(lpUserGLT);
        index = 0;
        LoadListBox(&listOptions[0], "?", -1, -1);
    }
    HighlightListBoxItem(&listOptions[0], index);
}

void GameListOptionsPopUp(DialogClass parent) {
    safeStrCpy(lpUserGLT, appData.gameListTags, LPUSERGLT_SIZE);
    GLT_TagsToList(lpUserGLT);

    GenericPopUp(listOptions, _("Game-list options"), TransientDlg, parent, MODAL, 0);
}

void GameListOptionsProc(void) { GameListOptionsPopUp(BoardWindow); }

/*----------------------------- Error popup in various uses -----------------------------*/

/* [HGM] Note: XBoard has always had some pathologic behavior with multiple simultaneous error popups, (which can occur even for
   modal popups when asynchrounous events, e.g. caused by engine, request a popup), and this new implementation reproduces that as
   well: Only the shell of the last instance is remembered in shells[ErrorDlg] (which replaces errorShell), so that PopDowns ordered
   from the code always refer to that instance, and once that is down, have no clue as to how to reach the others. For the Delete
   Window button calling PopDown this has now been repaired, as the action routine assigned to it gets the shell passed as
   argument. */
int errorUp = FALSE;

void ErrorPopDown(void) {
    if (!errorUp) {
        return;
    }
    dialogError = FALSE;
    errorUp = FALSE;
    /* On an explicit request, we pop down any error dialog. */
    PopDown(ErrorDlg);
    PopDown(FatalDlg);
    if (errorExitStatus != -1) {
        ExitEvent(errorExitStatus);
    }
}

int ErrorOK(int n) {
    dialogError = FALSE;
    errorUp = FALSE;
    /* Kludge: non-modal dialogs have one less (dummy) option. */
    PopDown(n == 1 ? FatalDlg : ErrorDlg);
    if (errorExitStatus != -1) {
        ExitEvent(errorExitStatus);
    }
    /* Prevent a second pop-down. */
    return FALSE;
}

static Option errorOptions[] = {
 {0, 0,         0, NULL, NULL,             NULL, NULL, Label,   NULL}, /* dummy option: will never be displayed */
 {0, 0,         0, NULL, NULL,             NULL, NULL, Label,   NULL}, /* textValue field will be set before popup */
 {0, NO_CANCEL, 0, NULL, (void *)&ErrorOK, "",   NULL, EndMark, ""  }
};

void ErrorPopUp(char * title, char * label, int modal) {
    errorUp = TRUE;
    errorOptions[1].name = label;
    if (dialogError = shellUp[TransientDlg]) {
        /* Pop up as daughter of the transient dialog. */
        GenericPopUp(errorOptions + 1, title, FatalDlg, TransientDlg, MODAL, 0);
    } else if (dialogError = shellUp[MasterDlg]) {
        /* Pop up as daughter of the master dialog. */
        GenericPopUp(errorOptions + 1, title, FatalDlg, MasterDlg, MODAL, 0);
    } else {
        /* Kludge: option start address indicates modality. */
        GenericPopUp(errorOptions + modal, title, modal ? FatalDlg : ErrorDlg, BoardWindow, modal, 0);
    }
}

void DisplayError(String message, int error) {
    char buf[MSG_SIZ];

    if (error == 0) {
        if (appData.debugMode || appData.matchMode) {
            fprintf(stderr, "%s: %s\n", programName, message);
        }
    } else {
        if (appData.debugMode || appData.matchMode) {
            fprintf(stderr, "%s: %s: %s\n", programName, message, strerror(error));
        }
        snprintf(buf, sizeof(buf), "%s: %s", message, strerror(error));
        message = buf;
    }
    ErrorPopUp(_("Error"), message, FALSE);
}


void DisplayMoveError(String message) {
    fromX = -1;
    fromY = -1;
    ClearHighlights();
    /* Selective redraw would miss the from-square of the rejected move, displayed empty after drag, but not marked damaged! */
    DrawPosition(TRUE, NULL);

    if (appData.debugMode || appData.matchMode) {
        fprintf(stderr, "%s: %s\n", programName, message);
    }
    if (appData.popupMoveErrors) {
        ErrorPopUp(_("Error"), message, FALSE);
    } else {
        DisplayMessage(message, "");
    }
}


void DisplayFatalError(String message, int error, int status) {
    char buf[MSG_SIZ], logout = appData.icsActive;

    if (status == 666) {
        /* Ignore this error when ICS Console window is up. */
        if (shellUp[ChatDlg]) {
            return;
        }
        status = 0;
    } else if (status == 6666) {
        /* 6666 is a kludge that indicates that the ICS connection already closed. */
        status = 0;
        logout = 0;
    }

    errorExitStatus = status;
    if (error == 0) {
        fprintf(stderr, "%s: %s\n", programName, message);
    } else {
        fprintf(stderr, "%s: %s: %s\n", programName, message, strerror(error));
        snprintf(buf, sizeof(buf), "%s: %s", message, strerror(error));
        message = buf;
    }
    if (mainOptions[W_BOARD].handle) {
        if (appData.popupExitMessage) {
            if (logout) {
                /* Logout so that no new games will be started. */
                SendToICS("logout\n");
            }
            ErrorPopUp(status ? _("Fatal Error") : _("Exiting"), message, TRUE);
        } else {
            ExitEvent(status);
        }
    }
}

void DisplayInformation(String message) {
    ErrorPopDown();
    ErrorPopUp(_("Information"), message, TRUE);
}

void DisplayNote(String message) {
    ErrorPopDown();
    ErrorPopUp(_("Note"), message, FALSE);
}

void DisplayTitle(char * text) {
    char title[MSG_SIZ];
    char icon[MSG_SIZ];

    if (text == NULL) {
        text = "";
    }

    if (partnerUp) {
        SetDialogTitle(DummyDlg, text);
        return;
    }

    if (*text != NULLCHAR) {
        safeStrCpy(icon, text, sizeof(icon) / sizeof(icon[0]));
        safeStrCpy(title, text, sizeof(title) / sizeof(title[0]));
    } else if (appData.icsActive) {
        snprintf(icon, sizeof(icon), "%s", appData.icsHost);
        snprintf(title, sizeof(title), "%s: %s", programName, appData.icsHost);
    } else if (appData.cmailGameName[0] != NULLCHAR) {
        snprintf(icon, sizeof(icon), "%s", "CMail");
        snprintf(title, sizeof(title), "%s: %s", programName, "CMail");
#ifdef GOTHIC
        /* [HGM] license: This stuff should really be done in back-end, but WinBoard already had a pop-up for it */
    } else if (gameInfo.variant == VariantGothic) {
        safeStrCpy(icon, programName, sizeof(icon) / sizeof(icon[0]));
        safeStrCpy(title, GOTHIC, sizeof(title) / sizeof(title[0]));
#endif
#ifdef FALCON
    } else if (gameInfo.variant == VariantFalcon) {
        safeStrCpy(icon, programName, sizeof(icon) / sizeof(icon[0]));
        safeStrCpy(title, FALCON, sizeof(title) / sizeof(title[0]));
#endif
    } else if (appData.noChessProgram) {
        safeStrCpy(icon, programName, sizeof(icon) / sizeof(icon[0]));
        safeStrCpy(title, programName, sizeof(title) / sizeof(title[0]));
    } else {
        safeStrCpy(icon, first.tidy, sizeof(icon) / sizeof(icon[0]));
        snprintf(title, sizeof(title), "%s: %s", programName, first.tidy);
    }
    SetWindowTitle(text, title, icon);
}

char * textPtr;
char * texEscapes[] = {"s-1", "s0", "&", "*(L", "*(R", NULL};

int GetNext(FILE * f) {
    if (textPtr) {
        return *textPtr ? *textPtr++ : EOF;
    }
    return fgetc(f);
}

static char * ReadLine(FILE * f) {
    static char buf[MSG_SIZ];
    int i = 0, c;
    while ((c = GetNext(f)) != '\n') {
        if (c == EOF) {
            return NULL;
        }
        buf[i++] = c;
    }
    buf[i] = NULLCHAR;
    return buf;
}

void GetHelpText(FILE * f, char * name) {
    char *line, buf[MSG_SIZ], title[MSG_SIZ], text[10000], *p = text, *q = text;
    int len, cnt = 0;
    while (*name == '\n') {
        name++;
    }
    snprintf(buf, MSG_SIZ, ".B %s", name);
    len = strlen(buf);
    for (len = 3; buf[len] && buf[len] != '(' && buf[len] != ':' && buf[len] != '.' && buf[len] != '?' && buf[len] != '\n'; len++)
        ;
    buf[len] = NULLCHAR;
    while (buf[--len] == ' ') {
        buf[len] = NULLCHAR;
    }
    len++;
    snprintf(title, MSG_SIZ, "Help on '%s'", buf + 3);
    while ((line = ReadLine(f))) {
        if (!strncmp(line, buf, len) || !strncmp(line, ".SS ", 4) && !strncmp(line + 4, buf + 3, len - 3) ||
         !strncmp(line, ".IX Item \"", 10) && !strncmp(line + 10, buf + 3, len - 3)) {
            while ((line = ReadLine(f)) &&
             (cnt == 0 || strncmp(line, ".B ", 3) && strncmp(line, ".SS ", 4) && strncmp(line, ".IX ", 4))) {
                if (!*line) {
                    *p++ = '\n';
                    *p++ = '\n';
                    q = p;
                    continue;
                }
                if (*line == '.') {
                    continue;
                }
                *p++ = ' ';
                cnt++;
                while (*line) {
                    if (*line < ' ') {
                        line++;
                        continue;
                    }
                    if (*line == '\\') {
                        char ** esc;
                        line++;
                        for (esc = texEscapes; *esc; esc++) {
                            len = strlen(*esc);
                            if (!strncmp(*esc, line, len)) {
                                line += len;
                                break;
                            }
                        }
                        continue;
                    }
                    if (*line == ' ' && p - q > 80) {
                        *line = '\n', q = p;
                    }
                    *p++ = *line++;
                }
                if (p - text > 9900) {
                    break;
                }
            }
            *p = NULLCHAR;
            ErrorPopUp(title, text, FALSE);
            return;
        }
    }
    snprintf(text, MSG_SIZ, "No help available on '%s'\n", buf + 3);
    DisplayNote(text);
}

void DisplayHelp(char * name) {
    static char *xboardMan, *manText[2], tidy[MSG_SIZ], engMan[MSG_SIZ];
    char buf[MSG_SIZ], adapter[MSG_SIZ], *eng;
    int n = 0;
    FILE * f;
    if (!xboardMan) {
        /* obtain path to XBoard's man file */
        xboardMan = BufferCommandOutput("man -w xboard", MSG_SIZ);
        if (xboardMan) {
            /* strip off traling linefeed */
            xboardMan[strlen(xboardMan) - 1] = NULLCHAR;
        }
    }
    if (currentCps) {
        /* for engine options we have to look in engine manual */
        /* get (tidied) engine name in buf */
        snprintf(buf, MSG_SIZ, "man -w ");
        /* name of binary we are actually running */
        TidyProgramName(currentCps->program, "localhost", adapter);
        TidyProgramName(currentCps == &first ? appData.firstChessProgram : appData.secondChessProgram, "localhost", buf + 7);
        if (strcmp(buf + 7, adapter) && StrCaseStr(name, adapter) == name) {
            /* option starts with name of apparent proxy for engine */
            /* use adapter manual */
            safeStrCpy(buf + 7, adapter, MSG_SIZ - 7);
            /* strip adapter name of option */
            name += strlen(adapter);
            while (*name == ' ') {
                name++;
            }
        }
        if (strcmp(buf, tidy)) {
            /* It is a different engine than the one from last time, so any currently-held text is worthless. */
            free(manText[1]);
            manText[1] = NULL;
            /* remember current engine */
            safeStrCpy(tidy, buf, MSG_SIZ);
            /* obtain path to its man file */
            eng = BufferCommandOutput(tidy, MSG_SIZ);
            if (*eng) {
                /* and remember that too */
                safeStrCpy(engMan, eng, strlen(eng));
            } else {
                *engMan = NULLCHAR;
            }
            free(eng);
        }
        safeStrCpy(buf, engMan, MSG_SIZ);
        /* use engine man */
        n = 1;
    } else {
        /* use xboard man */
        snprintf(buf, MSG_SIZ, "%s", xboardMan);
    }
    f = fopen(buf, "r");
    if (f) {
        char * msg = "Right-clicking menu item or dialog text pops up help on it";
        free_then_strdup(&appData.suppress, msg);
        if (strstr(buf, ".gz")) {
            /* man file is gzipped */
            if (!manText[n]) {
                /* unzipped text not buffered yet */
                snprintf(tidy, MSG_SIZ, "gunzip -c %s", buf);
                /* store unzipped in buffer */
                manText[n] = BufferCommandOutput(tidy, 250000);
            }
            /* use buffered unzipped text */
            textPtr = manText[n];
        } else {
            /* use plaintext man file directly */
            textPtr = NULL;
        }
        GetHelpText(f, name);
        fclose(f);
    } else if (currentCps) {
        DisplayNote("No manual is installed for this engine");
    }
}

#define PAUSE_BUTTON "P"
#define PIECE_MENU_SIZE 18
static String pieceMenuStrings[2][PIECE_MENU_SIZE + 1] = {
 {N_("White"), "----", N_("Pawn"), N_("Knight"), N_("Bishop"), N_("Rook"), N_("Queen"), N_("King"), "----", N_("Elephant"),
  N_("Cannon"), N_("Archbishop"), N_("Chancellor"), "----", N_("Promote"), N_("Demote"), N_("Empty square"), N_("Clear board"),
  NULL},
 {N_("Black"), "----", N_("Pawn"), N_("Knight"), N_("Bishop"), N_("Rook"), N_("Queen"), N_("King"), "----", N_("Elephant"),
  N_("Cannon"), N_("Archbishop"), N_("Chancellor"), "----", N_("Promote"), N_("Demote"), N_("Empty square"), N_("Clear board"),
  NULL}
};
/* must be in same order as pieceMenuStrings! */
static ChessSquare pieceMenuTranslation[2][PIECE_MENU_SIZE] = {
 {WhitePlay, (ChessSquare)0, WhitePawn, WhiteKnight, WhiteBishop, WhiteRook, WhiteQueen, WhiteKing, (ChessSquare)0, WhiteAlfil,
  WhiteCannon, WhiteAngel, WhiteMarshall, (ChessSquare)0, PromotePiece, DemotePiece, EmptySquare, ClearBoard},
 {BlackPlay, (ChessSquare)0, BlackPawn, BlackKnight, BlackBishop, BlackRook, BlackQueen, BlackKing, (ChessSquare)0, BlackAlfil,
  BlackCannon, BlackAngel, BlackMarshall, (ChessSquare)0, PromotePiece, DemotePiece, EmptySquare, ClearBoard},
};

#define DROP_MENU_SIZE 6
static String dropMenuStrings[DROP_MENU_SIZE + 1] = {"----", N_("Pawn"), N_("Knight"), N_("Bishop"), N_("Rook"), N_("Queen"), NULL};
/* must be in same order as dropMenuStrings! */
static ChessSquare dropMenuTranslation[DROP_MENU_SIZE] = {
 (ChessSquare)0, WhitePawn, WhiteKnight, WhiteBishop, WhiteRook, WhiteQueen};

/* [HGM] experimental code to pop up window just like the main window, using GenercicPopUp */

static Option * Exp(int n, int x, int y);
void MenuCallback(int n);
void SizeKludge(int n);
static Option * LogoW(int n, int x, int y);
static Option * LogoB(int n, int x, int y);

static int pmFromX = -1, pmFromY = -1;
void * userLogo;

void DisplayLogos(Option * w1, Option * w2) {
    void *whiteLogo = first.programLogo, *blackLogo = second.programLogo;
    if (appData.autoLogo) {
        if (appData.noChessProgram) {
            whiteLogo = blackLogo = NULL;
        }
        if (appData.icsActive) {
            whiteLogo = blackLogo = second.programLogo;
        }
        switch (gameMode) {
        /* pick logos based on game mode */
        case IcsObserving:
            /* ICS logo */
            whiteLogo = second.programLogo;
            blackLogo = second.programLogo;
        default:
            break;
        case IcsPlayingWhite:
            if (!appData.zippyPlay) {
                whiteLogo = userLogo;
            }
            /* ICS logo */
            blackLogo = second.programLogo;
            break;
        case IcsPlayingBlack:
            /* ICS logo */
            whiteLogo = second.programLogo;
            blackLogo = appData.zippyPlay ? first.programLogo : userLogo;
            break;
        case TwoMachinesPlay:
            if (first.twoMachinesColor[0] == 'b') {
                whiteLogo = second.programLogo;
                blackLogo = first.programLogo;
            }
            break;
        case MachinePlaysWhite:
            blackLogo = userLogo;
            break;
        case MachinePlaysBlack:
            whiteLogo = userLogo;
            blackLogo = first.programLogo;
        }
    }
    DrawLogo(w1, whiteLogo);
    DrawLogo(w2, blackLogo);
}

/* user callback for board context menus */
static void PMSelect(int n) {
    if (pmFromX < 0 || pmFromY < 0) {
        return;
    }
    if (n == W_DROP) {
        DropMenuEvent(dropMenuTranslation[values[n]], pmFromX, pmFromY);
    } else {
        EditPositionMenuEvent(pieceMenuTranslation[n - W_MENUW][values[n]], pmFromX, pmFromY);
    }
}

static void CCB(int n) {
    shiftKey = (ShiftKeys() & 3) != 0;
    if (n < 0) {  /* button != 1 */
        n = -n;
        if (shiftKey && (gameMode == MachinePlaysWhite || gameMode == MachinePlaysBlack)) {
            AdjustClock(n == W_BLACK, 1);
        }
    } else {
        ClockClick(n == W_BLACK);
    }
}

Option mainOptions[] = {
 /* description of main window in terms of generic dialog creator */
 {0, 0xca, 0, NULL, NULL, "", NULL, BarBegin, ""}, /* menu bar */
 {0, COMBO_CALLBACK, 0, NULL, (void *)&MenuCallback, NULL, NULL, DropDown, N_("_File")},
 {0, COMBO_CALLBACK, 0, NULL, (void *)&MenuCallback, NULL, NULL, DropDown, N_("_Edit")},
 {0, COMBO_CALLBACK, 0, NULL, (void *)&MenuCallback, NULL, NULL, DropDown, N_("_View")},
 {0, COMBO_CALLBACK, 0, NULL, (void *)&MenuCallback, NULL, NULL, DropDown, N_("_Mode")},
 {0, COMBO_CALLBACK, 0, NULL, (void *)&MenuCallback, NULL, NULL, DropDown, N_("_Action")},
 {0, COMBO_CALLBACK, 0, NULL, (void *)&MenuCallback, NULL, NULL, DropDown, N_("E_ngine")},
 {0, COMBO_CALLBACK, 0, NULL, (void *)&MenuCallback, NULL, NULL, DropDown, N_("_Options")},
 {0, COMBO_CALLBACK, 0, NULL, (void *)&MenuCallback, NULL, NULL, DropDown, N_("_Help")},
 {0, 0, 0, NULL, (void *)&SizeKludge, "", NULL, BarEnd, ""},
 {0, LR | T2T | BORDER | SAME_ROW, 0, NULL, NULL, NULL, NULL, Label, "1"}, /* optional title in window */
 {50, LL | TT, 100, NULL, (void *)&LogoW, NULL, NULL, Skip, ""}, /* white logo */
 {12, L2L | T2T, 200, NULL, (void *)&CCB, NULL, NULL, Label, "White"}, /* white clock */
 {13, R2R | T2T | SAME_ROW, 200, NULL, (void *)&CCB, NULL, NULL, Label, "Black"}, /* black clock */
 {50, RR | TT | SAME_ROW, 100, NULL, (void *)&LogoB, NULL, NULL, Skip, ""}, /* black logo */
 {0, LR | T2T | BORDER, 401, NULL, NULL, "", NULL, Skip, "2"}, /* backup for title in window (if no room for other) */
 {0, LR | T2T | BORDER, 270, NULL, NULL, NULL, NULL, Label, "message", &appData.font}, /* message field */
 {0, RR | TT | SAME_ROW, 125, NULL, NULL, "", NULL, BoxBegin, ""}, /* (optional) button bar */
 {0, 0, 0, NULL, (void *)&ToStartEvent, NULL, NULL, Button, N_("<<"), &appData.font},
 {0, SAME_ROW, 0, NULL, (void *)&BackwardEvent, NULL, NULL, Button, N_("<"), &appData.font},
 {0, SAME_ROW, 0, NULL, (void *)&PauseEvent, NULL, NULL, Button, N_(PAUSE_BUTTON), &appData.font},
 {0, SAME_ROW, 0, NULL, (void *)&ForwardEvent, NULL, NULL, Button, N_(">"), &appData.font},
 {0, SAME_ROW, 0, NULL, (void *)&ToEndEvent, NULL, NULL, Button, N_(">>"), &appData.font},
 {0, 0, 0, NULL, NULL, "", NULL, BoxEnd, ""},
 {401, LR | TB, 401, NULL, (char *)&Exp, NULL, NULL, Graph, "shadow board"}, /* board */
 {2, COMBO_CALLBACK, 0, NULL, (void *)&PMSelect, NULL, pieceMenuStrings[0], PopUp, "menuW"},
 {2, COMBO_CALLBACK, 0, NULL, (void *)&PMSelect, NULL, pieceMenuStrings[1], PopUp, "menuB"},
 {-1, COMBO_CALLBACK, 0, NULL, (void *)&PMSelect, NULL, dropMenuStrings, PopUp, "menuD"},
 {0, NO_OK, 0, NULL, NULL, "", NULL, EndMark, ""}
};

Option * LogoW(int n, int x, int y) {
    if (n == 10) {
        DisplayLogos(&mainOptions[W_WHITE - 1], NULL);
    }
    return NULL;
}

Option * LogoB(int n, int x, int y) {
    if (n == 10) {
        DisplayLogos(NULL, &mainOptions[W_BLACK + 1]);
    }
    return NULL;
}

/* callback called by GenericPopUp immediately after sizing the menu bar */
void SizeKludge(int n) {
    int width = desired_board_dimension_in_pixels(BOARD_WIDTH, squareSize, lineGap);
    int w = width - 44 - mainOptions[n].min;
    /* width left behind menu bar */
    mainOptions[W_TITLE].max = w;
    /* if no reasonable amount of space for title, force small layout */
    if (w < 0.4 * width) {
        mainOptions[W_SMALL].type = mainOptions[W_TITLE].type, mainOptions[W_TITLE].type = Skip;
    }
}

void MenuCallback(int n) {
    MenuProc * proc = (MenuProc *)(((MenuItem *)(mainOptions[n].choice))[values[n]].proc);

    if (!proc) {
        RecentEngineEvent(values[n] - firstEngineItem);
    } else {
        (proc)();
    }
}

static Option * Exp(int n, int x, int y) {
    static int but1, but3, oldW, oldH, oldX, oldY;
    int menuNr = -3, sizing, f, r;
    TimeMark now;
    extern Boolean right;

    /* Kludgy way to let button 1 double as button 3 when the back-end requests this. */
    if (right) {
        if (but1 && n == 0) {
            but1 = 0;
            but3 = 1;
        } else if (n == -1) {
            n = -3;
            right = FALSE;
        }
    }

    /* Motion? */
    if (n == 0) {
        oldX = x;
        oldY = y;
        if (SeekGraphClick(Press, x, y, 1)) {
            return NULL;
        }
        if ((but1 || dragging == 2) && !PromoScroll(x, y)) {
            DragPieceMove(x, y);
        }
        if (but3) {
            MovePV(x, y, desired_board_dimension_in_pixels(BOARD_HEIGHT, squareSize, lineGap));
        }
        if (appData.highlightDragging) {
            f = EventToSquare(x, BOARD_WIDTH);
            if (flipView && f >= 0) {
                f = BOARD_WIDTH - 1 - f;
            }
            r = EventToSquare(y, BOARD_HEIGHT);
            if (!flipView && r >= 0) {
                r = BOARD_HEIGHT - 1 - r;
            }
            HoverEvent(x, y, f, r);
        }
        return NULL;
    }
    if (n != 10 && PopDown(PromoDlg)) {
        /* User starts fiddling with board when promotion dialog is up. */
        fromX = -1;
        fromY = -1;
    } else {
        GetTimeMark(&now);
    }
    shiftKey = ShiftKeys();
    controlKey = (shiftKey & 0xc) != 0;
    shiftKey = (shiftKey & 3) != 0;
    switch (n) {
    case 1:
        LeftClick(Press, x, y), but1 = 1;
        break;
    case -1:
        LeftClick(Release, x, y), but1 = 0;
        break;
    case 2:
        shiftKey = !shiftKey;
        /* Intentionally fall through. */
    case 3:
        menuNr = RightClick(Press, x, y, &pmFromX, &pmFromY), but3 = 1;
        break;
    case -2:
        shiftKey = !shiftKey;
        /* Intentionally fall through. */
    case -3:
        menuNr = RightClick(Release, x, y, &pmFromX, &pmFromY), but3 = 0;
        break;
    case 4:
        Wheel(-1, oldX, oldY);
        break;
    case 5:
        Wheel(1, oldX, oldY);
        break;
    case 10:
        sizing = (oldW != x || oldH != y);
        oldW = x;
        oldH = y;
        InitDrawingHandle(mainOptions + W_BOARD);
        if (sizing && SubtractTimeMarks(&now, &programStartTime) > 10000) {
            /* Except during program start-up, don't redraw while sizing. */
            return NULL;
        }
        DrawPosition(TRUE, NULL);
        /* Intentionally fall through. */
    default:
        return NULL;
    }

    switch (menuNr) {
    case 0:
        return &mainOptions[shiftKey ? W_MENUW : W_MENUB];
    case 1:
        SetupDropMenu();
        return &mainOptions[W_DROP];
    case 2:
    case -1:
        ErrorPopDown();
    case -2:
    default:
        /* -3, so no clicks caught. */
        break;
    }
    return NULL;
}

Option * BoardPopUp(int squareSize, int lineGap, void * clockFontThingy) {
    int size = desired_board_dimension_in_pixels(BOARD_WIDTH, squareSize, lineGap);
    int logo = appData.logoSize;
    /* width fudge, needed for unknown reasons to not clip board */
    int f = 2 * appData.fixedSize;
    int i;

    mainOptions[W_WHITE].choice = (char **)clockFontThingy;
    mainOptions[W_BLACK].choice = (char **)clockFontThingy;
    mainOptions[W_BOARD].value = desired_board_dimension_in_pixels(BOARD_HEIGHT, squareSize, lineGap);
    /* Board size. */
    mainOptions[W_BOARD].max = mainOptions[W_SMALL].max = size;
    /* Board title, with the border subtracted. */
    mainOptions[W_SMALL].max = size - 2;
    /* Clock width */
    mainOptions[W_BLACK].max = mainOptions[W_WHITE].max = size / 2 - 3;
    /* Message. */
    mainOptions[W_MESSG].max = appData.showButtonBar ? size - 135 + f : size - 2 + f;
    /* Menu bar. */
    mainOptions[W_MENU].max = size - 40;
    mainOptions[W_TITLE].type = appData.titleInWindow ? Label : Skip;
    if (logo && logo <= size / 4) {
        /* Activate logos. */
        mainOptions[W_WHITE - 1].type = mainOptions[W_BLACK + 1].type = Graph;
        mainOptions[W_WHITE - 1].max = mainOptions[W_BLACK + 1].max = logo;
        mainOptions[W_WHITE - 1].value = mainOptions[W_BLACK + 1].value = logo / 2;
        mainOptions[W_WHITE].min |= SAME_ROW;
        mainOptions[W_WHITE].max = mainOptions[W_BLACK].max -= logo + 4;
        mainOptions[W_WHITE].name = mainOptions[W_BLACK].name = "Double\nHeight";
    }
    if (!appData.showButtonBar) {
        for (i = W_BUTTON; i < W_BOARD; i++) {
            mainOptions[i].type = Skip;
        }
    }
    for (i = 0; i < 8; i++) {
        mainOptions[i + 1].choice = (char **)menuBar[i].mi;
    }
    AppendEnginesToMenu(appData.recentEngineList);
    mainOptions[W_BOARD].choice = NULL;
    /* Always top-level. */
    GenericPopUp(mainOptions, "XBoard", BoardWindow, BoardWindow, NONMODAL, 1);
    return mainOptions;
}

static Option * SecondaryBoardExposeCallbackFn(int n, int x, int y) {
    if (n == 10) {
        /* It's an expose event. */
        flipView = !flipView;
        partnerUp = !partnerUp;
        /* [HGM] dual: draw other board in other orientation. */
        DrawPosition(TRUE, NULL);
        flipView = !flipView;
        partnerUp = !partnerUp;
    }
    return NULL;
}

/* These are for the secondary board window. */
Option dualOptions[] = {
 {0,   L2L | T2T,            198, NULL, NULL, NULL, NULL, Label,   "White"               }, /* white clock */
 {0,   R2R | T2T | SAME_ROW, 198, NULL, NULL, NULL, NULL, Label,   "Black"               }, /* black clock */
 {0,   LR | T2T | BORDER,    401, NULL, NULL, NULL, NULL, Label,   "Experimental feature"}, /* message field */
 {401, LR | TT,              401, NULL, (char *)&SecondaryBoardExposeCallbackFn,
                                              NULL, NULL, Graph,   "Secondary board"     }, /* board */
 {0,   NO_OK,                0,   NULL, NULL, "",   NULL, EndMark, ""                    }
};

void SecondaryBoardPopUp(void) {
    int size = desired_board_dimension_in_pixels(BOARD_WIDTH, squareSize, lineGap);
    /* Copy parameters from the primary board. */
    dualOptions[0].choice = mainOptions[W_WHITE].choice;
    dualOptions[1].choice = mainOptions[W_BLACK].choice;
    dualOptions[3].value = desired_board_dimension_in_pixels(BOARD_HEIGHT, squareSize, lineGap);
    /* board width */
    dualOptions[3].max = dualOptions[2].max = size;
    /* clock width */
    dualOptions[0].max = dualOptions[1].max = size / 2 - 3;
    GenericPopUp(dualOptions, "XBoard", DummyDlg, BoardWindow, NONMODAL, appData.topLevel);
    SecondaryBoardResize(dualOptions + 3);
}

static char clockMsg[2][MSG_SIZ];

void DisplayWhiteClock(long timeRemaining, int highlight) {
    /* printing message prevails over printing color:time */
    int m = (clockMsg[0][0] != 0);
    if (appData.noGUI) {
        return;
    }
    if (twoBoards && partnerUp) {
        DisplayTimerLabel(&dualOptions[0], _("White"), timeRemaining, highlight);
        return;
    }
    DisplayTimerLabel(&mainOptions[W_WHITE], m ? clockMsg[0] : _("White"), timeRemaining, highlight + 2 * m);
    if (highlight) {
        SetClockIcon(0);
    }
}

void DisplayBlackClock(long timeRemaining, int highlight) {
    int m = (clockMsg[1][0] != 0);
    if (appData.noGUI) {
        return;
    }
    if (twoBoards && partnerUp) {
        DisplayTimerLabel(&dualOptions[1], _("Black"), timeRemaining, highlight);
        return;
    }
    DisplayTimerLabel(&mainOptions[W_BLACK], m ? clockMsg[1] : _("Black"), timeRemaining, highlight + 2 * m);
    if (highlight) {
        SetClockIcon(1);
    }
}

void SetClockMessage(int n, char * msg) { safeStrCpy(clockMsg[n], !msg ? "" : *msg ? msg : clockMsg[!n], MSG_SIZ); }

/*---------------------------------------------*/

void DisplayMessage(char * message, char * extMessage) {
    /* display a message in the message widget */

    char buf[MSG_SIZ];

    if (extMessage) {
        if (*message) {
            snprintf(buf, sizeof(buf), "%s  %s", message, extMessage);
            message = buf;
        } else {
            message = extMessage;
        };
    };

    /* [HGM] make available */
    safeStrCpy(lastMsg, message, MSG_SIZ);

    /* need to test if messageWidget already exists, since this function
       can also be called during the startup, if for example a Xresource
       is not set up correctly */
    if (mainOptions[W_MESSG].handle) {
        SetWidgetLabel(&mainOptions[W_MESSG], message);
    }

    return;
}

/*----------------------------------- File Browser -------------------------------*/

#ifdef HAVE_DIRENT_H
# include <dirent.h>
#else
# include <sys/dir.h>
# define dirent direct
#endif

#include <sys/stat.h>

#define MAXFILES 1000

static DialogClass savDlg;
static ChessProgramState * savCps;
static FILE ** savFP;
static char *fileName, *extFilter, *savMode, **namePtr;
static int folderPtr, filePtr, oldVal, byExtension, extFlag, pageStart, cnt;
static char curDir[MSG_SIZ], title[MSG_SIZ], *folderList[MAXFILES], *fileList[MAXFILES];

static char * FileTypes[] = {"Chess Games", "Chess Positions", "Tournaments", "Opening Books", "Sound files", "Images",
 "Settings (*.ini)", "Log files", "All files", NULL, "PGN", "Old-Style Games", "FEN", "Old-Style Positions", NULL, NULL};

static char * Extensions[] = {".pgn .game", ".fen .epd .pos", ".trn", ".bin", ".wav", ".png", ".ini", ".log", "", "INVALID", ".pgn",
 ".game", ".fen", ".pos", NULL, ""};

void DirSelProc(int n, int sel);
void FileSelProc(int n, int sel);
void SetTypeFilter(int n);
int BrowseOK(int n);
void Switch(int n);
void CreateDir(int n);

Option browseOptions[] = {
 {0,   LR | T2T,             500, NULL, NULL,                   NULL,                 NULL,      Label,    title              },
 {0,   L2L | T2T,            250, NULL, NULL,                   NULL,                 NULL,      Label,    N_("Directories:") },
 {0,   R2R | T2T | SAME_ROW, 100, NULL, NULL,                   NULL,                 NULL,      Label,    N_("Files:")       },
 {0,   R2R | TT | SAME_ROW,  70,  NULL, (void *)&Switch,        NULL,                 NULL,      Button,   N_("by name")      },
 {0,   R2R | TT | SAME_ROW,  70,  NULL, (void *)&Switch,        NULL,                 NULL,      Button,   N_("by type")      },
 {300, L2L | TB,             250, NULL, (void *)folderList,     (char *)&DirSelProc,  NULL,      ListBox,  ""                 },
 {300, R2R | TB | SAME_ROW,  250, NULL, (void *)fileList,       (char *)&FileSelProc, NULL,      ListBox,  ""                 },
 {0,   0,                    300, NULL, (void *)&fileName,      NULL,                 NULL,      TextBox,  N_("Filename:")    },
 {0,   SAME_ROW,             120, NULL, (void *)&CreateDir,     NULL,                 NULL,      Button,   N_("New directory")},
 {0,   COMBO_CALLBACK,       150, NULL, (void *)&SetTypeFilter, NULL,                 FileTypes, ComboBox, N_("File type:")   },
 {0,   SAME_ROW,             0,   NULL, (void *)&BrowseOK,      "",                   NULL,      EndMark,  ""                 }
};

int BrowseOK(int n) {
    if (!fileName[0]) {
        /* It is enough to have a file selected. */
        /* Kludge: if a callback is specified, we browse for a file, otherwise we browse for a path. */
        if (browseOptions[6].textValue) {
            int sel = SelectedListBoxItem(&browseOptions[6]);
            if (sel < 0 || sel >= filePtr) {
                return FALSE;
            }
            free_then_strdup(&fileName, fileList[sel]);
        } else {
            free_then_strdup(&fileName, curDir);
        }
    }
    if (!fileName[0]) {
        /* Refuse OK when no file. */
        return FALSE;
    }
    if (!savMode[0]) {
        /* Browsing for name only (dialog Browse button). */
        if (fileName[0] == '/') {
            /* We already had a path name. */
            snprintf(title, MSG_SIZ, "%s", fileName);
        } else {
            snprintf(title, MSG_SIZ, "%s/%s", curDir, fileName);
        }
        SetWidgetText((Option *)savFP, title, savDlg);
        /* Could return to Engine Settings dialog! */
        currentCps = savCps;
        return TRUE;
    }
    *savFP = fopen(fileName, savMode);
    if (*savFP == NULL) {
        /* Refuse OK if file not openable. */
        return FALSE;
    }
    free_then_strdup(namePtr, fileName);
    ScheduleDelayedEvent(DelayedLoad, 50);
    /* Not sure this is ever non-null. */
    currentCps = savCps;
    return TRUE;
}

int AlphaNumCompare(char * p, char * q) {
    while (*p) {
        if (isdigit(*p) && isdigit(*q) && atoi(p) != atoi(q)) {
            return (atoi(p) > atoi(q) ? 1 : -1);
        }
        if (*p != *q) {
            break;
        }
        p++, q++;
    }
    if (*p == *q) {
        return 0;
    }
    return (*p > *q ? 1 : -1);
}

int Comp(const void * s, const void * t) {
    char *p = *(char **)s, *q = *(char **)t;
    if (extFlag) {
        char * h;
        int r;
        while (h = strchr(p, '.')) {
            p = h + 1;
        }
        if (p == *(char **)s) {
            p = "";
        }
        while (h = strchr(q, '.')) {
            q = h + 1;
        }
        if (q == *(char **)t) {
            q = "";
        }
        r = AlphaNumCompare(p, q);
        if (r) {
            return r;
        }
    }
    return AlphaNumCompare(*(char **)s, *(char **)t);
}

void ListDir(int pathFlag) {
    DIR * dir;
    struct dirent * dp;
    struct stat statBuf;
    static int lastFlag;

    if (pathFlag < 0) {
        pathFlag = lastFlag;
    }
    lastFlag = pathFlag;
    dir = opendir(".");
    getcwd(curDir, MSG_SIZ);
    snprintf(title, MSG_SIZ, "%s   %s", _("Contents of"), curDir);
    /* clear listing */
    cnt = 0;
    filePtr = 0;
    folderPtr = 0;

    while (dp = readdir(dir)) {
        char * s = dp->d_name;
        if (!stat(s, &statBuf) && S_ISDIR(statBuf.st_mode)) {
            /* stat succeeds and tells us it is a directory */
            if (s[0] == '.' && strcmp(s, "..")) {
                /* suppress hidden directories, except ".." */
                continue;
            }
            free_then_strdup(&folderList[folderPtr], s);
            if (folderPtr < MAXFILES - 2) {
                folderPtr++;
            }
        } else if (!pathFlag) {
            char *s = dp->d_name, match = 0;
            if (s[0] == '.') {
                /* suppress hidden files */
                continue;
            }
            if (extFilter[0]) {
                /* [HGM] filter on extension */
                char *p = extFilter, *q;
                do {
                    if (q = strchr(p, ' ')) {
                        *q = 0;
                    }
                    if (strstr(s, p)) {
                        match++;
                    }
                    if (q) {
                        *q = ' ';
                    }
                } while (q && (p = q + 1));
                if (!match) {
                    continue;
                }
            }
            if (filePtr == MAXFILES - 2) {
                continue;
            }
            if (cnt++ < pageStart) {
                continue;
            }
            free_then_strdup(&fileList[filePtr], s);
            filePtr++;
        }
    }
    if (filePtr == MAXFILES - 2) {
        free_then_strdup(&fileList[filePtr], _("  next page"));
        filePtr++;
    }
    free(folderList[folderPtr]);
    folderList[folderPtr] = NULL;
    free(fileList[filePtr]);
    fileList[filePtr] = NULL;
    closedir(dir);
    extFlag = 0;
    qsort((void *)folderList, folderPtr, sizeof(char *), &Comp);
    extFlag = byExtension;
    qsort((void *)fileList, filePtr < MAXFILES - 2 ? filePtr : MAXFILES - 2, sizeof(char *), &Comp);
}

void Refresh(int pathFlag) {
    ListDir(pathFlag);
    LoadListBox(&browseOptions[5], "", -1, -1);
    LoadListBox(&browseOptions[6], "", -1, -1);
    SetWidgetLabel(&browseOptions[0], title);
}

static char msg1[] = N_("FIRST TYPE DIRECTORY NAME HERE");
static char msg2[] = N_("TRY ANOTHER NAME");

void CreateDir(int n) {
    char *name, *errmsg = "";
    GetWidgetText(&browseOptions[n - 1], &name);
    if (!strcmp(name, msg1) || !strcmp(name, msg2)) {
        return;
    }
    if (!name[0]) {
        errmsg = _(msg1);
    } else if (mkdir(name, 0755)) {
        errmsg = _(msg2);
    } else {
        chdir(name);
        Refresh(-1);
    }
    SetWidgetText(&browseOptions[n - 1], errmsg, BrowserDlg);
}

void Switch(int n) {
    if (byExtension == (n == 4)) {
        return;
    }
    extFlag = byExtension = (n == 4);
    qsort((void *)fileList, filePtr < MAXFILES - 2 ? filePtr : MAXFILES - 2, sizeof(char *), &Comp);
    LoadListBox(&browseOptions[6], "", -1, -1);
}

void SetTypeFilter(int n) {
    int j = values[n];
    if (j == browseOptions[n].value) {
        /* no change */
        return;
    }
    browseOptions[n].value = j;
    SetWidgetLabel(&browseOptions[n], FileTypes[j]);
    free_then_strdup(&extFilter, Extensions[j]);
    pageStart = 0;
    /* uses pathflag remembered by ListDir */
    Refresh(-1);
    /* do not disturb combo settings of underlying dialog */
    values[n] = oldVal;
}

void FileSelProc(int n, int sel) {
    if (sel < 0 || fileList[sel] == NULL) {
        return;
    }
    if (sel == MAXFILES - 2) {
        pageStart = cnt;
        Refresh(-1);
        return;
    }
    free_then_strdup(&fileName, fileList[sel]);
    if (BrowseOK(0)) {
        PopDown(BrowserDlg);
    }
}

void DirSelProc(int n, int sel) {
    if (!chdir(folderList[sel])) {
        /* cd succeeded, so we are in new directory now */
        Refresh(-1);
    }
}

void StartDir(char * filter, char * newName) {
    static char *gamesDir, *trnDir, *imgDir, *bookDir, *dirDir;
    static char curDir[MSG_SIZ];
    char ** res = NULL;
    if (!filter || !*filter) {
        return;
    }
    if (strstr(filter, "dir")) {
        res = &dirDir;
        if (!dirDir) {
            dirDir = strdup(dataDir);
        }
    } else if (strstr(filter, "pgn")) {
        res = &gamesDir;
    } else if (strstr(filter, "bin")) {
        res = &bookDir;
    } else if (strstr(filter, "png")) {
        res = &imgDir;
    } else if (strstr(filter, "trn")) {
        res = &trnDir;
    } else if (strstr(filter, "fen")) {
        res = &appData.positionDir;
    }
    if (res) {
        if (newName) {
            char *p, *q;
            if (*newName) {
                free_then_strdup(res, newName);
                for (p = *res; q = strchr(p, '/');) {
                    p = q + 1;
                }
                *p = NULLCHAR;
            }
        }
        if (*curDir) {
            chdir(curDir);
            *curDir = NULLCHAR;
        } else {
            getcwd(curDir, MSG_SIZ);
            if (*res && **res) {
                chdir(*res);
            }
        }
    }
}

void Browse(DialogClass dlg, char * label, char * proposed, char * ext, Boolean pathFlag, char * mode, char ** name, FILE ** fp) {
    int j = 0;
    savFP = fp;
    /* save params, for use in callback */
    savMode = mode;
    namePtr = name;
    savCps = currentCps;
    oldVal = values[9];
    savDlg = dlg;
    free_then_strdup(&extFilter, ext);
    free_then_strdup(&fileName, proposed ? proposed : "");
    /* look up actual value in list of possible values, to get selection nr */
    for (j = 0; Extensions[j]; j++) {
        if (extFilter && !strcmp(extFilter, Extensions[j])) {
            break;
        }
    }
    if (Extensions[j] == NULL) {
        j++;
        free_then_strdup(&FileTypes[j], extFilter);
    }
    browseOptions[9].value = j;
    /* disable file listbox during path browsing */
    browseOptions[6].textValue = (char *)(pathFlag ? NULL : &FileSelProc);
    pageStart = 0;
    ListDir(pathFlag);
    currentCps = NULL;
    GenericPopUp(browseOptions, label, BrowserDlg, dlg, MODAL, 0);
    SetWidgetLabel(&browseOptions[9], FileTypes[j]);
}

static char * openName;
FileProc fileProc;
char * fileOpenMode;
FILE * openFP;

void DelayedLoad(void) { (void)(*fileProc)(openFP, 0, openName); }

void FileNamePopUp(char * label, char * def, char * filter, FileProc proc, char * openMode) {
    /* A previous developer couldn't see a way not to use global variables here.  TODO: Investigate. */
    fileProc = proc;
    fileOpenMode = openMode;
    FileNamePopUpWrapper(label, def, filter, proc, FALSE, openMode, &openName, &openFP);
}

void ActivateTheme(int col) {
    if (appData.overrideLineGap >= 0) {
        lineGap = appData.overrideLineGap;
    } else {
        lineGap = defaultLineGap;
    }
    InitDrawingParams(strcmp(oldPieceDir, appData.pieceDirectory));
    InitDrawingSizes(-1, 0);
    DrawPosition(TRUE, NULL);
}

char * Shorten(char * s) {
    static char buf[MSG_SIZ];
    if (strstr(s, dataDir) != s) {
        return s;
    }
    snprintf(buf, MSG_SIZ, "~~%s", s + strlen(dataDir));
    return buf;
}
