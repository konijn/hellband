"use strict";

define(['os','globals'], function gcu(os,g)
{
  let can_use_color = false,
      can_fix_color = false,
      colortable = new Int16Array(length);
  
  /*
   * Save the normal keymap
  */
  function keymap_norm_prepare()
  {
    //LOL, no idea what this does..
  }

  function keymap_game_prepare()
  {
    //Still no idea..
  }


  /*
   * Prepare "curses" for use by the file "term.js"
   *
   * Installs the "hook" functions defined above, and then activates
   * the main screen "term", which clears the screen and such things.
   *
   * Someone should really check the semantics of "initscr()"
  */
  function init_gcu()
  {
    let i;

    /* Extract the normal keymap */
    keymap_norm_prepare();

    if (!os.initscr()){
      return (-1);
    }

    /* Hack -- Require large screen, or Quit with message */
    i = ((os.LINES < 24) || (os.COLS < 80));
    if (i){
      os.quit("Angband needs an 80x24 'curses' screen");
    }
  
    /*** Init the Color-pairs and set up a translation table ***/
    /* Prepare the "Angband Colors" */
    os.init_color(0,     0,    0,    0); /* Black */
    os.init_color(1,  1000, 1000, 1000); /* White */
    os.init_color(2,   500,  500,  500); /* Grey */
    os.init_color(3,  1000,  500,    0); /* Orange */
    os.init_color(4,   750,    0,    0); /* Red */
    os.init_color(5,     0,  500,  250); /* Green */
    os.init_color(6,     0,    0, 1000); /* Blue */
    os.init_color(7,   500,  250,    0); /* Brown */
    os.init_color(8,   250,  250,  250); /* Dark-grey */
    os.init_color(9,   750,  750,  750); /* Light-grey */
    os.init_color(10, 1000,    0, 1000); /* Purple */
    os.init_color(11, 1000, 1000,    0); /* Yellow */
    os.init_color(12, 1000,    0,    0); /* Light Red */
    os.init_color(13,    0, 1000,    0); /* Light Green */
    os.init_color(14,    0, 1000, 1000); /* Light Blue */
    os.init_color(15,  750,  500,  250); /* Light Brown */
  
    /* Extract the game keymap */
    keymap_game_prepare();

    /*** Now prepare the term ***/

    /* Initialize the term */
    let t = os.term_init( 80, 24, 256);

    t.icky_corner = true;

    /* Erase with "white space" */
    t.attr_blank = g.TERM_WHITE;
    t.char_blank = ' ';

    /* Set some hooks */
    t.init_hook = os.Term_init_gcu;
    t.nuke_hook = os.Term_nuke_gcu;

    /* Set some more hooks */
    t.text_hook = os.Term_text_gcu;
    t.wipe_hook = os.Term_wipe_gcu;
    t.curs_hook = os.Term_curs_gcu;
    t.xtra_hook = os.Term_xtra_gcu;

    /* Save the term */
    g.term_screen = t;

    /* Activate it */
    os.term_activate(g.term_screen);

    /* Success */
    return (0);
  }

  return {init_gcu};

});