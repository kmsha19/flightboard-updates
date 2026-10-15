// Run with Node.js from any working directory; no browser or dependencies.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(path.resolve(__dirname,'..','..','FlightBoardWeb.h'),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)?.[1];
assert.ok(script,'The embedded JavaScript must exist');
new vm.Script(script,{filename:'FlightBoardWeb.h embedded JavaScript'});
const start=script.indexOf('function canvasText('),end=script.indexOf('function renderConnection()');
assert.ok(start>=0&&end>start,'The preview drawing helpers must exist');
let assertions=2,draws=[],rects=[],images=[];
const check=(value,message)=>{assertions++;assert.ok(value,message);};
const equal=(actual,expected,message)=>{assertions++;assert.equal(actual,expected,message);};
const fontSize=font=>Number(font.match(/([\d.]+)px/)[1]);
const g={font:'10px monospace',fillStyle:'',textBaseline:'top',
  measureText(text){return{width:String(text).length*fontSize(this.font)*.6};},
  fillText(text,x,y,maxWidth=Infinity){draws.push({text:String(text),x,y,width:Math.min(this.measureText(text).width,maxWidth),height:fontSize(this.font),font:this.font,color:this.fillStyle});},
  fillRect(x,y,width,height){rects.push({x,y,width,height});},
  drawImage(canvas,x,y){images.push({x,y,width:canvas.width,height:canvas.height});},
  createImageData(width,height){return{width,height,data:new Uint8ClampedArray(width*height*4)};},
  putImageData(frame,x,y){images.push({x,y,width:frame.width,height:frame.height});},
  save(){},restore(){}
};
const canvas={style:{},getContext:()=>g},elements=new Map([['ledPreview',canvas],['previewToggle',{open:true}]]);
const route={status:'known',origin:{code:'CDG'},destination:{code:'BER'}};
const plane={callsign:'AFR1234',type:'A320',distanceKm:8.4,speedKmh:819.6,speedAvailable:true,altitudeFt:34000,altitudeAvailable:true,route};
const context=vm.createContext({captureText:layout=>draws.push(layout),String,Number,Math,demo:false,serverOnline:true,currentView:'home',draftVisual:null,panelApp:s=>s?.app||s?.display?.app||'aviation',selected:plane,
  APP_NAMES:{},WIDGET_APPS:['message','timer','score','pixel'],widgetView:()=>false,currentContentId:'',contentCache:new Map(),widgetRemaining:w=>w?.remainingSeconds,
  state:{display:{mode:'manual',effectiveBrightness:60}},logoCache:new Map([['AFR1234',[]]]),
  MODE_NAMES:{auto:'Auto',manual:'Épinglé',clock:'Horloge'},
  el:id=>{if(!elements.has(id))elements.set(id,{});return elements.get(id);},
  document:{hidden:false,createElement:()=>({})},window:{matchMedia:()=>({matches:true})},performance:{now:()=>0},paintLogo:()=>{},fetchLogo:()=>{},
  fmt:(n,d)=>Number(n).toLocaleString('fr-FR',{minimumFractionDigits:d,maximumFractionDigits:d})
});
vm.runInContext('const data=()=>state,previewPlane=()=>selected,routeFor=p=>p.route;'+script.slice(start,end),context);
vm.runInContext('const paintOriginal=paintPanelText;paintPanelText=(...args)=>{const layout=paintOriginal(...args);captureText(layout);return layout;};',context);
function render(p=plane,display={mode:'manual',effectiveBrightness:60}){
  context.selected=p;context.state={display};draws=[];rects=[];images=[];
  vm.runInContext('drawPreview()',context);
}
function text(value){return draws.find(item=>item.text===value);}
function withinPanel(){
  const items=[...draws,...rects,...images];
  check(items.every(item=>item.x>=0&&item.x+item.width<=128),'All text, logo and glyph pixels must fit the panel width');
  check(items.every(item=>item.y>=0&&item.y+item.height<=64),'All text, logo and glyph pixels must fit the panel height');
  check(rects.every(item=>[item.x,item.y,item.width,item.height].every(Number.isInteger)),'All bitmap strokes must fall exactly on LEDs');
}
render();withinPanel();
equal(text('AFR1234').x,31);equal(images[0].y,2);equal(text('CDG > BER').height,14);
function headerFits(typeLabel,callsign='AFR1234'){
  const title=text(callsign),type=text(typeLabel);
  check(title&&type,'The callsign and aircraft type must both be present');
  equal(title.y,2);equal(title.height,7);equal(type.x,31);equal(type.y,11);equal(type.height,7);
  equal(type.color,'#a2b0b0');
  check(title.y+title.height<type.y,'The callsign and type must occupy separate rows');
  check(type.y+type.height<19,'The type must stay above the route divider');
  check(images.every(logo=>logo.x+logo.width<title.x&&logo.x+logo.width<type.x),'The logo must remain separate from both header lines');
  check(title.x+title.width<=128&&type.x+type.width<=128,'Both header lines must fit the panel width');
}
headerFits('A320');
for(const [label,value,unit,left,right,color] of [['DIST','8,4','KM',2,40,'#a9d8c6'],['VIT','820','KM/H',45,83,'#edf0e9'],['ALT','34000','FT',88,126,'#e6bf81']]){
  const reading=text(value);check(reading,'The metric value must be present');
  check(reading.x>=left-.5&&reading.x+reading.width<=right+.5,'The value must stay inside its column');
  equal(text(label).y,37);equal(reading.y,44);equal(text(unit).y,57);equal(reading.color,color);
}
render({...plane,callsign:'VERY-LONG-CALLSIGN',route:{status:'known',origin:{code:'LFPG'},destination:{code:'EDDB'}},distanceKm:49.9,speedKmh:1234567,altitudeFt:100000});withinPanel();
headerFits('A320','VERY-LONG-CA');
check(text('LFPG > EDDB'),'ICAO routes must remain visible');check(text('1234567').width<=38,'Large values must remain within their metric column');
for(const type of ['A320','B738','B38M','PC21','A1','AB3']){
  render({...plane,type});withinPanel();headerFits(type);
  equal(Boolean(text('TYPE --')),false,'A valid type must be displayed as received');
}
for(const type of [undefined,null,'','—','a320','A','A320X','B-38',' A320','A320 ','A32\n','A320\n']){
  render({...plane,type});withinPanel();headerFits('TYPE --');
  equal(Boolean(text('A320')),false,'A missing or invalid type must not be inferred from the callsign or airline');
}
render({...plane,distanceKm:NaN,speedAvailable:false,altitudeAvailable:false});withinPanel();equal(draws.filter(item=>item.text==='--').length,3);
render({...plane,altitudeFt:0});equal(text('0').y,44,'Zero altitude is a real measurement');
for(const [status,label] of [['not_requested','RECHERCHE DU TRAJET'],['loading','RECHERCHE DU TRAJET'],['unknown','TRAJET NON RENSEIGNE'],['error','TRAJET INDISPONIBLE'],['unconfirmed','TRAJET NON CONFIRME']]){
  render({...plane,route:{status}});withinPanel();check(text(label),'Route status must remain explicit');equal(draws.some(item=>item.text.includes(' > ')),false,'Unknown routes must not invent airports');
}
render(plane,{mode:'manual',sleeping:true,effectiveBrightness:0});equal(draws.length,0,'Night sleep must keep the panel dark');
render(plane,{mode:'clock',timeSynced:true,clock:'14:32',date:'15/10/2026'});withinPanel();check(text('14:32'));equal(draws.some(item=>item.text==='DIST'),false,'Clock mode must not show flight metrics');
console.log(`Preview UI: ${assertions} assertions passed`);
