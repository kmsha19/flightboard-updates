// Remote updates: bounded downloads, integrity, explicit install and stale-response guards.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const html=fs.readFileSync(path.resolve(__dirname,'..','..','FlightBoardWeb.h'),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)?.[1];assert.ok(script);new vm.Script(script);
const start=script.indexOf('const UPDATE_FAMILY='),end=script.indexOf('AIRLINE_OPTIONS.forEach',start);assert.ok(start>=0&&end>start);
let assertions=2,fetches=[],responses=[],uploads=[],apiCalls=[],timers=new Map(),timerId=0;
const equal=(a,b,message)=>{assertions++;assert.equal(a,b,message);};
const check=(value,message)=>{assertions++;assert.ok(value,message);};
const includes=(value,part)=>check(value.includes(part),value+' should include '+part);
const elements=new Map();
function element(){return{open:false,hidden:false,disabled:false,textContent:'',value:'',files:[],dataset:{},showModal(){this.open=true;},close(){this.open=false;}};}
const info={version:'2.0.7',firmwareFamily:'FlightBoard.ESP32S3.Application.v2',otaSupported:true,updateBusy:false,maxFirmwareBytes:1310720,releaseManifestUrl:'https://updates.example/releases/latest.json',updateToken:'token',uptimeSeconds:500};
const bytes=new Uint8Array(64);bytes[0]=0xE9;for(let i=1;i<bytes.length;i++)bytes[i]=i;
const digest=value=>crypto.createHash('sha256').update(Buffer.from(value)).digest('hex');
const release={schema:1,version:'2.0.8',firmwareFamily:info.firmwareFamily,url:'https://updates.example/releases/FlightBoard-2.0.8.bin',size:bytes.length,sha256:digest(bytes),notes:'Améliorations du radar.\nRéglages conservés.'};
const context=vm.createContext({URL,AbortController,TypeError,TextDecoder,Uint8Array,ArrayBuffer,Number,String,Array,Boolean,Date,
  File:class extends Blob{constructor(parts,name,options){super(parts,options);this.name=name;}},
  el:id=>{if(!elements.has(id))elements.set(id,element());return elements.get(id);},
  setTimeout:(callback,delay)=>{timers.set(++timerId,{callback,delay});return timerId;},clearTimeout:id=>timers.delete(id),
  fetch:async(url,options)=>{fetches.push({url,options});const response=responses.shift();if(response instanceof Error)throw response;if(typeof response==='function')return response(options);if(!response)throw new Error('Unexpected network request '+url);return response;},
  requestJson:async(url,options)=>{apiCalls.push({url,options});return{...context.device};},
  sha256:digest,fmt:value=>String(Math.round(value)),render:()=>{},refresh:()=>{},notify:()=>{},
  recordUpload:async(file,device)=>{uploads.push({file,device,bytes:new Uint8Array(await file.arrayBuffer())});},
  device:{...info},restartedVersion:'2.0.8',
});
vm.runInContext("let demo=false,systemInfo=null,firmwareReady=null,uploadBusy=false,firmwareValidationRun=0,updateEpoch=0,releaseInfo=null,releaseBusy=false,releaseController=null,releaseRun=0;"+script.slice(start,end),context);
vm.runInContext("uploadFirmware=recordUpload;waitForRestart=async()=>({version:restartedVersion});",context);
const run=code=>vm.runInContext(code,context),tick=()=>new Promise(resolve=>setImmediate(resolve));
function response(data,options={}){
  const chunks=options.chunks||[data],headers={get:()=>options.length===undefined?String(data.length):options.length};let index=0;
  return{ok:options.status===undefined||options.status===200,status:options.status||200,redirected:Boolean(options.redirected),type:'cors',headers,
    body:{getReader:()=>({read:async()=>index<chunks.length?{done:false,value:chunks[index++]}:{done:true},cancel:async()=>{}})}};
}
function manifest(value=release,options){return response(new TextEncoder().encode(JSON.stringify(value)),options);}
async function open(device={}){run('closeUpdate()');elements.get('updateDialog')?.close();fetches=[];responses=[];uploads=[];apiCalls=[];timers.clear();context.device={...info,...device};context.restartedVersion='2.0.8';await run('openUpdate()');}
async function available(value=release){await open();responses.push(manifest(value));await run('checkRemoteUpdate()');}
function validate(value=release,device=info){context.candidate=value;context.metadata=device;return run('validateRelease(candidate,metadata)');}
function rejected(value,device=info){assertions++;assert.throws(()=>validate(value,device));}
(async()=>{
  equal(run("newerVersion('2.0.10','2.0.9')"),true);equal(run("newerVersion('2.1.0','2.9.0')"),false);equal(run("newerVersion('2.0.8','2.0.8')"),false);
  for(const version of ['2.0.8-beta','v2.0.8','02.0.8','2.0','2.0.8+1','9007199254740992.0.0'])rejected({...release,version});
  for(const patch of [{schema:2},{firmwareFamily:'OtherBoard'},{size:0},{size:1.5},{size:1310721},{sha256:'bad'},{notes:42}])rejected({...release,...patch});
  rejected(release,{...info,firmwareFamily:'OtherBoard'});equal(validate().url,release.url);
  for(const url of ['http://updates.example/releases/a.bin','https://evil.example/releases/a.bin','https://updates.example/other/a.bin','https://updates.example/releases-other/a.bin','https://user:pass@updates.example/releases/a.bin','https://updates.example/releases/a.bin#fragment','https://updates.example/releases/../a.bin','https://updates.example/releases/%2e%2e/a.bin','https://updates.example/releases/%252e%252e/a.bin','https://updates.example/releases/x%2fa.bin','https://updates.example/releases/a.txt'])rejected({...release,url});
  rejected(release,{...info,releaseManifestUrl:'http://updates.example/releases/latest.json'});
  await open({releaseManifestUrl:''});equal(fetches.length,0);equal(elements.get('checkUpdateBtn').disabled,true);equal(elements.get('firmwareFile').disabled,false);equal(elements.get('manualUpdate').open,false);equal(elements.get('updateBtn').hidden,true);includes(elements.get('updateFeedback').textContent,'pas encore configurées');
  await open({otaSupported:false});equal(elements.get('checkUpdateBtn').disabled,true);equal(elements.get('firmwareFile').disabled,true);await run('checkRemoteUpdate()');equal(fetches.length,0);
  await available();equal(uploads.length,0,'Searching must never flash firmware');equal(fetches.length,1,'Searching downloads only the manifest');equal(elements.get('updateBtn').textContent,'Installer la version 2.0.8');equal(elements.get('releaseNotes').textContent,release.notes);equal(elements.get('updateBtn').disabled,false);equal(elements.get('updateBtn').hidden,false);
  for(const request of fetches){equal(request.options.redirect,'error');equal(request.options.credentials,'omit');equal(request.options.mode,'cors');}
  responses.push(response(bytes));await run('installRemoteFirmware()');equal(uploads.length,1);assertions++;assert.deepEqual(uploads[0].bytes,bytes);equal(elements.get('updateBtn').dataset.reload,'true');equal(elements.get('updateBtn').textContent,'Recharger l’interface');equal(run('uploadBusy'),false);equal(run('releaseBusy'),false);equal(timers.size,0);
  check(apiCalls.every(call=>call.url==='/api/system'&&!call.options),'Remote update setup must not change settings or send a URL to the device');
  await available();context.restartedVersion='2.0.7';responses.push(response(bytes));await run('installRemoteFirmware()');equal(uploads.length,1);includes(elements.get('updateFeedback').textContent,'version 2.0.7, au lieu de 2.0.8');check(!elements.get('updateFeedback').textContent.includes('Mise à jour terminée'),'Rollback must not be reported as success');equal(elements.get('updateBtn').textContent,'Recharger l’interface');equal(elements.get('updateBtn').disabled,false);
  for(const version of ['2.0.7','2.0.6']){await available({...release,version});equal(run('releaseInfo'),null);equal(elements.get('updateBtn').disabled,true);includes(elements.get('updateFeedback').textContent,'à jour');equal(uploads.length,0);}
  for(const [failure,text] of [[new TypeError('CORS'),'Impossible de joindre'],[response(new Uint8Array(),{status:404}),'pas disponible'],[response(new Uint8Array(),{status:503}),'indisponible'],[manifest(release,{redirected:true}),'Le lien']]){
    await open();responses.push(failure);await run('checkRemoteUpdate()');includes(elements.get('updateFeedback').textContent,text);equal(run('releaseBusy'),false);equal(elements.get('checkUpdateBtn').disabled,false);equal(uploads.length,0);
  }
  await open();responses.push(response(new TextEncoder().encode('not JSON')));await run('checkRemoteUpdate()');includes(elements.get('updateFeedback').textContent,'illisible');
  await available({...release,sha256:'0'.repeat(64)});responses.push(response(bytes));await run('installRemoteFirmware()');equal(uploads.length,0);includes(elements.get('updateFeedback').textContent,'contrôle');equal(elements.get('updateCloseBtn').disabled,false);
  await available();responses.push(response(bytes.slice(0,32)));await run('installRemoteFirmware()');equal(uploads.length,0);includes(elements.get('updateFeedback').textContent,'incomplet');
  await available();responses.push(response(new Uint8Array(65),{length:null}));await run('installRemoteFirmware()');equal(uploads.length,0);includes(elements.get('updateFeedback').textContent,'taille autorisée');
  await available();responses.push(response(new Uint8Array(),{length:'9999999'}));await run('installRemoteFirmware()');equal(uploads.length,0);includes(elements.get('updateFeedback').textContent,'taille autorisée');
  await available();context.sha256=undefined;responses.push(response(bytes));await run('installRemoteFirmware()');equal(uploads.length,0);includes(elements.get('updateFeedback').textContent,'vérification');context.sha256=digest;
  await available();context.device.version='2.0.9';responses.push(response(bytes));await run('installRemoteFirmware()');equal(uploads.length,0,'A device upgraded since the search must not receive an older version');includes(elements.get('updateFeedback').textContent,'a changé');
  await available();context.device.firmwareFamily='OtherBoard';responses.push(response(bytes));await run('installRemoteFirmware()');equal(uploads.length,0);includes(elements.get('updateFeedback').textContent,'compatible');
  await open();let resolveManifest;responses.push(()=>new Promise(resolve=>resolveManifest=resolve));const pending=run('checkRemoteUpdate()');await tick();run('closeUpdate()');elements.get('updateDialog').close();check(fetches[0].options.signal.aborted,'Closing the dialog must abort a pending download');resolveManifest(manifest());await pending;equal(run('releaseInfo'),null,'A response arriving after close must be ignored');equal(uploads.length,0);
  await available();let resolveBinary;responses.push(()=>new Promise(resolve=>resolveBinary=resolve));const installing=run('installRemoteFirmware()');await tick();run('closeUpdate()');elements.get('updateDialog').close();resolveBinary(response(bytes));await installing;equal(uploads.length,0,'Closing during download must prevent OTA');equal(run('uploadBusy'),false);
  await open();responses.push(options=>new Promise((resolve,reject)=>options.signal.addEventListener('abort',()=>reject(new Error('aborted')))));const checking=run('checkRemoteUpdate()');await tick();const deadline=[...timers.values()].find(timer=>timer.delay===15000);check(deadline);deadline.callback();await checking;includes(elements.get('updateFeedback').textContent,'trop de temps');equal(run('releaseBusy'),false);equal(elements.get('checkUpdateBtn').disabled,false);
  await open();elements.get('firmwareFile').files=[new context.File([bytes],'manual.bin')];await run('validateFirmware()');equal(elements.get('updateBtn').disabled,false);equal(elements.get('updateBtn').textContent,'Installer ce programme');await run('installFirmware()');equal(fetches.length,0,'Manual fallback must not contact the release server');equal(uploads.length,1);
  await open();run('demo=true');await run('checkRemoteUpdate()');await run('installRemoteFirmware()');equal(fetches.length,0);equal(uploads.length,0,'Demo must not make update requests');
  console.log(`Remote update UI: ${assertions} assertions passed`);
})().catch(error=>{console.error(error);process.exitCode=1;});
