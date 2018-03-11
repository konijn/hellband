
"use strict";

define(['log'],function(log){

  let active_term,
      colors = [];

  function quit(why){
    //Where are we now?
    let location = window.location.href,
    //Split the URL up, we need to work with the last part
    locationParts = location.split('/'),
    //Change location from index.html (and possible arguments) to quit.html
    newLocation = location.chainedPop().chainedPush('quit.html?reason=' + why).join('/');
    //Actually go there
    window.location.href = newLocation;
  }
  
  function initscr(){
    return ROT.isSupported();
  }
  //display = new ROT.Display({width:PP.width, height:PP.height});
  //PP.kb.listen();
  //PP.mouse.listen();
    
  function whenReady(f){
    document.addEventListener("DOMContentLoaded", f);
  }
  
  function init_color(index, red, green, blue){
    colors[index] = red << 16 + green << 8 + blue;
  }

  function term_init(cols,rows, keys){
    let term = new ROT.Display({width:cols, height:rows});
    document.body.appendChild(term.getContainer());
    return term;
  }

  function term_activate(term){
    active_term = term;
  }
  //Writable Storage Open will tell you whether there is something to read
  function ws_open(locator){
    return localStorage.getItem(locator) !== undefined;
  }
  
  function ws_make(locator,value){
    value = value || '';
    localStorage.setItem(locator, value);
    return true;
  }
  
  function draw(col, row, ch, fg, bg){
    fg = fg === undefined ? undefined : colors[fg];
    bg = bg === undefined ? undefined : colors[bg];
    active_term.draw(col, row, ch, fg, bg);
  }
  
  function get_active_term(){
    return active_term;
  }
    
  return {quit, whenReady, initscr, init_color, term_init, term_activate, get_active_term, ws_make, ws_open, draw};
});



