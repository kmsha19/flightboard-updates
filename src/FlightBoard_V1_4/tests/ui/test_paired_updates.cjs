const fs=require('node:fs'),vm=require('node:vm'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const script=fs.readFileSync('FlightBoardWeb.h','utf8').match(/<script>([\s\S]*?)<\/script>/)[1];
const asset=Buffer.alloc(500,42);asset[0]=31;asset[1]=139;asset[2]=8;
const hash=crypto.createHash('sha256').update(asset).digest('hex');
const info={pairedUpdates:true,releaseManifestUrl:'https://updates.example/main/latest.json'};
const expected={size:500,sha256:hash,url:'https://updates.example/main/store/zcs-web-'+hash+'.bin'};
let uploaded=[],failDownload=false,corruptDownload=false,failStage=false;
const nodes=new Map();const c=vm.createContext({URL,TextDecoder,Uint8Array,Number,Array,Blob,FormData,AbortController,setTimeout,Promise,
 el:id=>{if(!nodes.has(id))nodes.set(id,{});return nodes.get(id);},
 sha256:x=>crypto.createHash('sha256').update(Buffer.from(x)).digest('hex'),
 downloadReleaseBytes:async()=>{if(failDownload)throw Error('offline');return corruptDownload?Buffer.alloc(500):asset;},
 requestJson:async(url,options)=>{if(url==='/api/content')return{available:true,token:'t',interface:{stagingId:'__web_next'}};if(failStage)throw Error('Storage full');uploaded.push({url,options});return{};}});
vm.runInContext(script.slice(script.indexOf('function releaseUrl('),script.indexOf('function validateRelease('))+script.slice(script.indexOf('function firmwareInterface('),script.indexOf('function cancelReleaseRequest(')),c);
const image=Buffer.from('firmware bytes ZCSWEB1:500:'+hash+': other bytes');
const file=new Blob([image]);c.file=file;c.info=info;c.release={interface:expected};
let checks=0;const check=x=>{checks++;assert.ok(x);};
(async()=>{
 const parsed=c.firmwareInterface(image,info);assert.deepEqual(JSON.parse(JSON.stringify(parsed)),expected);checks++;
 check(await c.preparePairedInterface(file,{...info,pairedUpdates:false},null)===null);check(uploaded.length===0);
 await c.preparePairedInterface(file,info,{interface:expected});check(uploaded.length===1);check(uploaded[0].url.includes('id=__web_next'));check(uploaded[0].options.headers['X-Content-Token']==='t');
 assert.deepEqual(Buffer.from(await uploaded[0].options.body.get('file').arrayBuffer()),asset);checks++;
 for(const bad of [Buffer.from('legacy firmware'),Buffer.concat([image,image]),Buffer.from('ZCSWEB1:99999:'+hash+':')]){checks++;assert.throws(()=>c.firmwareInterface(bad,info));}
 for(const release of [{},{interface:{...expected,sha256:'0'.repeat(64)}},{interface:{...expected,size:501}},{interface:{...expected,url:'https://evil.example/file.bin'}}]){checks++;await assert.rejects(()=>c.preparePairedInterface(file,info,release));}
 const count=uploaded.length;failDownload=true;checks++;await assert.rejects(()=>c.preparePairedInterface(file,info,null));check(uploaded.length===count);failDownload=false;
 corruptDownload=true;checks++;await assert.rejects(()=>c.preparePairedInterface(file,info,null));check(uploaded.length===count);corruptDownload=false;
 failStage=true;checks++;await assert.rejects(()=>c.preparePairedInterface(file,info,null));check(uploaded.length===count);
 console.log('Paired update UI:',checks,'assertions passed');
})().catch(e=>{console.error(e);process.exitCode=1;});
