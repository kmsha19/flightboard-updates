// Selection changes must not display a delayed logo from a previous flight.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(path.resolve(__dirname,'..','..','FlightBoardWeb.h'),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)?.[1];
assert.ok(script,'The embedded JavaScript must exist');
new vm.Script(script,{filename:'FlightBoardWeb.h embedded JavaScript'});
const start=script.indexOf('function renderAirlineLogo('),end=script.indexOf('function paintLogo(');
assert.ok(start>=0&&end>start,'The airline image helper must exist');
assert.match(html,/<span id="logoFallback"[^>]*><svg/,
  'The fallback must have an HTML wrapper: SVGElement.hidden does not reflect the hidden attribute');
let assertions=3;
const equal=(actual,expected,message)=>{++assertions;assert.equal(actual,expected,message);};
const logo={hidden:true,alt:'',removeAttribute(name){delete this[name];}},fallback={hidden:false},requests=[];
const context=vm.createContext({String,encodeURIComponent,now:1000,Date:{now:()=>context.now},
  Image:class {constructor(){requests.push(this);}},
  el:id=>id==='logo'?logo:fallback,
});
vm.runInContext("let logoImageId='',logoImageEpoch=0,logoImageFailedAt=0;"+script.slice(start,end),context);
const render=plane=>{context.plane=plane;vm.runInContext('renderAirlineLogo(plane)',context);};
render({callsign:' afr842 ',airline:'AIR FRANCE'});
equal(requests.length,1);equal(requests[0].src,'/api/logo/image?flight=AFR842&v=2.0.5');
equal(logo.hidden,true);equal(fallback.hidden,false);
render({callsign:'AFR842',airline:'AIR FRANCE'});equal(requests.length,1,'Polling must not reload the same image');
requests[0].onload();equal(logo.hidden,false);equal(fallback.hidden,true);equal(logo.alt,'Logo AIR FRANCE');
render({callsign:'BEL123',airline:'BRUSSELS AIRLINES'});
equal(logo.hidden,true,'The previous airline must disappear immediately');equal(logo.src,undefined);equal(fallback.hidden,false);
render({callsign:'VRE456',airline:"AIR CÔTE D’IVOIRE"});
requests[1].onload();equal(logo.hidden,true,'A delayed Brussels image cannot appear on the Air Côte d’Ivoire card');
requests[2].onload();equal(logo.src,'/api/logo/image?flight=VRE456&v=2.0.5');equal(logo.hidden,false);
requests[1].onerror();equal(logo.hidden,false,'A stale error must not hide the current image');
render({callsign:'CRL789',airline:'CORSAIR'});requests[3].onerror();
equal(logo.hidden,true);equal(fallback.hidden,false);equal(logo.alt,'');
context.now+=29999;render({callsign:'CRL789'});equal(requests.length,4,'Failed images must not retry every poll');
context.now++;render({callsign:'CRL789'});equal(requests.length,5,'Temporary failures can recover after thirty seconds');
requests[4].onload();equal(logo.hidden,false);equal(logo.alt,'Logo de la compagnie');
render(null);equal(logo.hidden,true);equal(logo.src,undefined);equal(fallback.hidden,false);
requests[4].onload();equal(logo.hidden,true,'A late image cannot appear after the selected flight leaves');
render({callsign:''});equal(requests.length,5,'Missing callsigns use the neutral fallback');
render({callsign:'A&B'});equal(requests[5].src,'/api/logo/image?flight=A%26B&v=2.0.5','Callsigns must remain inside the query value');
console.log(`Airline image UI: ${assertions} assertions passed`);
