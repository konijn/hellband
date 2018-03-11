
"use strict";

define(['log','globals','os'],function(log,g,os){

  /*
  * Hack -- Show information on the screen, one line at a time.
  * Avoid the top two lines, to avoid interference with "msg_print()".
  */
  function note(msg)
  {
    prt(msg, 2, 0);
  }
  
  /*
  * Display a string on the screen using an attribute, and clear
  * to the end of the line.
  */
  function c_prt(attr, str, row, col)
  {
	  /* Hack -- fake monochrome */
	  if (!g.use_colour) attr = g.TERM_WHITE;
	  /* We add spaces till we fill a screen */
	  str = str + ' '.repeat(os.get_active_term().getOptions().width - col);
	  /* Dump the attr/text */
	  term_putstr(col, row, attr, str);
  }
  
  /*
  * As above, but in "white"
  */
  function prt(str, row, col)
  {
	  /* Spawn */
	  c_prt(g.TERM_WHITE, str, row, col);
  }

  /*
  * Place a string at a given location
  */
  function term_putstr(col, row, attr, s)
  {
    s.each((c,i) => os.draw(col + i, row, c, attr));
  }
  
  return {note,term_putstr,prt,c_prt};
});