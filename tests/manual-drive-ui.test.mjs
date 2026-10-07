import {readFileSync} from 'node:fs';
import {runInNewContext} from 'node:vm';
import {test} from 'node:test';
import assert from 'node:assert/strict';
const source=readFileSync(new URL('../WebUI.h',import.meta.url),'utf8');
const driveSource=source.slice(source.indexOf('let drive=null,'),source.indexOf("window.addEventListener('blur'"));
const flush=async()=>{for(let i=0;i<8;i++)await Promise.resolve();};
function dashboard(){
  let now=0;const calls=[],timers=new Map(),toasts=[];
  const buttons=Object.fromEntries(['forward','backward','left','right','stop'].map(dir=>[dir,{
    dataset:{dir},disabled:false,handlers:{},classList:{add(){},remove(){}},setPointerCapture(){},
    addEventListener(name,callback){this.handlers[name]=callback;}
  }]));
  const context={AbortController,URLSearchParams,Uint32Array,
    crypto:{getRandomValues:a=>{a[0]=123;return a;}},performance:{now:()=>now},
    document:{querySelectorAll:()=>Object.values(buttons)},toast:(...args)=>toasts.push(args),
    setTimeout:(callback,delay)=>{const id=Symbol();timers.set(id,{callback,delay});return id;},clearTimeout:id=>timers.delete(id),
    request:(path,options)=>new Promise((resolve,reject)=>calls.push({path,options,resolve,reject})),
    fetch:(path,options)=>{calls.push({path,options,keepalive:true});return Promise.resolve();}
  };
  const api=runInNewContext(driveSource+`;({startDrive,releaseDrive,handlePageExit,syncDriveClock,
    state:()=>({drive,driveBusy,driveEpoch,driveFlight,driveClock}),
    setTelemetry:controller=>telemetryController=controller})`,context);
  api.syncDriveClock({uptimeMs:10000,driveBootId:55},0);
  return {api,calls,timers,toasts,buttons,advance:ms=>now+=ms,
    fire:()=>{const [id,timer]=timers.entries().next().value;timers.delete(id);timer.callback();return timer.delay;},
    down:dir=>buttons[dir].handlers.pointerdown({button:0,pointerId:1,preventDefault(){}}),
    up:dir=>buttons[dir].handlers.pointerup({})};
}
test('stop works during an outstanding forward request, aborts it and sends a higher sequence',async()=>{
  const d=dashboard();d.down('forward');await flush();assert.equal(d.calls.length,1);
  d.down('stop');assert.equal(d.calls.length,2);assert.equal(d.api.state().drive,null);
  assert.equal(d.calls[0].options.signal.aborted,true);
  assert.equal(d.calls[1].options.body.get('cmd'),'stop');
  assert.ok(+d.calls[1].options.body.get('seq')>+d.calls[0].options.body.get('seq'));
  d.calls[0].resolve({});await flush();assert.equal(d.timers.size,0);assert.equal(d.calls.length,2);
});
test('heartbeat cadence includes network time and never queues parallel forward requests',async()=>{
  const d=dashboard();d.down('forward');await flush();d.advance(60);d.calls[0].resolve({});await flush();
  assert.equal([...d.timers.values()][0].delay,40);d.advance(40);d.fire();await flush();
  assert.equal(d.calls.length,2);assert.equal(d.calls[1].options.body.get('issued'),'10100');
  assert.equal(d.calls[1].options.timeoutMs,350);assert.equal(d.timers.size,0);
});
test('direction change waits for stop acknowledgement and old completions cannot stop the new drive',async()=>{
  const d=dashboard();d.down('forward');await flush();d.down('backward');await flush();
  assert.deepEqual(d.calls.map(c=>c.options.body.get('cmd')),['forward','stop']);
  d.advance(20);d.calls[1].resolve({});await flush();assert.equal(d.fire(),10);await flush();
  assert.equal(d.calls[2].options.body.get('cmd'),'backward');assert.equal(d.api.state().driveBusy,true);
  d.calls[0].resolve({});await flush();assert.equal(d.calls.length,3);assert.equal(d.api.state().driveBusy,true);
  d.up('forward');assert.equal(d.api.state().drive,'backward');
  d.up('backward');assert.equal(d.api.state().drive,null);assert.equal(d.calls[3].options.body.get('cmd'),'stop');
});
test('a stalled heartbeat stops locally, sends stop, and never retries movement automatically',async()=>{
  const d=dashboard();d.down('forward');await flush();d.calls[0].reject(new Error('network stalled'));await flush();
  assert.equal(d.api.state().drive,null);assert.equal(d.calls[1].options.body.get('cmd'),'stop');
  assert.equal(d.timers.size,0);assert.equal(d.toasts.length,1);
});
test('page exit preserves idle autonomy and sends one sequenced keepalive stop for active drive',async()=>{
  const idle=dashboard();idle.api.handlePageExit();assert.equal(idle.calls.length,0);
  const d=dashboard();d.down('forward');await flush();d.api.handlePageExit();
  assert.equal(d.calls.length,2);assert.equal(d.calls[1].options.keepalive,true);
  assert.equal(d.calls[1].options.body.get('cmd'),'stop');assert.equal(d.calls[0].options.signal.aborted,true);
});
test('manual drive aborts telemetry and stop remains available when directional controls are disabled',async()=>{
  const d=dashboard();const telemetry=new AbortController();d.api.setTelemetry(telemetry);d.down('forward');await flush();
  assert.equal(telemetry.signal.aborted,true);d.buttons.stop.disabled=true;d.down('stop');
  assert.equal(d.calls[1].options.body.get('cmd'),'stop');
});
test('reboot releases the current drive and older responses cannot replace newer clock synchronization',async()=>{
  const d=dashboard();d.api.syncDriveClock({uptimeMs:10030,driveBootId:55},30);
  d.api.syncDriveClock({uptimeMs:10010,driveBootId:55},10);assert.equal(d.api.state().driveClock.uptime,10030);
  d.down('forward');await flush();d.api.syncDriveClock({uptimeMs:20,driveBootId:99},40);
  assert.equal(d.api.state().drive,null);assert.equal(d.api.state().driveClock.boot,99);
  d.api.syncDriveClock({uptimeMs:10090,driveBootId:55},35);
  assert.equal(d.api.state().driveClock.boot,99);
});
test('changing mode waits for a pending manual stop before starting autonomy',async()=>{
  let releaseStop;const stopped=new Promise(resolve=>releaseStop=resolve),commands=[];
  const actSource=source.slice(source.indexOf('async function act('),source.indexOf('function setOnline('));
  const act=runInNewContext(actSource+'; act',{
    releaseDrive:()=>stopped,command:(...args)=>{commands.push(args);return Promise.resolve();},toast(){}
  });
  const action=act('mode','interactive');await flush();assert.equal(commands.length,0);
  releaseStop();await action;assert.equal(commands[0][0],'mode');assert.equal(commands[0][1],'interactive');
});
test('actual request helper propagates release abort and removes its timeout and abort listener',async()=>{
  const requestSource=source.slice(source.indexOf('async function request('),source.indexOf('async function command('));
  const external=new AbortController();let fetchSignal,cleared=false,synchronized=false;
  const request=runInNewContext(requestSource+'; request',{
    AbortController,performance:{now:()=>1},syncDriveClock:()=>synchronized=true,messageFor:x=>x,
    setTimeout:()=>123,clearTimeout:id=>cleared=id===123,
    fetch:(path,options)=>new Promise((resolve,reject)=>{
      fetchSignal=options.signal;fetchSignal.addEventListener('abort',()=>reject(Object.assign(new Error(),{name:'AbortError'})),{once:true});
    })
  });
  const pending=request('/action',{signal:external.signal,timeoutMs:350});external.abort();
  await assert.rejects(pending,/ติดต่อหุ่นยนต์ไม่ได้/);
  assert.equal(fetchSignal.aborted,true);assert.equal(cleared,true);assert.equal(synchronized,false);
});
