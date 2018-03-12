"use strict";

define(['log'],function(log){
  return {
    //File locations
    ANGBAND_DIR: undefined,
    ANGBAND_DIR_EDIT: undefined,
    ANGBAND_DIR_FILE: undefined,
    ANGBAND_DIR_HELP: undefined,
    ANGBAND_DIR_INFO: undefined,
    ANGBAND_DIR_PREF: undefined,
    ANGBAND_DIR_USER: undefined,
    ANGBAND_DIR_APEX: undefined,
    ANGBAND_DIR_BONE: undefined,
    ANGBAND_DIR_DATA: undefined,
    ANGBAND_DIR_SAVE: undefined,
    ANGBAND_DIR_DUMP: undefined,
    ANGBAND_DIR_XTRA: undefined,
    ANGBAND_SYS: undefined,
    //Runtime Parameters
    arg_fiddle: false,
    arg_sound:false,
    arg_graphics: false,
    arg_force_roguelike: false,
    arg_force_original: false,
    //Cached file contents
    fc_news:'',
    use_colour: true,
    /*
    * Angband "attributes" (with symbols, and base (R,G,B) codes)
    *
    * The "(R,G,B)" codes are given in "fourths" of the "maximal" value,
    * and should "gamma corrected" on most (non-Macintosh) machines.
    */
    TERM_DARK:    0,       /* 'd' */       /* 0,0,0 */
    TERM_WHITE:   1,       /* 'w' */       /* 4,4,4 */
    TERM_SLATE:   2,       /* 's' */       /* 2,2,2 */
    TERM_ORANGE:  3,       /* 'o' */       /* 4,2,0 */
    TERM_RED:     4,       /* 'r' */       /* 3,0,0 */
    TERM_GREEN:   5,       /* 'g' */       /* 0,2,1 */
    TERM_BLUE:    6,       /* 'b' */       /* 0,0,4 */
    TERM_UMBER:   7,       /* 'u' */       /* 2,1,0 */
    TERM_L_DARK:  8,       /* 'D' */       /* 1,1,1 */
    TERM_L_WHITE: 9,       /* 'W' */       /* 3,3,3 */
    TERM_VIOLET:  10,      /* 'v' */       /* 4,0,4 */
    TERM_YELLOW:  11,      /* 'y' */       /* 4,4,0 */
    TERM_L_RED:   12,      /* 'R' */       /* 4,0,0 */
    TERM_L_GREEN: 13,      /* 'G' */       /* 0,4,0 */
    TERM_L_BLUE:  14,      /* 'B' */       /* 0,4,4 */
    TERM_L_UMBER: 15,      /* 'U' */       /* 3,2,1 */
    VERSION: '0.8.7',
    f_info: []
  };
});