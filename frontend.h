/*
 * frontend.h -- Interface exported by all XBoard front ends
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Free
 * Software Foundation, Inc.
 *
 * Enhancements Copyright 2005 Alessandro Scotti
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

#ifndef XB_FRONTEND
#define XB_FRONTEND

#include <stdio.h>

char *T_ (char *s);
void ModeHighlight (void);
void SetICSMode (void);
void SetGNUMode (void);
void SetNCPMode (void);
void SetCmailMode (void);
void SetTrainingModeOn (void);
void SetTrainingModeOff (void);
void SetUserThinkingEnables (void);
void SetMachineThinkingEnables (void);
void DisplayTitle (String title);
void DisplayMessage (String message, String extMessage);
void DisplayMoveError (String message);

void DisplayNote (String message);

void DisplayInformation (String message);
void AskQuestion (String title, String question, String replyPrefix,
                  ProcRef pr);
void DisplayIcsInteractionTitle (String title);
void ParseArgsFromString (char *p);
void ParseArgsFromFile (FILE *f);
void DrawPosition (int fullRedraw, Board board);
void DrawPositionX (int fullRedraw, Board board);
void ResetFrontEnd (void);
void NotifyFrontendLogin (void);
void CommentPopUp (String title, String comment);
void CommentPopDown (void);
void EditCommentPopUp (int index, String title, String text);
void ErrorPopDown (void);
int  EventToSquare (int x, int limit);
void DrawSeekAxis (int x, int y, int xTo, int yTo);
void DrawSeekBackground (int left, int top, int right, int bottom);
void DrawSeekText (char *buf, int x, int y);
void DrawSeekDot (int x, int y, int color);
void PopUpMoveDialog (char first);

void RingBell (void);
int  Roar (void);
void PlayIcsWinSound (void);
void PlayIcsLossSound (void);
void PlayIcsDrawSound (void);
void PlayIcsUnfinishedSound (void);
void PlayAlarmSound (void);
void PlayTellSound (void);
int  PlaySoundFile (char *name);
void PlaySoundByColor (void);
void EchoOn (void);
void EchoOff (void);
void Raw (void);
void Colorize (ColorClass cc, int continuation);
char *InterpretFileName (char *name, char *dir);
void DoSleep (int n);
void DoEvents (void);

char *UserName (void);
char *HostName (void);

int ClockTimerRunning (void);
int StopClockTimer (void);
void StartClockTimer (long millisec);
void DisplayWhiteClock (long timeRemaining, int highlight);
void DisplayBlackClock (long timeRemaining, int highlight);
void SetClockMessage (int color, char *msg);
void UpdateLogos (int display);

int LoadGameTimerRunning (void);
int StopLoadGameTimer (void);
void StartLoadGameTimer (long millisec);
void AutoSaveGame (void);
Boolean ParseSettingsFile (char *name, char **addr);

void ScheduleDelayedEvent (DelayedEventCallback cb, long millisec);
DelayedEventCallback GetDelayedEvent (void);
void CancelDelayedEvent (void);
// [HGM] mouse: next six used by mouse handler, which was moved to backend
extern int fromX, fromY, toX, toY;
void PromotionPopUp (char choice);
void DragPieceBegin (int x, int y, Boolean instantly);
void DragPieceEnd (int x, int y);
void DragPieceMove (int x, int y);
void LeftClick (ClickType c, int x, int y);
int  RightClick (ClickType c, int x, int y, int *col, int *row);
void Wheel (int dir, int x, int y);

int StartChildProcess (char *cmdLine, char *dir, ProcRef *pr, int priority);
void DestroyChildProcess (ProcRef pr, int/*boolean*/ signal);
void InterruptChildProcess (ProcRef pr);
char *BufferCommandOutput (char *command, int size);
void RunCommand (char *buf);

int OpenTelnet (char *host, char *port, ProcRef *pr);
int OpenTCP (char *host, char *port, ProcRef *pr);
int OpenCommPort (char *name, ProcRef *pr);
int OpenLoopback (ProcRef *pr);
int OpenRcmd (char *host, char *user, char *cmd, ProcRef *pr);

typedef void (*InputCallback) (InputSourceRef isr, void *closure,
				 char *buf, int count, int error);
/* pr == NoProc means the local keyboard */
InputSourceRef AddInputSource (ProcRef pr, int lineByLine,
				 InputCallback func, void *closure);
void RemoveInputSource (InputSourceRef isr);

/* pr == NoProc means the local display */
int OutputToProcess (ProcRef pr, char *message, int count, int *outError);
int OutputToProcessDelayed (ProcRef pr, char *message, int count,
			      int *outError, long msdelay);

void CmailSigHandlerCallBack (InputSourceRef isr, void *closure,
				char *buf, int count, int error);

extern ProcRef cmailPR;
extern int shiftKey, controlKey;
extern char dataDir[], manDir[];

/* in xgamelist.c or winboard.c */
void GLT_ClearList();
void GLT_DeSelectList();
void GLT_AddToList( char *name );
Boolean GLT_GetFromList( int index, char *name );

extern char lpUserGLT[];
extern char *homeDir;

/* these are in wgamelist.c */
void GameListPopUp (FILE *fp, char *filename);
void GameListPopDown (void);
void GameListHighlight (int index);
void GameListDestroy (void);
void GameListUpdate (void);
FILE *GameFile (void);

/* these are in wedittags.c */
void EditTagsPopUp (char *tags, char **dest);
void TagsPopUp (char *tags, char *msg);
void TagsPopDown (void);

void ParseIcsTextColors (void);
int  ICSInitScript (void);
void StartAnalysisClock (void);
void EngineOutputPopUp (void);
void EgineOutputPopDown (void);

void SetHighlights (int fromX, int fromY, int toX, int toY);
void ClearHighlights (void);
void SetPremoveHighlights (int fromX, int fromY, int toX, int toY);
void ClearPremoveHighlights (void);

void AnimateAtomicCapture (Board board, int fromX, int fromY, int toX, int toY);
void ShutDownFrontEnd (void);
void BoardToTop (void);
void AnimateMove (Board board, int fromX, int fromY, int toX, int toY);
void HistorySet (char movelist[][2*MOVE_LEN], int first, int last, int current);
void FreezeUI (void);
void ThawUI (void);
void ChangeDragPiece (ChessSquare piece);
void CopyFENToClipboard (void);
extern char *programName;
extern int commentUp;
extern char *engineListFile;
extern char *firstChessProgramNames;
extern char *icsTextMenuString;
extern int mute;

void GreyRevert (Boolean grey);
void EnableNamedMenuItem (char *menuRef, int state);

typedef struct FrontEndProgramStats_TAG {
    int which;
    int depth;
    u64 nodes;
    int score;
    int time;
    char * pv;
    char * hint;
    int an_move_index;
    int an_move_count;
} FrontEndProgramStats;

/* [AS] */
void SetProgramStats (FrontEndProgramStats * stats);

void EngineOutputPopUp (void);
void EngineOutputPopDown (void);
int  EngineOutputIsUp (void);
int  EngineOutputDialogExists (void);
void EvalGraphPopUp (void);
Boolean EvalGraphIsUp (void);
int  EvalGraphDialogExists (void);
void SlavePopUp (void);
void ActivateTheme (int new);
char *Col2Text (int n);
char *Shorten (char *s);

/* these are in xhistory.c  */
Boolean MoveHistoryIsUp (void);
void HistoryPopUp (void);
void FindMoveByCharIndex (int char_index);

#endif /* XB_FRONTEND */
