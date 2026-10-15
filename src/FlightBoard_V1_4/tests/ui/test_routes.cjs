// Run with Node.js from any working directory; no dependencies or network.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(path.resolve(__dirname,'..','..','FlightBoardWeb.h'),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)?.[1];
assert.ok(script,'The embedded JavaScript must exist');
new vm.Script(script,{filename:'FlightBoardWeb.h embedded JavaScript'});
const routeStart=script.indexOf('// Callsigns can change'),routeEnd=script.indexOf('function project(p,s)');
assert.ok(routeStart>=0&&routeEnd>routeStart,'Route helper boundaries must exist');
const code=script.slice(routeStart,routeEnd);
const elements=new Map(),requests=[];
const known={status:'known',source:'adsbdb',indicative:true,origin:{code:'CDG',city:'Paris',name:'Paris-Charles de Gaulle'},destination:{code:'BER',city:'Berlin',name:'Berlin Brandenburg'}};
let assertions=2; const equal=(a,b)=>{assertions++;assert.equal(a,b);};
const tick=()=>new Promise(resolve=>setImmediate(resolve));
const context=vm.createContext({console,Date:{now:()=>context.now},Map,String,Array,Boolean,encodeURIComponent,
  now:10000,demo:false,uploadBusy:false,wifi:false,good:true,routeBusy:false,routeEpoch:0,routeFocus:'',routeCache:new Map(),routeLastRequest:new Map(),
  latest:{aircraft:[],display:{mode:'auto',sleeping:false}},selected:null,active:null,
  el:id=>{if(!elements.has(id))elements.set(id,{setAttribute(name,value){this[name]=value;}});return elements.get(id);},
  requestJson:(path)=>new Promise((resolve,reject)=>requests.push({path,resolve,reject})),drawPreview:()=>{},
});
const appCode=script.slice(script.indexOf('const APP_NAMES='),script.indexOf('let currentView='))+script.slice(script.indexOf('function panelApp('),script.indexOf('function activeFilters('));
vm.runInContext(`const data=()=>latest,usable=()=>good,selectedPlane=()=>selected,previewPlane=()=>active,aircraft=s=>s.aircraft,wifiIsOpen=()=>wifi;${appCode}${code}`,context);
const run=source=>vm.runInContext(source,context);
function plane(hex,call){return{hex,callsign:call};}
function select(p,active=p){context.selected=p;context.active=active;context.latest.aircraft=[p,...(active&&active!==p?[active]:[])];}
(async()=>{
  equal(run("routeKey({hex:'abc123',callsign:'AFR1'})===routeKey({hex:'abc123',callsign:'AFR2'})"),false);
  context.payload=known;equal(run('normalizeRoute(payload).status'),'known');
  context.payload={...known,source:'adsb.lol-vrs'};equal(run('normalizeRoute(payload).source'),'adsb.lol-vrs');
  context.payload={...known,status:'unconfirmed',reason:'position_mismatch'};
  equal(run('normalizeRoute(payload).status'),'unconfirmed');
  equal(run('normalizeRoute(payload).origin'),undefined);
  select({...plane('abc123','RYR3UF'),route:context.payload});run('renderRoute(selected)');
  equal(elements.get('routeNote').textContent,'Trajet non confirmé');
  equal(elements.get('routeAirports').hidden,true);
  equal(elements.get('route').getAttribute?.('aria-label')||elements.get('route')['aria-label'],'Provenance et destination');
  context.routeCache.set('abc123|RYR3UF',{route:known,at:context.now});
  equal(run('routeFor(selected).status'),'unconfirmed');
  context.routeCache.clear();
  context.payload={...known,destination:{code:'BADCODE'}};equal(run('normalizeRoute(payload).status'),'unknown');
  context.payload={status:'invented'};equal(run('normalizeRoute(payload).status'),'error');
  select(plane('abc123','AFR1'));
  for(const property of ['demo','uploadBusy','wifi']){context[property]=true;run('syncRoutes()');equal(requests.length,0);context[property]=false;}
  context.good=false;run('syncRoutes()');equal(requests.length,0);context.good=true;
  for(const app of ['clock','animation','weather']){context.latest.app=app;run('syncRoutes()');equal(requests.length,0);}
  context.latest.app='aviation';
  run('syncRoutes()');equal(requests.length,1);equal(requests[0].path,'/api/route?hex=abc123');
  run('syncRoutes()');equal(requests.length,1);
  requests[0].resolve({hex:'abc123',callsign:'AFR1',route:{status:'loading'}});await tick();
  run('syncRoutes()');equal(requests.length,1);context.now+=3999;run('syncRoutes()');equal(requests.length,1);context.now++;
  run('syncRoutes()');equal(requests.length,2);requests[1].resolve({hex:'abc123',callsign:'AFR1',route:known});await tick();
  equal(elements.get('routeOriginCode').textContent,'CDG');equal(elements.get('routeDestinationCity').textContent,'Berlin');equal(elements.get('routeNote').textContent,'Itinéraire indicatif');equal(context.latest.aircraft[0].route.status,'known');
  context.now+=5000;run('syncRoutes()');equal(requests.length,2);
  select(plane('def456','RYR2'));run('syncRoutes()');equal(requests.length,3);
  select(plane('f00456','UPS3'));run('syncRoutes()');requests[2].resolve({hex:'def456',callsign:'RYR2',route:known});await tick();
  equal(context.routeCache.has('def456|RYR2'),false);equal(context.latest.aircraft[0].route,undefined);
  run('syncRoutes()');equal(requests.length,4);requests[3].resolve({hex:'f00456',callsign:'OTHER',route:known});await tick();equal(context.routeCache.has('f00456|UPS3'),false);
  context.now+=4000;run('syncRoutes()');equal(requests.length,5);requests[4].resolve({hex:'f00456',callsign:'UPS3',route:{status:'unknown'}});await tick();
  equal(elements.get('routeNote').textContent,'Trajet non renseigné');equal(elements.get('routeAirports').hidden,true);
  select(plane('f00456','UPS3'));context.now+=4000;run('syncRoutes()');equal(requests.length,5);
  context.now+=60000;run('syncRoutes()');equal(requests.length,6);requests[5].reject(new Error('offline'));await tick();
  equal(elements.get('routeNote').textContent,'Trajet temporairement indisponible');
  select(plane('f00456','UPS3'));context.now+=4000;run('syncRoutes()');equal(requests.length,6);context.now+=30000;run('syncRoutes()');equal(requests.length,7);
  context.wifi=true;requests[6].resolve({hex:'f00456',callsign:'UPS3',route:known});await tick();equal(context.routeCache.get('f00456|UPS3').route.status,'error');context.wifi=false;
  const a=plane('ac0001','BAW4'),b=plane('ac0002','DLH5');select(a,b);run('syncRoutes()');equal(requests.length,8);requests[7].resolve({hex:a.hex,callsign:a.callsign,route:{status:'loading'}});await tick();
  context.now+=4000;run('syncRoutes()');equal(requests.length,9);equal(requests[8].path,'/api/route?hex=ac0002');requests[8].resolve({hex:b.hex,callsign:b.callsign,route:known});await tick();
  context.demo=true;select({...a,route:known});run('renderRoute(selected)');equal(elements.get('routeNote').textContent,'Trajet fictif · démonstration');
  console.log(`Route UI: ${assertions} assertions passed`);
})().catch(error=>{console.error(error);process.exitCode=1;});
