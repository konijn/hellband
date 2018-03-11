//Unicode trick Ƨ
"use strict";

//Current debug level
//TODO: Allow for command switch to set log level
const logLevel = 4; //WARNING

function log(level, ...rest){
  //Bad caller, no log level.
  if(!rest){
    console.log(...level);
  }else if(level === 0 || level === 4 || level === 8 || level === 16){
    if(level >= logLevel){
      console.log(...rest);
    }
  } else{
    console.log(level, ...rest);
  }
}

log.DEBUG = 0;
log.WARNING = 4;
log.ERROR = 8;
log.CATASTROPHE = 16;

log(log.DEBUG, 'Log is loaded', log);

define(x=>log);