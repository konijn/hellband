"use strict";

/*
* Explain a broken "lib" folder and quit (see below).
*/
function init_angband_aux(why)
{
	/* Why */
	console(why);

	/* Explain */
	console.log("The 'lib' directory is probably missing or broken.");

	/* Explain */
	console.log("See the 'README' file for more information.");

	/* Quit with error */
	quit("Fatal Error.");
}