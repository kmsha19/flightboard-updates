// Radar failures must explain the cause without presenting stale aircraft as live.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(path.resolve(__dirname,'..','..','FlightBoardWeb.h'),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)?.[1];
assert.ok(script,'The embedded JavaScript must exist');
new vm.Script(script,{filename:'FlightBoardWeb.h embedded JavaScript'});
const section=(start,end)=>{const a=script.indexOf(start),b=script.indexOf(end,a);assert.ok(a>=0&&b>a);return script.slice(a,b);};
let assertions=1;
const equal=(actual,expected,message)=>{++assertions;assert.equal(actual,expected,message);};
const includes=(actual,expected,message)=>{++assertions;assert.ok(actual.includes(expected),message||actual+' should include '+expected);};
const elements=new Map();
const element=()=>({children:[],style:{},scrollTop:0,replaceChildren(){this.children=[];},append(child){this.children.push(child);},querySelectorAll(){return[];}});
const context=vm.createContext({String,Number,Boolean,Math,Array,Date:{now:()=>context.now},
  now:100000,lastStateAt:100000,DEMO:{},demo:false,serverOnline:true,currentView:'aviation',selectedHex:'',commandBusy:false,uploadBusy:false,favoriteBusy:false,drawPreview:()=>{},syncRoutes:()=>{},
  el:id=>{if(!elements.has(id))elements.set(id,element());return elements.get(id);},
  document:{activeElement:null,createElement:element},renderBrightnessStatus:()=>{},renderModules:()=>{},renderAirlineLogo:()=>{},renderRoute:()=>{},setPressed:()=>{},
});
const helpers=section('function age(s)','function selectedPlane(s)');
const renderers=section('function renderRadar()','function canvasText(');
const connection=section('function renderConnection()','async function refresh()');
const zcs=section('const APP_NAMES=','let currentView=')+section('function panelApp(','function showModule(');
vm.runInContext('const data=()=>state,selectedPlane=s=>s.aircraft?.[0]||null,key=p=>p.hex;'+zcs+helpers+renderers+connection,context);
const run=source=>vm.runInContext(source,context);
const oldPlane={hex:'abc123',callsign:'AFR1234'};
function state(radar={},extra={}){
  context.state={wifiConnected:true,radarOk:false,ageSeconds:0,aircraft:[oldPlane],display:{activeHex:oldPlane.hex},radar,...extra};
  return run('radarNotice(state)');
}
function renderUnavailable(){
  run('renderRadar();renderDetails();renderConnection()');
  equal(elements.get('planeLayer').children.length,0,'Unavailable data must not draw old aircraft');
  equal(elements.get('flightList').children.length,1,'Only the status message must remain in the list');
  equal(elements.get('details').hidden,true,'The old flight card must be hidden');
  equal(elements.get('sendBtn').disabled,true,'An old aircraft must not be sent to the panel');
  equal(run('previewPlane(state)'),null,'The LED preview must not reuse an old flight');
  equal(elements.get('flightList').children[0].textContent,run('radarNotice(state).detail'));
  equal(elements.get('radarTitle').textContent,run('radarNotice(state).title'));
  equal(elements.get('emptyTitle').textContent,run('radarNotice(state).title'));
  equal(elements.get('connection').textContent,context.serverOnline?run('radarNotice(state).label'):'Panneau hors ligne');
}
let notice=state({status:'error',error:'http',httpStatus:429,retrySeconds:30});
equal(notice.label,'Radar limité');equal(notice.title,'Limite temporaire du service');includes(notice.detail,'automatiquement à un rythme réduit');includes(notice.detail,'limite temporairement');includes(notice.detail,'30 s');renderUnavailable();
context.now+=7000;includes(run('radarNotice(state).detail'),'23 s');
context.now+=50000;includes(run('radarNotice(state).detail'),'Nouvelle tentative attendue.');
equal(run('radarNotice(state).detail.includes("-27")'),false,'The countdown must never become negative');
context.now=context.lastStateAt;
for(const code of [500,502,503]){notice=state({status:'error',error:'http',httpStatus:code,retrySeconds:60});equal(notice.title,'Service radar indisponible');includes(notice.detail,'HTTP '+code);}
notice=state({status:'error',error:'http',httpStatus:403});equal(notice.title,'Requête radar refusée');includes(notice.detail,'HTTP 403');
for(const [error,label,fragment] of [
  ['network','Connexion radar impossible','joindre'],['http_begin','Connexion radar impossible','ouvrir'],
  ['json_incomplete','Lecture interrompue','incomplète'],['timeout','Radar trop lent','à temps'],
  ['empty_response','Réponse radar vide','aucune position exploitable'],['json_empty','Réponse radar vide','aucune position exploitable'],
  ['response_too_large','Réponse trop grande','capacité'],['json_invalid','Données invalides','format'],
  ['schema','Données invalides','attendues'],['json_memory','Mémoire insuffisante','mémoire'],['memory','Mémoire insuffisante','mémoire']
]){notice=state({status:'error',error,retrySeconds:30});equal(notice.label,label);includes(notice.detail,fragment);renderUnavailable();}
notice=state({status:'starting',error:'none'});equal(notice.label,'Démarrage radar');
notice=state({status:'fetching',error:''});equal(notice.label,'Relevé en cours');renderUnavailable();
notice=state({status:'unavailable',error:'worker'});equal(notice.label,'Radar indisponible');includes(notice.detail,'Redémarrez');
notice=state({status:'paused',pauseReason:'update'});equal(notice.title,'Mise à jour en cours');
notice=state({status:'paused',pauseReason:'wifi'});equal(notice.label,'Radar suspendu');equal(notice.title,'Configuration Wi-Fi');includes(notice.detail,'après la fermeture');
notice=state({status:'paused',pauseReason:'wifi',error:'none'},{radarOk:true,wifiConnected:true});equal(notice.label,'Radar suspendu','A deliberate Wi-Fi configuration pause must take precedence over the fresh snapshot');equal(notice.title,'Configuration Wi-Fi');run('renderConnection()');equal(elements.get('connection').textContent,'Radar suspendu');equal(elements.get('dot').style.background,'var(--amber)','A suspended radar must not keep the live green indicator');
notice=state({status:'paused',pauseReason:'wifi',error:'none'},{radarOk:true,wifiConnected:false});equal(notice.label,'Wi-Fi à reconnecter','An actual disconnected Wi-Fi still needs reconnection');
notice=state({status:'error',error:'wifi'});equal(notice.label,'Wi-Fi à reconnecter');
notice=state({status:'error',error:'future_code'});equal(notice.label,'Relevé échoué');
notice=state(undefined,{radar:undefined});equal(notice.label,'Radar en attente');renderUnavailable();
notice=state({status:'ready',error:'none'},{radarOk:true,ageSeconds:91});equal(notice.label,'Positions expirées');renderUnavailable();
notice=state({status:'ready',error:''},{radarOk:true,ageSeconds:89});equal(notice.label,'En direct');
context.now+=2000;equal(run('usable(state)'),false);equal(run('radarNotice(state).label'),'Positions expirées');renderUnavailable();
context.now=context.lastStateAt;
notice=state({status:'fetching',error:'none'},{radarOk:true,ageSeconds:30});equal(notice.label,'En direct','A fresh valid snapshot remains usable during the next fetch');
equal(run('previewPlane(state).callsign'),'AFR1234');
notice=state(undefined,{radar:undefined,radarOk:true,ageSeconds:0,aircraft:[]});equal(notice.label,'En direct','Older firmware remains compatible');
run('renderRadar()');equal(elements.get('radarTitle').textContent,'Un ciel tranquille','Only a successful empty result can report an empty sky');
for(const intervalSeconds of [15,20,25,30,60,120]){state({status:'ready',intervalSeconds},{radarOk:true,ageSeconds:5,aircraft:[]});run('render()');equal(elements.get('updated').textContent,'Actualisé il y a 5 s');equal(elements.get('radarCadence').textContent,'Actualisation : '+intervalSeconds+' s');equal(elements.get('radarCadence').hidden,false);}
state({status:'error',error:'http',httpStatus:429,intervalSeconds:20,retrySeconds:120},{radarOk:false,ageSeconds:5});run('render()');equal(elements.get('updated').textContent,'En attente des données');equal(elements.get('radarCadence').textContent,'Actualisation : 20 s');equal(elements.get('radarCadence').hidden,false,'Cadence remains visible while waiting for the radar');
for(const intervalSeconds of [undefined,0,-1]){state({status:'ready',intervalSeconds},{radarOk:true,ageSeconds:5,aircraft:[]});run('render()');equal(elements.get('updated').textContent,'Actualisé il y a 5 s');equal(elements.get('radarCadence').hidden,true);}
notice=state({status:'ready'},{wifiConnected:false,radarOk:true});equal(notice.label,'Wi-Fi à reconnecter');renderUnavailable();
context.serverOnline=false;state({status:'ready'},{radarOk:true});equal(run('radarNotice(state).label'),'Flight Board hors ligne');renderUnavailable();
context.demo=true;equal(run('radarNotice(state).label'),'Démonstration');equal(run('usable(state)'),true,'Demo stays independent of live service status');
console.log(`Radar status UI: ${assertions} assertions passed`);
