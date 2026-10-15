// Remote data remains data: checked catalogue, bounded packs, explicit install/activation.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../..'),html=fs.readFileSync(path.join(root,'FlightBoardWeb.h'),'utf8'),script=html.match(/<script>([\s\S]*?)<\/script>/)[1];
let assertions=0;const check=(v,m)=>{assertions++;assert.ok(v,m);},eq=(a,b,m)=>{assertions++;assert.equal(a,b,m);},throws=(fn,m)=>{assertions++;assert.throws(fn,m);};
const hash=bytes=>crypto.createHash('sha256').update(Buffer.from(bytes)).digest('hex');
const catalog=JSON.parse(fs.readFileSync(path.join(root,'store/catalog.json'),'utf8'));
function harness(){
 const elements=new Map(),remote=new Map(),requests=[],h={requests,remote,installed:[],failInstall:false,failCatalog:false,confirm:false};
 function el(id){if(!elements.has(id))elements.set(id,{textContent:'',hidden:false,disabled:false,children:[],classList:{toggle(){}},setAttribute(){},focus(){},querySelector(){return null;}});return elements.get(id);}
 for(const item of catalog.items){const bytes=fs.readFileSync(path.join(root,'store',item.url.split('/store/')[1]));remote.set(item.url,bytes);}
 const catalogUrl='https://raw.githubusercontent.com/kmsha19/flightboard-updates/main/store/catalog.json';remote.set(catalogUrl,Buffer.from(JSON.stringify(catalog)));
 const context=vm.createContext({String,Number,Boolean,Math,Array,Object,Set,Map,JSON,Date,URL,TextEncoder,TextDecoder,Uint8Array,AbortController,FormData,Blob,encodeURIComponent,
  APP_NAMES:{aviation:'Flight Board',weather:'Météo',clock:'Horloge',animation:'Animations',message:'Message',timer:'Minuteur',score:'Scores',pixel:'Pixel art'},
  latest:null,drawPreview:()=>{},currentView:'home',draftVisual:null,appBusy:false,uploadBusy:false,demo:false,serverOnline:true,widgetEditorKey:'',
  state:{version:'4.1.0',app:'weather',contentId:'',display:{brightness:20}},data:()=>context.state,
  el,localStorage:{getItem:()=>null,setItem(){}},document:{querySelectorAll:()=>[]},notify:message=>h.notice=message,render:()=>{},setPressed:()=>{},showModule:view=>context.currentView=view,
  refresh:async()=>{},post:async(route,body)=>{requests.push({route,body});},confirm:()=>h.confirm,sha256:hash,
  downloadReleaseBytes:async(url,max)=>{requests.push({url,max});if(h.failCatalog&&url===catalogUrl)throw Error('Network down');const bytes=remote.get(url);if(!bytes)throw Error('No fixture '+url);if(bytes.length>max)throw Error('Too large');return Uint8Array.from(bytes).buffer;},
  requestJson:async(route,options={})=>{requests.push({route,options});if(route==='/api/content')return{items:h.installed.map(pack=>{const{pixels,palette,...meta}=pack;return meta;}),capacity:14,token:'content-token',logos:{installed:true}};
   if(route.startsWith('/api/content/install')){if(h.failInstall)throw Error('Panneau occupe.');eq(options.method,'POST');eq(options.headers['X-Content-Token'],'content-token');const file=options.body.get('file'),pack=JSON.parse(await file.text());h.installed=h.installed.filter(p=>p.id!==pack.id).concat(pack);return{ok:true};}
   if(route.startsWith('/api/content/data')){const id=new URL(route,'http://device/').searchParams.get('id');return h.installed.find(pack=>pack.id===id);}
   if(route.startsWith('/api/content?id=')){eq(options.headers['X-Content-Token'],'content-token');if(options.method==='DELETE')h.installed=h.installed.filter(pack=>pack.id!==decodeURIComponent(route.split('=')[1]));return{ok:true};}
   throw Error('Unexpected endpoint '+route);},
 });
 h.context=context;h.el=el;h.run=code=>vm.runInContext(code,context);h.set=(key,value)=>context[key]=value;
 h.run(script.slice(script.indexOf('const LIBRARY_ITEMS='),script.indexOf('const APP_NAMES=')));
 h.run(script.slice(script.indexOf('let contentLogos='),script.indexOf('let widgetEditorKey=')));
 h.run(script.slice(script.indexOf('function stableVersion('),script.indexOf('function validateRelease(')));
 h.run('renderLibrary=()=>{};warmContentThumbnails=()=>{};');
 return h;
}
(async()=>{
 const h=harness();eq(h.requests.length,0,'Loading UI helpers never contacts GitHub');h.set('catalog',catalog);
 eq(h.run('validateStore(catalog).length'),8,'All published original packs satisfy the shipped contract');
 for(const changed of [{schema:2},{items:[...catalog.items,catalog.items[0]]},{items:[{...catalog.items[0],id:'UPPER'}]},{items:[{...catalog.items[0],id:'x'.repeat(33)}]},{items:[{...catalog.items[0],version:'1.0.0'}]},{items:[{...catalog.items[0],version:0}]},{items:[{...catalog.items[0],size:49153}]},{items:[{...catalog.items[0],sha256:'a'.repeat(63)}]},{items:[{...catalog.items[0],title:'é'.repeat(33)}]},{items:[{...catalog.items[0],minFirmware:'latest'}]}]){h.set('invalid',{...catalog,...changed});throws(()=>h.run('validateStore(invalid)'));}
 const base='https://raw.githubusercontent.com/kmsha19/flightboard-updates/main/store/';
 for(const url of ['http://raw.githubusercontent.com/kmsha19/flightboard-updates/main/store/a.json',base+'../latest.json',base+'a.json?redirect=1',base+'a.json#x',base+'%2e%2e/a.json','https://raw.githubusercontent.com/attacker/repo/main/store/a.json','https://evil.test/a.json','https://user:pass@raw.githubusercontent.com/kmsha19/flightboard-updates/main/store/a.json',base+'x.bin']){h.set('url',url);throws(()=>h.run('storeUrl(url)'),'Untrusted URL '+url);}
 const item=catalog.items.find(item=>item.type==='pixel'),bytes=h.remote.get(item.url),pack=JSON.parse(bytes),mapped={...item,contentId:item.id,app:item.type,store:true};h.set('item',mapped);h.set('pack',pack);
 eq(h.run('validateContentPack(pack,item).id'),pack.id);
 for(const delta of [{width:65},{height:65},{frameCount:33},{frameMs:79},{frameMs:2001},{palette:[]},{palette:[65536]},{pixels:pack.pixels.slice(1)},{pixels:'g'.repeat(pack.pixels.length)},{version:pack.version+1},{id:'different'}]){h.set('invalidPack',{...pack,...delta});throws(()=>h.run('validateContentPack(invalidPack,item)'));}
 const before=JSON.stringify(h.context.state);await h.run('openStore()');eq(h.run('storeItems.length'),8);check(h.requests.some(r=>r.url===base+'catalog.json'),'Only explicit Store opening fetches catalogue');eq(JSON.stringify(h.context.state),before);
 await h.run('getContentPack(item)');eq(h.run('storeContentCache.get(item.contentId).id'),pack.id);eq(h.run('contentCache.size'),0,'Store preview does not replace installed/active asset cache');eq(h.installed.length,0,'Viewing an asset never installs it');check(!h.requests.some(r=>r.route==='/api/app'),'Viewing never activates the panel');
 h.remote.set(item.url,Buffer.from('{}'));h.run('storeContentCache.clear()');await assert.rejects(h.run('getContentPack(item)'));assertions++;eq(h.installed.length,0,'Wrong hash/size cannot install');h.remote.set(item.url,bytes);
 await h.run('installContent(item)');eq(h.installed.length,1);eq(h.installed[0].id,pack.id);eq(JSON.stringify(h.context.state),before,'Installing leaves the current app untouched');check(!h.requests.some(r=>r.route==='/api/app'),'Installing never implicitly activates');eq(h.run('contentBusy'),false);
 await h.run('getContentPack(contentItem(contentItems[0]))');eq(h.run('contentCache.get(item.contentId).version'),pack.version,'Installed preview comes from the device');
 h.failCatalog=true;await h.run('openStore(true)');eq(h.run('storeItems.length'),8,'A failed catalogue refresh retains the cached catalogue');eq(h.run('contentItems.length'),1,'Store failure cannot erase installed contents');check(h.el('contentStatus').textContent.includes('bibliothèque'));h.failCatalog=false;
 h.failInstall=true;const previous=h.installed.length;await h.run('installContent(item)');eq(h.installed.length,previous);eq(h.run('contentBusy'),false);check(h.el('contentStatus').textContent.includes('occupe'),'Busy device gives an explicit retryable error');h.failInstall=false;
 const count=h.requests.length;for(const [flag,value] of [['serverOnline',false],['demo',true],['uploadBusy',true],['appBusy',true]]){const old=h.context[flag];h.context[flag]=value;await h.run('installContent(item)');eq(h.requests.length,count,'Blocked install '+flag);h.context[flag]=old;}
 h.run('currentContentId=item.contentId');await h.run('removeContent()');eq(h.installed.length,1,'Unconfirmed deletion makes no change');h.confirm=true;await h.run('removeContent()');eq(h.installed.length,0);check(!h.requests.some(r=>r.route==='/api/app'));
 // Pack favorites survive before the asynchronous installed list arrives.
 h.context.localStorage.getItem=()=>JSON.stringify(['pack:'+pack.id,'weather','not-installed-builtin']);eq(JSON.stringify(h.run('readLibraryFavorites()')),JSON.stringify(['pack:'+pack.id,'weather']));
 console.log(`Store UI: ${assertions} assertions passed`);
})().catch(error=>{console.error(error);process.exitCode=1;});
