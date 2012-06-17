#ifndef _CURSES_H
#define _CURSES_H


/// \todo complete.

#include <inttypes.h>
#include <sys/types.h>
#include <stdio.h>
#include <term.h>
#include <termios.h>
#include <wchar.h>

// TODO: check EOF and WEOF

#define	EOF			-13
#define	ERR			-1
#define	FALSE		0
#define	OK			0
#define	TRUE		1
#define	WEOF		-13


#define	_XOPEN_CURSES		1

typedef	uint32_t	attr_t;
typedef	uint32_t	chtype;
typedef	void*		SCREEN;
typedef	uint32_t	wchar_t;
typedef	int32_t		wint_t;
typedef	uint32_t*	cchar_t;
typedef	void*		WINDOW;


#define	WA_ALTCHARSET  			(1<<0)
#define	WA_BLINK				(1<<1)  
#define	WA_BOLD    				(1<<2)  
#define	WA_DIM      			(1<<3)  
#define	WA_HORIZONTAL			(1<<4)  
#define	WA_INVIS 				(1<<5)  
#define	WA_LEFT   				(1<<6)  
#define	WA_LOW     				(1<<7)  
#define	WA_PROTECT  			(1<<8)  
#define	WA_REVERSE   			(1<<9)  
#define	WA_RIGHT      			(1<<10)  
#define	WA_STANDOUT				(1<<11)  
#define	WA_TOP     				(1<<12)  
#define	WA_UNDERLINE			(1<<13)  
#define	WA_VERTICAL  			(1<<14)  



#define	A_ALTCHARSET			(1<<0)
#define	A_BLINK					(1<<1)
#define	A_BOLD					(1<<2)
#define	A_DIM					(1<<3)
#define	A_INVIS					(1<<4)
#define	A_PROTECT				(1<<5)
#define	A_REVERSE				(1<<6)
#define	A_STANDOUT				(1<<7)
#define	A_UNDERLINE				(1<<8)



/*





A_ATTRIBUTES   Bit-mask to extract attributes
A_CHARTEXT     Bit-mask to extract a character
A_COLOR        Bit-mask to extract colour-pair information


Line-drawing Constants
The <curses.h> header defines the symbolic constants shown in the leftmost two columns of the following table for use in drawing lines. The symbolic constants that begin with ACS_ are char constants. The symbolic constants that begin with WACS_ are cchar_t constants for use with the wide-character interfaces that take a pointer to a cchar_t.

In the POSIX locale, the characters shown in the POSIX Locale Default column are used when the terminal database does not specify a value using the acsc capability.

char Constant 	cchar_t Constant 	POSIX Locale Default 	Glyph Description
ACS_ULCORNER 	WACS_ULCORNER 	+ 	upper left-hand corner
ACS_LLCORNER 	WACS_LLCORNER 	+ 	lower left-hand corner
ACS_URCORNER 	WACS_URCORNER 	+ 	upper right-hand corner
ACS_LRCORNER 	WACS_LRCORNER 	+ 	lower right-hand corner
ACS_RTEE 	WACS_RTEE 	+ 	right tee
ACS_LTEE 	WACS_LTEE 	+ 	left tee
ACS_BTEE 	WACS_BTEE 	+ 	bottom tee
ACS_TTEE 	WACS_TTEE 	+ 	top tee
ACS_HLINE 	WACS_HLINE 	- 	horizontal line
ACS_VLINE 	WACS_VLINE 	| 	vertical line
ACS_PLUS 	WACS_PLUS 	+ 	plus
ACS_S1 	WACS_S1 	- 	scan line 1
ACS_S9 	WACS_S9 	_ 	scan line 9
ACS_DIAMOND 	WACS_DIAMOND 	+ 	diamond
ACS_CKBOARD 	WACS_CKBOARD 	: 	checker board (stipple)
ACS_DEGREE 	WACS_DEGREE 	' 	degree symbol
ACS_PLMINUS 	WACS_PLMINUS 	# 	plus/minus
ACS_BULLET 	WACS_BULLET 	o 	bullet
ACS_LARROW 	WACS_LARROW 	< 	arrow pointing left
ACS_RARROW 	WACS_RARROW 	> 	arrow pointing right
ACS_DARROW 	WACS_DARROW 	v 	arrow pointing down
ACS_UARROW 	WACS_UARROW 	^ 	arrow pointing up
ACS_BOARD 	WACS_BOARD 	# 	board of squares
ACS_LANTERN 	WACS_LANTERN 	# 	lantern symbol
ACS_BLOCK 	WACS_BLOCK 	# 	solid square block



#define COLOR_BLACK	0
#define COLOR_RED	1
#define COLOR_GREEN	2
#define COLOR_YELLOW	3
#define COLOR_BLUE	4
#define COLOR_MAGENTA	5
#define COLOR_CYAN	6
#define COLOR_WHITE	7

Coordinate-related Macros
The following coordinate-related macros are defined:


void   getbegyx(WINDOW *win, int y, int x);
void   getmaxyx(WINDOW *win, int y, int x);
void   getparyx(WINDOW *win, int y, int x);
void   getyx(WINDOW *win, int y, int x);

Key Codes
The following symbolic constants representing function key values are defined:

Key Code 	Description
KEY_CODE_YES 	Used to indicate that a wchar_t variable
contains a key code
KEY_BREAK 	Break key
KEY_DOWN 	Down arrow key
KEY_UP 	Up arrow key
KEY_LEFT 	Left arrow key
KEY_RIGHT 	Right arrow key
KEY_HOME 	Home key
KEY_BACKSPACE 	Backspace
KEY_F0 	Function keys; space for 64 keys is reserved
KEY_F(n) 	For 0 <=n<=63
KEY_DL 	Delete line
KEY_IL 	Insert line
KEY_DC 	Delete character
KEY_IC 	Insert char or enter insert mode
KEY_EIC 	Exit insert char mode
KEY_CLEAR 	Clear screen
KEY_EOS 	Clear to end of screen
KEY_EOL 	Clear to end of line
KEY_SF 	Scroll 1 line forward
KEY_SR 	Scroll 1 line backward (reverse)
KEY_NPAGE 	Next page
KEY_PPAGE 	Previous page
KEY_STAB 	Set tab
KEY_CTAB 	Clear tab
KEY_CATAB 	Clear all tabs
KEY_ENTER 	Enter or send
KEY_SRESET 	Soft (partial) reset
KEY_RESET 	Reset or hard reset
KEY_PRINT 	Print or copy
KEY_LL 	Home down or bottom
KEY_A1 	Upper left of keypad
KEY_A3 	Upper right of keypad
KEY_B2 	Center of keypad
KEY_C1 	Lower left of keypad
KEY_C3 	Lower right of keypad
The virtual keypad is a 3-by-3 keypad arranged as follows:

A1 	UP 	A3
LEFT 	B2 	RIGHT
C1 	DOWN 	C3

Each legend, such as A1, corresponds to a symbolic constant for a key code from the preceding table, such as KEY_A1.
The following symbolic constants representing function key values are also defined:

Key Code 	Description
KEY_BTAB 	Back tab key
KEY_BEG 	Beginning key
KEY_CANCEL 	Cancel key
KEY_CLOSE 	Close key
KEY_COMMAND 	Cmd (command) key
KEY_COPY 	Copy key
KEY_CREATE 	Create key
KEY_END 	End key
KEY_EXIT 	Exit key
KEY_FIND 	Find key
KEY_HELP 	Help key
KEY_MARK 	Mark key
KEY_MESSAGE 	Message key
KEY_MOVE 	Move key
KEY_NEXT 	Next object key
KEY_OPEN 	Open key
KEY_OPTIONS 	Options key
KEY_PREVIOUS 	Previous object key
KEY_REDO 	Redo key
KEY_REFERENCE 	Reference key
KEY_REFRESH 	Refresh key
KEY_REPLACE 	Replace key
KEY_RESTART 	Restart key
KEY_RESUME 	Resume key
KEY_SAVE 	Save key
KEY_SBEG 	Shifted beginning key
KEY_SCANCEL 	Shifted cancel key
KEY_SCOMMAND 	Shifted command key
KEY_SCOPY 	Shifted copy key
KEY_SCREATE 	Shifted create key
KEY_SDC 	Shifted delete char key
KEY_SDL 	Shifted delete line key
KEY_SELECT 	Select key
KEY_SEND 	Shifted end key
KEY_SEOL 	Shifted clear line key
KEY_SEXIT 	Shifted exit key
KEY_SFIND 	Shifted find key
KEY_SHELP 	Shifted help key
KEY_SHOME 	Shifted home key
KEY_SIC 	Shifted input key
KEY_SLEFT 	Shifted left arrow key
KEY_SMESSAGE 	Shifted message key
KEY_SMOVE 	Shifted move key
KEY_SNEXT 	Shifted next key
KEY_SOPTIONS 	Shifted options key
KEY_SPREVIOUS 	Shifted prev key
KEY_SPRINT 	Shifted print key
KEY_SREDO 	Shifted redo key
KEY_SREPLACE 	Shifted replace key
KEY_SRIGHT 	Shifted right arrow
KEY_SRSUME 	Shifted resume key
KEY_SSAVE 	Shifted save key
KEY_SSUSPEND 	Shifted suspend key
KEY_SUNDO 	Shifted undo key
KEY_SUSPEND 	Suspend key
KEY_UNDO 	Undo key
Function Prototypes
The following are declared as functions, and may also be defined as macros:


int    addch(const chtype);
int    addchnstr(const chtype *, int);
int    addchstr(const chtype *);
int    addnstr(const char *, int);
int    addnwstr(const wchar_t *, int);
int    addstr(const char *);
int    add_wch(const cchar_t *);
int    add_wchnstr(const cchar_t *, int);
int    add_wchstr(const cchar_t *);
int    addwstr(const wchar_t *);
int    attroff(int);
int    attron(int);
int    attrset(int);
int    attr_get(attr_t *, short *, void *);
int    attr_off(attr_t, void *);
int    attr_on(attr_t, void *);
int    attr_set(attr_t, short, void *);
int    baudrate(void);
int    beep(void);
int    bkgd(chtype);
void   bkgdset(chtype);
int    bkgrnd(const cchar_t *);
void   bkgrndset(const cchar_t *);
int    border(chtype, chtype, chtype, chtype, chtype, chtype, chtype,
chtype);
int    border_set(const cchar_t *, const cchar_t *, const cchar_t *,
const cchar_t *, const cchar_t *, const cchar_t *,
const cchar_t *, const cchar_t *);
int    box(WINDOW *, chtype, chtype);
int    box_set(WINDOW *, const cchar_t *, const cchar_t *);
bool   can_change_color(void);
int    cbreak(void); 
int    chgat(int, attr_t, short, const void *);
int    clearok(WINDOW *, bool);
int    clear(void);
int    clrtobot(void);
int    clrtoeol(void);
int    color_content(short, short *, short *, short *);
int    COLOR_PAIR(int);
int    color_set(short,void *);
int    copywin(const WINDOW *, WINDOW *, int, int, int, int, int, int,
int);
int    curs_set(int);
int    def_prog_mode(void);
int    def_shell_mode(void);
int    delay_output(int);
int    delch(void);
int    deleteln(void);
void   delscreen(SCREEN *); 
int    delwin(WINDOW *);
WINDOW *derwin(WINDOW *, int, int, int, int);
int    doupdate(void);
WINDOW *dupwin(WINDOW *);
int    echo(void);
int    echochar(const chtype);
int    echo_wchar(const cchar_t *);
int    endwin(void);
char   erasechar(void);
int    erase(void);
int    erasewchar(wchar_t *);
void   filter(void);
int    flash(void);
int    flushinp(void);
chtype getbkgd(WINDOW *);
int    getbkgrnd(cchar_t *);
int    getcchar(const cchar_t *, wchar_t *, attr_t *, short *, void *);
int    getch(void);
int    getnstr(char *, int);
int    getn_wstr(wint_t *, int);
int    getstr(char *);
int    get_wch(wint_t *);
WINDOW *getwin(FILE *);
int    get_wstr(wint_t *);
int    halfdelay(int);
bool   has_colors(void);
bool   has_ic(void);
bool   has_il(void);
int    hline(chtype, int);
int    hline_set(const cchar_t *, int);
void   idcok(WINDOW *, bool);
int    idlok(WINDOW *, bool);
void   immedok(WINDOW *, bool);
chtype inch(void);
int    inchnstr(chtype *, int);
int    inchstr(chtype *);
WINDOW *initscr(void);
int    init_color(short, short, short, short);
int    init_pair(short, short, short);
int    innstr(char *, int);
int    innwstr(wchar_t *, int);
int    insch(chtype);
int    insdelln(int);
int    insertln(void);
int    insnstr(const char *, int);
int    ins_nwstr(const wchar_t *, int);
int    insstr(const char *);
int    instr(char *);
int    ins_wch(const cchar_t *);
int    ins_wstr(const wchar_t *);
int    intrflush(WINDOW *, bool);
int    in_wch(cchar_t *);
int    in_wchnstr(cchar_t *, int);
int    in_wchstr(cchar_t *);
int    inwstr(wchar_t *);
bool   isendwin(void);
bool   is_linetouched(WINDOW *, int);
bool   is_wintouched(WINDOW *);
char   *keyname(int);
char   *key_name(wchar_t);
int    keypad(WINDOW *, bool);
char   killchar(void);
int    killwchar(wchar_t *);
int    leaveok(WINDOW *, bool);
char   *longname(void);
int    meta(WINDOW *, bool);
int    move(int, int);
int    mvaddch(int, int, const chtype);
int    mvaddchnstr(int, int, const chtype *, int);
int    mvaddchstr(int, int, const chtype *);
int    mvaddnstr(int, int, const char *, int);
int    mvaddnwstr(int, int, const wchar_t *, int);
int    mvaddstr(int, int, const char *);
int    mvadd_wch(int, int, const cchar_t *);
int    mvadd_wchnstr(int, int, const cchar_t *, int);
int    mvadd_wchstr(int, int, const cchar_t *);
int    mvaddwstr(int, int, const wchar_t *);
int    mvchgat(int, int, int, attr_t, short, const void *);
int    mvcur(int, int, int, int);
int    mvdelch(int, int);
int    mvderwin(WINDOW *, int, int);
int    mvgetch(int, int);
int    mvgetnstr(int, int, char *, int);
int    mvgetn_wstr(int, int, wint_t *, int);
int    mvgetstr(int, int, char *);
int    mvget_wch(int, int, wint_t *);
int    mvget_wstr(int, int, wint_t *);
int    mvhline(int, int, chtype, int);
int    mvhline_set(int, int, const cchar_t *, int);
chtype mvinch(int, int);
int    mvinchnstr(int, int, chtype *, int);
int    mvinchstr(int, int, chtype *);
int    mvinnstr(int, int, char *, int);
int    mvinnwstr(int, int, wchar_t *, int);
int    mvinsch(int, int, chtype);
int    mvinsnstr(int, int, const char *, int);
int    mvins_nwstr(int, int, const wchar_t *, int);
int    mvinsstr(int, int, const char *);
int    mvinstr(int, int, char *);
int    mvins_wch(int, int, const cchar_t *);
int    mvins_wstr(int, int, const wchar_t *);
int    mvin_wch(int, int, cchar_t *);
int    mvin_wchnstr(int, int, cchar_t *, int);
int    mvin_wchstr(int, int, cchar_t *);
int    mvinwstr(int, int, wchar_t *);
int    mvprintw(int, int, char *,  ...);
int    mvscanw(int, int, char *, ...);
int    mvvline(int, int, chtype, int);
int    mvvline_set(int, int, const cchar_t *, int);
int    mvwaddch(WINDOW *, int, int, const chtype);
int    mvwaddchnstr(WINDOW *, int, int, const chtype *, int);
int    mvwaddchstr(WINDOW *, int, int, const chtype *);
int    mvwaddnstr(WINDOW *, int, int, const char *, int);
int    mvwaddnwstr(WINDOW *, int, int, const wchar_t *, int);
int    mvwaddstr(WINDOW *, int, int, const char *);
int    mvwadd_wch(WINDOW *, int, int, const cchar_t *);
int    mvwadd_wchnstr(WINDOW *, int, int, const cchar_t *, int);
int    mvwadd_wchstr(WINDOW *, int, int, const cchar_t *);
int    mvwaddwstr(WINDOW *, int, int, const wchar_t *);
int    mvwchgat(WINDOW *, int, int, int, attr_t, short, const void *);
int    mvwdelch(WINDOW *, int, int);
int    mvwgetch(WINDOW *, int, int);
int    mvwgetnstr(WINDOW *, int, int, char *, int);
int    mvwgetn_wstr(WINDOW *, int, int, wint_t *, int);
int    mvwgetstr(WINDOW *, int, int, char *);
int    mvwget_wch(WINDOW *, int, int, wint_t *);
int    mvwget_wstr(WINDOW *, int, int, wint_t *);
int    mvwhline(WINDOW *, int, int, chtype, int);
int    mvwhline_set(WINDOW *, int, int, const cchar_t *, int);
int    mvwin(WINDOW *, int, int);
chtype mvwinch(WINDOW *, int, int);
int    mvwinchnstr(WINDOW *, int, int, chtype *, int);
int    mvwinchstr(WINDOW *, int, int, chtype *);
int    mvwinnstr(WINDOW *, int, int, char *, int);
int    mvwinnwstr(WINDOW *, int, int, wchar_t *, int);
int    mvwinsch(WINDOW *, int, int, chtype);
int    mvwinsnstr(WINDOW *, int, int, const char *, int);
int    mvwins_nwstr(WINDOW *, int, int, const wchar_t *, int);
int    mvwinsstr(WINDOW *, int, int, const char *);
int    mvwinstr(WINDOW *, int, int, char *);
int    mvwins_wch(WINDOW *, int, int, const cchar_t *);
int    mvwins_wstr(WINDOW *, int, int, const wchar_t *);
int    mvwin_wch(WINDOW *, int, int, cchar_t *);
int    mvwin_wchnstr(WINDOW *, int, int, cchar_t *, int);
int    mvwin_wchstr(WINDOW *, int, int, cchar_t *);
int    mvwinwstr(WINDOW *, int, int, wchar_t *);
int    mvwprintw(WINDOW *, int, int, char *, ...);
int    mvwscanw(WINDOW *, int, int, char *, ...);
int    mvwvline(WINDOW *, int, int, chtype, int);
int    mvwvline_set(WINDOW *, int, int, const cchar_t *, int);
int    napms(int);
WINDOW *newpad(int, int);
SCREEN *newterm(char *, FILE *, FILE *);
WINDOW *newwin(int, int, int, int);
int    nl(void);
int    nocbreak(void);
int    nodelay(WINDOW *, bool);
int    noecho(void);
int    nonl(void);
void   noqiflush(void);
int    noraw(void);
int    notimeout(WINDOW *, bool);
int    overlay(const WINDOW *, WINDOW *);
int    overwrite(const WINDOW *, WINDOW *);
int    pair_content(short, short *, short *);
int    PAIR_NUMBER(int);
int    pechochar(WINDOW *, chtype);
int    pecho_wchar(WINDOW *, const cchar_t*);
int    pnoutrefresh(WINDOW *, int, int, int, int, int, int);
int    prefresh(WINDOW *, int, int, int, int, int, int);
int    printw(char *, ...);
int    putp(const char *);
int    putwin(WINDOW *, FILE *);
void   qiflush(void);
int    raw(void);
int    redrawwin(WINDOW *);
int    refresh(void);
int    reset_prog_mode(void);
int    reset_shell_mode(void);
int    resetty(void);
int    ripoffline(int, int (*)(WINDOW *, int));
int    savetty(void);
int    scanw(char *, ...);
int    scr_dump(const char *);
int    scr_init


*/

#endif

