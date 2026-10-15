#!/usr/bin/env node
'use strict';
const assert=require('assert'),fs=require('fs'),path=require('path'),vm=require('vm'),crypto=require('crypto'),zlib=require('zlib');
const root=path.resolve(__dirname,'../..');
const metadata=JSON.parse(fs.readFileSync(path.join(root,'assets/zcs-web.json')));
const bytes=fs.readFileSync(path.join(root,'assets/zcs-web.bin'));
const header=fs.readFileSync(path.join(root,'ZcsBootstrap.h'),'utf8');
const gzip=Buffer.from([...header.matchAll(/0x([A-F0-9]{2})/g)].map(x=>parseInt(x[1],16)));
const html=zlib.gunzipSync(gzip).toString('utf8');
const code=html.match(/<script>\n([\s\S]*?)<\/script>/)[1];
let checks=0;function ok(v,m){checks++;assert.ok(v,m);}function eq(a,b,m){checks++;assert.deepStrictEqual(a,b,m);}
function state(){
  const nodes={},requests=[];let reloads=0,handler=async()=>{throw Error('unexpected network request');};
  const get=id=>nodes[id]||(nodes[id]={textContent:'',value:'',checked:false,files:[],disabled:false,listeners:{},addEventListener(type,fn){this.listeners[type]=fn;}});
  const ctx={console,Blob,FormData,AbortController,Uint8Array,ArrayBuffer,TextDecoder,setTimeout,clearTimeout,
    sha256:data=>crypto.createHash('sha256').update(Buffer.from(data)).digest('hex'),
    document:{getElementById:get,querySelectorAll:()=>['download','reload','install','connect','update'].map(get)},
    location:{reload:()=>reloads++},fetch:async(url,options)=>{requests.push({url,options});return handler(url,options);}};
  vm.runInNewContext(code,ctx);
  return {get,requests,ctx,handler:fn=>handler=fn,reloads:()=>reloads};
}
const response=(data,ok=true,declared=data.length)=>({ok,headers:{get:()=>String(declared)},body:{getReader:()=>{let sent=false;return {async read(){if(sent)return {done:true};sent=true;return {done:false,value:Uint8Array.from(data)};},async cancel(){sent=true;}};}}});
const json=value=>response(Buffer.from(JSON.stringify(value)));
const raw=()=>response(bytes);
const click=(s,id)=>s.get(id).listeners.click();
(async()=>{
  ok(gzip.length<=4096,'Rescue UI fits the embedded budget');
  ok(!html.includes('__WEB_'),'Every fingerprint placeholder was resolved');
  ok(html.includes(metadata.sha256)&&html.includes(metadata.url),'Rescue UI pins the firmware-matching content');
  new vm.Script(code);checks++;
  let s=state();eq(s.requests.length,0,'Loading rescue UI does not modify or fetch anything');
  await click(s,'install');ok(s.get('status').textContent.includes('Choisissez'),'Missing file is explained');eq(s.requests.length,0,'Missing file never starts an upload');
  s.get('webfile').files=[{size:3}];await click(s,'install');eq(s.requests.length,0,'Wrong-sized file rejected before network');
  s.get('webfile').files=[{size:bytes.length,arrayBuffer:async()=>new Uint8Array(bytes.length).buffer}];await click(s,'install');ok(s.get('status').textContent.includes('incorrect'),'Wrong SHA is explained');eq(s.requests.length,0,'Wrong SHA cannot reach the panel');
  s=state();s.handler(async url=>url===metadata.url?raw():url==='/api/content'?json({available:true,token:'test-token'}):json({items:[]}));
  await click(s,'download');
  eq(s.requests.map(r=>r.url),[metadata.url,'/api/content','/api/content/install?id=__web&size='+bytes.length+'&sha256='+metadata.sha256],'Download uses only the pinned URL and exact upload query');
  const install=s.requests[2];eq(install.options.method,'POST','Install is explicit POST');eq(install.options.headers['X-Content-Token'],'test-token','Fresh session token sent in header');
  eq(Buffer.from(await install.options.body.get('file').arrayBuffer()),bytes,'Uploaded bytes exactly equal verified content');eq(s.reloads(),1,'Verified successful install opens the interface');
  ok(!s.get('download').disabled,'Buttons re-enabled after success');
  s=state();s.get('webfile').files=[{size:bytes.length,arrayBuffer:async()=>Uint8Array.from(bytes).buffer}];s.handler(async url=>url==='/api/content'?json({available:true,token:'offline-token'}):json({}));
  await click(s,'install');eq(s.requests.length,2,'Offline import only contacts local panel');ok(s.requests.every(r=>r.url.startsWith('/')),'Offline recovery needs no internet');eq(s.reloads(),1,'Offline import succeeds');
  s=state();s.handler(async url=>url===metadata.url?raw():json({available:false}));await click(s,'download');eq(s.requests.length,2,'Unavailable store never uploads');ok(s.get('status').textContent.includes('stockage'),'Unavailable storage explained');eq(s.reloads(),0,'Failed install never reloads');
  s=state();s.handler(async()=>response(Buffer.from(JSON.stringify({error:'Panneau occupé'})),false));await click(s,'download');eq(s.get('status').textContent,'Panneau occupé','API error is surfaced safely');ok(!s.get('download').disabled,'Failure permits retry');eq(s.reloads(),0,'Failed download never activates');
  s=state();s.handler(async()=>response(bytes,true,bytes.length+1));await click(s,'download');ok(s.get('status').textContent.includes('volumineux'),'Oversized declared body is refused before reading');eq(s.requests.length,1,'Oversized download cannot contact panel');
  s=state();s.handler(async()=>response(Buffer.alloc(bytes.length+1),true,0));await click(s,'download');ok(s.get('status').textContent.includes('volumineux'),'Chunked body exceeding exact limit is refused');eq(s.requests.length,1,'Chunked overflow cannot contact panel');
  s=state();s.handler(async()=>response(bytes.subarray(0,bytes.length-1),true,0));await click(s,'download');ok(s.get('status').textContent.includes('correspond'),'Truncated response fails exact size validation');eq(s.requests.length,1,'Truncated body never uploads');
  s=state();s.get('ssid').value='My Network';s.get('password').value='mypassword';s.handler(async()=>json({message:'Connexion en cours'}));let prevented=false;
  await s.get('wifi').listeners.submit({preventDefault:()=>prevented=true});await new Promise(resolve=>setTimeout(resolve,0));
  ok(prevented,'WiFi form never submits credentials in URL');eq(s.requests[0].url,'/api/wifi','WiFi uses existing local route');eq(JSON.parse(s.requests[0].options.body),{ssid:'My Network',password:'mypassword',open:false},'WiFi payload preserves credentials only in request body');eq(s.get('password').value,'','Password cleared after request');
  s=state();s.get('ssid').value='Open Network';s.get('password').value='ignored';s.get('open').checked=true;s.handler(async()=>json({message:'Connexion'}));s.get('wifi').listeners.submit({preventDefault(){}});await new Promise(resolve=>setTimeout(resolve,0));eq(JSON.parse(s.requests[0].options.body).password,'','Open networks never send a leftover password');
  s=state();const firmware=new Blob(['firmware bytes']);s.get('firmware').files=[Object.assign(firmware,{name:'application.bin'})];s.handler(async url=>url==='/api/system'?json({updateToken:'ota-token'}):json({ok:true}));await click(s,'update');
  eq(s.requests.map(r=>r.url),['/api/system','/api/update'],'Manual firmware recovery uses existing guarded OTA');eq(JSON.parse(JSON.stringify(s.requests[1].options.headers)),{'X-Update-Token':'ota-token','X-Firmware-Size':String(firmware.size)},'OTA includes token and exact size');eq(s.requests[1].options.body.get('firmware').name,'application.bin','Firmware field keeps expected filename');ok(s.get('status').textContent.includes('redémarre'),'Successful OTA explains restart');
  console.log(`Bootstrap recovery: ${checks} assertions passed; ${gzip.length} embedded bytes`);
})().catch(error=>{console.error(error);process.exitCode=1;});
