console.log('Hello World');

var fs = require('fs');
var context;

function process(filename){
  let path = "../src/" + filename,
      out = filename.split('.')[0] + '.js',
      i;
  fs.openSync(path, 'r+');
  let lines = fs.readFileSync(path,'ascii');
  context = {};//Evil, evil global ;)
  lines = lines.map(grinder);
  fs.writeFileSync(out, lines.join('\n'));
}

function grinder(s){
  let trimmed = s.trim();
  
  if(trimmed.startsWith('/*'))
    context.commenting = contex.commented = true;
  if(context.commenting && trimmed.endsWith('*/'))
    context.commenting = false;
    
  if(context.commenting || context.commented){
    return s;
  }
    
    
  context.commented = false;
}



process("main.c");
