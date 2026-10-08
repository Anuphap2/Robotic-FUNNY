// Exercise the actual dashboard validation code, not a duplicate validator.
// Run with Node 18+: node --test tests/config-ui.test.mjs
import { readFileSync } from 'node:fs';
import { resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { runInNewContext } from 'node:vm';
import { test } from 'node:test';
import assert from 'node:assert/strict';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const source = readFileSync(resolve(root,'WebUI.h'),'utf8');
const schema = [...readFileSync(resolve(root,'RuntimeConfig.h'),'utf8').matchAll(/X\(\w+,"([^"]+)","([^"]+)","([^"]+)",(-?\d+),(-?\d+),(-?\d+),"([^"]+)","([^"]+)"\)/g)]
  .map(([,key,label,group,min,max,def,unit,hint])=>({key,label,group,min:+min,max:+max,default:+def,unit,hint}));
const defaults = () => Object.fromEntries(schema.map(s=>[s.key,s.default]));
const validatorSource = source.slice(source.indexOf('function validateValues('),source.indexOf('function renderConfig('));
const validate = runInNewContext(validatorSource+'; validateValues',{config:{schema}});
test('default configuration and supported boundary combinations are accepted',()=>{
  assert.equal(schema.length,51); assert.equal(new Set(schema.map(s=>s.key)).size,51);
  validate(defaults());
  validate({...defaults(),head_min:20,head_max:150,head_center:60,head_trim:10,head_reverse:1});
  validate({...defaults(),motor_timeout_ms:300,audio_volume_pct:0,telemetry_interval_ms:5000,servo_enabled:0});
});
test('every field rejects values beyond both bounds, fractions and strings',()=>{
  for(const s of schema){
    for(const value of [s.min-1,s.max+1,s.default+.5,String(s.default),NaN,null]){
      assert.throws(()=>validate({...defaults(),[s.key]:value}),undefined,`${s.key}: ${value}`);
    }
  }
});
test('all three servo homes respect physical limits after reverse and trim',()=>{
  for(const axis of ['head','left','right']){
    assert.throws(()=>validate({...defaults(),[axis+'_min']:100,[axis+'_max']:80}));
    assert.throws(()=>validate({...defaults(),[axis+'_min']:20,[axis+'_max']:160,[axis+'_center']:0,[axis+'_trim']:-10}));
    assert.throws(()=>validate({...defaults(),[axis+'_min']:20,[axis+'_max']:160,[axis+'_center']:0,[axis+'_reverse']:1}));
  }
});
test('obsolete distances remain loadable without restricting obstacle settings',()=>{
  validate({...defaults(),proximity_close_cm:80,proximity_greeting_cm:45,auto_stop_cm:100,auto_clear_cm:30,follow_distance_cm:30,follow_max_cm:60});
  for(const key of ['follow_distance_cm','follow_max_cm','target_delta_cm','proximity_close_cm','auto_turn_ms'])
    assert.equal(schema.find(s=>s.key===key).group,'legacy');
});

test('imports cannot include GPIO keys, unknown settings or missing fields',()=>{
  assert.throws(()=>validate({...defaults(),PIN_SERVO_HEAD:2}));
  const missing=defaults();delete missing.audio_enabled;assert.throws(()=>validate(missing));
  const renamed=defaults();delete renamed.audio_enabled;renamed.not_a_setting=1;assert.throws(()=>validate(renamed));
});
test('blank numeric form values are not silently exported as zero',()=>{
  const inputs=Object.fromEntries(schema.map(s=>['cfg-'+s.key,{value:String(s.default),checked:!!s.default}]));
  inputs['cfg-head_trim'].value='';
  const formSource=source.slice(source.indexOf('function valuesFromForm('),source.indexOf('function dirtyKeys('));
  const read=runInNewContext(formSource+'; valuesFromForm',{config:{schema},$:id=>inputs[id]});
  assert.equal(Number.isNaN(read().head_trim),true);assert.throws(()=>validate(read()));
});
const importerSource=source.slice(source.indexOf('function importValues('),source.indexOf("$('importConfig').onchange="));
const importValues=runInNewContext(validatorSource+importerSource+'; importValues',{config:{schema}});
test('v1 imports preserve all 42 settings and add only new behavior defaults',()=>{
  const old=Object.fromEntries(schema.slice(0,42).map(s=>[s.key,s.default]));
  old.head_trim=10;old.audio_volume_pct=55;
  const result=importValues({schemaVersion:1,values:old});
  assert.equal(result.head_trim,10);assert.equal(result.audio_volume_pct,55);
  for(const s of schema.slice(42))assert.equal(result[s.key],s.default);
  assert.throws(()=>importValues({schemaVersion:1,values:{...old,PIN_ECHO:4}}));
  const missing={...old};delete missing.head_trim;
  assert.throws(()=>importValues({schemaVersion:1,values:missing}));
});
test('v2 import validates the complete draft and bounded request fits the server',()=>{
  const values=defaults();values.auto_pulse_ms=60;
  assert.equal(importValues({schemaVersion:2,values}).auto_pulse_ms,60);
  assert.throws(()=>importValues({schemaVersion:3,values}));
  assert.throws(()=>importValues({schemaVersion:2,values:{...values,auto_pulse_ms:301}}));
  const maximum=Object.fromEntries(schema.map(s=>[s.key,Math.abs(s.min)>s.max?s.min:s.max]));
  assert.ok(new URLSearchParams({revision:2147483647,...maximum}).toString().length<=2048);
});
test('Interactive starts with one mode command and has no behavior or target-confirmation controls',()=>{
  assert.ok(!source.includes('id="interactiveChoice"'));
  assert.ok(!source.includes('id="confirmTarget"'));
  const controls={'startInteractive':{},'stopInteractive':{}};
  const calls=[];
  const clickSource=source.slice(source.indexOf("$('startInteractive').onclick="),source.indexOf("$('startShow').onclick="));
  runInNewContext(clickSource,{$:id=>controls[id],act:(...args)=>calls.push(args)});
  controls.startInteractive.onclick();
  assert.equal(calls.length,1);assert.equal(calls[0][0],'mode');assert.equal(calls[0][1],'interactive');
  controls.stopInteractive.onclick();
  assert.equal(calls[1][0],'interactive');assert.equal(calls[1][1],'stop');
});
test('avoidance telemetry has no target acquisition or follow presentation',()=>{
  const controls={interactiveState:{}};let rows;
  const renderSource=source.slice(source.indexOf('const decisionLabels='),source.indexOf('function updateTelemetry(d)'));
  const render=runInNewContext(renderSource+'; updateInteractive',{
    $:id=>controls[id],facts:(id,values)=>rows=values
  });
  for(const behavior of ['roam','avoid','stop']){
    render({mode:'interactive',behavior,targetClass:'static_object',decision:'scan_left',targetLocked:true});
    assert.ok(controls.interactiveState.textContent.startsWith('อัตโนมัติ'));
    assert.equal(rows.length,4);assert.equal(rows[2][0],'สันทนาการ');
    assert.ok(!JSON.stringify(rows).includes('เป้าหมาย'));
  }
  render({mode:'manual'});assert.equal(controls.interactiveState.textContent,'ยังไม่เริ่ม');
});

test('telemetry distinguishes scanning from turning and explains no echo',()=>{
  let rows;
  const renderSource=source.slice(source.indexOf('const decisionLabels='),source.indexOf('function updateTelemetry(d)'));
  const render=runInNewContext(renderSource+'; updateInteractive',{
    $:()=>({}),facts:(id,values)=>rows=values
  });
  for(const [decision,expected] of [['roam_cruise','ต่อเนื่อง'],['no_echo_clear','เดินต่อ'],['measurement_wait','รอผลวัด'],['obstacle_brake','หยุด'],['scan_left','สแกนซ้าย'],['scan_right','สแกนขวา'],['avoid_left','เลี้ยวซ้าย'],['avoid_right','เลี้ยวขวา']]){
    render({mode:'interactive',behavior:'avoid',targetClass:'static_object',decision});
    assert.ok(rows[1][1].includes(expected),decision);
  }
});

test('legacy fields stay hidden even while searching, but remain in saved config',()=>{
  assert.ok(source.includes("details.hidden=group==='legacy'"));
  assert.ok(source.includes("g.hidden=g.dataset.group==='legacy'||"));
  assert.equal(schema.length,51);
});
