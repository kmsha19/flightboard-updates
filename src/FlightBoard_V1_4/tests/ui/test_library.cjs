// V4 library: discovery stays local; only explicit activation changes the panel.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(path.resolve(__dirname,'..','..','FlightBoardWeb.h'),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)[1];
new vm.Script(script);
const start=script.indexOf('const LIBRARY_ITEMS='),end=script.indexOf('const APP_NAMES=',start);
assert.ok(start>=0&&end>start,'Library helpers must be present in the shipped UI');
const source=script.slice(start,end);
let assertions=0;
const check=(value,message)=>{++assertions;assert.ok(value,message);};
const equal=(actual,expected,message)=>{++assertions;assert.equal(actual,expected,message);};
const same=(actual,expected,message)=>{++assertions;assert.deepEqual(JSON.parse(JSON.stringify(actual)),expected,message);};
const settle=async()=>{for(let i=0;i<8;i++)await Promise.resolve();};
function harness(){
  const elements=new Map(),posts=[],notices=[],storage=new Map();
  const h={posts,notices,storage,fail:'',getFails:false,setFails:false,held:false,pending:null};
  function makeElement(id=''){
    const selectors=new Map(),classes=new Set();
    return {id,value:'',textContent:'',hidden:false,disabled:false,dataset:{},style:{},attributes:{},events:{},children:[],replacements:0,
      classList:{toggle(name,on){if(on)classes.add(name);else classes.delete(name);},contains:name=>classes.has(name)},
      setAttribute(name,value){this.attributes[name]=value;},addEventListener(name,fn){this.events[name]=fn;},
      querySelector(selector){
        if(id==='libraryGrid'){
          const match=selector.match(/^\[data-library-id="([^"]+)"\] \.libraryFavorite$/);
          const card=match?this.children.find(child=>child.dataset.libraryId===match[1]):selector==='.libraryFavorite'?this.children[0]:null;
          return card?card.querySelector('.libraryFavorite'):null;
        }
        if(!selectors.has(selector))selectors.set(selector,makeElement());return selectors.get(selector);
      },focus(options){h.focused=this;h.focusOptions=options;},
      querySelectorAll(selector){return selector==='[data-library-id]'?this.children:[];},
      replaceChildren(...children){this.children=children;this.replacements++;},append(...children){this.children.push(...children);}
    };
  }
  function el(id){if(!elements.has(id))elements.set(id,makeElement(id));return elements.get(id);}
  const state={app:'weather',visual:'ocean',place:'Lieu test',display:{app:'weather',visual:'ocean',timeSynced:true,clock:'14:32',effectiveBrightness:40,sleeping:false},weather:{available:true,temperatureC:18,code:2,isDay:true}};
  const context=vm.createContext({String,Number,Boolean,Math,Array,Object,Set,Map,JSON,Date,el,
    state,currentView:'home',draftVisual:null,appBusy:false,uploadBusy:false,demo:false,serverOnline:true,
    APP_NAMES:{aviation:'Flight Board',weather:'Météo',clock:'Horloge',animation:'Animations',message:'Message',timer:'Minuteur',score:'Scores',pixel:'Pixel art'},WIDGET_APPS:['message','timer','score','pixel'],widgetEditorKey:'',widgetDirty:false,widgetDraft:null,contentLogos:null,widgetView:()=>['message','timer','score','pixel'].includes(context.currentView),setPressed:()=>{},
    data:()=>context.state,panelApp:s=>s?.app||s?.display?.app||'aviation',
    localStorage:{getItem(key){if(h.getFails)throw Error('Storage unavailable');return storage.get(key)??null;},setItem(key,value){if(h.setFails)throw Error('Storage full');storage.set(key,value);}},
    document:{querySelectorAll:()=>[],createElement:()=>makeElement()},notify:message=>notices.push(message),
    render:()=>{context.renders++;},renders:0,renderModules:()=>{},
    showModule:app=>{context.currentView=app;context.draftVisual=null;},
    post:async(route,body)=>{posts.push({route,body:JSON.parse(JSON.stringify(body))});if(h.held)await new Promise((resolve,reject)=>{h.pending={resolve,reject};});if(h.fail)throw Error(h.fail);return{ok:true};},
    refresh:async()=>{context.refreshes++;},refreshes:0,
  });
  h.context=context;h.el=el;h.run=code=>vm.runInContext(code,context);
  h.run(source);h.run(script.slice(script.indexOf('function previewIsDraft('),script.indexOf('function previewVisible(')));h.run('const actualRenderLibrary=renderLibrary;renderLibrary=()=>{libraryRenders++;};libraryThumbnail=(canvas,item)=>{canvas.thumbnailId=item.id;};');context.libraryRenders=0;
  h.items=JSON.parse(h.run('JSON.stringify(LIBRARY_ITEMS)'));
  return h;
}
(async()=>{
  const markup=html.slice(0,html.indexOf('<script')),ids=[...markup.matchAll(/\bid="([^"]+)"/g)].map(m=>m[1]);
  equal(new Set(ids).size,ids.length,'The redesigned DOM keeps IDs unique');
  for(const id of new Set([...script.matchAll(/\bel\('([^']+)'\)/g)].map(m=>m[1])))check(ids.includes(id),'Required UI element missing: '+id);
  const h=harness(),items=h.items;
  equal(items.length,20,'The library exposes live/practical apps, eight clock presets and seven animations');
  equal(new Set(items.map(i=>i.id)).size,20,'Favorites require unique stable IDs');
  for(const app of ['clock','animation']){
    const rows=items.filter(i=>i.app===app);equal(rows.length,app==='clock'?8:7);
    same(rows.map(i=>i.visual).sort(),app==='clock'?['aurora','chrono','lava','ocean','plain','rain','stars','warp']:['aurora','lava','ocean','plain','rain','stars','warp']);
  }
  equal(items.filter(i=>i.app==='aviation').length,1);equal(items.filter(i=>i.app==='weather').length,1);
  for(const item of items){check(typeof item.id==='string'&&item.id.length>0);check(typeof item.name==='string'&&item.name.length>0);check(typeof item.description==='string');}
  equal(h.run("normalizeLibraryText('Étoilé')"),'etoile');
  h.run("libraryCategory='all';libraryQuery='METEO'");same(h.run('visibleLibraryItems().map(i=>i.app)'),['weather','clock'],'Search ignores accents and case, and matches the bedside clock description');
  h.run("libraryCategory='clock';libraryQuery=''");equal(h.run('visibleLibraryItems().length'),8);
  h.run("libraryQuery='océan'");same(h.run('visibleLibraryItems().map(i=>i.visual)'),['ocean'],'Category and query must intersect');
  h.run("libraryCategory='all';libraryQuery='   '");equal(h.run('visibleLibraryItems().length'),20,'Whitespace-only query should not hide the catalogue');
  h.run("libraryQuery='zzzz-no-match'");equal(h.run('visibleLibraryItems().length'),0);
  equal(h.posts.length,0,'Searching and filtering must remain local');

  const clock=items.find(i=>i.app==='clock'&&i.visual==='aurora'),animation=items.find(i=>i.app==='animation'&&i.visual==='warp'),weather=items.find(i=>i.app==='weather');
  const key='zcs-library-favorites';
  for(const value of [null,'{broken','{}','null','42','"weather"']){if(value===null)h.storage.delete(key);else h.storage.set(key,value);same(h.run('readLibraryFavorites()'),[],'Invalid local storage must not break startup');}
  h.storage.set(key,JSON.stringify([clock.id,'missing-id',clock.id,null,42,weather.id]));same(h.run('readLibraryFavorites()'),[clock.id,weather.id],'Favorites contain known unique string IDs only');
  h.getFails=true;same(h.run('readLibraryFavorites()'),[],'Blocked browser storage must be safe');h.getFails=false;
  h.run('libraryFavorites=[];libraryCategory="favorites";libraryQuery=""');equal(h.run('visibleLibraryItems().length'),0);
  h.run('toggleLibraryFavorite('+JSON.stringify(clock.id)+')');same(h.run('visibleLibraryItems().map(i=>i.id)'),[clock.id]);same(JSON.parse(h.storage.get(key)),[clock.id]);equal(h.posts.length,0);
  h.run('toggleLibraryFavorite('+JSON.stringify(clock.id)+')');equal(h.run('visibleLibraryItems().length'),0);
  h.setFails=true;h.run('toggleLibraryFavorite('+JSON.stringify(weather.id)+')');same(h.run('visibleLibraryItems().map(i=>i.id)'),[weather.id],'Quota errors must leave in-session favorites usable');h.setFails=false;
  const favoritesBefore=h.run('JSON.stringify(libraryFavorites)');h.run("toggleLibraryFavorite('missing-id')");equal(h.run('JSON.stringify(libraryFavorites)'),favoritesBefore,'Unknown IDs cannot become favorites');equal(h.posts.length,0);

  const physical=JSON.stringify(h.context.state);
  for(const item of [clock,animation,weather]){
    h.run('openLibraryItem('+JSON.stringify(item.id)+')');equal(h.context.currentView,item.app);equal(h.context.draftVisual,item.visual||null);equal(h.posts.length,0,'Inspecting a card never changes the real panel');equal(JSON.stringify(h.context.state),physical);
  }
  h.run("openLibraryItem('missing-id')");equal(h.context.currentView,'weather');equal(h.posts.length,0);
  h.run('openLibraryItem('+JSON.stringify(clock.id)+')');
  await h.run('activateLibraryItem('+JSON.stringify(clock.id)+')');same(h.posts.at(-1),{route:'/api/app',body:{app:'clock',visual:'aurora'}},'Activation sends app and selected ambience atomically');equal(h.context.refreshes,1);equal(h.context.draftVisual,null);equal(h.context.appBusy,false);
  await h.run('activateLibraryItem('+JSON.stringify(weather.id)+')');same(h.posts.at(-1),{route:'/api/app',body:{app:'weather'}},'Live apps must not overwrite stored visual selection');
  const count=h.posts.length;
  for(const [flag,value] of [['demo',true],['serverOnline',false],['appBusy',true],['uploadBusy',true]]){const prior=h.context[flag];h.context[flag]=value;await h.run('activateLibraryItem('+JSON.stringify(animation.id)+')');equal(h.posts.length,count,'Blocked state '+flag+' cannot activate');h.context[flag]=prior;}
  await h.run("activateLibraryItem('missing-id')");equal(h.posts.length,count);
  h.run('openLibraryItem('+JSON.stringify(animation.id)+')');h.fail='Connexion interrompue';await h.run('activateLibraryItem('+JSON.stringify(animation.id)+')');equal(h.context.draftVisual,'warp','A failed activation must retain the inspected preset');equal(h.context.appBusy,false);check(h.notices.some(s=>s.includes(h.fail))||h.el('moduleFeedback').textContent.includes(h.fail),'Activation failures must be visible');h.fail='';
  h.held=true;const pending=h.run('activateLibraryItem('+JSON.stringify(animation.id)+')');await settle();equal(h.context.appBusy,true);const waiting=h.posts.length;await h.run('activateLibraryItem('+JSON.stringify(weather.id)+')');equal(h.posts.length,waiting,'Only one activation may be in flight');h.pending.resolve();await pending;equal(h.context.appBusy,false);h.held=false;
  // A reply must not discard a new scene inspected while activation is pending.
  const race=harness();race.held=true;const applying=race.run('activateLibraryItem('+JSON.stringify(clock.id)+')');await settle();race.run('openLibraryItem('+JSON.stringify(animation.id)+')');race.pending.resolve();await applying;equal(race.context.currentView,'animation');equal(race.context.draftVisual,'warp','Late activation success preserves a newer inspected preset');

  // Polling updates live badges without rebuilding cards or losing keyboard focus.
  const dom=harness();dom.run('renderLibrary=actualRenderLibrary;actualRenderLibrary()');const grid=dom.el('libraryGrid');equal(grid.children.length,20);equal(grid.replacements,1);const firstCard=grid.children[0];
  const weatherCard=grid.children.find(card=>card.dataset.libraryId===weather.id),clockCard=grid.children.find(card=>card.dataset.libraryId===clock.id);
  equal(weatherCard.querySelector('.libraryLiveBadge').hidden,false);equal(weatherCard.querySelector('.libraryActivate').disabled,true);equal(clockCard.querySelector('.libraryLiveBadge').hidden,true);equal(clockCard.querySelector('.libraryActivate').disabled,false);
  dom.context.state.app='clock';dom.context.state.visual='aurora';dom.run('actualRenderLibrary()');equal(grid.replacements,1,'Polls retain the existing focusable cards');equal(grid.children[0],firstCard);equal(clockCard.querySelector('.libraryLiveBadge').hidden,false);equal(weatherCard.querySelector('.libraryLiveBadge').hidden,true);
  dom.context.serverOnline=false;dom.run('actualRenderLibrary()');check(grid.children.every(card=>card.querySelector('.libraryActivate').disabled),'Offline browsing must disable every activation button');check(grid.children.every(card=>card.querySelector('.libraryLiveBadge').hidden),'Offline snapshots are not live');equal(grid.replacements,1);
  dom.context.serverOnline=true;dom.run('libraryCategory="favorites";actualRenderLibrary()');equal(grid.children.length,0);equal(dom.el('libraryEmpty').hidden,false);check(dom.el('libraryEmpty').textContent.includes('étoile'));
  dom.run('libraryCategory="clock";actualRenderLibrary()');equal(grid.children.length,8);equal(dom.el('libraryEmpty').hidden,true);equal(dom.el('libraryCount').textContent,'8 contenus');
  const card=grid.children[0];card.querySelector('.libraryOpen').events.click();equal(dom.context.currentView,'clock');equal(dom.posts.length,0,'Discover buttons stay local');
  card.querySelector('.libraryFavorite').events.click();equal(dom.posts.length,0,'Favorite buttons stay local');dom.run('actualRenderLibrary()');equal(grid.children[0].querySelector('.libraryFavorite').attributes['aria-pressed'],'true');equal(dom.focused,grid.children[0].querySelector('.libraryFavorite'),'Toggling a favorite retains focus on the rebuilt star');check(dom.focused!==card.querySelector('.libraryFavorite'),'Focus moves to a connected replacement button');equal(dom.focusOptions.preventScroll,true);
  dom.run('libraryFavorites='+JSON.stringify([clock.id,animation.id])+';libraryCategory="favorites";libraryQuery="";actualRenderLibrary()');grid.children.find(card=>card.dataset.libraryId===clock.id).querySelector('.libraryFavorite').events.click();equal(grid.children.length,1);equal(dom.focused,grid.children[0].querySelector('.libraryFavorite'),'Removing a filtered favorite focuses the next visible star');grid.children[0].querySelector('.libraryFavorite').events.click();equal(grid.children.length,0);equal(dom.focused,dom.el('librarySearch'),'Removing the last favorite returns focus to search');equal(dom.focusOptions.preventScroll,true);
  dom.run('libraryCategory="all";actualRenderLibrary()');
  dom.run('libraryQuery="no-such-content";actualRenderLibrary()');equal(grid.children.length,0);equal(dom.el('libraryEmpty').hidden,false);check(dom.el('libraryEmpty').textContent.includes('Aucun résultat'));

  // Browsing renders a separate scene and must never mutate the physical state.
  const preview=harness();
  for(const view of ['home','aviation','weather']){preview.context.currentView=view;equal(preview.run('previewIsDraft()'),false);equal(preview.run('previewData()'),preview.context.state,'Unmodified views reuse physical state');}
  for(const item of [clock,animation]){
    preview.run('openLibraryItem('+JSON.stringify(item.id)+')');
    const before=JSON.stringify(preview.context.state),clone=preview.run('previewData()');
    equal(preview.run('previewIsDraft()'),true);check(clone!==preview.context.state);check(clone.display!==preview.context.state.display);
    equal(clone.app,item.app);equal(clone.display.app,item.app);equal(clone.visual,item.visual);equal(clone.display.visual,item.visual);equal(clone.display.clock,'14:32');equal(JSON.stringify(preview.context.state),before);
    clone.display.clock='00:00';equal(preview.context.state.display.clock,'14:32','The preview owns its display object');
  }
  preview.context.state.app='clock';preview.context.state.display.app='clock';preview.context.state.visual='aurora';preview.context.state.display.visual='aurora';preview.context.currentView='clock';preview.context.draftVisual='aurora';equal(preview.run('previewIsDraft()'),false);
  preview.context.draftVisual='ocean';equal(preview.run('previewIsDraft()'),true,'A different ambience within the active app is still a draft');equal(preview.run('previewData().visual'),'ocean');equal(preview.context.state.visual,'aurora');
  preview.context.state.display.sleeping=true;equal(preview.run('previewData().display.sleeping'),false,'Browsing can preview content during scheduled sleep');equal(preview.context.state.display.sleeping,true,'Preview does not wake the physical panel');
  preview.context.state.display.brightness=25;preview.context.state.display.effectiveBrightness=0;equal(preview.run('previewData().display.effectiveBrightness'),25,'A sleeping panel must not black out a draft preview');equal(preview.context.state.display.effectiveBrightness,0);delete preview.context.state.display.brightness;equal(preview.run('previewData().display.effectiveBrightness'),60,'Draft has a usable brightness if an older snapshot omits the day setting');
  preview.context.state=null;equal(preview.run('previewData()'),null,'No device snapshot must not fabricate live data');
  console.log(`V4 library UI: ${assertions} assertions passed`);
})().catch(error=>{console.error(error);process.exitCode=1;});
