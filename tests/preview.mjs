// Local browser fixture only. It does not connect to or control real hardware.
// Run: node tests/preview.mjs
import { readFileSync } from 'node:fs';
import { createServer } from 'node:http';
import { fileURLToPath } from 'node:url';
import { dirname, resolve } from 'node:path';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const read = name => readFileSync(resolve(root, name), 'utf8');
const html = read('WebUI.h').split('R"rawliteral(')[1].split(')rawliteral"')[0];
const schema = [...read('RuntimeConfig.h').matchAll(/X\(\w+,"([^"]+)","([^"]+)","([^"]+)",(-?\d+),(-?\d+),(-?\d+),"([^"]+)","([^"]+)"\)/g)]
  .map(([,key,label,group,min,max,def,unit,hint]) => ({key,label,group,min:+min,max:+max,default:+def,unit,hint}));
if (schema.length !== 51 || new Set(schema.map(s=>s.key)).size !== 51) throw Error('Invalid schema fixture');
const settings = {revision:0,version:'2.3.0',storage:'defaults',schemaVersion:2,
  values:Object.fromEntries(schema.map(s=>[s.key,s.default])), schema,
  pins:{'OLED SDA':21,'OLED SCL':22,'Ultrasonic TRIG':5,'Ultrasonic ECHO':17,'Buzzer':25,'Microphone AO':34,'Head servo':18,'Left arm servo':19,'Right arm servo':23,'Motor A1A':26,'Motor A1B':27,'Motor B1A':14,'Motor B1B':12},oledAddress:60};
const catalog = {
  songs:[...read('SongSystem.h').split('const SongItem SONG_REGISTRY[] = {')[1].split('};')[0].matchAll(/\{"([^"]+)",\s*"([^"]+)",\s*"([^"]+)",\s*"([^"]+)"/g)].map(([,id,title,cat,icon])=>({id,title,cat,icon})),
  faces:[...read('FaceSystem.h').split('const FaceItem FACE_REGISTRY[] = {')[1].split('};')[0].matchAll(/\{"([^"]+)",\s*"([^"]+)",\s*"([^"]+)"/g)].map(([,id,title,icon])=>({id,title,icon})),
  motions:[...read('MotionSystem.h').split('const MotionRoutine MOTION_REGISTRY[] = {')[1].split('};')[0].matchAll(/\{"([^"]+)",\s*"([^"]+)",\s*"([^"]+)"/g)].map(([,id,title,icon])=>({id,title,icon}))
};
let motor='stop',mode='manual',playing=false,timbre='music_box',automaticStarted=0;
// Timed UI scenarios only: no production classifier or motor is simulated here.
function automaticView(){
  const idle={interactive:'auto',behavior:'observe',targetClass:'no_data',decision:'inactive',targetLocked:false,movingVotes:0,dist:100,distanceValid:true,dir:motor};
  if(mode!=='interactive')return idle;
  const stages=[
    {behavior:'observe',targetClass:'collecting',decision:'collecting'},
    {behavior:'roam',targetClass:'clear',decision:'roam_cruise',dist:220,dir:'forward'},
    {behavior:'observe',targetClass:'moving_target',decision:'checking_movement',dist:100,movingVotes:1},
    {behavior:'follow',targetClass:'moving_target',decision:'follow_cruise',dist:100,targetLocked:true,movingVotes:2,dir:'forward'},
    {behavior:'follow',targetClass:'static_object',decision:'follow_hold',dist:70,targetLocked:true},
    {behavior:'greet',targetClass:'static_object',decision:'greeting',dist:40,targetLocked:true},
    {behavior:'stop',targetClass:'clear',decision:'target_lost',dist:250},
    {behavior:'observe',targetClass:'collecting',decision:'collecting'},
    {behavior:'avoid',targetClass:'static_object',decision:'scan_turn',dist:50,dir:'left'},
    {behavior:'avoid',targetClass:'static_object',decision:'avoid_retry',dist:50},
    {behavior:'roam',targetClass:'clear',decision:'roam_cruise',dist:220,dir:'forward'},
    {behavior:'stop',targetClass:'no_data',decision:'no_echo_stop',dist:-1,distanceValid:false}
  ];
  return {...idle,...stages[Math.floor((Date.now()-automaticStarted)/1800)%stages.length]};
}
const actions=[];
createServer(async (req,res)=>{
  const url=new URL(req.url,'http://127.0.0.1');
  const send=(status,data)=>{res.writeHead(status,{'Content-Type':'application/json','Cache-Control':'no-store'});res.end(JSON.stringify(data));};
  if(url.pathname==='/'){res.writeHead(200,{'Content-Type':'text/html; charset=utf-8'});res.end(html);return;}
  if(url.pathname==='/responsive-preview'){
    const width=Math.max(320,Math.min(1400,Number(url.searchParams.get('width'))||375));
    res.writeHead(200,{'Content-Type':'text/html; charset=utf-8'});
    res.end(`<html><body style="margin:0;background:#253149"><iframe title="Robot preview ${width}px" src="/" style="width:${width}px;height:100vh;border:0;display:block;margin:auto"></iframe></body></html>`);return;
  }
  if(url.pathname==='/telemetry')return send(200,{vol:42,peak:118,distanceAgeMs:22,baseline:2048,calibrating:false,...automaticView(),mode,face:'normal',timbre,isPlaying:playing,heapFree:178240,heapMin:161300,heapLargest:145800,uptimeMs:1234567,maxLoopUs:13300,oledReady:true,servoReady:true,audioReady:true,motorReady:true,rssi:-55,networkMode:'station',ip:'192.168.1.42',version:'2.3.0',revision:settings.revision,pollMs:settings.values.telemetry_interval_ms});
  if(url.pathname==='/api/catalog')return send(200,catalog);
  if(url.pathname==='/api/config'&&req.method==='GET')return send(200,settings);
  if(req.method==='POST'){
    let body='';for await(const chunk of req)body+=chunk;
    if(req.headers['x-robot-request']!=='1')return send(403,{error:'robot_header_required'});
    const args=new URLSearchParams(body);
    if(url.pathname==='/api/config'){
      if(Number(args.get('revision'))!==settings.revision)return send(409,{error:'config_changed_reload'});
      // This is a UI fixture, not a replacement for the firmware validator.
      const next={...settings.values};for(const s of schema){if(args.has(s.key))next[s.key]=Number(args.get(s.key));}
      if(schema.some(s=>!Number.isInteger(next[s.key])||next[s.key]<s.min||next[s.key]>s.max))return send(422,{error:'invalid_config'});
      const changed=JSON.stringify(next)!==JSON.stringify(settings.values);
      if(changed){settings.values=next;settings.revision++;settings.storage='saved';mode='manual';motor='stop';playing=false;}
      return send(200,{ok:true,changed,revision:settings.revision});
    }
    if(url.pathname==='/action'){
      const action=Object.fromEntries(args);actions.push(action);
      if(action.type==='motor'){mode='manual';motor=action.cmd;}
      if(action.type==='mode'){mode=action.cmd;motor='stop';if(mode==='interactive')automaticStarted=Date.now();}
      if(action.type==='interactive'){motor='stop';if(action.cmd==='stop'){mode='manual';playing=false;}else if(['start','greet','roam','follow'].includes(action.cmd)){mode='interactive';automaticStarted=Date.now();}else return send(422,{error:'unknown_command_or_hardware_disabled'});}
      if(action.type==='song')playing=action.cmd!=='stop';
      if(action.type==='showcase'){mode='showcase';playing=true;}
      if(action.type==='timbre')timbre=action.cmd;
      if(action.type==='system'&&action.cmd==='stop_all'){motor='stop';mode='manual';playing=false;}
      return send(200,{ok:true});
    }
  }
  if(url.pathname==='/fixture/actions')return send(200,actions);
  send(404,{error:'not_found'});
}).listen(8765,'127.0.0.1',()=>console.log('Robot browser fixture: http://127.0.0.1:8765 (no real hardware)'));
