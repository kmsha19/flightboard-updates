// Exercise the real slider queue against deliberately delayed device replies.
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(path.resolve(__dirname,'..','..','FlightBoardWeb.h'),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)[1];
new vm.Script(script);
const source=script.slice(script.indexOf('function cityMatching('),script.indexOf('function renderFavorites('));
let assertions=0;
const equal=(actual,expected,message)=>{++assertions;assert.equal(actual,expected,message);};
const check=(value,message)=>{++assertions;assert.ok(value,message);};
const settle=async()=>{for(let i=0;i<8;i++)await Promise.resolve();};
const settings=()=>({place:'Test',lat:48,lon:2,radiusKm:50,brightness:60,timezone:'paris',nightEnabled:true,nightStart:1320,nightEnd:420,nightBrightness:15,nightAction:'dim'});
function harness(){
  const elements=new Map(),requests=[],timers=[],notices=[];
  let pending=0,maxPending=0;
  function el(id){if(!elements.has(id))elements.set(id,{value:'',textContent:'',checked:false,disabled:false,open:false,showModal(){this.open=true;},close(){this.open=false;}});return elements.get(id);}
  function request(method,url,body){
    pending++;maxPending=Math.max(maxPending,pending);
    return new Promise((resolve,reject)=>requests.push({method,url,body:body&&JSON.parse(JSON.stringify(body)),done:false,
      resolve(result){this.done=true;pending--;resolve(result);},reject(message){this.done=true;pending--;reject(Error(message));}}));
  }
  const context=vm.createContext({Number,String,Boolean,Math,Object,TextEncoder,CITIES:{},el,
    demo:false,uploadBusy:false,settingsBusy:false,serverOnline:true,settingsEpoch:0,currentSettings:settings(),latest:{display:{brightness:60}},selectedHex:'abc',
    post:(url,body)=>request('POST',url,body),requestJson:url=>request('GET',url),
    setTimeout:(fn,ms)=>{timers.push({fn,ms});},notify:message=>notices.push(message),
    refresh:async()=>{context.refreshCount++;},refreshCount:0,renderFavorites:()=>{},loadFavorites:()=>{context.favoriteLoads++;},favoriteLoads:0,
  });
  const run=code=>vm.runInContext(code,context);
  run(source);run('fillSettings(currentSettings)');el('settingsDialog').open=true;
  return {context,el,run,requests,timers,notices,get maxPending(){return maxPending;},
    input(value,key='brightness'){el(key==='nightBrightness'?'nightBrightness':'brightnessInput').value=String(value);run('queueBrightness('+JSON.stringify(key)+')');},
    async respond(index,result){requests[index].resolve(result);await settle();},
    async reject(index,message='Connexion interrompue'){requests[index].reject(message);await settle();},
    async tick(){equal(timers.length,1,'There must be only one pacing timer');const timer=timers.shift();equal(timer.ms,150);timer.fn();await settle();},
  };
}
(async()=>{
  check(html.includes("addEventListener('input',()=>queueBrightness('brightness'))"),'Dragging must trigger the live queue without submitting the form');
  // The slow first reply must never roll the thumb or newer requested value back.
  {
    const h=harness();h.input(30);h.input(90);h.input(45);
    equal(h.requests.length,1,'Only the first position may be in flight');equal(h.requests[0].url,'/api/display/brightness');equal(h.requests[0].body.brightness,30);
    equal(h.el('brightnessInput').value,'45');equal(h.el('brightnessValue').textContent,'45 %');
    await h.respond(0,{brightness:30,brightnessPending:true});
    equal(h.context.currentSettings.brightness,60,'A reply superseded by a pending drag must not overwrite the settings model');equal(h.el('brightnessInput').value,'45');equal(h.requests.length,1);
    await h.tick();equal(h.requests.length,2);equal(h.requests[1].body.brightness,45,'Intermediate positions must be replaced by the latest position');
    await h.respond(1,{brightness:45,brightnessPending:true});await h.tick();
    equal(h.maxPending,1,'Device writes must never overlap');equal(h.context.currentSettings.brightness,45);equal(h.el('brightnessInput').value,'45');equal(h.run('brightnessWork'),null);
    check(h.el('brightnessFeedback').textContent.includes('mémorisation…'),'An applied response awaiting flash persistence must not claim it is saved');
    h.context.latest.display={brightness:45,brightnessPending:false,brightnessSaveError:false};h.run('renderBrightnessStatus()');equal(h.el('brightnessFeedback').textContent,'Mémorisée automatiquement.');
    h.context.latest.display={brightness:45,brightnessPending:true,brightnessSaveError:true};h.run('renderBrightnessStatus()');check(h.el('brightnessFeedback').textContent.includes('mémorisation impossible'));check(h.el('brightnessFeedback').textContent.includes('Nouvelle tentative automatique'));
  }
  // Closing/reopening the dialog does not discard an already requested value.
  {
    const h=harness();h.input(25);h.input(80);h.el('settingsDialog').close();
    const opening=h.run('openSettings()');await settle();equal(h.requests.length,1,'Reopening must wait for the live queue before reading persisted settings');
    await h.respond(0,{brightness:25,brightnessPending:true});await h.tick();equal(h.requests[1].body.brightness,80);
    await h.respond(1,{brightness:80,brightnessPending:true});await h.tick();equal(h.requests.length,3);equal(h.requests[2].method,'GET');equal(h.requests[2].url,'/api/settings');
    await h.respond(2,{...settings(),brightness:80,brightnessPending:true});await opening;
    equal(h.el('settingsDialog').open,true);equal(h.el('brightnessInput').value,80);equal(h.context.currentSettings.brightness,80);equal(h.context.favoriteLoads,1);
  }
  // A failed write remains visibly unconfirmed until a subsequent user retry succeeds.
  {
    const h=harness();h.input(50);await h.reject(0);await h.tick();
    equal(h.context.currentSettings.brightness,60);check(h.el('brightnessFeedback').textContent.includes('non confirmée'));check(h.el('brightnessFeedback').textContent.includes('Connexion interrompue'));
    h.run('renderBrightnessStatus()');check(h.el('brightnessFeedback').textContent.includes('non confirmée'),'Polling must not erase a write failure');equal(h.requests.length,1,'Failures must not trigger an uncontrolled HTTP retry loop');
    h.input(55);equal(h.requests.length,2);await h.respond(1,{brightness:55,brightnessPending:false});await h.tick();
    equal(h.context.currentSettings.brightness,55);equal(h.el('brightnessFeedback').textContent,'Mémorisée automatiquement.');equal(h.run('Object.keys(brightnessErrors).length'),0);
    h.input(70);h.el('settingsDialog').close();await h.reject(2,'Panneau hors ligne');await h.tick();check(h.notices.at(-1).includes('Panneau hors ligne'),'A failure after closing the dialog must still be reported');
  }
  // The final requested position must still be sent if the preceding position failed.
  {
    const h=harness();h.input(30);h.input(90);await h.reject(0);await h.tick();equal(h.requests[1].body.brightness,90);
    await h.respond(1,{brightness:90,brightnessPending:false});await h.tick();equal(h.context.currentSettings.brightness,90);equal(h.run('Object.keys(brightnessErrors).length'),0);
  }
  // Full settings save must wait for queued writes, preserving both brightness and other edits.
  {
    const h=harness();h.el('placeInput').value='Lieu modifié';h.input(30);h.input(75);
    const saving=h.run('saveSettings({preventDefault(){}})');await settle();equal(h.requests.length,1);equal(h.el('brightnessInput').disabled,true);equal(h.context.settingsBusy,true);
    await h.respond(0,{brightness:30,brightnessPending:true});await h.tick();equal(h.requests[1].body.brightness,75);equal(h.requests.length,2);
    await h.respond(1,{brightness:75,brightnessPending:true});await h.tick();equal(h.requests.length,3);equal(h.requests[2].url,'/api/settings');equal(h.requests[2].body.brightness,75);equal(h.requests[2].body.place,'Lieu modifié');equal(h.requests[2].body.nightBrightness,15);equal(h.maxPending,1);
    await h.respond(2,{});await saving;equal(h.el('settingsDialog').open,false);equal(h.context.settingsBusy,false);equal(h.context.currentSettings.brightness,75);equal(h.context.currentSettings.place,'Lieu modifié');equal(h.context.refreshCount,1);equal(h.el('brightnessInput').disabled,false);
  }
  {
    const h=harness();h.input(35);const saving=h.run('saveSettings({preventDefault(){}})');
    await h.respond(0,{brightness:35,brightnessPending:true});await h.tick();await h.reject(1,'Réglages non enregistrés');await saving;
    equal(h.el('settingsDialog').open,true);equal(h.context.settingsBusy,false);equal(h.el('settingsFeedback').textContent,'Réglages non enregistrés');equal(h.context.currentSettings.brightness,35,'Failure saving other settings must not undo the successful live brightness change');
  }
  for(const [flag,value] of [['demo',true],['uploadBusy',true],['settingsBusy',true],['serverOnline',false]]){
    const h=harness();h.context[flag]=value;h.run('renderBrightnessStatus()');equal(h.el('brightnessInput').disabled,true,flag+' must disable the live slider');h.input(95);equal(h.requests.length,0,flag+' must prevent writes even for programmatically dispatched input');equal(h.context.currentSettings.brightness,60);
  }
  check(html.includes("addEventListener('input',()=>queueBrightness('nightBrightness'))"),'Night slider must apply without submitting');
  // Interleaved day/night drags retain the last requested value of each field.
  {
    const h=harness();h.input(25,'nightBrightness');h.input(35);h.input(10,'nightBrightness');h.input(40);
    equal(h.requests.length,1);equal(h.requests[0].body.nightBrightness,25);equal(h.requests[0].body.brightness,undefined);
    await h.respond(0,{brightness:60,nightBrightness:25,brightnessPending:true});
    equal(h.context.currentSettings.nightBrightness,15);equal(h.el('nightBrightness').value,'10');
    await h.tick();equal(h.requests[1].body.nightBrightness,10);equal(h.requests[1].body.brightness,40);
    await h.respond(1,{brightness:40,nightBrightness:10,brightnessPending:true});await h.tick();
    equal(h.context.currentSettings.nightBrightness,10);equal(h.context.currentSettings.brightness,40);equal(h.maxPending,1);
    h.context.latest.display={brightness:40,nightBrightness:10,brightnessPending:false};h.run('renderBrightnessStatus()');
    equal(h.el('nightBrightnessFeedback').textContent,'Mémorisée automatiquement.');
  }
  // A night-only edit cannot save unsaved schedule or location fields.
  {
    const h=harness();h.el('nightStart').value='23:30';h.el('placeInput').value='Draft';h.input(20,'nightBrightness');
    equal(Object.keys(h.requests[0].body).join(','),'nightBrightness');h.el('settingsDialog').close();
    await h.respond(0,{brightness:60,nightBrightness:20,brightnessPending:true});await h.tick();
    equal(h.context.currentSettings.nightBrightness,20);equal(h.context.currentSettings.nightStart,1320);equal(h.context.currentSettings.place,'Test');
  }
  // General save waits for night writes and retains the latest slider position.
  {
    const h=harness();h.input(25,'nightBrightness');h.input(30,'nightBrightness');
    const saving=h.run('saveSettings({preventDefault(){}})');equal(h.el('nightBrightness').disabled,true);
    await h.respond(0,{brightness:60,nightBrightness:25,brightnessPending:true});await h.tick();
    await h.respond(1,{brightness:60,nightBrightness:30,brightnessPending:true});await h.tick();
    equal(h.requests[2].url,'/api/settings');equal(h.requests[2].body.nightBrightness,30);
    await h.respond(2,{});await saving;equal(h.context.currentSettings.nightBrightness,30);
  }
  // Failure belongs to the slider that failed; using the other one cannot hide it.
  {
    const h=harness();h.input(25,'nightBrightness');await h.reject(0);await h.tick();
    check(h.el('nightBrightnessFeedback').textContent.includes('non confirmée'));
    h.input(50);await h.respond(1,{brightness:50,nightBrightness:15,brightnessPending:false});await h.tick();
    check(h.el('nightBrightnessFeedback').textContent.includes('non confirmée'));equal(h.context.currentSettings.nightBrightness,15);
    h.input(20,'nightBrightness');await h.respond(2,{brightness:50,nightBrightness:20,brightnessPending:false});await h.tick();
    equal(h.el('nightBrightnessFeedback').textContent,'Mémorisée automatiquement.');
  }
  for(const disabled of ['off','sleep','offline','update']){
    const h=harness();if(disabled==='off')h.el('nightEnabled').checked=false;
    if(disabled==='sleep')h.el('nightAction').value='sleep';if(disabled==='offline')h.context.serverOnline=false;if(disabled==='update')h.context.uploadBusy=true;
    h.run('nightControls()');equal(h.el('nightBrightness').disabled,true);h.input(40,'nightBrightness');equal(h.requests.length,0);
  }
  console.log(`Live brightness UI: ${assertions} assertions passed`);
})().catch(error=>{console.error(error);process.exitCode=1;});
