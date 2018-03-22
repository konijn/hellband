#! /usr/bin/env node

var fs = require('fs');
let buffer = fs.readFileSync('r_info.txt', 'utf8'),
    content = '',
    lastLine = '';

for(c of buffer)
  content += c;

let lines = content.split('\n'),
    currentMonster = 0;

for(line of lines){
  //Make sure descriptions end with a space
  if(line.startsWith('D:')){
    if(!line.endsWith(' ')){
      //Cannot be bothered to find out why,but this removes all D:'s
      //line = line + ' ';
    }
  }
  //Renumber to keep everything clean
  if(line.startsWith('N')){
    let parts = line.split(':');
    line = 'N:' + currentMonster++ + ':'  + parts[2];
  }
  console.log(line);
}