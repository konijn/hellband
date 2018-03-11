"use strict";

/*
* Explain a broken "lib" folder and quit (see below).
*/


define(['log'],function(log){

  function init_angband_aux(why)
  {
	  /* Why */
	  log(ERROR, why);
	  /* Explain */
	  log(ERROR, "The 'lib' directory is probably missing or broken.");
	  /* Guide */
	  log(ERROR, "See the 'README' file for more information.");
	  /* Quit with error */
	  quit("Fatal Error.");
  }
  
  function init_angband()
  {
    //TODO
    log(log.ERROR, "init.init_angband is not yet implemented");
  }
  
  return {init_angband_aux, init_angband};
});