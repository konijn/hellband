//Unicode trick Ƨ
 "use strict";

const
  DEBUG = 0,
  WARNING = 4,
  ERROR = 8,
  CATASTROPHE = 16;

var logLevel = WARNING;

function log(level, ...rest){
  //Bad caller, no log level.
  if(!rest){
    console.log(...level);
  }else if(level === DEBUG || level === WARNING || level === ERROR || level === CATASTROPHE){
    if(level >= logLevel)
      console.log(...rest);
  }else{
    console.log(level, ...rest);
  }
}