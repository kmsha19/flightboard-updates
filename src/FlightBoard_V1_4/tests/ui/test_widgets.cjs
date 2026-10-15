// Real widget editors, countdown snapshots and preview isolation, without hardware.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(path.resolve(__dirname,'..','..','FlightBoardWeb.h'),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)[1];new vm.Script(script);
let assertions=0;const check=(value,message)=>{assertions++;assert.ok(value,message);};const equal=(a,b,message)=>{assertions++;assert.equal(a,b,message);};
const same=(a,b,message)=>{assertions++;assert.deepEqual(JSON.parse(JSON.stringify(a)),JSON.parse(JSON.stringify(b)),message);};
const elements=new Map(),posts=[];
function element(id){if(!elements.has(id))elements.set(id,{value:'',textContent:'',hidden:false,disabled:false,options:[],replaceChildren(...items){this.options=items;this.value=items[0]?.value||'';},add(item){this.options.push(item);}});return elements.get(id);}
const context=vm.createContext({String,Number,Math,Array,Date:class extends Date{static now(){return context.now;}},now:100000,lastStateAt:100000,
  latest:{app:'aviation',widget:{text:'BONJOUR',color:0x07e0,durationSeconds:300,remainingSeconds:300,running:false},display:{sleeping:false}},
  currentView:'message',currentContentId:'',storeMode:false,draftVisual:null,demo:false,serverOnline:true,contentBusy:false,uploadBusy:false,appBusy:false,
  APP_NAMES:{message:'Message',timer:'Minuteur',score:'Scores',pixel:'Pixel'},notify:()=>{},render:()=>{},
  contentCache:new Map(),el:element,document:{querySelectorAll:()=>[]},Option:class{constructor(text,value){this.text=text;this.value=value;}},
  data:()=>context.latest,panelApp:s=>s?.app||s?.display?.app||'aviation',selectedContent:()=>null,selectedContentPack:()=>context.pack,
  renderModules:()=>{context.renders++;},drawPreview:()=>{context.draws++;},renders:0,draws:0,
  post:async(route,payload)=>{posts.push({route,payload:JSON.parse(JSON.stringify(payload))});if(context.holdPost)await new Promise(resolve=>{context.releasePost=resolve;});if(context.postError)throw Error(context.postError);},
  refresh:async()=>{context.refreshes++;},refreshes:0
});
const run=code=>vm.runInContext(code,context);
run(script.slice(script.indexOf("let widgetEditorKey="),script.indexOf('function renderModules()')));
run(script.slice(script.indexOf('function previewIsDraft()'),script.indexOf('function previewVisible()')));
run(script.slice(script.indexOf('async function activateModule()'),script.indexOf('async function chooseVisual(')));
function reset(){posts.length=0;context.holdPost=false;context.postError='';context.currentView='message';context.currentContentId='';context.storeMode=false;context.demo=false;context.serverOnline=true;context.contentBusy=false;context.uploadBusy=false;context.appBusy=false;context.now=context.lastStateAt=100000;run("widgetBusy=false;widgetDirty=false;widgetDraft=null;widgetEditorKey='';scoreBusy=false;");}
element('messageText').value='  École de Chloé  ';element('messageColor').value='#00ff00';
same(run('widgetPayload()'),{app:'message',text:'ECOLE DE CHLOE',color:0x07e0});
element('messageColor').value='#ff0000';equal(run('widgetPayload().color'),0xf800);element('messageColor').value='#0000ff';equal(run('widgetPayload().color'),0x001f);
for(const text of ['', ' '.repeat(10),'A'.repeat(121),'BONJOUR\nTOUS','HELLO 😀']){element('messageText').value=text;assertions++;assert.throws(()=>run('widgetPayload()'));}
element('messageText').value='A'.repeat(120);equal(run('widgetPayload().text.length'),120);
context.currentView='timer';for(const [minutes,seconds,expected] of [[0,1,1],[1,0,60],[99,59,5999]]){element('timerMinutes').value=String(minutes);element('timerSeconds').value=String(seconds);same(run('widgetPayload()'),{app:'timer',durationSeconds:expected});}
for(const [minutes,seconds] of [[0,0],[-1,30],[100,0],[0,60],[1.5,0],[0,1.5],['bad',3]]){element('timerMinutes').value=String(minutes);element('timerSeconds').value=String(seconds);assertions++;assert.throws(()=>run('widgetPayload()'));}
context.currentView='score';element('scoreLeague').value='fra.1';element('scoreDate').value='2026-10-15';element('scoreEvent').value='123456';same(run('widgetPayload()'),{app:'score',league:'fra.1',date:'20261015',eventId:'123456'});
for(const [field,value] of [['scoreLeague','unsupported'],['scoreDate','2026-1-1'],['scoreEvent','123'],['scoreEvent','12345x']]){const before=element(field).value;element(field).value=value;assertions++;assert.throws(()=>run('widgetPayload()'));element(field).value=before;}

// The timer advances locally from the most recent device sample, stops at zero,
// and cannot accelerate when the browser clock moves backwards.
context.latest={app:'timer',widget:{durationSeconds:300,remainingSeconds:62,running:true},display:{sleeping:false}};context.currentView='timer';context.now=102500;
equal(run('widgetRemaining(latest.widget)'),60);equal(run('previewIsDraft()'),false);same(run('previewData().widget'),{durationSeconds:300,remainingSeconds:60,running:true});equal(context.latest.widget.remainingSeconds,62);
context.now=200000;equal(run('widgetRemaining(latest.widget)'),0);context.now=90000;equal(run('widgetRemaining(latest.widget)'),62);
context.latest.widget.running=false;context.now=200000;equal(run('widgetRemaining(latest.widget)'),62);
equal(run('widgetRemaining({remainingSeconds:62,running:true},{})'),62,'An unrelated snapshot does not inherit elapsed time from the active state');
context.latest.widget.running=true;context.now=102500;context.latest.contentId='tea';context.currentContentId='tea';context.pack={id:'tea',app:'timer',durationSeconds:180};context.contentCache.set('tea',context.pack);
equal(run('previewIsDraft()'),false);equal(run('previewData().widget.remainingSeconds'),60,'Installed timer content uses the device countdown, not the original pack duration');
equal(run('previewData().widget.running'),true,'Running state must survive the installed content cache');
context.currentContentId='coffee';context.pack={id:'coffee',app:'timer',durationSeconds:90};equal(run('previewIsDraft()'),true);equal(run('previewData().widget.remainingSeconds'),90);check(!run('previewData().widget.running'),'Inspecting a timer pack must not start it');equal(context.latest.contentId,'tea');equal(context.latest.widget.remainingSeconds,62);
context.currentContentId='';context.currentView='message';run("widgetDraft={text:'NOUVEAU',color:65535};widgetDirty=true;");context.latest.display={sleeping:true,brightness:60,effectiveBrightness:0};
const draft=run('previewData()');equal(draft.app,'message');equal(draft.widget.text,'NOUVEAU');equal(draft.display.sleeping,false);equal(draft.display.effectiveBrightness,60);equal(context.latest.app,'timer');equal(context.latest.display.sleeping,true);equal(context.latest.widget.remainingSeconds,62);
context.currentView='pixel';context.currentContentId='arcade';context.pack={id:'arcade',app:'pixel',width:2,height:1,frameCount:1,frameMs:100,palette:[0,65535],pixels:'01'};same(run('previewData().widget.pixels'),'01');equal(posts.length,0,'Inspecting or editing a preview never sends a panel command');

(async()=>{
  reset();element('messageText').value='Salut';element('messageColor').value='#ffffff';
  check(await run('saveWidget()'));same(posts,[{route:'/api/widget',payload:{app:'message',text:'SALUT',color:65535}}]);check(element('widgetFeedback').textContent.includes('active reste inchangée'));equal(context.latest.app,'timer','Saving a message does not activate it');
  for(const guard of ['demo','uploadBusy','contentBusy']){reset();context[guard]=true;equal(await run('saveWidget()'),false);equal(posts.length,0);}
  reset();context.serverOnline=false;equal(await run('saveWidget()'),false);equal(posts.length,0);
  reset();context.currentContentId='installed';equal(await run('saveWidget()'),false);equal(posts.length,0,'Installed preset source cannot be overwritten by the native editor');
  reset();run('widgetBusy=true');equal(await run('saveWidget()'),false);equal(posts.length,0);
  reset();element('messageText').value='';equal(await run('saveWidget()'),false);equal(posts.length,0);check(element('widgetFeedback').textContent.includes('120'));
  reset();element('messageText').value='Valide';context.postError='Mémoire indisponible';run("widgetDraft={text:'VALIDE'};widgetDirty=true;");equal(await run('saveWidget()'),false);equal(run('widgetBusy'),false);equal(run('widgetDirty'),true);equal(run('widgetDraft.text'),'VALIDE');equal(element('widgetFeedback').textContent,'Mémoire indisponible');
  for(const action of ['start','pause','resume','reset']){reset();context.currentView='timer';await run(`timerCommand('${action}')`);same(posts,[{route:'/api/timer',payload:{action}}]);equal(run('widgetBusy'),false);}
  for(const guard of ['demo','uploadBusy','contentBusy']){reset();context[guard]=true;await run("timerCommand('start')");equal(posts.length,0);}
  reset();context.serverOnline=false;await run("timerCommand('start')");equal(posts.length,0);
  reset();context.postError='Action refusée';await run("timerCommand('resume')");equal(run('widgetBusy'),false);equal(element('widgetFeedback').textContent,'Action refusée');
  for(const navigation of ['module','content']){
    reset();element('messageText').value='Bonjour';element('messageColor').value='#ffffff';context.holdPost=true;
    const activation=run('activateModule()');equal(posts.length,1);equal(posts[0].route,'/api/widget');
    if(navigation==='module')context.currentView='timer';else context.currentContentId='a-different-message';
    context.releasePost();await activation;equal(posts.length,1,'Navigating during save cancels activation of the selection the user has left');equal(run('widgetBusy'),false);equal(context.appBusy,false);
  }
  console.log(`Widget UI: ${assertions} assertions passed`);
})().catch(error=>{console.error(error);process.exitCode=1;});
