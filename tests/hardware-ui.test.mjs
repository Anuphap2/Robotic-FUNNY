import {readFileSync} from 'node:fs';
import {runInNewContext} from 'node:vm';
import {test} from 'node:test';
import assert from 'node:assert/strict';
const source=readFileSync(new URL('../WebUI.h',import.meta.url),'utf8');
const hardwareSource=source.slice(source.indexOf('function hardwareFacts('),source.indexOf('function updateTelemetry('));
const facts=runInNewContext(hardwareSource+'; hardwareFacts',{config:{values:{motor_enabled:1,servo_enabled:1}}});
const asMap=d=>Object.fromEntries(facts(d));
test('live disabled settings override a cached enabled config',()=>{
  const rows=asMap({motorEnabled:false,servoEnabled:false,motorReady:true,headServoReady:true,leftServoReady:true,rightServoReady:true});
  for(const key of ['มอเตอร์','เซอร์โวหัว','แขนซ้าย','แขนขวา'])assert.equal(rows[key],'ปิดใน config');
});
test('a failed servo is reported individually while other channels stay ready',()=>{
  const rows=asMap({motorEnabled:true,servoEnabled:true,motorReady:true,headServoReady:true,leftServoReady:false,rightServoReady:true});
  assert.equal(rows['เซอร์โวหัว'],'สัญญาณ 50 Hz พร้อม');assert.equal(rows['แขนซ้าย'],'ตั้งสัญญาณไม่สำเร็จ');assert.equal(rows['แขนขวา'],'สัญญาณ 50 Hz พร้อม');
});
test('watchdog failure, panic and brownout are distinct diagnostic states',()=>{
  assert.match(asMap({motorReady:false,motorInitError:'ESP_ERR_NO_MEM'})['มอเตอร์'],/ESP_ERR_NO_MEM/);
  assert.match(asMap({resetReason:4})['สาเหตุเริ่มระบบล่าสุด'],/panic/);
  assert.match(asMap({resetReason:9})['สาเหตุเริ่มระบบล่าสุด'],/brownout/);
});
test('old telemetry never invents individual servo readiness',()=>{
  assert.equal(asMap({servoReady:true})['เซอร์โวหัว'],'รอข้อมูลรายตัว');
});
test('GPIO readback separates low outputs, polarity and the direction-switch wait',()=>{
  const start=source.indexOf('function motorSignalFacts('),end=source.indexOf('function hardwareFacts(');
  const signal=runInNewContext(source.slice(start,end)+'; motorSignalFacts');
  const rows=mask=>Object.fromEntries(signal({motorGPIO:mask,motorPending:true,motorCommandAgeMs:12,motorLeaseMs:388}));
  assert.equal(rows(0)['GPIO ล้อซ้าย'],'LOW / LOW');
  assert.equal(rows(5)['GPIO ล้อซ้าย'],'HIGH / LOW');assert.equal(rows(5)['GPIO ล้อขวา'],'HIGH / LOW');
  assert.equal(rows(10)['GPIO ล้อซ้าย'],'LOW / HIGH');assert.equal(rows(10)['GPIO ล้อขวา'],'LOW / HIGH');
  assert.equal(rows(6)['GPIO ล้อซ้าย'],'LOW / HIGH');assert.equal(rows(6)['GPIO ล้อขวา'],'HIGH / LOW');
  assert.match(rows(0)['เปลี่ยนทิศ'],/เว้นช่วง/);assert.equal(rows(0)['อายุคำสั่งล่าสุด'],'12 ms');
  assert.equal(Object.fromEntries(signal({}))['สัญญาณมอเตอร์'],'รอข้อมูลจากบอร์ด');
});
test('single-axis test sends only the selected saved-config angle and blocks dirty drafts',()=>{
  const start=source.indexOf('function testServoAxis('),end=source.indexOf("document.querySelectorAll('[data-servo-axis]')",start);
  let dirty=false;const calls=[],toasts=[];
  const run=runInNewContext(source.slice(start,end)+'; testServoAxis',{
    dirtyKeys:()=>dirty?['head_trim']:[],toast:(...args)=>toasts.push(args),
    $:id=>({value:{testHead:'100',testLeft:'40',testRight:'140'}[id]}),act:(...args)=>calls.push(args)
  });
  run('left');assert.equal(calls[0][0],'servo');assert.equal(calls[0][1],'axis');
  assert.equal(calls[0][2].axis,'left');assert.equal(calls[0][2].angle,'40');
  dirty=true;run('head');assert.equal(calls.length,1);assert.equal(toasts.length,1);
});
