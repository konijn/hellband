
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
  
  function term_draw(col, row, attr, ch){
    os.draw(col, row, ch, attr);
  }
  
  /*
  * Map a ch to a color index
  */
  let ch_color_map = {
    d:0,  /* TERM_DARK */
    w:1,  /* TERM_WHITE */
    s:2,  /* TERM_SLATE*/
    o:3,  /* TERM_ORANGE */
    r:4,  /* TERM_RED */
    g:5,  /* TERM_GREEN */
    b:6,  /* TERM_BLUE */
    u:7,  /* TERM_UMBER */
    D:8,  /* TERM_L_DARK */
    W:9,  /* TERM_L_WHITE*/
    v:10, /* TERM_VIOLET*/
    y:11, /* TERM_YELLOW*/
    R:12, /* TERM_L_RED*/
    G:13, /* TERM_L_GREEN*/
    B:14, /* TERM_L_BLUE*/
    U:15  /* TERM_L_UMBER*/
  }
  function map_ch_color(ch){
    return ch_color_map[ch];
  }
  
  return {note,term_putstr,prt,c_prt,map_ch_color,term_draw};
});