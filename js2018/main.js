"use strict";
/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * or profit purposes provided that this source, copyright, and statement
 * are included in all such copies.
 *
 * Ben Harrison and others have released all changes to the Angband code
 * under the terms of the GNU General Public License (version 2), as well
 * as under the traditional Angband license. It may be redistributed under
 * the terms of the GPL (version 2 or any later version), or under the terms
 * of the traditional Angband license.
 *
 * All changes in Hellband are Copyright (c) 2005-2018 Tom J Demuyt aka Konijn
 * I Tom J Demuyt release all changes to the Angband code under the same terms.
 */

requirejs(['base', 'init2', 'log','os','files','globals','util','dungeon','gcu'],
  function (base, init2, log, os, files, g, util, dungeon,gcu) {

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
	g.ANGBAND_DIR = path;
	
	/*** Build the sub-directory names ***/
	g.ANGBAND_DIR_EDIT = path + 'edit';
	g.ANGBAND_DIR_FILE = path + 'file';
	g.ANGBAND_DIR_HELP = path + 'help';
	g.ANGBAND_DIR_INFO = path + 'info';
	g.ANGBAND_DIR_PREF = path + 'pref';
	g.ANGBAND_DIR_USER = path + 'user';
	g.ANGBAND_DIR_APEX = path + 'apex';
	g.ANGBAND_DIR_BONE = path + 'bone';
	g.ANGBAND_DIR_DATA = path + 'data';
	g.ANGBAND_DIR_SAVE = path + 'save';
	g.ANGBAND_DIR_DUMP = path + 'dump';
	g.ANGBAND_DIR_XTRA = path + 'xtra';
	
	log(log.DEBUG, "User dir: ", g.ANGBAND_DIR_USER);

	/*** Verify the "news" file ***/

  newsFile = g.ANGBAND_DIR_FILE + '/news.txt',
  client = new XMLHttpRequest();
      
  client.open('GET', newsFile);
  
  client.onreadystatechange = function() {
    g.fc_news = client.responseText;
    console.log(g.fc_news);
  };
  client.onError = function(){
		log(log.ERROR, client.statusText, "Cannot access the news file at ", newsFile);
		init2.init_angband_aux(client.statusText);
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
	let path = './lib/';
	/* Initialize */
	init_file_paths(path,1);
}

/*
* A hook for "quit()".
* No Closing down really in JS, we navigate to the quit page is all
*/
function quit_hook(s)
{
  os.quit(s);
}


/*
 * Simple "main" function
 * The passing of start parameters is kinda hard
 * TODO: acccept these parameters from the URL?
 */
function main(argv)
{
  argv = base.listify(argv);
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
    g.arg_fiddle = g.arg_fiddle || arg == 'F';
    g.arg_sound = g.arg_sound || arg == 'V';
    g.arg_graphics = g.arg_graphics || arg == 'G';
		g.arg_force_roguelike = g.arg_force_roguelike || arg == 'R';
    g.arg_force_original = g.arg_force_original || arg == 'O';
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
		return os.quit('Hellband was started with the wrong parameters');
  }
  
	/* Process the player name */
	files.process_player_name();

	/* Install "quit" hook */
	g.quit_aux = quit_hook;

  /* GCU, baby!! */
  gcu.init_gcu(argc, argv);
	g.ANGBAND_SYS = "gcu";
  done = 1;

	/* Hack -- If requested, display scores and return, info would be gone if we quit */
	if (show_score > 0){
	  return files.display_scores(0, show_score);
	}

	/* Initialize */
	init2.init_angband();

	/* Wait for response */
	util.pause_line(23);

	/* Play the game */
	dungeon.play_game(new_game);

	/* Exit */
	return (0);
}

//console.log('Are we ready yet?');
//os.whenReady(main);
main();

});