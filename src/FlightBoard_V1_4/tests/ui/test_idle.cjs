// The idle clock stays readable and animates only while its preview is visible.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(path.resolve(__dirname,'..','..','FlightBoardWeb.h'),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)?.[1];
assert.ok(script);new vm.Script(script);
const start=script.indexOf('function canvasText('),end=script.indexOf('function renderConnection()');
assert.ok(start>=0&&end>start);
let assertions=2,frames=[],texts=[],fills=[],timerId=0;
const equal=(a,b,message)=>{assertions++;assert.equal(a,b,message);};
const check=(value,message)=>{assertions++;assert.ok(value,message);};
const timers=new Map(),events=new Map(),motion={matches:false,addEventListener:(event,callback)=>events.set('motion:'+event,callback)};
const g={font:'10px monospace',fillStyle:'',
  measureText(text){return{width:String(text).length*Number(this.font.match(/(\d+)px/)[1])*.6};},
  fillText(text,x,y,width){texts.push({text,x,y,width,font:this.font,color:this.fillStyle,shadow:this.shadowColor});},
  fillRect(x,y,width,height){fills.push({x,y,width,height,color:this.fillStyle});},
  createImageData(width,height){return{width,height,data:new Uint8ClampedArray(width*height*4)};},
  putImageData(frame,x,y){equal(frame.width,128);equal(frame.height,64);equal(x,0);equal(y,0);frames.push(Uint8ClampedArray.from(frame.data));},
  save(){},restore(){}
};
const canvas={style:{},getContext:()=>g},toggle={open:true,addEventListener:(event,callback)=>events.set('preview:'+event,callback)};
const elements=new Map([['ledPreview',canvas],['previewToggle',toggle]]);
const context=vm.createContext({captureText:layout=>texts.push(layout),String,Number,Boolean,Math,demo:false,serverOnline:true,currentView:'home',draftVisual:null,panelApp:s=>s?.app||s?.display?.app||'aviation',active:null,now:0,
  state:{display:{mode:'clock',clock:'07:08',date:'15/10/2026',timeSynced:true,effectiveBrightness:60}},
  MODE_NAMES:{auto:'Auto',manual:'Épinglé',clock:'Horloge'},window:{matchMedia:()=>motion},
  document:{hidden:false,addEventListener:(event,callback)=>events.set('document:'+event,callback)},
  performance:{now:()=>context.now},
  setTimeout:(callback,delay)=>{check(delay>=1000/15,'Animation must stay below 15 scheduled frames/s');timers.set(++timerId,{callback,delay});return timerId;},
  clearTimeout:id=>timers.delete(id),
  el:id=>{if(!elements.has(id))elements.set(id,{});return elements.get(id);},
});
vm.runInContext('const data=()=>state,previewPlane=()=>active;'+script.slice(start,end),context);
vm.runInContext('const paintOriginal=paintPanelText;paintPanelText=(...args)=>{const layout=paintOriginal(...args);captureText(layout);return layout;};',context);
const wiring=script.indexOf("el('previewToggle').addEventListener('toggle',drawPreview)");
vm.runInContext(script.slice(wiring,script.indexOf('refresh();loadFavorites();',wiring)),context);
const run=source=>vm.runInContext(source,context);
function draw(){frames=[];texts=[];fills=[];run('drawPreview()');}
function reset(display={}){context.state.display={mode:'clock',clock:'07:08',timeSynced:true,effectiveBrightness:60,...display};}
draw();equal(timers.size,1);equal([...timers.values()][0].delay,80);equal(texts.length,1,'Only the time belongs on the synchronized idle screen');
equal(texts[0].text,'07:08','The clock must use the device time');equal(texts[0].color,'#fff');equal(texts[0].height,28);equal(texts[0].scale,4,'Clock glyphs must use integer enlargement');
equal(canvas.style.filter,'brightness(0.6)');
const first=frames[0];let cyan=0,blue=0,black=0,maxClockChannel=0;
for(let y=0;y<64;y++)for(let x=0;x<128;x++){
  const i=(y*128+x)*4,green=first[i+1],b=first[i+2];
  if(green>180&&b>180)cyan++;if(green<90&&b>180)blue++;if(green===0&&b===0)black++;
  if(x>=14&&x<=114&&y>=18&&y<=46)maxClockChannel=Math.max(maxClockChannel,green,b);
}
check(cyan>0&&blue>0&&black>0,'The fluid field needs cyan cores, electric blue edges and black space');
check(maxClockChannel<=57,'The area behind the clock must remain dark even under a bright fluid core');
const channel=maxClockChannel/255,luminance=((channel+.055)/1.055)**2.4;
check(1.05/(luminance+.05)>10,'White digits must retain at least 10:1 contrast against the clock background');
draw();equal(timers.size,1,'Repeated renders must not duplicate the animation timer');assertions++;assert.deepEqual(frames[0],first,'The same phase must produce exactly the same pixels');
context.now=40;draw();assertions++;assert.deepEqual(frames[0],first,'Sub-frame updates must retain the current phase');
context.now=5000;draw();assertions++;assert.notDeepEqual(frames[0],first,'The shapes must move over time');
const queued=[...timers.entries()][0];timers.delete(queued[0]);context.now+=80;queued[1].callback();equal(timers.size,1,'An animation tick schedules only one successor');
toggle.open=false;events.get('preview:toggle')();equal(timers.size,0,'Closing the preview stops the animation');draw();equal(frames.length,0);equal(fills.length,0,'A collapsed preview must not repaint the canvas');
toggle.open=true;events.get('preview:toggle')();equal(timers.size,1,'Reopening the preview resumes the animation');
context.document.hidden=true;events.get('document:visibilitychange')();equal(timers.size,0,'Hidden tabs must stop animation');draw();equal(frames.length,0);equal(fills.length,0);
context.document.hidden=false;events.get('document:visibilitychange')();equal(timers.size,1);
motion.matches=true;events.get('motion:change')();equal(timers.size,0,'Reduced motion must remove the animation timer');draw();const reducedFrame=frames[0];
context.now+=8000;draw();assertions++;assert.deepEqual(frames[0],reducedFrame,'Reduced motion must freeze the fluid shapes');
context.state.display.clock='12:34';draw();equal(texts[0].text,'12:34','A static background must still refresh the device clock');
reset({timeSynced:false,clock:'23:59'});draw();equal(texts[0].text,'--:--');equal(texts[1].text,'ATTENTE HEURE');equal(texts.length,2,'Unsynchronized time must not display the stale hour or a date');
reset({clock:'29:99'});draw();equal(texts[0].text,'--:--','Invalid hours must not be presented as synchronized time');
reset({mode:'auto'});equal(run('idlePreview(state)'),true,'Auto without an active flight uses the idle clock');
context.active={callsign:'AFR1234'};equal(run('idlePreview(state)'),false,'A real active flight takes priority in Auto');
reset({mode:'clock'});equal(run('idlePreview(state)'),true,'Explicit clock mode does not depend on visible flights');
context.active=null;reset({mode:'manual'});equal(run('idlePreview(state)'),false,'An unavailable pinned flight must not silently become clock mode');
motion.matches=false;reset({nightActive:true,effectiveBrightness:15});draw();equal(canvas.style.filter,'brightness(0.15)');equal(timers.size,1);
reset({sleeping:true,effectiveBrightness:0});draw();equal(canvas.style.filter,'brightness(0)','Zero effective brightness must be respected');equal(timers.size,0);equal(frames.length,0);equal(texts.length,0);equal(fills.length,1);equal(fills[0].color,'#000','Night sleep must clear the panel to black');
context.serverOnline=false;draw();equal(texts.length,0,'Night sleep stays dark even after the device disconnects');
reset();draw();equal(frames.length,0);equal(timers.size,0,'A disconnected device must not animate an apparently live clock');
context.demo=true;draw();equal(timers.size,1,'Demo can animate without a device connection');
context.demo=false;context.serverOnline=true;context.state.app='animation';context.state.visual='stars';reset();draw();equal(timers.size,1);equal(texts.length,0,'Standalone animations must not show a clock or aviation text');
context.state.visual='plain';draw();equal(timers.size,0,'A static visual needs no animation timer');equal(frames.length,0);equal(texts.length,0);
context.state.app='clock';draw();equal(timers.size,0);equal(texts[0].text,'07:08','A plain clock still displays the synchronized hour');
for(const visual of ['aurora','ocean','warp']){
  context.state.app='animation';context.state.visual=visual;context.now=0;draw();equal(timers.size,1);equal(texts.length,0);
  const initial=JSON.stringify({frames:frames.map(f=>Array.from(f)),fills});context.now=3200;draw();check(JSON.stringify({frames:frames.map(f=>Array.from(f)),fills})!==initial,'The new animation must visibly advance');
  context.state.app='clock';draw();equal(timers.size,1);equal(texts[0].text,'07:08','New clock styles retain actual synchronized time');
}
context.state.app='weather';context.state.weather={available:false,status:'loading'};context.state.visual='stars';draw();equal(timers.size,0,'Unavailable weather must stay static');check(texts.some(t=>t.text==='METEO INDISPONIBLE'));
context.state.weather={available:true,temperatureC:19.4,windKmh:12.3,code:0,isDay:true};context.state.visual='plain';context.now=0;draw();equal(timers.size,1,'Weather animates independently of the clock visual');
const sunny=JSON.stringify(fills);context.now=3200;draw();check(JSON.stringify(fills)!==sunny,'The sun rays must advance');
motion.matches=true;draw();equal(timers.size,0);const still=JSON.stringify(fills);context.now=8000;draw();equal(JSON.stringify(fills),still,'Reduced motion freezes the weather icon');
motion.matches=false;draw();equal(timers.size,1);toggle.open=false;events.get('preview:toggle')();equal(timers.size,0);toggle.open=true;draw();equal(timers.size,1);
for(const invalid of [{temperatureC:NaN},{windKmh:null},{code:999},{code:52},{available:false}]){const valid=context.state.weather;context.state.weather={...valid,...invalid};draw();equal(timers.size,0,'Invalid weather must not keep an animation timer');context.state.weather=valid;}
context.state.display.sleeping=true;draw();equal(timers.size,0);equal(texts.length,0);context.state.display.sleeping=false;context.serverOnline=false;draw();equal(timers.size,0,'Offline weather must stop its apparent live animation');
console.log(`Idle clock UI: ${assertions} assertions passed`);
