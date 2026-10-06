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
  assert.equal(schema.length,42); assert.equal(new Set(schema.map(s=>s.key)).size,42);
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
test('proximity zones cannot overlap or collapse',()=>{
  assert.throws(()=>validate({...defaults(),proximity_close_cm:45,proximity_greeting_cm:45}));
  assert.throws(()=>validate({...defaults(),proximity_close_cm:80,proximity_greeting_cm:45}));
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
