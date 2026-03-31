/*
 * backend.h -- Interface exported by XBoard back end
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2016, 2026 Free Software Foundation, Inc.
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

#ifndef XB_BACKEND
#define XB_BACKEND

#include <stdint.h>

#include "lists.h"

typedef int (*FileProc)(FILE * f, int n, char * title);

extern char * wbOptions;
extern int gotPremove;
extern GameMode gameMode;
extern int matchMode;
extern int matchGame;
extern int pausing, cmailMsgLoaded, flipView, mute;
extern char white_holding[], black_holding[];
extern int currentMove, backwardMostMove, forwardMostMove;
extern int blackPlaysFirst;
extern FILE * debugFP;
extern char * programVersion;
extern Board boards[];
extern char marker[BOARD_RANKS][BOARD_FILES];
extern char lastMsg[MSG_SIZ];
extern Boolean bookUp;
extern Boolean addToBookFlag;
extern int tinyLayout, smallLayout;
extern Boolean mcMode;
extern int dragging;
extern char variantError[];
extern char lastTalker[];
extern int transparency[];

void MarkMenuItem(char * menuRef, int state);
char * CmailMsg(void);
/* Tord: Added the useFEN960 parameter in PositionToFEN() below */
char * PositionToFEN(int move, char * useFEN960, int moveCounts);
void AlphaRank(char * s, int n); /* [HGM] Shogi move preprocessor */
void EditPositionPasteFEN(char * fen);
void TimeDelay(long ms);
void SendMultiLineToICS(char * text);
void AnalysisPeriodicEvent(int force);
void SetWhiteToPlayEvent(void);
void SetBlackToPlayEvent(void);
void UploadGameEvent(void);
void InitBackEnd1(void);
void InitBackEnd2(void);
int HasPromotionChoice(int fromX, int fromY, int toX, int toY, char * choice, int sweep);
int InPalace(int row, int column);
int PieceForSquare(int x, int y);
int OKToStartUserMove(int x, int y);
void Reset(int redraw, int init);
void ResetGameEvent(void);
Boolean HasPattern(const char * text, const char * pattern);
Boolean SearchPattern(const char * text, const char * pattern);
int LoadGame(FILE * f, int n, char * title, int useList);
int LoadGameFromFile(char * filename, int n, char * title, int useList);
int CmailLoadGame(FILE * f, int n, char * title, int useList);
int ReloadGame(int offset);
int SaveSelected(FILE * f, int dummy, char * dummy2);
int SaveGame(FILE * f, int dummy, char * dummy2);
int SaveGameToFile(char * filename, int append);
int LoadPosition(FILE * f, int n, char * title);
int ReloadPosition(int offset);
int SavePosition(FILE * f, int dummy, char * dummy2);
int DrawSeekGraph(void);
int SeekGraphClick(ClickType click, int x, int y, int moving);
void EditPositionEvent(void);
void FlipViewEvent(void);
void MachineWhiteEvent(void);
void MachineBlackEvent(void);
void TwoMachinesEvent(void);
void EditGameEvent(void);
void TrainingEvent(void);
void IcsClientEvent(void);
void ForwardEvent(void);
void BackwardEvent(void);
void ToEndEvent(void);
void ToStartEvent(void);
void ToNrEvent(int to);
void RevertEvent(Boolean annotate);
void RetractMoveEvent(void);
void MoveNowEvent(void);
void TruncateGameEvent(void);
void PauseEvent(void);
void CallFlagEvent(void);
void ClockClick(int which);
void AcceptEvent(void);
void DeclineEvent(void);
void RematchEvent(void);
void DrawEvent(void);
void AbortEvent(void);
void AdjournEvent(void);
void ResignEvent(void);
void UserAdjudicationEvent(int result);
void StopObservingEvent(void);
void StopExaminingEvent(void);
void PonderNextMoveEvent(int newState);
void ShowThinkingEvent(void);
void PeriodicUpdatesEvent(int newState);
void HintEvent(void);
void BookEvent(void);
void AboutGameEvent(void);
void ExitEvent(int status);
char * DefaultFileName(char *);
ChessMove UserMoveTest(int fromX, int fromY, int toX, int toY, int promoChar, Boolean captureOwn);
void UserMoveEvent(int fromX, int fromY, int toX, int toY, int promoChar);
void DecrementClocks(void);
char * TimeString(long millisec);
void AutoPlayGameLoop(void);
void AdjustClock(Boolean which, int dir);
void DisplayBothClocks(void);
void EditPositionMenuEvent(ChessSquare selection, int x, int y);
void DropMenuEvent(ChessSquare selection, int x, int y);
int ParseTimeControl(char * tc, float ti, int mps);
void EscapeExpand(char * p, char * q);
void ProcessICSInitScript(FILE * f);
void EditCommentEvent(void);
void ReplaceComment(int index, char * text);
int ReplaceTags(char * tags, GameInfo * gi); /* returns nonzero on error */
void AppendComment(int index, char * text, Boolean addBraces);
void LoadVariation(int index, char * text);
void ReloadCmailMsgEvent(int unregister);
void MailMoveEvent(void);
void EditTagsEvent(void);
void GetMoveListEvent(void);
void ExitAnalyzeMode(void);
int AnalyzeModeEvent(void);
void AnalyzeFileEvent(void);
void MatchEvent(int mode);
void RecentEngineEvent(int nr);
void TypeInEvent(char first);
void TypeInDoneEvent(char * move);
void InitPosition(int redraw);
void NewSettingEvent(int option, int * feature, char * command, int value);
void SettingsMenuIfReady(void);
void DoEcho(void);
void DontEcho(void);
void TidyProgramName(char * prog, char * host, char * buf);
void SetGameInfo(void);
void AskQuestionEvent(char * title, char * question, char * replyPrefix, char * which);
Boolean ParseOneMove(
 char * move, int moveNum, ChessMove * moveType, int * fromX, int * fromY, int * toX, int * toY, char * promoChar);
char * VariantName(VariantClass v);
VariantClass StringToVariant(char * e);
void OutputChatMessage(int partner, char * mess);
void EditPositionDone(Boolean fakeRights);
Boolean GetArgValue(char * name);
Boolean LoadPV(int x, int y);
Boolean LoadMultiPV(int x, int y, char * buf, int index, int * start, int * end, int pane);
void UnLoadPV(void);
void MovePV(int x, int y, int h);
int PromoScroll(int x, int y);
void EditBookEvent(void);
Boolean DisplayBook(int moveNr);
void SaveToBook(char * text);
void AddBookMove(char * text);
void PlayBookMove(char * text, int index);
void HoverEvent(int hiX, int hiY, int x, int y);
int PackGame(Board board);
Boolean ParseFEN(Board board, int * blackPlaysFirst, char * fen, Boolean autoSize);
void ApplyMove(int fromX, int fromY, int toX, int toY, int promoChar, Board board);
void PackMove(int fromX, int fromY, int toX, int toY, ChessSquare promoPiece);
void ics_printf(char * format, ...);
int GetEngineLine(char * nick, int engine);
void AddGameToBook(int always);
void FlushBook(void);
char PieceToChar(ChessSquare p);
int LoadPieceDesc(char * s);

char * StrStr(char * string, char * match);
char * StrCaseStr(char * string, char * match);
char * StrSave(char * s);
char * StrSavePtr(char * s, char ** savePtr);
char * SavePart(char *);
char * safeStrCpy(char * dst, const char * src, size_t count);

int StrCaseCmp(char * s1, char * s2);
int ToLower(int c);
int ToUpper(int c);

extern GameInfo gameInfo;

/* ICS vars used with backend.c and zippy.c */
enum ICS_TYPE { ICS_GENERIC, ICS_ICC, ICS_FICS, ICS_CHESSNET /* not really supported */ };
extern enum ICS_TYPE ics_type;

/* pgntags.c prototypes
 */
char * PGNTags(GameInfo *);
void PrintPGNTags(FILE * f, GameInfo *);
int ParsePGNTag(char *, GameInfo *);
char * PGNResult(ChessMove result);


/* gamelist.c prototypes
 */
/* A game node in the double linked list of games.
 */
typedef struct XB_ListGame {
    ListNode node;
    int number;
    int position;
    int moves;
    unsigned long offset; /*  Byte offset of game within file.     */
    GameInfo gameInfo; /*  Note that some entries may be NULL. */
} ListGame;

extern int handSize;
extern int border;
extern int doubleClick;
extern int storedGames;
extern int opponentKibitzes;
extern ChessSquare gatingPiece;
extern List gameList;
extern int lastLoadGameNumber;
void ClearGameInfo(GameInfo *);
int GameListBuild(FILE *);
void GameListInitGameInfo(GameInfo *);
char * GameListLine(int, GameInfo *);
char * GameListLineFull(int, GameInfo *);
void InitSearch(void);
int GameContainsPosition(FILE * f, ListGame * lg);
void GLT_TagsToList(char * tags);
void GLT_ParseList(void);
int NamesToList(char * name, char ** engines, char ** mnemonics, char * group);
int CreateTourney(char * name);
char * MakeName(char * templ);
void SwapEngines(int n);
void Substitute(char * participants, int expunge);

/* returns static data */
extern char * StripHighlight(char *);
/* returns static data */
extern char * StripHighlightAndTitle(char *);

extern void ics_update_width(int new_width);
extern Boolean set_cont_sequence(char * new_seq);
extern int wrap(char * dest, char * src, int count, int width, int * lp);
int Explode(Board board, int fromX, int fromY, int toX, int toY);

typedef enum {
    CheckBox,
    ComboBox,
    TextBox,
    Button,
    Spin,
    ResetButton,
    SaveButton,
    ListBox,
    Graph,
    PopUp,
    FileName,
    PathName,
    Slider,
    Message,
    Fractional,
    Label,
    Icon,
    BoxBegin,
    BoxEnd,
    BarBegin,
    BarEnd,
    DropDown,
    Break,
    GroupBox,
    EndMark,
    Skip
} Control;

// TODO: Avoid stashing arbitrary additional information within name.
// To find some examples, look for name + MSG_SIZ - 100 and name + MSG_SIZ - 104.
typedef struct XB_OPT {  // [HGM] options: descriptor of UCI-style option
    int value;  // current setting, starts as default
    int min;  // Also used for flags
    int max;
    void * handle;  // for use by front end
    void * target;  // for use by front end
    char * textValue;  // points to beginning of text value in name field
    char ** choice;  // points to array of combo choices in cps->combo
    Control type;
    char * name;  // holds both option name and text value (in allocated memory)
    char ** font;
} Option;

typedef struct XB_CPS {
    char * which;
    int maybeThinking;
    ProcRef pr;
    InputSourceRef isr;
    char * twoMachinesColor; /* "white\n" or "black\n" */
    char * program;
    char * host;
    char * dir;
    struct XB_CPS * other;
    char * initString;
    char * computerString;
    int sendTime; /* 0=don't, 1=do, 2=test */
    int sendDrawOffers;
    int useSigint;
    int useSigterm;
    int offeredDraw; /* countdown */
    int reuse;
    int useSetboard; /* 0=use "edit"; 1=use "setboard" */
    int extendedEdit; /* 1=also set holdings with "edit" */
    int useSAN; /* 0=use coordinate notation; 1=use SAN */
    int usePing; /* 0=not OK to use ping; 1=OK */
    int lastPing;
    int lastPong;
    int usePlayother; /* 0=not OK to use playother; 1=OK */
    int useColors; /* 0=avoid obsolete white/black commands; 1=use them */
    int useUsermove; /* 0=just send move; 1=send "usermove move" */
    int sendICS; /* 0=don't use "ics" command; 1=do */
    int sendName; /* 0=don't use "name" command; 1=do */
    int sdKludge; /* 0=use "sd DEPTH" command; 1=use "depth\nDEPTH" */
    int stKludge; /* 0=use "st TIME" command; 1=use "level 1 TIME" */
    int excludeMoves; /* 0=don't use "exclude" command; 1=do */
    char * tidy;
    int matchWins;
    char * variants;
    int analysisSupport;
    int analyzing;
    int protocolVersion;
    int initDone;
    int pseudo;

    /* Added by Tord: */
    int useFEN960; /* 0=use "KQkq" style FENs, 1=use "HAha" style FENs */
    int useOOCastle; /* 0="O-O" notation for castling, 1="king capture rook" notation */
    /* End of additions by Tord */

    int scoreIsAbsolute; /* [AS] 0=don't know (standard), 1=score is always from white side */
    int isUCI; /* [AS] 0=no (Winboard), 1=UCI (requires Polyglot) */
    int hasOwnBookUCI; /* [AS] 0=use GUI or Polyglot book, 1=has own book */

    /* [HGM] time odds */
    float timeOdds; /* factor through which we divide time for this engine  */
    int debug; /* [HGM] ignore engine debug lines starting with '#'    */
    int maxNrOfSessions; /* [HGM] secondary TC: max args in 'level' command */
    int accumulateTC; /* [HGM] secondary TC: how to handle extra sessions   */
    int drawDepth; /* [HGM] egbb: search depth to play egbb draws        */
    int nps; /* [HGM] nps: factor for node count to replace time   */
    int supportsNPS;
    int alphaRank; /* [HGM] shogi: engine uses shogi-type coordinates    */
    int maxCores; /* [HGM] SMP: engine understands cores command        */
    int memSize; /* [HGM] memsize: engine understands memory command   */
    char * egtFormats; /* [HGM] EGT: supported tablebase formats             */
    int bookSuspend; /* [HGM] book: go was deferred because of book hit    */
    int pause; /* [HGM] pause: 1=supports it, 2=actually paused      */
    int dice; /* [HGM] dice: engine understands pips command        */
    int highlight; /* [HGM] engine wants to get lift and put commands    */
    int nrOptions; /* [HGM] options: remembered option="..." features    */
#define MAX_OPTIONS 200
    Option option[MAX_OPTIONS];
    int comboCnt;
    char * comboList[20 * MAX_OPTIONS];
    char * optionSettings;
    void * programLogo; /* [HGM] logo: bitmap of the logo                    */
    char * fenOverride; /* [HGM} FRC: force FEN casling & ep fields by hand  */
    char userError; /* [HGM] crash: flag to suppress fatal-error messages*/
    char reload; /* [HGM] options: flag to resend options with xreuse */
} ChessProgramState;

extern ChessProgramState first, second;

/* Search stats from chessprogram */
typedef struct {
    char movelist[2 * MSG_SIZ]; /* Last PV we were sent */
    int depth; /* Current search depth */
    int nr_moves; /* Total nr of root moves */
    int moves_left; /* Moves remaining to be searched */
    char move_name[MOVE_LEN]; /* Current move being searched, if provided */
    uint64_t nodes; /* # of nodes searched */
    int time; /* Search time (centiseconds) */
    int score; /* Score (centipawns) */
    int got_only_move; /* If last msg was "(only move)" */
    int got_fail; /* 0 - nothing, 1 - got "--", 2 - got "++" */
    int ok_to_send; /* handshaking between send & recv */
    int line_is_book; /* 1 if movelist is book moves */
    int seen_stat; /* 1 if we've seen the stat01: line */
} ChessProgramStats;

extern ChessProgramStats_Move pvInfoList[MAX_MOVES];
extern Boolean shuffleOpenings;
extern ChessProgramStats programStats;
extern int remoteEchoOption;
extern int opponentKibitzes;  // used by wengineo.c
extern int errorExitStatus;
extern char * recentEngines;
extern char * currentEngine[];
extern Boolean partnerUp, twoBoards;
extern char engineVariant[];
void SaveEngineSettings(int n);
void SaveEngineList(void);
char * EngineDefinedVariant(ChessProgramState * cps, int n);
// [HGM] really in front-end, but CPS not known in frontend.h
void SettingsPopUp(ChessProgramState * cps);
int WaitForEngine(ChessProgramState * cps, DelayedEventCallback x);
void Load(ChessProgramState * cps, int n);
int MultiPV(ChessProgramState * cps, int kind);
void MoveHistorySet(char movelist[][2 * MOVE_LEN], int first, int last, int current, ChessProgramStats_Move * pvInfo);
void MakeEngineOutputTitle(void);
void LoadTheme(void);
void CreateBookEvent(void);
char * SupportedVariant(char * list, VariantClass v, int w, int h, int s, int proto, char * engine);
char * CollectPieceDescriptors(void);
void RefreshSettingsDialog(ChessProgramState * cps, int val);
void StartChessProgram(ChessProgramState * cps);
void SendToICS(char * s);
int PosFlags(int n);


/* A point in time */
// TODO: Modernize how this structure is defined and used.
typedef struct {
    long sec; /* Assuming this is >= 32 bits */
    int ms; /* Assuming this is >= 16 bits */
} TimeMark;

extern TimeMark programStartTime;

void GetTimeMark(TimeMark *);
long SubtractTimeMarks(TimeMark *, TimeMark *);

#endif /* XB_BACKEND */
