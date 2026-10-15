// Compare native output against the actual tablet bitmap renderer, without a browser.
// Run: node tests/host/test_panel_parity.cjs (requires clang++ or CXX).
const fs=require('node:fs'),path=require('node:path'),os=require('node:os');
const vm=require('node:vm'),assert=require('node:assert/strict'),child=require('node:child_process');
const project=path.resolve(__dirname,'..','..'),tmp=fs.mkdtempSync(path.join(os.tmpdir(),'flightboard-panel-'));
try {
  const binary=path.join(tmp,'panel-test');
  let result=child.spawnSync(process.env.CXX||'clang++',['-std=c++17','-Os','-Wall','-Wextra','-Werror','-I'+project,path.join(__dirname,'test_panel.cpp'),'-o',binary],{encoding:'utf8'});
  assert.equal(result.status,0,result.stderr);
  result=child.spawnSync(binary,[tmp],{encoding:'utf8'});
  assert.equal(result.status,0,result.stderr);process.stdout.write(result.stdout);
  const html=fs.readFileSync(path.join(project,'FlightBoardWeb.h'),'utf8');
  const script=html.match(/<script>([\s\S]*?)<\/script>/)[1];
  const start=script.indexOf('function canvasText('),end=script.indexOf('function renderConnection()');
  assert.ok(start>=0&&end>start);
  let pixels=new Uint16Array(8192);
  function rgb565(r,g,b){return((r&248)<<8)|((g&252)<<3)|(b>>3);}
  function parseColor(value){
    if(value.startsWith('rgb('))return rgb565(...value.match(/\d+/g).map(Number));
    let hex=value.slice(1);if(hex.length===3)hex=Array.from(hex,c=>c+c).join('');
    return rgb565(parseInt(hex.slice(0,2),16),parseInt(hex.slice(2,4),16),parseInt(hex.slice(4,6),16));
  }
  const g={fillStyle:'#000',
    fillRect(x,y,w,h){
      const c=parseColor(this.fillStyle);
      assert.ok([x,y,w,h].every(Number.isInteger));
      assert.ok(x>=0&&y>=0&&x+w<=128&&y+h<=64);
      for(let py=y;py<y+h;py++)for(let px=x;px<x+w;px++)pixels[py*128+px]=c;
    },
    createImageData(width,height){return {width,height,data:new Uint8ClampedArray(width*height*4)};},
    putImageData(frame){for(let i=0;i<8192;i++)pixels[i]=rgb565(frame.data[i*4],frame.data[i*4+1],frame.data[i*4+2]);}
  };
  const canvas={style:{},getContext:()=>g},elements=new Map([['ledPreview',canvas],['previewToggle',{open:true}]]);
  const motion={matches:true};
  const context=vm.createContext({String,Number,Math,demo:false,serverOnline:true,currentView:'home',draftVisual:null,panelApp:s=>s?.app||s?.display?.app||'aviation',selected:null,state:null,now:0,
    APP_NAMES:{},WIDGET_APPS:['message','timer','score','pixel'],widgetView:()=>false,currentContentId:'',contentCache:new Map(),widgetRemaining:w=>w?.remainingSeconds,
    logoCache:new Map(),MODE_NAMES:{auto:'Auto',manual:'Épinglé',clock:'Horloge'},
    el:id=>{if(!elements.has(id))elements.set(id,{});return elements.get(id);},
    document:{hidden:false},window:{matchMedia:()=>motion},performance:{now:()=>context.now},fetchLogo:()=>{},
    setTimeout:()=>1,clearTimeout:()=>{},
    fmt:(n,d)=>Number(n).toLocaleString('fr-FR',{minimumFractionDigits:d,maximumFractionDigits:d})
  });
  vm.runInContext('const data=()=>state,previewPlane=()=>selected,routeFor=p=>p.route;'+['asciiMessage','timeLabel'].map(name=>script.match(new RegExp('function '+name+'\\([^\\n]+'))[0]).join('\n')+'\n'+script.slice(start,end),context);
  const plane={callsign:'AFR1234',type:'A320',distanceKm:8.4,speedKmh:819.6,speedAvailable:true,altitudeFt:34000,altitudeAvailable:true,
    route:{status:'known',origin:{code:'CDG'},destination:{code:'BER'}}};
  const display={mode:'auto',timeSynced:true,clock:'14:32'};
  function compare(name,selected,settings=display,online=true,extra={},now=0,reduced=true,directWidget=false){
    context.selected=selected;context.state={display:settings,...extra};context.serverOnline=online;context.now=now;motion.matches=reduced;pixels=new Uint16Array(8192);
    if(directWidget){g.fillStyle='#000';g.fillRect(0,0,128,64);if(!settings.sleeping)context.drawWidgetPreview(g,extra.app,extra.widget,reduced?0:now/1000);}
    else vm.runInContext('drawPreview()',context);
    const actual=fs.readFileSync(path.join(tmp,name+'.bin'));
    assert.equal(actual.length,16384);
    let different=0;
    for(let i=0;i<8192;i++){
      const native=actual.readUInt16LE(i*2);
      if(native!==pixels[i]){
        different++;
        assert.fail(`${name}: pixel ${i%128},${Math.floor(i/128)} differs: native ${native}, tablet ${pixels[i]}`);
      }
    }
    assert.equal(different,0,`${name}: every pixel must agree`);
    console.log(`PASS panel/tablet ${name}: ${8192-different}/8192 identical RGB565 pixels`);
  }
  compare('plane',plane);
  const styles=['classic','modern','digital','arcade','compact'],fontApps=['aviation','clock','weather','message','timer','score'];
  for(let f=0;f<styles.length;f++)for(let a=0;a<fontApps.length;a++){
    const app=fontApps[a],fonts=Object.fromEntries(fontApps.map(key=>[key,styles[f]]));
    const extra={app,fonts,weather:{available:true,temperatureC:-3.6,windKmh:12,code:0,isDay:true},widget:{text:"C'EST BIEN!",color:0x07e0,remainingSeconds:5999,running:false,home:'PSG',away:'OM',status:'69 MIN',homeScore:12,awayScore:9,available:true,stale:false}};
    compare('font_'+f+'_'+a,plane,display,true,extra);
    if(a===0)compare('font_idle_'+f,plane,{...display,mode:'clock'},true,extra);
    if(a===3)compare('font_scroll_'+f,plane,display,true,{...extra,widget:{text:'W'.repeat(120),color:0xf800}},6400,false);
  }

  for(const type of ['B738','B38M','PC21','A1','AB3'])compare('type_'+type,{...plane,type});
  compare('type_missing',{...plane,type:undefined});
  compare('type_invalid',{...plane,type:'A320X'});
  const large={...plane,callsign:'VERY-LONG-CALLSIGN',distanceKm:49.9,speedKmh:1234567,altitudeFt:100000,
    route:{status:'known',origin:{code:'LFPG'},destination:{code:'EDDB'}}};
  compare('large',large);
  compare('negative',{...large,distanceKm:1234.5,speedKmh:-1.5,altitudeFt:-1000});
  compare('missing',{...large,distanceKm:NaN,speedAvailable:false,altitudeAvailable:false,route:{status:'unconfirmed'}});
  compare('stale',null,{...display,mode:'manual'});
  compare('connecting',null,display,false);
  compare('clock',null,{...display,mode:'clock'});
  compare('unsynced',null,{...display,mode:'clock',timeSynced:false});
  for(const visual of ['lava','stars','rain','plain','aurora','ocean','warp'])for(const app of ['clock','animation']){
    const name=app+'_'+visual,extra={app,visual};
    compare(name,plane,{...display,mode:'manual'},true,extra);
    compare(name+'_moving',plane,{...display,mode:'manual'},true,extra,3200,false);
    compare(name+'_phase',plane,{...display,mode:'manual'},true,extra,1040,false);
    compare(name+'_wrap',plane,{...display,mode:'manual'},true,extra,4294967295,false);
    compare(name+'_reduced',plane,{...display,mode:'manual'},true,extra,3200,true);
  }
  const widgetCompare=(name,app,widget,now=0,reduced=false,settings=display)=>compare('widget_'+name,null,settings,true,{app,widget},now,reduced,true);
  widgetCompare('message_short','message',{text:'BONJOUR',color:0x07e0});
  widgetCompare('message_empty','message',{text:'',color:0x07e0});
  widgetCompare('message_punctuation','message',{text:"C'EST BIEN!",color:0x07e0});
  const longMessage={text:'W'.repeat(120),color:0xf800};
  widgetCompare('message_long','message',longMessage,6400);
  widgetCompare('message_wrap','message',longMessage,4294967295);
  widgetCompare('message_reduced','message',longMessage,6400,true);
  for(const [name,remainingSeconds,running] of [['running',60,true],['paused',62,false],['finished',0,false],['large',5999,true],['invalid',100000,false]])widgetCompare('timer_'+name,'timer',{remainingSeconds,running});
  let score={home:'PSG',away:'OM',status:'69 MIN',homeScore:0,awayScore:0,available:true,stale:false};
  widgetCompare('score_zero','score',score);
  widgetCompare('score_stale','score',{...score,homeScore:999,awayScore:999,stale:true});
  score={home:'ABCDEFGHIJKL',away:'MNOPQRSTUVWX',status:'123456789012345678901234',homeScore:12,awayScore:9,available:true,stale:false};
  widgetCompare('score_labels','score',score);widgetCompare('score_missing','score',{...score,available:false});
  let pixel={width:3,height:1,frameCount:2,frameMs:100,palette:[0xf800,0x07e0,0x001f],pixels:'012210'};
  widgetCompare('pixel_odd','pixel',pixel);widgetCompare('pixel_next','pixel',pixel,100);widgetCompare('pixel_wrap','pixel',pixel,4294967295);widgetCompare('pixel_reduced','pixel',pixel,100,true);
  pixel={...pixel,width:1,height:3};widgetCompare('pixel_portrait','pixel',pixel);widgetCompare('pixel_invalid','pixel',{...pixel,pixels:''});
  widgetCompare('missing','pixel',null);widgetCompare('sleep','pixel',pixel,0,false,{...display,sleeping:true});
  const weather={available:true,stale:false,temperatureC:19.4,windKmh:12.3,code:0,isDay:true};
  const compareWeather=(name,w,settings=display)=>compare('weather_'+name,plane,settings,true,{app:'weather',weather:w});
  compareWeather('missing',{available:false});compareWeather('sun',weather);
  compareWeather('night',{...weather,isDay:false});
  for(const code of [3,45,61,71,95,999])compareWeather('code_'+code,{...weather,code});
  for(const code of [0,1,2,3,45,48,51,53,55,56,57,61,63,65,66,67,71,73,75,77,80,81,82,85,86,95,96,99])for(const isDay of [false,true]){
    const name='weather_animated_'+code+(isDay?'_day':'_night'),extra={app:'weather',visual:'plain',weather:{...weather,code,isDay}};
    compare(name,plane,display,true,extra,3200,false);
    compare(name+'_phase',plane,display,true,extra,1040,false);
    compare(name+'_wrap',plane,display,true,extra,4294967295,false);
    compare(name+'_reduced',plane,display,true,extra,3200,true);
  }
  const negative={...weather,code:61,temperatureC:-3.6,windKmh:100.2};
  compareWeather('negative',negative);compareWeather('stale',{...negative,stale:true});
  compareWeather('nonfinite',{...negative,stale:true,temperatureC:NaN,windKmh:NaN});
  const chronoDisplay={...display,dateLabel:'LUN 16 OCT 2026',nightActive:false};
  const chronoWeather={available:true,stale:false,temperatureC:19.4,windKmh:12.3,code:0,isDay:true};
  const chronoExtra=(w=chronoWeather,fonts)=>Object.assign({app:'clock',visual:'chrono',weather:w},fonts?{fonts}:{});
  compare('clock_chrono',plane,chronoDisplay,true,chronoExtra());
  compare('clock_chrono_night',plane,{...chronoDisplay,nightActive:true},true,chronoExtra());
  compare('clock_chrono_unsynced',plane,{...chronoDisplay,timeSynced:false,clock:'',dateLabel:''},true,chronoExtra());
  compare('clock_chrono_nodate',plane,{...chronoDisplay,dateLabel:''},true,chronoExtra());
  compare('clock_chrono_noweather',plane,chronoDisplay,true,chronoExtra({available:false}));
  compare('clock_chrono_stale',plane,chronoDisplay,true,chronoExtra({...chronoWeather,stale:true,code:61,temperatureC:-3.6}));
  compare('clock_chrono_code',plane,chronoDisplay,true,chronoExtra({...chronoWeather,code:45,isDay:false,temperatureC:2.5}));
  for(const style of styles)compare('clock_chrono_font_'+style,plane,chronoDisplay,true,chronoExtra(chronoWeather,Object.fromEntries(fontApps.map(key=>[key,style]))));
  compare('zcs_sleep',plane,{...display,sleeping:true},true,{app:'weather',weather});
} finally { fs.rmSync(tmp,{recursive:true,force:true}); }
