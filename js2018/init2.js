"use strict";

/*
* Explain a broken "lib" folder and quit (see below).
*/


define(['log','os','cmd4','globals','term'],function(log, os, cmd4, g, term){

  function init_angband_aux(why)
  {
    /* Why */
    log(log.ERROR, why);
    /* Explain */
    log(log.ERROR, "The 'lib' directory is probably missing or broken.");
    /* Guide */
    log(log.ERROR, "See the 'README' file for more information.");
    /* Quit with error */
    os.quit("Fatal Error.");
  }

  /*
  * Main Angband initialization entry point
  *
  * Verify some files, display the "news.txt" file, create
  * the high score 'file', initialize all internal arrays, and
  * load the basic "user pref files".
  *
  * No need to be careful to keep track of the order in which
  * things were initialized, in particular, we *know* that rot.js
  * is available when this function is called in the "z-term.c"
  * package.
  
  * Note that this function displays the "news_color" file.
  * It was verified prior to this functional call that the file is there
  *
  * Note that this function attempts to verify (or create) the
  * "high score" file, and the game aborts (cleanly) on failure.
  * The JS version (or any version since a least a decade) prevents
  * failure to extract all sub-directories (even empty ones).
  *
  * Note that various things are initialized by this function,
  * including everything that was once done by "init_some_arrays".
  *
  * This initialization involves the parsing of special files
  * in the "lib/data" and sometimes the "lib/edit" directories.
  *
  * Note that the "template" files are initialized first, since they
  * often contain errors.  This means that macros and message recall
  * and things like that are not available until after they are done.
  *
  * We load the default "user pref files" here in case any "colour"
  * changes are needed before character creation.
  *
  * term.note that the "graf-xxx.prf" file must be loaded separately,
  * if needed, in the first (?) pass through "TERM_XTRA_REACT".
  */
  
  function init_angband()
  {
    let buf;

    /* Aw screw it, everybody loves colour ;) */
    g.use_colour = 1;
    cmd4.do_cmd_load_screen(g.ANGBAND_DIR_FILE, "news_color.txt");

    /* Hack, now we cannot changes really news_color.txt, not drastically anyway */
    term.term_putstr(61, 5, g.TERM_RED, g.VERSION);

    /*** Verify (or create) the "high score" file ***/

    /* Build the filename */
    let scores_filename = g.ANGBAND_DIR_APEX + "/scores.raw";

    /* Attempt to open the high score file */
    let ws = os.ws_open(scores_filename);

    /* Failure */
    if (!ws)
    {
      /* Create a new high score file */
      ws = os.ws_make();

      /* Check for failure */
      if (!ws)
      {
        /* Crash and burn */
        init_angband_aux("Cannot create the '" + scores_filename + "' file!");
      }
    }

    /*** Initialize some arrays ***/

    /* Initialize feature info */
    term.note("[Initializing arrays... (features)]");
    if (!init_f_info()) quit("Cannot initialize features");

    /* Initialize object info */
    term.note("[Initializing arrays... (objects)]");
    if (init_k_info()) quit("Cannot initialize objects");

    /* Initialize artefact info */
    term.note("[Initializing arrays... (artefacts)]");
    if (init_a_info()) quit("Cannot initialize artefacts");

    /* Initialize ego-item info */
    term.note("[Initializing arrays... (ego-items)]");
    if (init_e_info()) quit("Cannot initialize ego-items");

    /* Initialize monster info */
    term.note("[Initializing arrays... (monsters)]");
    if (init_r_info()) quit("Cannot initialize monsters");

    /* Initialize feature info */
    term.note("[Initializing arrays... (vaults)]");
    if (init_v_info()) quit("Cannot initialize vaults");

    /* Initialize some other arrays */
    term.note("[Initializing arrays... (other)]");
    if (init_other()) quit("Cannot initialize other stuff");

    /* Initialize some other arrays */
    term.note("[Initializing arrays... (alloc)]");
    if (init_alloc()) quit("Cannot initialize alloc stuff");


    /*** Load default user pref files ***/

    /* Initialize feature info */
    term.note("[Initializing user pref files...]");

    /* build a name for the basic 'help' index */
    sprintf(syshelpfile,"help-%s.txt", "win");

    /* build a name for the basic 'help' index */
    sprintf(syshelpfile_birth,"chargen.txt");
  
  
    /* Access the "basic" pref file */
    strcpy(buf, "pref.prf");

    /* Process that file */
    process_pref_file(buf);

    /* Access the "user" pref file */
    sprintf(buf, "user.prf");

    /* Process that file */
    process_pref_file(buf);

    /* Access the "basic" system pref file */
    sprintf(buf, "pref-%s.prf", ANGBAND_SYS);

    /* Process that file */
    process_pref_file(buf);

    /* Access the "user" system pref file */
    sprintf(buf, "user-%s.prf", ANGBAND_SYS);

    /* Process that file */
    process_pref_file(buf);

    /* Done */
    term.note("[Initialization complete]");
  }
  

  /*
  * Initialize the "f_info" array, by parsing an ascii "template" file
  * Note, we dont do raw binaries anymore, there is no point any more.
  * So really, the below is more based on init_f_info_txt then init_f_info
  */
  /*BOOL*/ function init_f_info()
  {
    let path = g.ANGBAND_DIR_EDIT + '/f_info.txt',
     read_result = os.load_file(path),
     f_info = g.f_info,
     lines, line, version_okay, parts, current_feature;
    /* Did we read f_info correctly? */
    if(!read_result.ok)
      return false;
    /*Split the file content into one-line strings*/
    lines = read_result.content.split('\n');
    /*Treat every line*/
    for(line of lines){
      /* Hack -- Process 'V' for "Version" */
      if(line.startsWith('V')){
        version_okay = check_version(line);
        if(!version_okay){
          return false;
        }
      }
      /* Process 'N' for "New/Number/Name" */
      if(line.startsWith('N')){
        parts = line.split(':');
        current_feature = parts[1];
        f_info[current_feature] = {name: parts[2]};
      }
      /* Process 'M' for "Mimic" (one line only) */
      if(line.startsWith('M')){
        parts = line.split(':');
        f_info[current_feature].mimic = parts[1];
      }
      /* Process 'G' for "Graphics" (one line only) */
      if(line.startsWith('G')){
        parts = line.split(':');
        f_info[current_feature].f_char = parts[1];
        f_info[current_feature].f_attr = term.colour_char_to_attr(parts[2].trim());
      }
    }
    console.log(f_info);
    return true;
  }

  /*This assume a string starting with V:*/
  /*BOOL*/ function check_version(line)
  {
    return line.trim().mid(2) == g.VERSION;
  }

  return {init_angband_aux, init_angband};
});




