"use strict";

define(['log','globals','os'],function(log,g,os){

  function display_scores(from, to){
    //TODO
    log(log.ERROR, "files.display_scores is not yet implemented");
  }

  /*
  * Process the player name.
  * Extract a clean "base name".
  * Build the savefile name if needed.
  */
  function process_player_name()
  {
    //If we havent gotten a player name yet, then use PLAYER
    if(!g.player_name){
      g.player_name = 'PLAYER';
    }
    /* Cannot contain "icky" characters, even in 2018*/
    if(g.player_name != escape(g.player_name)){
      os.quit('The name ' + g.player_name + 'contains icky characters (' + escape(g.player_name) + ')');
    }
	  /* Change the savefile name */
	  g.savefile = g.savefile || g.player_name;
  }
  
  return {display_scores,process_player_name};
});