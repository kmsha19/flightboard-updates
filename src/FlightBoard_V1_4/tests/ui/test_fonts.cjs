// Bitmap text must be crisp, measurable and bounded without browser font metrics.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(path.resolve(__dirname,'..','..','FlightBoardWeb.h'),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)?.[1];
assert.ok(script);new vm.Script(script);
const start=script.indexOf('function canvasText('),end=script.indexOf('function previewMetric(');
assert.ok(start>=0&&end>start);
let assertions=2,rects=[];
const check=(value,message)=>{assertions++;assert.ok(value,message);};
const equal=(a,b,message)=>{assertions++;assert.equal(a,b,message);};
const g={fillStyle:'',fillRect(x,y,width,height){rects.push({x,y,width,height,color:this.fillStyle});},
  measureText(){throw new Error('Bitmap layout must not depend on browser fonts');},
  fillText(){throw new Error('Bitmap glyphs must not use antialiased browser text');}
};
const context=vm.createContext({g});vm.runInContext(script.slice(start,end),context);
const run=source=>vm.runInContext(source,context);
function paint(text,size=7,width=122){rects=[];context.text=text;context.size=size;context.width=width;return run("canvasCenteredText(g,text,64,10,width,size,'#edf0e9')");}
function verifyPixels(layout){
  check(rects.every(r=>[r.x,r.y,r.width,r.height].every(Number.isInteger)),'Every stroke must occupy an integer number of physical pixels');
  check(rects.every(r=>r.x>=layout.x&&r.x+r.width<=layout.x+layout.width&&r.y>=layout.y&&r.y+r.height<=layout.y+layout.height),'Glyph pixels must stay inside their measured extent');
  check(rects.every(r=>r.color==='#edf0e9'),'Bitmap text must retain the chosen panel colour');
}
let layout=paint('A');equal(layout.width,5);equal(layout.height,7);verifyPixels(layout);
const actual=Array.from({length:7},(_,y)=>Array.from({length:5},(_,x)=>rects.some(r=>r.x===layout.x+x&&r.y===10+y)?'#':'.').join(''));
assertions++;assert.deepEqual(actual,['.###.','#...#','#...#','#####','#...#','#...#','#...#'],'A must be an actual 5×7 glyph');
layout=paint('DIST',6,38);equal(layout.height,5);equal(layout.width,15);verifyPixels(layout);
for(const text of ['AFR1234','LFPG > EDDB','CDG > BER','1234567','34000','8,4','KM/H','TRAJET NON CONFIRME']){
  const width=text==='1234567'?38:122;
  layout=paint(text,text.includes(' > ')?14:7,width);verifyPixels(layout);check(layout.width<=width,'Text must fit its assigned panel area');
  equal(layout.text,text,'Fitting must retain every character when there is enough room');
}
layout=paint('LFPG > EDDB',14,122);equal(layout.height,14);equal(layout.scale,2,'Four-letter airport codes must remain enlarged and readable');
layout=paint('1234567',10,38);equal(layout.small,true,'An unusually long value must switch to the compact font');equal(layout.text,'1234567');verifyPixels(layout);
layout=paint('12345678901234567890',10,38);equal(layout.text,'...','An impossible value must not be silently truncated to a different number');check(layout.width<=38);verifyPixels(layout);
layout=paint('éàô ç',7);equal(layout.text,'EAO C','Accented names must remain legible with the local glyph set');verifyPixels(layout);
paint('?');const fallback=rects.map(r=>({...r}));paint('Ω');assertions++;assert.deepEqual(rects,fallback,'Unsupported characters must use a visible question mark');
layout=paint('07:08',30,116);equal(layout.height,28);equal(layout.width,100);equal(layout.scale,4);verifyPixels(layout);
check(layout.x>=12&&layout.x+layout.width<=116,'The clock must stay within the dark high-contrast centre area');
for(const [name,height,digits] of [['PANEL_FONT_5X7',7,2],['PANEL_FONT_3X5',5,1]]){
  const glyphs=run(name);
  check(Object.values(glyphs).every(bits=>bits.length===height*digits),'Every encoded glyph must contain all of its rows');
  check(Object.values(glyphs).every(bits=>/^[0-9A-F]+$/.test(bits)),'Every glyph must contain valid hexadecimal bitmap data');
}
console.log(`Bitmap font UI: ${assertions} assertions passed`);
