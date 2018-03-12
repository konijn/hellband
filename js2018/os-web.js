"use strict";

define(['log'], function(log) {

  let active_term,
    colors = [];

  function quit(why) {
    //Where are we now?
    let location = window.location.href,
      //Split the URL up, we need to work with the last part
      locationParts = location.split('/'),
      //Change location from index.html (and possible arguments) to quit.html
      newLocation = location.chainedPop().chainedPush('quit.html?reason=' + why).join('/');
    //Actually go there
    window.location.href = newLocation;
  }

  function initscr() {
    return ROT.isSupported();
  }
  //PP.kb.listen();
  //PP.mouse.listen();

  function init_color(index, red, green, blue) {
    /*Old Angband provides color ranges from 0 to 1000, rot.js expects 0 to 255*/
    red = Math.ceil(red * 255/ 1000);
    green = Math.ceil(green * 255/ 1000);
    blue = Math.ceil(blue * 255/ 1000);
    colors[index] = '#' + ('000000' + ((red << 16) + (green << 8) + blue).toString(16)).right(6);
    console.log(index,colors[index],red,green,blue);
  }

  function term_init(cols, rows, keys) {
    let term = new ROT.Display({
      width: cols,
      height: rows
    });
    document.body.appendChild(term.getContainer());
    return term;
  }

  function term_clear() {
    active_term.clear();
  }

  function term_activate(term) {
    active_term = term;
  }

  //Writable Storage Open will tell you whether there is something to read
  function ws_open(locator) {
    return localStorage.getItem(locator) !== undefined;
  }

  function ws_make(locator, value) {
    value = value || '';
    localStorage.setItem(locator, value);
    return true;
  }

  function draw(col, row, ch, fg, bg) {
    fg = fg === undefined ? undefined : colors[fg];
    bg = bg === undefined ? undefined : colors[bg];
    active_term.draw(col, row, ch, fg, bg);
  }

  function get_active_term() {
    return active_term;
  }

  function load_file(path, onSuccess, onFail) {
    let client = new XMLHttpRequest(),
      result;
    /*This WILL fail on file:/// which is a real bummer*/
    client.open('GET', path, false);

    client.onreadystatechange = function() {
      result = {
        content: client.responseText,
        ok: true
      };
      if (onSuccess) onFail(onSuccess);
    };
    client.onError = function() {
      result = {
        err: client.statusText,
        ok: false
      };
      if (onFail) onFail(client);
    };
    client.send();
    return result;
  }

  return {
    draw,
    get_active_term,
    init_color,
    initscr,
    load_file,
    quit,
    term_activate,
    term_clear,
    term_init,
    ws_make,
    ws_open,
  };
});