"use strict";

/*** Helper arrays for parsing ascii template files ***/

/* Monster Blow Method bitwise flags */
const r_info_blow_method = [
  "",
  "HIT",
  "TOUCH",
  "PUNCH",
  "KICK",
  "CLAW",
  "BITE",
  "STING",
  "XXX1",
  "BUTT",
  "CRUSH",
  "ENGULF",
  "CHARGE",  /* WAS: XXX2 */
  "CRAWL",
  "DROOL",
  "SPIT",
  "XXX3",
  "GAZE",
  "WAIL",
  "SPORE",
  "WORSHIP",
  "BEG",
  "INSULT",
  "MOAN",
  "SHOW",  /* WAS: XXX5 */
  null
];


/* Monster Blow Effects bitwise effects */
const r_info_blow_effect = [
  "",
  "HURT",
  "POISON",
  "UN_BONUS",
  "UN_POWER",
  "EAT_GOLD",
  "EAT_ITEM",
  "EAT_FOOD",
  "EAT_LITE",
  "ACID",
  "ELEC",
  "FIRE",
  "COLD",
  "BLIND",
  "CONFUSE",
  "TERRIFY",
  "PARALYZE",
  "LOSE_STR",
  "LOSE_INT",
  "LOSE_WIS",
  "LOSE_DEX",
  "LOSE_CON",
  "LOSE_CHA",
  "LOSE_ALL",
  "SHATTER",
  "EXP_10",
  "EXP_20",
  "EXP_40",
  "EXP_80",
  null
];


/* Monster race bitwise flags */
const r_info_flags1 = [
  "UNIQUE",
  "GUARDIAN",
  "MALE",
  "FEMALE",
  "CHAR_CLEAR",
  "CHAR_MULTI",
  "ATTR_CLEAR",
  "ATTR_MULTI",
  "ALWAYS_GUARD",
  "FORCE_MAXHP",
  "FORCE_SLEEP",
  "FORCE_EXTRA",
  "FRIEND",
  "FRIENDS",
  "ESCORT",
  "ESCORTS",
  "NEVER_BLOW",
  "NEVER_MOVE",
  "RAND_25",
  "RAND_50",
  "ONLY_GOLD",
  "ONLY_ITEM",
  "DROP_60",
  "DROP_90",
  "DROP_1D2",
  "DROP_2D2",
  "DROP_3D2",
  "DROP_4D2",
  "DROP_GOOD",
  "DROP_GREAT",
  "DROP_USEFUL",
  "DROP_CHOSEN"
];

/*
* Monster race flags
*/
const r_info_flags2 = [
  "STUPID",
  "SMART",
  "CAN_SPEAK",
  "REFLECTING",
  "INVISIBLE",
  "COLD_BLOOD",
  "EMPTY_MIND",
  "WEIRD_MIND",
  "MULTIPLY",
  "REGENERATE",
  "SHAPECHANGER",
  "ATTR_ANY",
  "POWERFUL",
  "ELDRITCH_HORROR",
  "AURA_FIRE",
  "AURA_ELEC",
  "OPEN_DOOR",
  "BASH_DOOR",
  "PASS_WALL",
  "KILL_WALL",
  "MOVE_BODY",
  "KILL_BODY",
  "TAKE_ITEM",
  "KILL_ITEM",
  "BRAIN_1",
  "BRAIN_2",
  "BRAIN_3",
  "BRAIN_4",
  "BRAIN_5",
  "BRAIN_6",
  "BRAIN_7",
  "BRAIN_8"
];

/*
* Monster race flags
*/
const r_info_flags3 = [
  "ORC",
  "TROLL",
  "GIANT",
  "DRAGON",
  "DEMON",
  "UNDEAD",
  "EVIL",
  "ANIMAL",
  "FALLEN_ANGEL",
  "GOOD",
  "XXXX", /* RF3_PLAYER_GHOST */
  "NONLIVING",
  "HURT_LITE",
  "HURT_ROCK",
  "HURT_FIRE",
  "HURT_COLD",
  "IM_ACID",
  "IM_ELEC",
  "IM_FIRE",
  "IM_COLD",
  "IM_POIS",
  "RES_TELE",
  "RES_NETH",
  "RES_WATE",
  "RES_PLAS",
  "RES_NEXU",
  "RES_DISE",
  "DEVIL",
  "NO_FEAR",
  "NO_STUN",
  "NO_CONF",
  "NO_SLEEP"
];

/*
* Monster race flags
*/
const r_info_flags4 = [
  "SHRIEK",
  "XXX2X4",
  "XXX3X4",
  "BA_SHARD",
  "ARROW_1",
  "ARROW_2",
  "ARROW_3",
  "ARROW_4",
  "BR_ACID",
  "BR_ELEC",
  "BR_FIRE",
  "BR_COLD",
  "BR_POIS",
  "BR_NETH",
  "BR_LITE",
  "BR_DARK",
  "BR_CONF",
  "BR_SOUN",
  "BR_CHAO",
  "BR_DISE",
  "BR_NEXU",
  "BR_TIME",
  "BR_INER",
  "BR_GRAV",
  "BR_SHAR",
  "BR_PLAS",
  "BR_WALL",
  "BR_MANA",
  "BA_SLIM",
  "BR_SLIM",
  "BA_CHAO",
  "BR_DISI",
];

/* Monster race bitwise flags */
const r_info_flags5 = [
  "BA_ACID",
  "BA_ELEC",
  "BA_FIRE",
  "BA_COLD",
  "BA_POIS",
  "BA_NETH",
  "BA_WATE",
  "BA_MANA",
  "BA_DARK",
  "DRAIN_MANA",
  "MIND_BLAST",
  "BRAIN_SMASH",
  "CAUSE_1",
  "CAUSE_2",
  "CAUSE_3",
  "CAUSE_4",
  "BO_ACID",
  "BO_ELEC",
  "BO_FIRE",
  "BO_COLD",
  "BO_POIS",
  "BO_NETH",
  "BO_WATE",
  "BO_MANA",
  "BO_PLAS",
  "BO_ICEE",
  "MISSILE",
  "SCARE",
  "BLIND",
  "CONF",
  "SLOW",
  "HOLD"
];

/* Monster race bitwise flags */
const r_info_flags6 = [
  "HASTE",
  "CAIN",
  "HEAL",
  "XXX2X6",
  "BLINK",
  "TPORT",
  "XXX3X6",
  "XXX4X6",
  "TELE_TO",
  "TELE_AWAY",
  "TELE_LEVEL",
  "XXX5",
  "DARKNESS",
  "TRAPS",
  "FORGET",
  "XXX6X6",
  "S_KIN",
  "S_REAVER",
  "S_MONSTER",
  "S_MONSTERS",
  "S_ANT",
  "S_SPIDER",
  "S_HOUND",
  "S_HYDRA",
  "S_DEVIL",
  "S_DEMON",
  "S_UNDEAD",
  "S_DRAGON",
  "S_HI_UNDEAD",
  "S_HI_DRAGON",
  "S_FALLEN",
  "S_UNIQUE"
];

/* Monster bitwise race flags */
const r_info_flags7 = [
  "ANNOYED",
  "NO_SPAWN",
  "STORM",
  "RES_DARK",
  "IM_DARK",
  "HEAL_DARK",
  "HURT_DARK",
  "RES_LITE",
  "IM_LITE",
  "HEAL_LITE",
  "RES_COLD",
  "HEAL_COLD",
  "RES_FIRE",
  "HEAL_FIRE",
  "RES_ACID",
  "RES_ELEC",
  "HEAL_ELEC",
  "HEAL_NETH",
  "RES_POIS",
  "AQUATIC",
  "FLIGHT",
  "NEUTRAL",
  "REBORN",
  "INFERNO_OPEN",
  "INFERNO_OPEN",
  "INFERNO_OPEN",
  "INFERNO_OPEN",
  "INFERNO_OPEN",
  "INFERNO_OPEN",
  "INFERNO_OPEN",
  "INFERNO_OPEN",
  "INFERNO_OPEN"
];

/* Object bitwise flags */
const k_info_flags1 = [
  "STR",
  "INT",
  "WIS",
  "DEX",
  "CON",
  "CHA",
  "XXX1",
  "XXX2",
  "STEALTH",
  "SEARCH",
  "INFRA",
  "TUNNEL",
  "SPEED",
  "BLOWS",
  "CHAOTIC",
  "VAMPIRIC",
  "SLAY_ANIMAL",
  "SLAY_EVIL",
  "SLAY_UNDEAD",
  "SLAY_DEMON",
  "SLAY_ANGEL", /*"SLAY_ORC",*/
  "KILL_ANGEL", /*"SLAY_TROLL",*/
  "SLAY_GIANT",
  "SLAY_DRAGON",
  "KILL_DRAGON",
  "VORPAL",
  "IMPACT",
  "BRAND_POIS",
  "BRAND_ACID",
  "BRAND_ELEC",
  "BRAND_FIRE",
  "BRAND_COLD"
];

/* Object bitwise flags */
const k_info_flags2 = [
  "SUST_STR",
  "SUST_INT",
  "SUST_WIS",
  "SUST_DEX",
  "SUST_CON",
  "SUST_CHA",
  "XXX1",
  "XXX2",
  "IM_ACID",
  "IM_ELEC",
  "IM_FIRE",
  "IM_COLD",
  "XXX3",
  "REFLECT",
  "FREE_ACT",
  "HOLD_LIFE",
  "RES_ACID",
  "RES_ELEC",
  "RES_FIRE",
  "RES_COLD",
  "RES_POIS",
  "RES_FEAR",
  "RES_LITE",
  "RES_DARK",
  "RES_BLIND",
  "RES_CONF",
  "RES_SOUND",
  "RES_SHARDS",
  "RES_NETHER",
  "RES_NEXUS",
  "RES_CHAOS",
  "RES_DISEN"
];

/* Object bitwise flags */
const k_info_flags3 = [
  "SH_FIRE",
  "SH_ELEC",
  "XP",
  "XXX4",
  "NO_TELE",
  "NO_MAGIC",
  "WRAITH",
  "TY_CURSE",
  "EASY_KNOW",
  "HIDE_TYPE",
  "SHOW_MODS",
  "INSTA_ART",
  "FEATHER",
  "LITE",
  "SEE_INVIS",
  "TELEPATHY",
  "SLOW_DIGEST",
  "REGEN",
  "XTRA_MIGHT",
  "XTRA_SHOTS",
  "IGNORE_ACID",
  "IGNORE_ELEC",
  "IGNORE_FIRE",
  "IGNORE_COLD",
  "ACTIVATE",
  "DRAIN_EXP",
  "TELEPORT",
  "AGGRAVATE",
  "BLESSED",
  "CURSED",
  "HEAVY_CURSE",
  "PERMA_CURSE"
];


define(['log','os','cmd4','globals','term'],function(log, os, cmd4, g, term){

  /*
  * Explain a broken "lib" folder and quit (see below).
  */
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
    if (!init_f_info()) os.quit("Cannot initialize features");

    /* Initialize object info */
    term.note("[Initializing arrays... (objects)]");
    if (!init_k_info()) os.quit("Cannot initialize objects");

    /* Initialize artefact info */
    term.note("[Initializing arrays... (artefacts)]");
    if (!init_a_info()) os.quit("Cannot initialize artefacts");

    /* Initialize ego-item info */
    term.note("[Initializing arrays... (ego-items)]");
    if (!init_e_info()) os.quit("Cannot initialize ego-items");

    /* Initialize monster info */
    term.note("[Initializing arrays... (monsters)]");
    if (!init_r_info()) os.quit("Cannot initialize monsters");

    /* Initialize feature info */
    term.note("[Initializing arrays... (vaults)]");
    if (!init_v_info()) os.quit("Cannot initialize vaults");

    /* Initialize some other arrays */
    term.note("[Initializing arrays... (other)]");
    if (!init_other()) os.quit("Cannot initialize other stuff");

    /* Initialize some other arrays */
    term.note("[Initializing arrays... (alloc)]");
    if (!init_alloc()) os.quit("Cannot initialize alloc stuff");


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
  * Initialize the "e_info" array, by parsing an ascii "template" file
  * Note, we dont do raw binaries anymore, there is no point any more.
  * So really, the below is more based on init_e_info_txt then init_e_info
  */
  /*BOOL*/ function init_r_info()
  {
    let path = g.ANGBAND_DIR_EDIT + '/r_info.txt',
     read_result = os.load_file(path),
     r_info = g.r_info,
     lines, line, version_okay, parts, current_race, r_ptr, allocation, i;
    /* Did we read k_info correctly? */
    if(!read_result.ok)
      return false;
    /*Split the file content into one-line strings*/
    lines = read_result.content.split('\n');
    
    /* This function needs the locals context, and can only take flag from the each call*/
    function handle_r_flag(flag){
      setFlags(r_ptr, flag, [r_info_flags1, r_info_flags2, r_info_flags3, r_info_flags7]);
      /*Allow the race to determine who escorts*/
      if(flag.startsWith("ESCORT_")){
        r_ptr.r_escort = flag.mid(7);
      }
      /*DROP_GREAT is pointless without DROP_GOOD*/
      if(flag == 'DROP_GREAT'){
        handle_r_flag('DROP_GOOD');
      }
      /*All uniques have max hitpoints*/
      if(flag == 'UNIQUE'){
        handle_r_flag('FORCE_MAXHP');
      }
      /*Storm is in essence blink with a different message*/
      if(flag == 'STORM'){
        handle_r_flag('BLINK');
      }
    }
    
    function handle_r_spell_flag(flag){
      setFlags(r_ptr, flag, [r_info_flags1, r_info_flags2, r_info_flags3, r_info_flags7]);
      if(flag.startsWith("1_IN_")){
        r_ptr.freq_spell = r_ptr.freq_inate = 100 / (flag.mid(5)+0);
      }
    }

    /*Treat every line*/
    for(line of lines){
      /*We split the line by colon into a list*/
      parts = line.split(':');
      /* Hack -- Process 'V' for "Version" */
      if(line.startsWith('V')){
        version_okay = check_version(line);
        if(!version_okay){
          return false;
        }
      }
      /* Process 'N' for "New/Number/Name" */
      if(line.startsWith('N')){
        current_race = parts[1];
        r_info[current_race] = {name: parts[2], blows: [],flags1: 0, flags2: 0, flags3: 0};
        r_ptr = r_info[current_race];
      }
      /* Process 'G' for "Graphics" (one line only) */
      if(line.startsWith('G')){
        r_ptr.f_char = parts[1];
        r_ptr.f_attr = term.colour_char_to_attr(parts[2].trim());
        /*Humans have open_door and bash_door, hardcoded for efficiency */
        if(parts[1]=='p'){
          r_ptr.flags2 = g.RF2_OPEN_DOOR + g.RF2_BASH_DOOR;
        }
        if(['A','D','d'].has(parts[1]=='p')){
          r_ptr.flags7 = g.RF7_FLIGHT;
        }
      }
      /* Process 'I' for "Info" (one line only) */
      if(line.startsWith('I')){
        r_ptr.speed = parts[1];
        r_ptr.num_blows = parts[2];
        r_ptr.hdice = parts[3].split('d')[0];
        r_ptr.hside = parts[3].split('d')[1];
        r_ptr.aaf = parts[4];
        r_ptr.ac = parts[5];
        r_ptr.sleep = parts[6];
      }
      /* Process 'W' for "More Info" (one line only) */
      if(line.startsWith('W')){
        r_ptr.level = parts[1];
        r_ptr.rarity = parts[2];
        r_ptr.extra = parts[3];
        r_ptr.mexp = parts[4];
      }
      /* Process 'D' for "Description" */
      if(line.startsWith('D')){
        /* If already read some text, then append with a newline*/
        if(r_ptr.text){
          r_ptr.text += ('\n' + parts[1]);
        }else{
          r_ptr.text = parts[1];
        }
      }
      /* Process 'B' for "Blows" (a number of lines) */
      if(line.startsWith('B')){
        let blow = {};
        blow.method = r_info_blow_method.indexOf(parts[1]);
        blow.effect = r_info_blow_effect.indexOf(parts[2]);
        if(parts[3]){
          blow.d_dice = parts[3].split('d')[0];
          blow.d_side = parts[3].split('d')[1];
        }
        r_ptr.blows.push(blow);
      }
      /* Process 'F' for "Flags" (any number of lines) */
      if(line.startsWith('F')){
        /* XXX XXX Chained Shenanigans */
        let flags = parts[1].split('|').map(s => s.trim());
        flags.each(handle_r_flag);
      }
      /* Process 'S' for "Spells" (any number of lines) */
      if(line.startsWith('S')){
        /* XXX XXX Chained Shenanigans */
        let flags = parts[1].split('|').map(s => s.trim());
        flags.each(handle_r_spell_flag);
      }
    }
    console.log(r_info);
    return true;
  }
  

  /*
  * Initialize the "e_info" array, by parsing an ascii "template" file
  * Note, we dont do raw binaries anymore, there is no point any more.
  * So really, the below is more based on init_e_info_txt then init_e_info
  */
  /*BOOL*/ function init_e_info()
  {
    let path = g.ANGBAND_DIR_EDIT + '/e_info.txt',
     read_result = os.load_file(path),
     e_info = g.e_info,
     lines, line, version_okay, parts, current_ego, allocation, i;
    /* Did we read k_info correctly? */
    if(!read_result.ok)
      return false;
    /*Split the file content into one-line strings*/
    lines = read_result.content.split('\n');
    /*Treat every line*/
    for(line of lines){
      /*We split the line by colon into a list*/
      parts = line.split(':');
      /* Hack -- Process 'V' for "Version" */
      if(line.startsWith('V')){
        version_okay = check_version(line);
        if(!version_okay){
          return false;
        }
      }
      /* Process 'N' for "New/Number/Name" */
      if(line.startsWith('N')){
        current_ego = parts[1];
        e_info[current_ego] = {name: parts[2], flags1: 0, flags2: 0, flags3: 0};
      }
      /* Process 'X' for "Info" (one line only) */
      if(line.startsWith('X')){
        e_info[current_ego].slot = parts[1];
        e_info[current_ego].rating = parts[2];
      }
      /* Process 'W' for "More Info" (one line only) */
      if(line.startsWith('W')){
        e_info[current_ego].level = parts[1];
        e_info[current_ego].extra = parts[2];
        e_info[current_ego].weight = parts[3];
        e_info[current_ego].cost = parts[4];
      }
      /* Hack -- Process 'C' for "creation" */
      if(line.startsWith('C')){
        e_info[current_ego].max_to_h = parts[1];
        e_info[current_ego].max_to_d = parts[2];
        e_info[current_ego].max_to_a = parts[3];
        e_info[current_ego].max_pval = parts[4];
      }
      if(line.startsWith('F')){
        /* XXX XXX Chained Shenanigans */
        let flags = parts[1].split('|').map(s => s.trim());
        flags.each(flag => setFlags(e_info[current_ego], flag, [k_info_flags1,k_info_flags2,k_info_flags3]));
      }
    }
    console.log(e_info);
    return true;
  }


  /*
  * Initialize the "k_info" array, by parsing an ascii "template" file
  * Note, we dont do raw binaries anymore, there is no point any more.
  * So really, the below is more based on init_k_info_txt then init_k_info
  */
  /*BOOL*/ function init_k_info()
  {
    let path = g.ANGBAND_DIR_EDIT + '/k_info.txt',
     read_result = os.load_file(path),
     k_info = g.k_info,
     lines, line, version_okay, parts, current_kind, allocation, i;
    /* Did we read k_info correctly? */
    if(!read_result.ok)
      return false;
    /*Split the file content into one-line strings*/
    lines = read_result.content.split('\n');
    /*Treat every line*/
    for(line of lines){
      /*We split the line by colon into a list*/
      parts = line.split(':');
      /* Hack -- Process 'V' for "Version" */
      if(line.startsWith('V')){
        version_okay = check_version(line);
        if(!version_okay){
          return false;
        }
      }
      /* Process 'N' for "New/Number/Name" */
      if(line.startsWith('N')){
        current_kind = parts[1];
        k_info[current_kind] = {name: parts[2], chance: [], flags1: 0, flags2: 0, flags3: 0};
      }
      /* Process 'D' for "Description" */
      if(line.startsWith('D')){
        /* If already read some text, then append with a newline*/
        if(k_info[current_kind].text){
          k_info[current_kind].text += ('\n' + parts[1]);
        }else{
          k_info[current_kind].text = parts[1];
        }
      }
      /* Process 'I' for "Info" (one line only) */
      if(line.startsWith('I')){
        k_info[current_kind].tval = parts[1];
        k_info[current_kind].sval = parts[2];
        k_info[current_kind].pval = parts[3];
      }
      /* Process 'W' for "More Info" (one line only) */
      /* Save the values */
      if(line.startsWith('W')){
        k_info[current_kind].level = parts[1];
        k_info[current_kind].extra = parts[2];
        k_info[current_kind].weight = parts[3];
        k_info[current_kind].cost = parts[4];
      }
      /* Process 'A' for "Allocation" (one line only) */
      if(line.startsWith('A')){
        /*Drop A*/
        parts.shift();
        /*Each part has a local and a chance*/
        for(i = 0; i < parts.length; i++){
          allocation = parts[i].split('/');
          k_info[current_kind].chance.push({locale: allocation[0], chance:allocation[1]});
        }
      }
      /* Hack -- Process 'P' for "power" and such */
      if(line.startsWith('P')){
        k_info[current_kind].ac = parts[1];
        k_info[current_kind].dd = parts[2].split('0')[0];
        k_info[current_kind].ds = parts[2].split('0')[1];
        k_info[current_kind].to_h = parts[3];
        k_info[current_kind].to_d = parts[4];
        k_info[current_kind].to_a =  parts[5];
      }
      /* Process 'G' for "Graphics" (one line only) */
      if(line.startsWith('G')){
        k_info[current_kind].f_char = parts[1];
        k_info[current_kind].f_attr = term.colour_char_to_attr(parts[2].trim());
      }
      if(line.startsWith('F')){
        /* XXX XXX Chained Shenanigans */
        let flags = parts[1].split('|').map(s => s.trim());
        flags.each(flag => setFlags(k_info[current_kind], flag, [k_info_flags1,k_info_flags2,k_info_flags3]));
      }
    }
    console.log(k_info);
    return true;
  }

  /*
  * Initialize the "a_info" array, by parsing an ascii "template" file
  * Note, we dont do raw binaries anymore, there is no point any more.
  * So really, the below is more based on init_a_info_txt then init_a_info
  */
  /*BOOL*/ function init_a_info()
  {
    let path = g.ANGBAND_DIR_EDIT + '/a_info.txt',
     read_result = os.load_file(path),
     a_info = g.a_info,
     lines, line, version_okay, parts, current_artifact, allocation, i;
    /* Did we read a_info correctly? */
    if(!read_result.ok)
      return false;
    /*Split the file content into one-line strings*/
    lines = read_result.content.split('\n');
    /*Treat every line*/
    for(line of lines){
      /*We split the line by colon into a list*/
      parts = line.split(':');
      /* Hack -- Process 'V' for "Version" */
      if(line.startsWith('V')){
        version_okay = check_version(line);
        if(!version_okay){
          return false;
        }
      }
      /* Process 'N' for "New/Number/Name" */
      if(line.startsWith('N')){
        current_artifact = parts[1];
        a_info[current_artifact] = {name: parts[2], chance: [], flags1: 0, flags2: 0, flags3: 0};
        a_info[current_artifact].flags3 = g.TR3_IGNORE_ACID + g.TR3_IGNORE_ELEC + g.TR3_IGNORE_FIRE + g.TR3_IGNORE_COLD;
      }
      /* Process 'I' for "Info" (one line only) */
      if(line.startsWith('I')){
        a_info[current_artifact].tval = parts[1];
        a_info[current_artifact].sval = parts[2];
        a_info[current_artifact].pval = parts[3];
      }
      /* Process 'W' for "More Info" (one line only) */
      /* Save the values */
      if(line.startsWith('W')){
        a_info[current_artifact].level = parts[1];
        a_info[current_artifact].extra = parts[2];
        a_info[current_artifact].weight = parts[3];
        a_info[current_artifact].cost = parts[4];
      }
      /* Hack -- Process 'P' for "power" and such */
      if(line.startsWith('P')){
        a_info[current_artifact].ac = parts[1];
        a_info[current_artifact].dd = parts[2].split('0')[0];
        a_info[current_artifact].ds = parts[2].split('0')[1];
        a_info[current_artifact].to_h = parts[3];
        a_info[current_artifact].to_d = parts[4];
        a_info[current_artifact].to_a =  parts[5];
      }
      if(line.startsWith('F')){
        /* XXX XXX Chained Shenanigans */
        let flags = parts[1].split('|').map(s => s.trim());
        flags.each(flag => setFlags(a_info[current_artifact], flag, [k_info_flags1,k_info_flags2,k_info_flags3]));
      }
    }
    console.log(a_info);
    return true;
  }


  function setFlags(o, flag, flag_sets){
    /* XXX XXX Chained & Ternary Shenanigans */
    let i, set, idx;
    for(i = 0; i < flag_sets.length; i++){
      set = flag_sets[i];
      idx = set.indexOf(flag);
      if(!!~idx){
        o['flags' + (i+1)] += (1<<idx);
      }
    }
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
      /*We split the line by colon into a list*/
      parts = line.split(':');
      /* Hack -- Process 'V' for "Version" */
      if(line.startsWith('V')){
        version_okay = check_version(line);
        if(!version_okay){
          return false;
        }
      }
      /* Process 'N' for "New/Number/Name" */
      if(line.startsWith('N')){
        current_feature = parts[1];
        f_info[current_feature] = {name: parts[2]};
      }
      /* Process 'M' for "Mimic" (one line only) */
      if(line.startsWith('M')){
        f_info[current_feature].mimic = parts[1];
      }
      /* Process 'G' for "Graphics" (one line only) */
      if(line.startsWith('G')){
        f_info[current_feature].f_char = parts[1];
        f_info[current_feature].f_attr = term.colour_char_to_attr(parts[2].trim());
      }
    }
    return true;
  }

  /*This assume a string starting with V:*/
  /*BOOL*/ function check_version(line)
  {
    return line.trim().mid(2) == g.VERSION;
  }

  return {init_angband_aux, init_angband};
});




