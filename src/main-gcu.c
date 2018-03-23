/* File: main-gcu.c */

/* Purpose: Allow use of Unix "curses" with Angband -BEN- */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 *
 *
 * James E. Wilson and Robert A. Koeneke released all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2 or any later version),
 * or under the terms of the traditional Angband license.
 *
 * Ben Harrison released all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2 or any later version),
 * or under the terms of the traditional Angband license.
 *
 * All changes in Hellband are Copyright (c) 2005-2007 Konijn
 * I Konijn  release all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2),
 * or under the terms of the traditional Angband license.
 */

/*
 * To use this file, you must define "USE_GCU" in the Makefile.
 *
 * Hack -- note that "angband.h" is included AFTER the #ifdef test.
 * This was necessary because of annoying "curses.h" silliness.
 *
 * Note that this file is not "intended" to support non-Unix machines,
 * nor is it intended to support VMS or other bizarre setups.
 *
 * Also, this package assumes that the underlying "curses" handles both
 * the "nonl()" and "cbreak()" commands correctly, see the "OPTION" below.
 *
 * This code should work with most versions of "curses" or "ncurses",
 * and the "main-ncu.c" file (and USE_NCU define) are no longer used.
 *
 * See also "USE_CAP" and "main-cap.c" for code that bypasses "curses"
 * and uses the "termcap" information directly, or even bypasses the
 * "termcap" information and sends direct vt100 escape sequences.
 *
 * XXX XXX XXX This file provides only a single "term" window.
 *
 * The "init" and "nuke" hooks are built so that only the first init and
 * the last nuke actually do anything, but the other functions are not
 * "correct" for multiple windows.  Minor changes would also be needed
 * to allow the system to handle the "locations" of the various windows.
 *
 * But in theory, it should be possible to allow a 50 line screen to be
 * split into two sub-screens (i.e. "term_screen" and "term_mirror").
 *
 * XXX XXX XXX Consider the use of "savetty()" and "resetty()".
 */

#include "angband.h"


#ifdef USE_GCU


/*
 * Hack -- play games with "bool"
 */
#undef bool

/*
 * Include the proper "header" file
 */
#ifdef USE_NCURSES
# include <ncurses.h>
#else
# include <curses.h>
#endif



/*
 * Hack -- try to guess which systems use what commands
 * Hack -- allow one of the "USE_Txxxxx" flags to be pre-set.
 * Mega-Hack -- try to guess when "POSIX" is available.
 * If the user defines two of these, we will probably crash.
 */
#if !defined(USE_TPOSIX)
# if !defined(USE_TERMIO) && !defined(USE_TCHARS)
#  if defined(_POSIX_VERSION)
#   define USE_TPOSIX
#  else
#   if defined(USG) || defined(linux) || defined(SOLARIS)
#    define USE_TERMIO
#   else
#    define USE_TCHARS
#   endif
#  endif
# endif
#endif

/*
 * Hack -- Amiga uses "fake curses" and cannot do any of this stuff
 */
#if defined(AMIGA)
# undef USE_TPOSIX
# undef USE_TERMIO
# undef USE_TCHARS
#endif




/*
 * POSIX stuff
 */
#ifdef USE_TPOSIX
# include <sys/ioctl.h>
# include <termios.h>
#endif

/*
 * One version needs this file
 */
#ifdef USE_TERMIO
# include <sys/ioctl.h>
# include <termio.h>
#endif

/*
 * The other needs this file
 */
#ifdef USE_TCHARS
# include <sys/ioctl.h>
# include <sys/resource.h>
# include <sys/param.h>
# include <sys/file.h>
# include <sys/types.h>
#endif


/*
 * XXX XXX Hack -- POSIX uses "O_NONBLOCK" instead of "O_NDELAY"
 *
 * They should both work due to the "(i != 1)" test below.
 */
#ifndef O_NDELAY
# define O_NDELAY O_NONBLOCK
#endif


/*
 * OPTION: some machines lack "cbreak()"
 * On these machines, we use an older definition
 */
/* #define cbreak() crmode() */


/*
 * OPTION: some machines cannot handle "nonl()" and "nl()"
 * On these machines, we can simply ignore those commands.
 */
/* #define nonl() */
/* #define nl() */


/*
 * Save the "normal" and "angband" terminal settings
 */

#ifdef USE_TPOSIX

static struct termios  norm_termios;

static struct termios  game_termios;

#endif

#ifdef USE_TERMIO

static struct termio  norm_termio;

static struct termio  game_termio;

#endif

#ifdef USE_TCHARS

static struct ltchars norm_special_chars;
static struct sgttyb  norm_ttyb;
static struct tchars  norm_tchars;
static int            norm_local_chars;

static struct ltchars game_special_chars;
static struct sgttyb  game_ttyb;
static struct tchars  game_tchars;
static int            game_local_chars;

#endif



/*
 * Hack -- Number of initialized "term" structures
 */
static int active = 0;


/*
 * The main screen information
 */
static term term_screen_body;


#ifdef A_COLOR

/*
 * Hack -- define "A_BRIGHT" to be "A_BOLD", because on many
 * machines, "A_BRIGHT" produces ugly "inverse" video.
 */
#ifndef A_BRIGHT
# define A_BRIGHT A_BOLD
#endif

/*
 * Software flag -- we are allowed to use color
 */
static int can_use_color = FALSE;

/*
 * Software flag -- we are allowed to change the colors
 */
static int can_fix_color = FALSE;

/*
 * Simple Angband to Curses color conversion table
 */
static int colortable[16];


/**
 * Background color we should draw with; either BLACK or DEFAULT
 */
static int bg_color = COLOR_BLACK;

#endif



/*
 * Place the "keymap" into its "normal" state
 */
static void keymap_norm(void)
{

#ifdef USE_TPOSIX

	/* restore the saved values of the special chars */
	(void)tcsetattr(0, TCSAFLUSH, &norm_termios);

#endif

#ifdef USE_TERMIO

	/* restore the saved values of the special chars */
	(void)ioctl(0, TCSETA, (char *)&norm_termio);

#endif

#ifdef USE_TCHARS

	/* restore the saved values of the special chars */
	(void)ioctl(0, TIOCSLTC, (char *)&norm_special_chars);
	(void)ioctl(0, TIOCSETP, (char *)&norm_ttyb);
	(void)ioctl(0, TIOCSETC, (char *)&norm_tchars);
	(void)ioctl(0, TIOCLSET, (char *)&norm_local_chars);

#endif

}


/*
 * Place the "keymap" into the "game" state
 */
static void keymap_game(void)
{

#ifdef USE_TPOSIX

	/* restore the saved values of the special chars */
	(void)tcsetattr(0, TCSAFLUSH, &game_termios);

#endif

#ifdef USE_TERMIO

	/* restore the saved values of the special chars */
	(void)ioctl(0, TCSETA, (char *)&game_termio);

#endif

#ifdef USE_TCHARS

	/* restore the saved values of the special chars */
	(void)ioctl(0, TIOCSLTC, (char *)&game_special_chars);
	(void)ioctl(0, TIOCSETP, (char *)&game_ttyb);
	(void)ioctl(0, TIOCSETC, (char *)&game_tchars);
	(void)ioctl(0, TIOCLSET, (char *)&game_local_chars);

#endif

}


/*
 * Save the normal keymap
 */
static void keymap_norm_prepare(void)
{

#ifdef USE_TPOSIX

	/* Get the normal keymap */
	tcgetattr(0, &norm_termios);

#endif

#ifdef USE_TERMIO

	/* Get the normal keymap */
	(void)ioctl(0, TCGETA, (char *)&norm_termio);

#endif

#ifdef USE_TCHARS

	/* Get the normal keymap */
	(void)ioctl(0, TIOCGETP, (char *)&norm_ttyb);
	(void)ioctl(0, TIOCGLTC, (char *)&norm_special_chars);
	(void)ioctl(0, TIOCGETC, (char *)&norm_tchars);
	(void)ioctl(0, TIOCLGET, (char *)&norm_local_chars);

#endif

}


/*
 * Save the keymaps (normal and game)
 */
static void keymap_game_prepare(void)
{

#ifdef USE_TPOSIX

	/* Acquire the current mapping */
	tcgetattr(0, &game_termios);

	/* Force "Ctrl-C" to interupt */
	game_termios.c_cc[VINTR] = (char)3;

	/* Force "Ctrl-Z" to suspend */
	game_termios.c_cc[VSUSP] = (char)26;

	/* Hack -- Leave "VSTART/VSTOP" alone */

	/* Disable the standard control characters */
	game_termios.c_cc[VQUIT] = (char)-1;
	game_termios.c_cc[VERASE] = (char)-1;
	game_termios.c_cc[VKILL] = (char)-1;
	game_termios.c_cc[VEOF] = (char)-1;
	game_termios.c_cc[VEOL] = (char)-1;

	/* Normally, block until a character is read */
	game_termios.c_cc[VMIN] = 1;
	game_termios.c_cc[VTIME] = 0;

#endif

#ifdef USE_TERMIO

	/* Acquire the current mapping */
	(void)ioctl(0, TCGETA, (char *)&game_termio);

	/* Force "Ctrl-C" to interupt */
	game_termio.c_cc[VINTR] = (char)3;

	/* Force "Ctrl-Z" to suspend */
	game_termio.c_cc[VSUSP] = (char)26;

	/* Hack -- Leave "VSTART/VSTOP" alone */

	/* Disable the standard control characters */
	game_termio.c_cc[VQUIT] = (char)-1;
	game_termio.c_cc[VERASE] = (char)-1;
	game_termio.c_cc[VKILL] = (char)-1;
	game_termio.c_cc[VEOF] = (char)-1;
	game_termio.c_cc[VEOL] = (char)-1;

	/* Normally, block until a character is read */
	game_termio.c_cc[VMIN] = 1;
	game_termio.c_cc[VTIME] = 0;

#endif

#ifdef USE_TCHARS

	/* Get the default game characters */
	(void)ioctl(0, TIOCGETP, (char *)&game_ttyb);
	(void)ioctl(0, TIOCGLTC, (char *)&game_special_chars);
	(void)ioctl(0, TIOCGETC, (char *)&game_tchars);
	(void)ioctl(0, TIOCLGET, (char *)&game_local_chars);

	/* Force suspend (^Z) */
	game_special_chars.t_suspc = (char)26;

	/* Cancel some things */
	game_special_chars.t_dsuspc = (char)-1;
	game_special_chars.t_rprntc = (char)-1;
	game_special_chars.t_flushc = (char)-1;
	game_special_chars.t_werasc = (char)-1;
	game_special_chars.t_lnextc = (char)-1;

	/* Force interupt (^C) */
	game_tchars.t_intrc = (char)3;

	/* Force start/stop (^Q, ^S) */
	game_tchars.t_startc = (char)17;
	game_tchars.t_stopc = (char)19;

	/* Cancel some things */
	game_tchars.t_quitc = (char)-1;
	game_tchars.t_eofc = (char)-1;
	game_tchars.t_brkc = (char)-1;

#endif

}




/*
 * Suspend/Resume
 */
static errr Term_xtra_gcu_alive(int v)
{
	/* Suspend */
	if (!v)
	{
		/* Go to normal keymap mode */
		keymap_norm();

		/* Restore modes */
		nocbreak();
		echo();
		nl();

		/* Hack -- make sure the cursor is visible */
		Term_xtra(TERM_XTRA_SHAPE, 1);

		/* Flush the curses buffer */
		(void)refresh();

#ifdef SPECIAL_BSD
		/* this moves curses to bottom right corner */
		mvcur(curscr->cury, curscr->curx, LINES - 1, 0);
#else
		/* this moves curses to bottom right corner */
		mvcur(curscr->_cury, curscr->_curx, LINES - 1, 0);
#endif

		/* Exit curses */
		endwin();

		/* Flush the output */
		(void)fflush(stdout);
	}

	/* Resume */
	else
	{
		/* Refresh */
		/* (void)touchwin(curscr); */
		/* (void)wrefresh(curscr); */

		/* Restore the settings */
		cbreak();
		noecho();
		nonl();

		/* Go to angband keymap mode */
		keymap_game();
	}

	/* Success */
	return (0);
}




/*
 * Init the "curses" system
 */
static void Term_init_gcu(term *t)
{
	/* Unused parameter */
	(void)t;
	
	/* Count init's, handle first */
	if (active++ != 0) return;

	/* Erase the screen */
	(void)clear();

	/* Reset the cursor */
	(void)move(0, 0);

	/* Flush changes */
	(void)refresh();

	/* Game keymap */
	keymap_game();
}


/*
 * Nuke the "curses" system
 */
static void Term_nuke_gcu(term *t)
{
	/* Unused parameter */
	(void)t;
	
	/* Count nuke's, handle last */
	if (--active != 0) return;

	/* Hack -- make sure the cursor is visible */
	Term_xtra(TERM_XTRA_SHAPE, 1);

#ifdef SPECIAL_BSD
	/* This moves curses to bottom right corner */
	mvcur(curscr->cury, curscr->curx, LINES - 1, 0);
#else
	/* This moves curses to bottom right corner */
	mvcur(curscr->_cury, curscr->_curx, LINES - 1, 0);
#endif

	/* Flush the curses buffer */
	(void)refresh();

	/* Exit curses */
	endwin();

	/* Flush the output */
	(void)fflush(stdout);

	/* Normal keymap */
	keymap_norm();
}




#ifdef USE_GETCH

/*
 * Process events, with optional wait
 */
static errr Term_xtra_gcu_event(int v)
{
	int i, k;

	/* Wait */
	if (v)
	{
		/* Paranoia -- Wait for it */
		nodelay(stdscr, FALSE);

		/* Get a keypress */
		i = getch();

		/* Mega-Hack -- allow graceful "suspend" */
		for (k = 0; (k < 10) && (i == ERR); k++) i = getch();

		/* Broken input is special */
		if (i == ERR) exit_game_panic();
		if (i == EOF) exit_game_panic();
	}

	/* Do not wait */
	else
	{
		/* Do not wait for it */
		nodelay(stdscr, TRUE);

		/* Check for keypresses */
		i = getch();

		/* Wait for it next time */
		nodelay(stdscr, FALSE);

		/* None ready */
		if (i == ERR) return (1);
		if (i == EOF) return (1);
	}

	/* Enqueue the keypress */
	Term_keypress(i);

	/* Success */
	return (0);
}

#else	/* USE_GETCH */

/*
 * Process events (with optional wait)
 */
static errr Term_xtra_gcu_event(int v)
{
	int i, k;

	char buf[2];

	/* Wait */
	if (v)
	{
		/* Wait for one byte */
		i = read(0, buf, 1);

		/* Hack -- Handle bizarre "errors" */
		if ((i <= 0) && (errno != EINTR)) exit_game_panic();
	}

	/* Do not wait */
	else
	{
		/* Get the current flags for stdin */
		k = fcntl(0, F_GETFL, 0);

		/* Oops */
		if (k < 0) return (1);

		/* Tell stdin not to block */
		if (fcntl(0, F_SETFL, k | O_NDELAY) < 0) return (1);

		/* Read one byte, if possible */
		i = read(0, buf, 1);

		/* Replace the flags for stdin */
		if (fcntl(0, F_SETFL, k)) return (1);
	}

	/* Ignore "invalid" keys */
	if ((i != 1) || (!buf[0])) return (1);

	/* Enqueue the keypress */
	Term_keypress(buf[0]);

	/* Success */
	return (0);
}

#endif	/* USE_GETCH */

static int scale_color(int i, int j, int scale)
{
    return (angband_colour_table[i][j] * (scale - 1) + 127) / 255;
}

static int create_color(int i, int scale)
{
    int r = scale_color(i, 1, scale);
    int g = scale_color(i, 2, scale);
    int b = scale_color(i, 3, scale);
    int rgb = 16 + scale * scale * r + scale * g + b;
    /* In the case of white and black we need to use the ANSI colors */
    if (r == g && g == b)
    {
        if (b == 0) rgb = 0;
        if (b == scale) rgb = 15;
    }
    return rgb;
}

/*
 * React to changes
 */
static errr Term_xtra_gcu_react(void)
{

#ifdef A_COLOR
    if (COLORS == 256 || COLORS == 88)
    {
        /* CTK: I snagged this color handling from current Vanilla */
        /* If we have more than 16 colors, find the best matches. These numbers
        * correspond to xterm/rxvt's builtin color numbers--they do not
        * correspond to curses' constants OR with curses' color pairs.
        *
        * XTerm has 216 (6*6*6) RGB colors, with each RGB setting 0-5.
        * RXVT has 64 (4*4*4) RGB colors, with each RGB setting 0-3.
        *
        * Both also have the basic 16 ANSI colors, plus some extra grayscale
        * colors which we do not use.
        */
        int i;
        int scale = COLORS == 256 ? 6 : 4;
        for (i = 0; i < 16; i++)
        {
            int fg = create_color(i, scale);
            init_pair(i + 1, fg, bg_color);
            colortable[i] = COLOR_PAIR(i + 1) | A_NORMAL;
        }
    }

#endif

	/* Success */
	return (0);
}

/*
 * Handle a "special request"
 */
static errr Term_xtra_gcu(int n, int v)
{
	/* Analyze the request */
	switch (n)
	{
		/* Clear screen */
		case TERM_XTRA_CLEAR:
		touchwin(stdscr);
		(void)clear();
		return (0);

		/* Make a noise */
		case TERM_XTRA_NOISE:
		(void)write(1, "\007", 1);
		return (0);

		/* Flush the Curses buffer */
		case TERM_XTRA_FRESH:
		(void)refresh();
		return (0);

#ifdef USE_CURS_SET

		/* Change the cursor visibility */
		case TERM_XTRA_SHAPE:
		curs_set(v);
		return (0);

#endif

		/* Suspend/Resume curses */
		case TERM_XTRA_ALIVE:
		return (Term_xtra_gcu_alive(v));

		/* Process events */
		case TERM_XTRA_EVENT:
		return (Term_xtra_gcu_event(v));

		/* Flush events */
		case TERM_XTRA_FLUSH:
		while (!Term_xtra_gcu_event(FALSE));
		return (0);

		/* Delay */
		case TERM_XTRA_DELAY:
		usleep(1000 * v);
		return (0);

      /* React to events */
      case TERM_XTRA_REACT:
      Term_xtra_gcu_react();
      return (0);

	}

	/* Unknown */
	return (1);
}


/*
 * Actually MOVE the hardware cursor
 */
static errr Term_curs_gcu(int x, int y)
{
	/* Literally move the cursor */
	move(y, x);

	/* Success */
	return (0);
}


/*
 * Erase a grid of space
 * Hack -- try to be "semi-efficient".
 */
static errr Term_wipe_gcu(int x, int y, int n)
{
	/* Place cursor */
	move(y, x);

	/* Clear to end of line */
	if (x + n >= SCREEN_WID)
	{
		clrtoeol();
	}

	/* Clear some characters */
	else
	{
		while (n-- > 0) addch(' ');
	}

	/* Success */
	return (0);
}






/*
 * Place some text on the screen using an attribute
 */
static errr Term_text_gcu(int x, int y, int n, byte a, cptr s)
{
	int i;

	char text[81];

	/* Obtain a copy of the text */
	for (i = 0; i < n; i++) text[i] = s[i];
	text[n] = 0;

	/* Move the cursor and dump the string */
	move(y, x);

#ifdef A_COLOR
	/* Set the color */
	if (can_use_color) attrset(colortable[a & 0x0F]);
#endif

	/* Add the text */
	addstr(text);

	/* Success */
	return (0);
}




/*
 * Prepare "curses" for use by the file "term.c"
 *
 * Installs the "hook" functions defined above, and then activates
 * the main screen "term", which clears the screen and such things.
 *
 * Someone should really check the semantics of "initscr()"
 */
errr init_gcu(void)
{
	int i;

	term *t = &term_screen_body;


	/* Extract the normal keymap */
	keymap_norm_prepare();


#if defined(USG)
	/* Initialize for USG Unix */
	if (initscr() == NULL) return (-1);
#else
	/* Initialize for others systems */
	if (initscr() == (WINDOW*)ERR) return (-1);
#endif

	/* Hack -- Require large screen, or Quit with message */
	i = ((LINES < 24) || (COLS < 80));
	if (i) quit("Angband needs an 80x24 'curses' screen");

	/*
	* flexible term width & heigh
	*/
	session_width = COLS;
	session_height = LINES-2;

#ifdef A_COLOR

	/*** Init the Color-pairs and set up a translation table ***/

	/* Do we have color, and enough color, available? */
	can_use_color = ((start_color() != ERR) && has_colors() &&
	                 (COLORS >= 8) && (COLOR_PAIRS >= 8));

#ifdef REDEFINE_COLORS
	/* Can we change colors? */
	can_fix_color = (can_use_color && can_change_color() &&
	                 (COLOR_PAIRS >= 16));
#endif

	/* Attempt to use customized colors */
	if (can_fix_color)
	{
		/* Prepare the color pairs */
		for (i = 0; i < 16; i++)
		{
			/* Reset the color */
			init_pair(i, i, i);

			/* Reset the color data */
			colortable[i] = (COLOR_PAIR(i) | A_NORMAL);
		}

		/* XXX XXX XXX Take account of "gamma correction" */

		/* Prepare the "Angband Colors" */
		init_color(0,     0,    0,    0);	/* Black */
		init_color(1,  1000, 1000, 1000);	/* White */
		init_color(2,   500,  500,  500);	/* Grey */
		init_color(3,  1000,  500,    0);	/* Orange */
		init_color(4,   750,    0,    0);	/* Red */
		init_color(5,     0,  500,  250);	/* Green */
		init_color(6,     0,    0, 1000);	/* Blue */
		init_color(7,   500,  250,    0);	/* Brown */
		init_color(8,   250,  250,  250);	/* Dark-grey */
		init_color(9,   750,  750,  750);	/* Light-grey */
		init_color(10, 1000,    0, 1000);	/* Purple */
		init_color(11, 1000, 1000,    0);	/* Yellow */
		init_color(12, 1000,    0,    0);	/* Light Red */
		init_color(13,    0, 1000,    0);	/* Light Green */
		init_color(14,    0, 1000, 1000);	/* Light Blue */
		init_color(15,  750,  500,  250);	/* Light Brown */
	}

	/* Attempt to use colors */
	else if (can_use_color)
	{
		/* Color-pair 0 is *always* WHITE on BLACK */

		/* Prepare the color pairs */
		init_pair(1, COLOR_RED,     COLOR_BLACK);
		init_pair(2, COLOR_GREEN,   COLOR_BLACK);
		init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
		init_pair(4, COLOR_BLUE,    COLOR_BLACK);
		init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(6, COLOR_CYAN,    COLOR_BLACK);
		init_pair(7, COLOR_BLACK,   COLOR_BLACK);

		/* Prepare the "Angband Colors" -- Bright white is too bright */
		colortable[0] = (COLOR_PAIR(7) | A_NORMAL);	/* Black */
		colortable[1] = (COLOR_PAIR(0) | A_NORMAL);	/* White */
		colortable[2] = (COLOR_PAIR(6) | A_NORMAL);	/* Grey XXX */
		colortable[3] = (COLOR_PAIR(1) | A_BRIGHT);	/* Orange XXX */
		colortable[4] = (COLOR_PAIR(1) | A_NORMAL);	/* Red */
		colortable[5] = (COLOR_PAIR(2) | A_NORMAL);	/* Green */
		colortable[6] = (COLOR_PAIR(4) | A_NORMAL);	/* Blue */
		colortable[7] = (COLOR_PAIR(3) | A_NORMAL);	/* Umber */
		colortable[8] = (COLOR_PAIR(7) | A_BRIGHT);	/* Dark-grey XXX */
		colortable[9] = (COLOR_PAIR(6) | A_BRIGHT);	/* Light-grey XXX */
		colortable[10] = (COLOR_PAIR(5) | A_NORMAL);	/* Purple */
		colortable[11] = (COLOR_PAIR(3) | A_BRIGHT);	/* Yellow */
		colortable[12] = (COLOR_PAIR(5) | A_BRIGHT);	/* Light Red XXX */
		colortable[13] = (COLOR_PAIR(2) | A_BRIGHT);	/* Light Green */
		colortable[14] = (COLOR_PAIR(4) | A_BRIGHT);	/* Light Blue */
		colortable[15] = (COLOR_PAIR(3) | A_NORMAL);	/* Light Umber XXX */
	}

#endif


	/*** Low level preparation ***/

#ifdef USE_GETCH

	/* Paranoia -- Assume no waiting */
	nodelay(stdscr, FALSE);

#endif

	/* Prepare */
	cbreak();
	noecho();
	nonl();

	/* Extract the game keymap */
	keymap_game_prepare();


	/*** Now prepare the term ***/

	/* Initialize the term */
	term_init(t, SCREEN_WID, SCREEN_HGT+2, 256);

	/* Avoid the bottom right corner */
	t->icky_corner = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Set some hooks */
	t->init_hook = Term_init_gcu;
	t->nuke_hook = Term_nuke_gcu;

	/* Set some more hooks */
	t->text_hook = Term_text_gcu;
	t->wipe_hook = Term_wipe_gcu;
	t->curs_hook = Term_curs_gcu;
	t->xtra_hook = Term_xtra_gcu;

	/* Save the term */
	term_screen = t;

	/* Activate it */
	Term_activate(term_screen);


	/* Success */
	return (0);
}


#endif /* USE_GCU */


