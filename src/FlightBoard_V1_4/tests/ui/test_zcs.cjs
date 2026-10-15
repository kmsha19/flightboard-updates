// ZCS hub navigation, app activation and filter requests use the real UI helpers.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(path.resolve(__dirname,'..','..','FlightBoardWeb.h'),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)[1];
new vm.Script(script);
let assertions=0;
const check=(value,message)=>{++assertions;assert.ok(value,message);};
const equal=(a,b,message)=>{++assertions;assert.equal(a,b,message);};
const elements=new Map(),posts=[];
function element(id){if(!elements.has(id))elements.set(id,{id,style:{},classList:{toggle(){}},options:[],textContent:'',value:'',checked:false,disabled:false,hidden:false,setAttribute(){},append(child){child.parentElement=this;},add(option){this.options.push(option);},showModal(){this.open=true;},close(){this.open=false;}});return elements.get(id);}
const context=vm.createContext({String,Number,Boolean,Math,Array,Date:{now:()=>context.now},now:100000,lastStateAt:100000,demo:false,serverOnline:true,commandBusy:false,uploadBusy:false,selectedHex:'AFR123',contentBusy:false,currentContentId:'',storeMode:false,contentItems:[],storeItems:[],contentCache:new Map(),storeContentCache:new Map(),libraryCategory:'all',contentNotice:()=>{},loadContent:async()=>{},DEMO:{},MODE_NAMES:{auto:'Auto',manual:'Épinglé',clock:'Horloge'},key:p=>p.hex||p.callsign,
  state:{app:'aviation',visual:'lava',place:'Test',display:{mode:'manual',activeHex:'AFR123',timeSynced:true,clock:'14:32'},aircraft:[{hex:'AFR123'}],totalCount:1},
  window:{scrollTo(){}},renderLibrary:()=>{},el:element,document:{querySelectorAll:()=>[]},Option:class{constructor(text,value){this.text=text;this.value=value;}},
  data:()=>context.demo?context.DEMO:context.state,fmt:(n,d=0)=>Number(n).toLocaleString('fr-FR',{minimumFractionDigits:d,maximumFractionDigits:d}),
  notify:message=>{context.message=message;},setDemo:value=>{context.demo=value;},
  post:async(route,body)=>{posts.push({route,body:JSON.parse(JSON.stringify(body))});if(context.holdPost)await new Promise(resolve=>{context.releasePost=resolve;});if(context.fail)throw Error(context.fail);},
  refresh:async()=>{context.refreshes++;},refreshes:0,
  render:()=>vm.runInContext('renderModules()',context)
});
const run=code=>vm.runInContext(code,context);
run(script.slice(script.indexOf('const APP_NAMES='),script.indexOf('function notify(')));
run(script.slice(script.indexOf('function previewIsDraft('),script.indexOf('function previewVisible(')));
run(script.slice(script.indexOf('function aircraft('),script.indexOf('function selectedPlane(')));
run(script.slice(script.indexOf('async function setMode('),script.indexOf('function cityMatching(')));
(async()=>{
  run('renderModules()');equal(element('hub').hidden,false);equal(element('aviationApp').hidden,true);
  run("showModule('aviation')");equal(element('aviationApp').hidden,false);equal(element('moduleTitle').textContent,'Flight Board');
  run("showModule('weather')");equal(element('weatherContent').hidden,false);equal(posts.length,0,'Browsing an app must not change the physical panel');
  equal(context.state.app,'aviation');equal(context.state.display.mode,'manual');equal(context.state.display.activeHex,'AFR123');
  equal(element('weatherTemp').textContent,'—','Weather must not show a fictional temperature');
  equal(element('weatherWind').textContent,'— km/h');equal(element('weatherCondition').textContent,'Prêt à afficher la météo');
  await run('activateModule()');equal(posts.length,1);equal(posts[0].route,'/api/app');equal(posts[0].body.app,'weather');equal(context.refreshes,1);
  context.fail='Connexion interrompue';await run('activateModule()');equal(element('moduleFeedback').textContent,'Connexion interrompue');equal(run('appBusy'),false);context.fail='';
  context.serverOnline=false;let count=posts.length;await run('activateModule()');equal(posts.length,count);context.serverOnline=true;
  context.uploadBusy=true;await run('activateModule()');equal(posts.length,count);context.uploadBusy=false;
  run("showModule('home')");await run('activateModule()');equal(posts.length,count);
  run("showModule('clock')");count=posts.length;await run("chooseVisual('stars')");equal(posts.length,count,'Choosing a style while browsing must not alter the active app');equal(run('draftVisual'),'stars');equal(context.state.display.mode,'manual');
  await run('activateModule()');equal(posts.at(-1).route,'/api/app');equal(posts.at(-1).body.app,'clock');equal(posts.at(-1).body.visual,'stars');equal(run('draftVisual'),null);
  context.state.app='clock';await run("chooseVisual('rain')");equal(posts.at(-1).route,'/api/visual');equal(posts.at(-1).body.visual,'rain');context.state.app='aviation';
  context.state.app='clock';context.state.visual='lava';run("showModule('clock');draftVisual='aurora'");count=posts.length;await run("chooseVisual('ocean')");equal(posts.length,count,'Changing an inspected preset within the active app stays local');equal(run('draftVisual'),'ocean');equal(context.state.visual,'lava');await run('activateModule()');equal(posts.at(-1).route,'/api/app');equal(posts.at(-1).body.app,'clock');equal(posts.at(-1).body.visual,'ocean');context.state.app='aviation';
  run("showModule('clock');draftVisual='aurora'");context.holdPost=true;const pendingActivation=run('activateModule()');run("showModule('animation');draftVisual='warp'");context.releasePost();await pendingActivation;context.holdPost=false;equal(run('currentView'),'animation');equal(run('draftVisual'),'warp','A late activation response must preserve a newer inspected module');
  context.state.app='clock';run("showModule('clock')");context.holdPost=true;const pendingVisual=run("chooseVisual('rain')");run("showModule('animation');draftVisual='warp'");context.releasePost();await pendingVisual;context.holdPost=false;equal(run('currentView'),'animation');equal(run('draftVisual'),'warp','A late visual response must preserve a newer inspected preset');context.state.app='aviation';
  run("showModule('clock')");
  run("showModule('not-an-app')");equal(run('currentView'),'clock');
  context.demo=true;run("showModule('weather')");equal(context.demo,false,'Leaving aviation demo must not fabricate weather');
  context.state.filters={logoOnly:true,airline:'logo_afr',type:'A320',maxAltitudeFt:12345};
  run('openFilters()');equal(element('filtersDialog').open,true);equal(element('filterLogo').checked,true);equal(element('filterAirline').value,'logo_afr');equal(element('filterType').value,'A320');equal(element('filterAltitude').value,'12345');equal(element('filterAltitude').options.length,1);
  run('openFilters()');equal(element('filterAltitude').options.length,1,'Opening existing custom altitude must not duplicate its option');
  element('filterType').value=' b738 ';await run('saveFilters()');equal(posts.at(-1).route,'/api/aviation/filters');equal(posts.at(-1).body.type,'B738');equal(posts.at(-1).body.logoOnly,true);equal(posts.at(-1).body.maxAltitudeFt,12345);equal(context.selectedHex,'');equal(element('filtersDialog').open,false);
  run('openFilters()');element('filterType').value='B-38';count=posts.length;await run('saveFilters()');equal(posts.length,count);check(element('filtersFeedback').textContent.includes('2 à 4'));equal(element('filtersDialog').open,true);
  element('filterType').value='A320';context.fail='Sauvegarde impossible';context.selectedHex='AFR123';await run('saveFilters()');equal(element('filtersFeedback').textContent,context.fail);equal(element('filtersDialog').open,true);equal(context.selectedHex,'AFR123');equal(run('filtersBusy'),false);equal(element('filtersApplyBtn').disabled,false);context.fail='';
  await run('saveFilters(true)');equal(posts.at(-1).body.type,'');equal(posts.at(-1).body.airline,'');equal(posts.at(-1).body.logoOnly,false);equal(posts.at(-1).body.maxAltitudeFt,0);
  context.DEMO={app:'aviation',display:{mode:'auto',activeHex:'afr'},aircraft:[{hex:'afr',callsign:'AFR123',type:'A320',altitudeAvailable:true,altitudeFt:10000},{hex:'ryr',callsign:'RYR123',type:'B738',altitudeAvailable:true,altitudeFt:12000}],filters:{}};
  context.demo=true;run("showModule('aviation')");count=posts.length;
  element('filterLogo').checked=true;element('filterAirline').value='logo_ryr';element('filterType').value='';element('filterAltitude').value='0';await run('saveFilters()');
  equal(posts.length,count,'Demo filters must stay local');equal(context.DEMO.display.activeHex,'ryr');equal(context.DEMO.display.mode,'auto');
  context.DEMO.display.mode='manual';context.DEMO.display.activeHex='afr';await run('saveFilters()');equal(context.DEMO.display.mode,'auto');equal(context.DEMO.display.activeHex,'ryr','An excluded manual pin must fall back to an eligible demo plane');
  context.DEMO.display.mode='manual';context.DEMO.display.activeHex='ryr';await run('saveFilters()');equal(context.DEMO.display.mode,'manual');equal(context.DEMO.display.activeHex,'ryr');
  await run("setMode('clock')");equal(context.DEMO.display.activeHex,'');await run("setMode('auto')");equal(context.DEMO.display.activeHex,'ryr','Returning to Auto must respect the demo filters');
  element('filterAirline').value='logo_bel';await run('saveFilters()');equal(context.DEMO.display.activeHex,'');equal(context.DEMO.display.mode,'auto');
  context.demo=false;
  context.state.app='weather';context.state.weather={status:'ready',available:true,temperatureC:0,windKmh:0,code:0,isDay:true,ageSeconds:15};run("showModule('weather')");equal(element('weatherTemp').textContent,'0°');equal(element('weatherWind').textContent,'0 km/h');equal(element('weatherCondition').textContent,'Ensoleillé');
  check(element('weatherAge').textContent.includes('Actualisé'));
  context.now=context.lastStateAt+1784000;run('renderModules()');check(element('weatherAge').textContent.includes('Actualisé'));check(element('weatherAge').textContent.includes('29 min'));
  context.now+=1000;run('renderModules()');check(element('weatherAge').textContent.includes('Dernier relevé'),'Cached weather must become old after elapsed browser time');check(element('weatherAge').textContent.includes('30 min'));equal(element('weatherTemp').textContent,'0°');
  context.now=context.lastStateAt;context.serverOnline=false;run('renderModules()');check(element('weatherAge').textContent.includes('Panneau hors ligne'));check(element('weatherAge').textContent.includes('Dernier relevé'));equal(element('weatherAge').textContent.includes('Actualisé'),false,'A lost connection must not label its last snapshot as freshly updated');
  context.now+=120000;run('renderModules()');check(element('weatherAge').textContent.includes('2 min'),'The last known observation must keep aging offline');
  context.serverOnline=true;context.now=context.lastStateAt;run('renderModules()');check(element('weatherAge').textContent.includes('Actualisé'));equal(element('weatherAge').textContent.includes('Panneau hors ligne'),false);
  for(const [temperatureC,label] of [[-1.5,'-1°'],[-.5,'0°'],[-0,'0°']]){context.state.weather.temperatureC=temperatureC;run('renderModules()');equal(element('weatherTemp').textContent,label,'Card temperature must round like the LED renderer');}
  context.state.weather={...context.state.weather,status:'stale',stale:true,temperatureC:-3,ageSeconds:1900};run('renderModules()');equal(element('weatherTemp').textContent,'-3°');check(element('weatherAge').textContent.includes('Dernier relevé'));check(element('weatherAge').textContent.includes('31 min'));
  for(const value of [null,undefined,NaN,'12']){context.state.weather.temperatureC=value;run('renderModules()');equal(element('weatherTemp').textContent,'—');}
  context.state.weather={status:'error'};run('renderModules()');equal(element('weatherCondition').textContent,'Météo temporairement indisponible');
  for(const code of [null,undefined,NaN,'0',-1,4,52,97,999]){context.weatherCode=code;equal(run('weatherDescription({code:weatherCode})[0]'),'Données indisponibles');}
  equal(element('previewToggle').parentElement.id,'modulePreviewHost');run("showModule('aviation')");equal(element('previewToggle').parentElement.id,'aviationPreviewHost');run("showModule('home')");equal(element('previewToggle').parentElement.id,'hubPreviewHost');
  console.log(`ZCS hub UI: ${assertions} assertions passed`);
})().catch(error=>{console.error(error);process.exitCode=1;});
