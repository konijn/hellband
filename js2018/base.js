//UTF enforcer Ƨ
"use strict";

//Base prototype enhancements

//(currentValue[, index[, array]])
Array.prototype.each = function ArrayEach(f){
  return this.forEach(f);
};

Array.prototype.repeatPush = function ArrayRepeatPush(x, n){
  while(n--)
    this.push(x);
  return this;
};

String.prototype.replaceAll = function StringReplaceAll(target, replacement) {
  return this.split(target).join(replacement);
};

Array.prototype.set = function ArraySet(index, replacement){
 listify(replacement).each((v,i) => (this[index+i] = v));
 return this;
};

Array.prototype.flatten = function ArrayFlatten(){
  var flattened = this.shift();
  this.each(a => a.each(v => flattened.push(v)));
  return flattened;
};

Array.prototype.chainedPush = function ArrayChainedPush(...args){
  this.push(...args);
  return this;
};

Array.prototype.chainedPop = function ArrayChainedPop(...args){
  this.pop();
  return this;
};

Array.prototype.unique = function ArrayUnique() {
  return this.sort().reduce( (accumulator, currentValue) => accumulator.has(currentValue)?accumulator:accumulator.chainedPush(currentValue) ,[]);
};

Array.prototype.has = Array.prototype.includes;

Array.prototype.shuffle = function ArrayShuffle(){
  var currentIndex = this.length, temporaryValue, randomIndex;
  // While there remain elements to shuffle...
  while (0 !== currentIndex) {
    // Pick a remaining element...
    randomIndex = Math.floor(Math.random() * currentIndex);
    currentIndex--;
    // And swap it with the current element.
    temporaryValue = this[currentIndex];
    this[currentIndex] = this[randomIndex];
    this[randomIndex] = temporaryValue;
  }
  return this;
};

String.prototype.set = function StringSet(index, replacement) {
  replacement = replacement || " ";
  return this.substr(0, index) + replacement+ this.substr(index + replacement.length);
};

//(currentValue[, index[, array]])
String.prototype.each = function StringEach(f){
  return this.split('').each(f);
};

String.prototype.has = function StringHas(s){
  return !!~this.indexOf(s);
};

String.prototype.left = function StringLeft(n){
  return this.substring(0,n);
};

String.prototype.right = function StringRight(n){
  return this.substr(-n);
}

String.prototype.mid = String.prototype.substr;

String.prototype.mod = function StringMod(i){
  return String.fromCharCode(this.charCodeAt(0) + i);
};

String.prototype.stripRight = function StringStripRight(n){
 return this.substring(0, this.length-n);
};

String.prototype.decode = function StringProcess(){
  let chars = [], color = [], cursorColor, context;

  function processCharacter(c){
    if(context=='escaped'){
      cursorColor = c;
      context = '';
    }else if( c == '`' ){
      context = 'escaped';
    } else {
      chars.push(c);
      color.push(cursorColor);
    }
  }
  this.each(processCharacter);
  return {color, chars, text: chars.join('')};
};

String.encode = function StringEncode(combo){
  let cursorColor = ' ', out = '';

  function processCouple(c, color){
    let currentColor = color || "";
    if(currentColor != cursorColor){
      out = out + '`' + currentColor;
      cursorColor = currentColor;
    }
    out += c;
  }

  combo.chars.each((c,i)=>processCouple(c,combo.color[i]));
  return out;
};

Number.prototype.loop = function NumberLoop(f){
  f = f || console.log;
  for(var i = 0; i < this; i++)
    f(i, this+0);
};

Map.prototype.each = Map.prototype.forEach;

Map.prototype.reverse = function MapReverse(){
  let list = [];
  this.each((value, key, map)=>list.push([value,key]));
  return new Map(list);
};

function listify(o){
  return o === undefined ? [] :
        Array.isArray(o) ? o : [o];
}

define({listify});