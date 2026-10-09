import fs from 'node:fs/promises';
import path from 'node:path';
import {pathToFileURL} from 'node:url';

const workspace = 'C:/Users/KPG_G/OneDrive/Documents/Arduino/sketch_oct2a';
const skill = 'C:/Users/KPG_G/.codex/plugins/cache/openai-primary-runtime/presentations/26.1007.11041/skills/presentations';
const runtime = 'C:/Users/KPG_G/.cache/codex-runtimes/codex-primary-runtime/dependencies';
process.env.RUNTIME_NODE_MODULES = `${runtime}/node/node_modules`;
const {importRuntimeModule} = await import(pathToFileURL(`${skill}/container_tools/runtime_helpers.mjs`));
const {Presentation, PresentationFile, FileBlob} = await importRuntimeModule('@oai/artifact-tool');
const {finalizePresentation} = await import(pathToFileURL(`${skill}/container_tools/artifact_tool_utils.mjs`));
const pres = Presentation.create({slideSize:{width:1280,height:720}});
const build = path.join(workspace,'.presentation-build');
const output = path.join(workspace,'output/presentations');
const FONT = 'Tahoma';
const C = {dark:'#07172C',ink:'#11283D',cyan:'#16A3B5',bright:'#69E5F4',white:'#F6F9FC',gray:'#546B7F',light:'#DFE9F0'};
let n = 0;

function text(s, str, x,y,w,h,size=28,color=C.ink,bold=false,align='left',name='body'){
  const shape=s.shapes.add({geometry:'textbox',name,position:{left:x,top:y,width:w,height:h},fill:'none',line:{fill:'none',width:0}});
  shape.text=str;
  shape.text.style={typeface:FONT,fontSize:size,bold,color,alignment:align,verticalAlignment:'top',autoFit:'none',wrap:'square',insets:0};
  return shape;
}
function slide(title,dark=false,notes=''){
  const s=pres.slides.add(); n++;
  s.background.fill=dark?C.dark:C.white;
  if(title) text(s,title,64,48,1152,88,44,dark?C.white:C.ink,true,'left','slide-title');
  text(s,String(n).padStart(2,'0'),1170,670,46,30,18,dark?'#8DAFC4':C.gray,false,'right','page-number');
  s.speakerNotes.text=notes;
  return s;
}
function row(s,label,body,y,{dark=false,x=64,w=1152,labelW=330,size=28}={}){
  text(s,label,x,y,labelW,62,size,dark?C.bright:C.cyan,true);
  text(s,body,x+labelW+24,y,w-labelW-24,74,size,dark?C.white:C.ink);
}
function notes(script,sources){return `${script}\n\nแหล่งข้อมูล: ${sources.map(p=>path.join(workspace,p)).join('\n')}`;}

// 01 Cover. Generated illustration is explicitly identified as a concept.
{
 const s=slide('',true,notes('แนะนำโครงงานหุ่นยนต์สันทนาการที่ใช้ ESP32 เป็นตัวควบคุม หุ่นยนต์รวมการเล่นเพลง การแสดงสีหน้า การขยับแขน และการเดินหลบสิ่งกีดขวาง ภาพบนปกเป็นภาพแนวคิดสร้างด้วย ImageGen ไม่ใช่ภาพหุ่นยนต์จริงของโครงงาน',['README.md']));
 const bytes=await fs.readFile(path.join(build,'assets/robot-concept.png'));
 s.images.add({blob:bytes.buffer.slice(bytes.byteOffset,bytes.byteOffset+bytes.byteLength),contentType:'image/png',alt:'ภาพแนวคิดหุ่นยนต์ล้อสองข้าง ไม่ใช่ภาพหุ่นยนต์จริง',fit:'cover',position:{left:0,top:0,width:1280,height:720}});
 text(s,'หุ่นยนต์สันทนาการ\nอัจฉริยะ',64,158,645,190,60,C.white,true,'left','deck-title');
 text(s,'Smart Recreational Robot',66,360,590,52,30,C.bright);
 text(s,'ควบคุมผ่านเว็บ เล่นเพลง เต้น\nและเดินหลบสิ่งกีดขวาง',66,446,590,100,30,C.white);
 text(s,'ภาพแนวคิด',1050,645,168,30,19,'#C1D5E2',false,'right');
}
// 02 Objective. A large topic anchor and two paragraphs form one flat composition.
{
 const s=slide('แนวคิดและวัตถุประสงค์',true,notes('โครงงานนี้มุ่งใช้หุ่นยนต์ในกิจกรรมสันทนาการและการเรียนรู้ ผู้ใช้ควบคุมจากมือถือผ่าน Wi-Fi ได้ และเลือกการตอบสนองจากเสียงหรือการเดินอิสระ คำว่าอัจฉริยะในโครงงานหมายถึงการอ่านเซนเซอร์และทำงานตามกฎของโปรแกรม ไม่ได้ระบุการใช้โมเดล AI',['README.md','RecreationEngine.h']));
 text(s,'กิจกรรม\nสันทนาการ',64,206,460,210,64,C.bright,true);
 text(s,'เพิ่มการมีส่วนร่วมด้วยเพลง\nสีหน้า และท่าทางของหุ่นยนต์',596,212,620,120,32,C.white);
 text(s,'เรียนรู้ระบบฝังตัวจากการควบคุมผ่านเว็บ\nและการตอบสนองต่อเซนเซอร์',596,374,620,120,30,C.white);
 text(s,'ตอบสนองด้วยเซนเซอร์และกฎในโปรแกรม',64,585,1090,50,26,'#AEC5D8');
}
// 03 Full native inventory table.
{
 const s=slide('อุปกรณ์หลักของหุ่นยนต์',false,notes('รายการหลักมีเก้าประเภท เซอร์โวสามตัวแยกเป็นหัวหนึ่งตัวและแขนสองตัว มอเตอร์ TT และโมดูล Step-down มาจากรายการอุปกรณ์ที่สรุปไว้ในบทสนทนาเดิม รุ่นเซอร์โวและรายละเอียดแบตเตอรี่ยังไม่ได้ระบุ กระแส 3A เป็นพิกัดโมดูลที่ระบุ ไม่ใช่ผลวัดการใช้กระแสของหุ่นยนต์',['README.md','Config.h']));
 const values=[['อุปกรณ์','จำนวน','หน้าที่'],['ESP32','1 บอร์ด','ประมวลผลและเชื่อมต่อ Wi-Fi'],['Ultrasonic HC-SR05','1 ตัว','วัดระยะและสแกนซ้าย–ขวา'],['เซอร์โวมอเตอร์','3 ตัว','หมุนหัว 1 ตัว และขยับแขน 2 ตัว'],['มอเตอร์เกียร์ DC แบบ TT 1:48','2 ตัว','ขับเคลื่อนล้อซ้ายและขวา'],['ไดรเวอร์มอเตอร์ L9110S','1 โมดูล','ควบคุมทิศทางการหมุนมอเตอร์'],['OLED SH1106 128×64','1 จอ','แสดงสีหน้าและสถานะ'],['เซนเซอร์เสียง KY-037','1 โมดูล','ตรวจจับเสียงและจังหวะ'],['Buzzer','1 ตัว','เล่นเพลงและเสียงเอฟเฟกต์'],['Step-down 5V 3A','1 โมดูล','ปรับแรงดันไฟเลี้ยง']];
 const t=s.tables.add({rows:10,columns:3,left:64,top:160,width:1152,height:480,columnWidths:[455,155,542],values});
 t.styleOptions={headerRow:false,bandedRows:false};
 t.borders.assign({fill:C.light,width:1,style:'solid'});
 for(let r=0;r<10;r++){
   t.rows[r].height=48;
   for(let c=0;c<3;c++){
     const cell=t.getCell(r,c); cell.fill=r===0?C.ink:(r%2?'#F6F9FC':'#ECF2F7');
     cell.text.style={typeface:FONT,fontSize:24,bold:r===0,color:r===0?C.white:C.ink,verticalAlignment:'middle',insets:{top:7,bottom:7,left:13,right:10},autoFit:'none'};
   }
 }
}
// 04 Sensors and processing.
{
 const s=slide('สมองและเซนเซอร์',false,notes('ESP32 รับข้อมูลเซนเซอร์และประสานการทำงานของอุปกรณ์ HC-SR05 ติดกับชุดที่หัวหมุนเพื่อวัดหลายทิศทาง KY-037 ตรวจระดับเสียงและใช้ตรวจจังหวะหรือเสียงปรบมือ ไม่ใช่ระบบรู้จำคำพูด ระบบปัจจุบันใช้การเดินหลบสิ่งกีดขวางและไม่มีโหมดตามคน',['Config.h','SensorManager.h','SensorManager.cpp','RecreationEngine.h']));
 row(s,'ESP32','รับข้อมูลเซนเซอร์ ควบคุมอุปกรณ์\nและให้บริการหน้าเว็บผ่าน Wi-Fi',190,{labelW:360});
 row(s,'HC-SR05','วัดระยะด้านหน้า และวัดซ้าย–ขวา\nเมื่อเซอร์โวหมุนหัวไปยังแต่ละด้าน',332,{labelW:360});
 row(s,'KY-037','ตรวจจับระดับเสียงและจังหวะปรบมือ\nเพื่อใช้ในโหมดสันทนาการ',474,{labelW:360});
}
// 05 Movement and expression.
{
 const s=slide('การเคลื่อนไหวและการแสดงออก',false,notes('มอเตอร์สองตัวขับล้อด้วย L9110S การสั่งล้อแต่ละข้างทำให้เดินหน้า ถอย และเลี้ยว เซอร์โวหัวหมุนเพื่อตรวจระยะด้านข้าง ส่วนแขนสองข้างขยับตามท่าที่ตั้งไว้ จอ OLED ใช้แสดงสีหน้าและบัซเซอร์สร้างเสียงตามโน้ตเพลง',['MotorController.h','MotionSystem.h','FaceSystem.h','SongSystem.h']));
 row(s,'มอเตอร์ 2 ตัว + L9110S','เดินหน้า ถอยหลัง และเลี้ยว',180,{labelW:445,size:27});
 row(s,'เซอร์โวหัว 1 ตัว','หันหัวเพื่อสแกนซ้ายและขวา',275,{labelW:445,size:27});
 row(s,'เซอร์โวแขน 2 ตัว','ขยับแขนตามท่าทางและเพลง',370,{labelW:445,size:27});
 row(s,'OLED SH1106','แสดงสีหน้าและสถานะการทำงาน',465,{labelW:445,size:27});
 row(s,'Buzzer','เล่นโน้ตเพลงและเสียงเอฟเฟกต์',560,{labelW:445,size:27});
}
// 06 Power and construction.
{
 const s=slide('โครงสร้างและไฟเลี้ยง',false,notes('รายการอุปกรณ์เดิมระบุ Step-down ตั้งจ่ายไฟ 5V พิกัด 3A ส่วนประกอบโครงสร้างและแบตเตอรี่ต้องเติมตามชิ้นงานจริง ยังไม่ระบุรุ่นเซอร์โว ชนิดหรือความจุของแบตเตอรี่ รวมถึงขนาดฐาน ข้อมูลบนหน้านี้จึงไม่ใช่แผนผังการต่อวงจรหรือคำยืนยันว่ากำลังจ่ายเพียงพอ',['README.md','Config.h']));
 text(s,'5V',64,190,390,115,92,C.cyan,true);
 text(s,'Step-down พิกัด 3A\nปรับแรงดันสำหรับไฟเลี้ยง',64,330,400,112,29,C.ink);
 text(s,'ส่วนประกอบเพิ่มเติม',555,186,655,56,32,C.ink,true);
 text(s,'แหล่งจ่ายไฟหรือแบตเตอรี่\nฐานหุ่นยนต์และชุดล้อ\nสายไฟ ขั้วต่อ และอุปกรณ์ยึด',555,264,655,166,29,C.ink);
 text(s,'ข้อมูลที่ยังไม่ได้ระบุ',555,486,655,54,28,C.cyan,true);
 text(s,'รุ่นเซอร์โว ชนิดและความจุแบตเตอรี่\nรวมถึงรายละเอียดฐานหุ่น',555,544,655,86,26,C.gray);
}
// 07 Four operating modes as flat rows.
{
 const s=slide('โหมดการทำงาน',true,notes('Manual คือการควบคุมตรงจากหน้าเว็บ Party ตอบสนองต่อระดับเสียงและจังหวะ Interactive คือการเดินอิสระพร้อมหลบสิ่งกีดขวาง Showcase ใช้เพลงประกอบท่าทางและสีหน้า โหมดเดินอิสระต้องมีอุปกรณ์ที่จำเป็นพร้อมใช้งานก่อนเริ่ม',['RecreationEngine.h','WebUI.h']));
 row(s,'Manual','ควบคุมหุ่นยนต์โดยตรงจากเว็บ',190,{dark:true,labelW:325,size:30});
 row(s,'Party','ขยับท่าทางตามเสียงและจังหวะ',305,{dark:true,labelW:325,size:30});
 row(s,'Interactive','เดินอิสระและหลบสิ่งกีดขวาง',420,{dark:true,labelW:325,size:30});
 row(s,'Showcase','เล่นเพลงพร้อมท่าทางและสีหน้า',535,{dark:true,labelW:325,size:30});
}
// 08 Navigation sequence is editable numbered text, not decorative drawings.
{
 const s=slide('ลำดับการหลบสิ่งกีดขวาง',true,notes('ระบบวัดด้านหน้าก่อน หากพบสิ่งกีดขวางใกล้จะหยุดและถอย จากนั้นหยุดล้อระหว่างหมุนหัววัดด้านซ้ายและขวา คืนหัวกลางแล้วเลี้ยวไปด้านที่เปิดกว่า หลังเลี้ยวต้องรับการวัดด้านหน้าใหม่ก่อนเดินต่อ หากทั้งสองด้านตันจะใช้ลำดับถอยและเลี้ยวตามค่าที่กำหนด ระยะเวลาถอยและเลี้ยวเป็นเวลาสั่งงาน ไม่ใช่ระยะทางหรือมุมที่วัดได้ กรณีไม่มี echo ไม่ได้พิสูจน์ว่าพื้นที่โล่ง เนื่องจากอาจเกิดจากเซนเซอร์ขาดการเชื่อมต่อได้ และไม่มีเซนเซอร์ตรวจพื้นต่างระดับหรือด้านหลัง',['ObstacleAvoidance.h','SonarReading.h','RecreationEngine.h','tests/VERIFICATION.md']));
 const steps=['วัดระยะด้านหน้า','หยุดและถอยเมื่อใกล้สิ่งกีดขวาง','หยุดล้อ แล้วหันหัววัดซ้าย–ขวา','คืนหัวกลาง แล้วเลี้ยวไปด้านที่โล่งกว่า','วัดด้านหน้าใหม่ก่อนเดินต่อ'];
 steps.forEach((v,i)=>{text(s,String(i+1).padStart(2,'0'),64,168+i*77,90,54,33,C.bright,true);text(s,v,186,168+i*77,1022,58,29,C.white);});
 text(s,'ไม่มีสัญญาณสะท้อนกลับไม่ได้ยืนยันว่าพื้นที่โล่ง',64,604,1145,50,26,'#FFD391');
}
// 09 How users access and control the real dashboard.
{
 const s=slide('การควบคุมผ่านเว็บ',false,notes('ผู้ใช้เชื่อมเครือข่าย Wi-Fi ที่กำหนด หรือ Access Point Robot-Recreation จากนั้นเปิด IP ที่ OLED แสดง หน้าเว็บทำงานในเครือข่ายได้โดยไม่ต้องใช้อินเทอร์เน็ต มีแท็บควบคุม ตั้งค่า Hardware และสถานะระบบ การปรับพารามิเตอร์ไม่ใช่การเปลี่ยน GPIO ผ่านหน้าเว็บ สถานะพร้อมใช้งานต้องอ่านจากหุ่นยนต์จริง',['README.md','WebUI.h','WebApi.h','RuntimeConfig.h']));
 text(s,'เชื่อม Wi-Fi\nแล้วเปิด IP บน OLED',64,200,485,160,46,C.cyan,true);
 text(s,'เข้าผ่านเครือข่ายเดิม\nหรือ AP ชื่อ Robot-Recreation',64,410,490,110,27,C.gray);
 row(s,'ควบคุม','เลือกโหมด เพลง สีหน้า\nท่าทาง และสั่งหยุด',188,{x:635,w:570,labelW:158,size:26});
 row(s,'ตั้งค่า Hardware','ปรับค่าการทำงาน\nและบันทึกการตั้งค่า',340,{x:635,w:570,labelW:205,size:25});
 row(s,'สถานะระบบ','ดูสถานะอุปกรณ์\nและข้อมูลการทำงาน',492,{x:635,w:570,labelW:175,size:26});
}
// 10 Honest current status plus demonstration sequence.
{
 const s=slide('สถานะโครงงานและการสาธิต',true,notes('รายงานการตรวจวันที่ 8 ตุลาคม 2026 ระบุว่า firmware 2.3.5 คอมไพล์สำหรับ ESP32 ผ่าน และการทดสอบ JavaScript ของหน้าเว็บผ่าน 28 รายการ ตัวเลขนี้ไม่ใช่ผลทดลองความสำเร็จในการหลบสิ่งกีดขวางจริง ในรายงานรอบนี้ไม่มีการอัปโหลด firmware และยังไม่ได้ขับหุ่นยนต์จริง ก่อนสาธิตต้องตรวจการทำงานของอุปกรณ์ และทดลองการควบคุม เพลง ท่าทาง และหลบสิ่งกีดขวางในพื้นที่ควบคุม',['tests/VERIFICATION.md','README.md']));
 text(s,'Firmware 2.3.5',64,190,520,65,36,C.bright,true);
 text(s,'คอมไพล์ ESP32 ผ่าน\nทดสอบซอฟต์แวร์หน้าเว็บผ่าน 28 รายการ',64,278,550,120,27,C.white);
 text(s,'ลำดับการสาธิต',720,190,480,62,34,C.white,true);
 text(s,'1  ควบคุมผ่านเว็บ\n2  แสดงเพลงและท่าทาง\n3  ทดลองหลบสิ่งกีดขวาง',720,280,480,174,28,C.white);
 text(s,'ยังต้องทดสอบการเคลื่อนที่และการหลบสิ่งกีดขวางบนหุ่นยนต์จริง',64,515,1136,92,28,'#FFD391');
 text(s,'ถาม–ตอบ',64,615,650,58,36,C.bright,true);
}

await fs.mkdir(output,{recursive:true});
await fs.mkdir(path.join(build,'previews'),{recursive:true});
const candidate = path.join(build,'candidate.pptx');
await (await PresentationFile.exportPptx(pres)).save(candidate);
console.log('Draft exported');
const finalPath=path.join(output,'smart-recreational-robot-th.pptx');
await finalizePresentation({workspaceDir:workspace,candidatePath:candidate,finalPath,pythonExecutable:`${runtime}/python/python.exe`,integrityValidatorPath:`${skill}/container_tools/inspect_presentation_package_integrity.py`,layoutValidatorPath:`${skill}/container_tools/inspect_presentation_layout_geometry.py`,layoutArgs:['--expected-slide-size-emu','12192000,6858000','--validate-bullet-geometry','--validate-heading-fit','--require-native-table-slide','3'],requiredNativeTableOwnerSlides:[3],fontPolicy:{basis:'design',families:[FONT]},verifyArtifactToolImport:true,receiptPath:path.join(build,'validation.json')});
console.log('Finalized',finalPath);
const finalDeck=await PresentationFile.importPptx(await FileBlob.load(finalPath));
for(const [i,s] of finalDeck.slides.items.entries()){
  const png=await finalDeck.export({slide:s,format:'png',scale:1});
  await fs.writeFile(path.join(build,'previews',`slide-${String(i+1).padStart(2,'0')}.png`),new Uint8Array(await png.arrayBuffer()));
  console.log('Rendered',i+1);
}
console.log('COMPLETE');
