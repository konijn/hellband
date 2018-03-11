"use strict";
/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 *
 * Ben Harrison and others have released all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2 or any later version),
 * or under the terms of the traditional Angband license.
 *
 * All changes in Hellband are Copyright (c) 2005-2007 Konijn
 * I Konijn  release all changes to the Angband code under the terms of the GNU General Public License (version 2),
 * as well as under the traditional Angband license. It may be redistributed under the terms of the GPL (version 2),
 * or under the terms of the traditional Angband license.
 */

//Globals
var ANGBAND_DIR,
    ANGBAND_DIR_EDIT,
    ANGBAND_DIR_FILE,
    ANGBAND_DIR_HELP,
    ANGBAND_DIR_INFO,
    ANGBAND_DIR_PREF,
    ANGBAND_DIR_USER,
    ANGBAND_DIR_APEX,
    ANGBAND_DIR_BONE,
    ANGBAND_DIR_DATA,
    ANGBAND_DIR_SAVE,
    ANGBAND_DIR_DUMP,
    ANGBAND_DIR_XTRA,
    ANGBAND_SYS;
    
var arg_fiddle, arg_sound, arg_graphics, arg_force_roguelike, arg_force_original;

//fc=>file content
var fc_news;

/*
 * Find the default paths to all of our important sub-directories.
 * The purpose of each sub-directory is described in "variable.js".
 *
 * All of the sub-directories must be located inside
 * the main "lib" directory, whose location is fixed in the
 * JavaScript version.
 *
 * The initial path must end with '/' All of the
 * "sub-directory" paths will NOT end with '/',
 * The special path_build() function in "util.js" is now obsolete.
 */
function init_file_paths(path, attempt)
{
	let separator = '/', newsFile, client;

	/*** Prepare the "path" ***/
	/* Hack -- save the main directory */
	ANGBAND_DIR = path;
	
	/*** Build the sub-directory names ***/
	ANGBAND_DIR_EDIT = path + 'edit';
	ANGBAND_DIR_FILE = path + 'file';
	ANGBAND_DIR_HELP = path + 'help';
	ANGBAND_DIR_INFO = path + 'info';
	ANGBAND_DIR_PREF = path + 'pref';
	ANGBAND_DIR_USER = path + 'user';
	ANGBAND_DIR_APEX = path + 'apex';
	ANGBAND_DIR_BONE = path + 'bone';
	ANGBAND_DIR_DATA = path + 'data';
	ANGBAND_DIR_SAVE = path + 'save';
	ANGBAND_DIR_DUMP = path + 'dump';
	ANGBAND_DIR_XTRA = path + 'xtra';
	
	log(DEBUG, "User dir: ", ANGBAND_DIR_USER);

	/*** Verify the "news" file ***/

  newsFile = ANGBAND_DIR_FILE + '/news.txt',
  client = new XMLHttpRequest();
      
  client.open('GET', newsFile);
  
  client.onreadystatechange = function() {
    fc_news = client.responseText;
    console.log(fc_news);
  };
  client.onError = function(){
		log(ERROR, client.statusText, "Cannot access the news file at ", newsFile);
		init_angband_aux(why);
  };
  client.send();
  console.log('Testing Synchronicity');
}

/*
* Initialize and verify access to the file paths, and the score file.
* Data files are retrieved with http (probably with require), scores are stored in localStorage
*/
function init_stuff()
{
  /* Things are simpler in js land ;) */
	let path = './';
	/* Initialize */
	init_file_paths(path,1);
}


/*
 * Simple "main" function
 * The passing of start parameters is kinda hard
 * TODO: acccept these parameters from the URL?
 */
function main(argv){
  argv = listify(argv);
	let i,
      done = false,
	    new_game = false,
      show_score = 0,
      mstr = '',
      args = true,
      oops = false,
      argc = argv.length;

	/* Get the file paths */
	init_stuff();

  for(i=0; i<argv.length; i++){
    let arg = argv[i];
    if(!arg.startsWith('-')){
      oops = true;
      break;
    }else{
      arg = arg.substring(1,2).toUpperCase();
    }
    new_game = new_game || arg == 'N';
    arg_fiddle = arg_fiddle || arg == 'F';
    arg_sound = arg_sound || arg == 'V';
    arg_graphics = arg_graphics || arg == 'G';
		arg_force_roguelike = arg_force_roguelike || arg == 'R';
    arg_force_original = arg_force_original || arg == 'O';
    if(arg=='S')
      show_score = arg.substring(1);
  }
  
  if(oops){
		console.log("You started hellband with this parameter :");
		console.log( argv[i] );
		console.log("Usage: Hellband [options] [-- subopts]");
		console.log("  -n       Start a new character");
		console.log("  -f       Request fiddle mode");
		console.log("  -v       Request sound mode");
		console.log("  -g       Request graphics mode");
		console.log("  -o       Request original keyset");
		console.log("  -r       Request rogue-like keyset");
		console.log("  -s<num>  Show <num> high scores");
    /* Actually abort the process */
		return quit('Hellband was started with the wrong parameters');
  }
  
	/* Process the player name */
	process_player_name();

	/* Install "quit" hook */
	quit_aux = quit_hook;

  /* GCU, baby!! */
  ANGBAND_SYS = "gcu";
  done = 1;

	/* Hack -- If requested, display scores and quit */
	if (show_score > 0){
	  return display_scores(0, show_score);
	}

	/* Initialize */
	init_angband();

	/* Wait for response */
	pause_line(23);

	/* Play the game */
	play_game(new_game);

	/* Exit */
	return (0);
}

main();