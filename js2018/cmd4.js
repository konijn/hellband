"use strict";

define(['log','globals','os','term'],function(log, g, os, term){

/*
* Hack -- load a screen dump from a file
*/
function do_cmd_load_screen(path, file)
{
  let i, y, x,
      a = 0,
      c = ' ',
      read_result,
      lines,
      okay = true;

  /* Build the filename */
  file = path + '/' + file;

  /* Read the file */
  read_result = os.load_file(file);

  /* Oops */
  if(!read_result.ok) return;

  /* Clear the screen */
  os.term_clear();

  /*Split the file content in lines*/
  lines = read_result.content.split('\n');

  /* Load the screen */
  for (y = 0; okay && (y < 24); y++)
  {
    /* Show each row */
    for (x = 0; x < 79; x++)
    {
      /*First 24 lines have chars, then there is blank line, and then the color chars*/
      /* Put the attr/char */
      if(lines[25+y][x]!= " "){
        x=x;
      }
      term.term_draw(x, y, term.colour_char_to_attr(lines[25+y][x]), lines[y][x]);
    }
  }
}



  return {do_cmd_load_screen};
});