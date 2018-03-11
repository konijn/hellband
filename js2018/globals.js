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
    fc_news:''
  };
});