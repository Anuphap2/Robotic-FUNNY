#ifndef SONG_SYSTEM_H
#define SONG_SYSTEM_H

#include <Arduino.h>
#include "Config.h"

// ============================================================================
// MUSICAL PITCH FREQUENCIES (Hz) - Scientific Pitch Notation
// ============================================================================
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247

#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494

#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988

#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define REST     0

// ============================================================================
// MUSIC NOTATION MACROS (Standard Musical Note Durations in Beats)
// 1 Beat = Quarter Note (♩)
// ============================================================================
#define N_WHOLE      4.00f  // 1 ห้องเต็ม (ตัวกลม)
#define N_DOTTED_HALF 3.00f // ตัวขาวประจุด
#define N_HALF       2.00f  // 2 จังหวะ (ตัวขาว)
#define N_DOTTED_QUARTER 1.50f // ตัวดำประจุด
#define N_QUARTER    1.00f  // 1 จังหวะ (ตัวดำ)
#define N_DOTTED_EIGHTH  0.75f // เขบ็ตหนึ่งชั้นประจุด
#define N_EIGHTH     0.50f  // ครึ่งจังหวะ (เขบ็ต 1 ชั้น)
#define N_SIXTEENTH  0.25f  // เสี้ยวจังหวะ (เขบ็ต 2 ชั้น)

// โน้ตเพลง 1 ตัว: ระดับเสียง + ความยาวจังหวะ (Beats)
struct Note {
  uint16_t freq;   // ความถี่เสียง เช่น NOTE_C5, REST
  float beats;     // ความยาวจังหวะ เช่น N_QUARTER, N_EIGHTH
};

// ข้อมูลเพลง 1 เพลง
struct SongItem {
  const char* id;          // Machine ID สำหรับเรียกผ่าน Web API
  const char* title;       // ชื่อเพลงภาษาไทย
  const char* category;    // หมวดหมู่: "recreation", "pop", "game", "sfx"
  const char* icon;        // ไอคอนแสดงบนแดชบอร์ด
  uint16_t bpm;            // ความเร็วเพลง (Beats Per Minute) ปรับช้า-เร็วได้ง่าย
  float gateRatio;         // อัตราส่วนเสียงดังต่อช่องว่าง (0.75-0.85 = คมชัดไม่กลืน)
  const Note* notes;       // ตัวโน้ตใน PROGMEM flash
  size_t length;           // จำนวนตัวโน้ต
};

// ============================================================================
// 1. หมวดเพลงสันทนาการและเพลงไทยยอดนิยม (AUTHENTIC THAI RECREATION MELODIES)
// จุดเด่น: ใช้ทำนองร้องจริง (Vocal Line) ระดับ Octave 5 ที่ Buzzer ขับเสียงได้ใสกังวาน
// ============================================================================

// 🐱 1.1 เพลงหนูมาลีมีลูกแมวเหมียว (Mary Had a Little Lamb)
// ทำนองร้อง: มี เร โด เร | มี มี มี - | เร เร เร - | มี ซอล ซอล - | มี เร โด เร | มี มี มี มี | เร เร มี เร | โด - - -
const Note SONG_NUMALEE[] PROGMEM = {
  // วรรค 1: หนู-มา-ลี-มี / ลูก-แมว-เหมียว
  {NOTE_E5, N_QUARTER}, {NOTE_D5, N_QUARTER}, {NOTE_C5, N_QUARTER}, {NOTE_D5, N_QUARTER},
  {NOTE_E5, N_QUARTER}, {NOTE_E5, N_QUARTER}, {NOTE_E5, N_HALF},

  // วรรค 2: ลูก-แมว-เหมียว
  {NOTE_D5, N_QUARTER}, {NOTE_D5, N_QUARTER}, {NOTE_D5, N_HALF},

  // วรรค 3: ลูก-แมว-เหมียว
  {NOTE_E5, N_QUARTER}, {NOTE_G5, N_QUARTER}, {NOTE_G5, N_HALF},

  // วรรค 4: หนู-มา-ลี-มี / ลูก-แมว-เหมียว
  {NOTE_E5, N_QUARTER}, {NOTE_D5, N_QUARTER}, {NOTE_C5, N_QUARTER}, {NOTE_D5, N_QUARTER},
  {NOTE_E5, N_QUARTER}, {NOTE_E5, N_QUARTER}, {NOTE_E5, N_QUARTER}, {NOTE_E5, N_QUARTER},

  // วรรค 5: ขน-มัน-ดู-คล้าย / สำ-ลี
  {NOTE_D5, N_QUARTER}, {NOTE_D5, N_QUARTER}, {NOTE_E5, N_QUARTER}, {NOTE_D5, N_QUARTER},
  {NOTE_C5, N_DOTTED_HALF}, {REST, N_QUARTER}
};

// 🐘 1.2 เพลงช้าง (เพลงเด็กและสันทนาการอันดับ 1 ของคนไทย)
// "ช้าง ช้าง ช้าง น้องเคยเห็นช้างหรือเปล่า ช้างมันตัวโตไม่เบา จมูกยาวๆ เรียกว่างวง..."
const Note SONG_CHANG[] PROGMEM = {
  // ช้าง-ช้าง-ช้าง
  {NOTE_G5, N_QUARTER}, {NOTE_G5, N_QUARTER}, {NOTE_G5, N_HALF},
  // น้อง-เคย-เห็น-ช้าง-หรือ-เปล่า
  {NOTE_G5, N_EIGHTH}, {NOTE_A5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_C5, N_DOTTED_QUARTER},
  // ช้าง-มัน-ตัว-โต-ไม่-เบา
  {NOTE_E5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_A5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_DOTTED_QUARTER},
  // จมูก-ยาว-ยาว-เรียก-ว่า-งวง
  {NOTE_G5, N_EIGHTH}, {NOTE_A5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_C5, N_DOTTED_QUARTER},
  // มี-เขี้ยว-ใต้-งวง-เรียก-ว่า-งา
  {NOTE_E5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_A5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_E5, N_QUARTER},
  // มี-หู-มี-ตา-หาง-ยาว
  {NOTE_G5, N_EIGHTH}, {NOTE_A5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_D5, N_QUARTER}, {NOTE_C5, N_HALF}
};

// 🍗 1.3 เพลงไก่ย่างถูกเผา (เพลงสันทนาการระดับตำนาน)
// "ไก่ย่างถูกเผา ไก่ย่างถูกเผา มันจะโดนไม้เสียบ เสียบตูดซ้าย เสียบตูดขวา ร้อนจริงจริง..."
const Note SONG_KAIYANG[] PROGMEM = {
  // ไก่-ย่าง-ถูก-เผา
  {NOTE_G5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_G5, N_QUARTER},
  // ไก่-ย่าง-ถูก-เผา
  {NOTE_G5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_G5, N_QUARTER},

  // มัน-จะ-โดน-ไม้-เสียบ
  {NOTE_A5, N_EIGHTH}, {NOTE_A5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_D5, N_QUARTER},
  // มัน-จะ-โดน-ไม้-เสียบ
  {NOTE_A5, N_EIGHTH}, {NOTE_A5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_D5, N_QUARTER},

  // เสียบ-ตูด-ซ้าย
  {NOTE_C5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_E5, N_QUARTER},
  // เสียบ-ตูด-ขวา
  {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_D5, N_QUARTER},

  // ร้อน-จริง-จริง (โด โด โด)
  {NOTE_C5, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_C5, N_QUARTER},
  // ร้อน-จริง-จริง (เร เร เร)
  {NOTE_D5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_D5, N_QUARTER},
  // ร้อน-จริง-จริง (โด เร โด)
  {NOTE_C5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_C5, N_HALF}
};

// 🍃 1.4 เพลงกิ่งก้านใบ (สันทนาการค่ายลูกเสือ/รับน้อง)
// "กิ่ง ก้าน ใบ ชะ ชะ ใบ ก้าน กิ่ง ... ฝนตกลมแรงจริงจริง"
const Note SONG_KINGKANBAI[] PROGMEM = {
  // กิ่ง-ก้าน-ใบ
  {NOTE_C5, N_QUARTER}, {NOTE_D5, N_QUARTER}, {NOTE_E5, N_HALF},
  // ชะ-ชะ
  {NOTE_G5, N_EIGHTH}, {NOTE_G5, N_QUARTER},
  // ใบ-ก้าน-กิ่ง
  {NOTE_E5, N_QUARTER}, {NOTE_D5, N_QUARTER}, {NOTE_C5, N_HALF},

  // กิ่ง-ก้าน-ใบ
  {NOTE_C5, N_QUARTER}, {NOTE_D5, N_QUARTER}, {NOTE_E5, N_HALF},
  // ชะ-ชะ
  {NOTE_G5, N_EIGHTH}, {NOTE_G5, N_QUARTER},
  // ใบ-ก้าน-กิ่ง
  {NOTE_E5, N_QUARTER}, {NOTE_D5, N_QUARTER}, {NOTE_C5, N_HALF},

  // ฝนตกลมแรงจริงจริง
  {NOTE_G5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_A5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_G5, N_QUARTER},
  // ฝนตกลมแรงจริงจริง
  {NOTE_G5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_A5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_C5, N_QUARTER},

  // ชะ-ชะ
  {NOTE_G5, N_EIGHTH}, {NOTE_G5, N_QUARTER},
  // กิ่ง-ก้าน-ใบ
  {NOTE_C5, N_QUARTER}, {NOTE_D5, N_QUARTER}, {NOTE_C5, N_HALF}
};

// 🛺 1.5 เพลงรถตุ๊กตุ๊ก (ขับรถตุ๊กตุ๊ก บรรทุกถ่าน รถขึ้นสะพาน รถลงสะพาน กระตุก กระตุก กระตุก)
const Note SONG_TUKTUK[] PROGMEM = {
  // ขับรถตุ๊กตุ๊ก
  {NOTE_C5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_G5, N_QUARTER},
  // บรรทุกถ่าน
  {NOTE_G5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_C5, N_QUARTER},
  // รถขึ้นสะพาน
  {NOTE_G5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_A5, N_EIGHTH}, {NOTE_G5, N_QUARTER},
  // รถลงสะพาน
  {NOTE_E5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_F5, N_EIGHTH}, {NOTE_E5, N_QUARTER},
  // รถเลี้ยวซ้าย
  {NOTE_C5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_E5, N_QUARTER},
  // รถเลี้ยวขวา
  {NOTE_E5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_C5, N_QUARTER},
  // กระตุก
  {NOTE_C5, N_EIGHTH}, {NOTE_D5, N_QUARTER},
  // กระตุก
  {NOTE_C5, N_EIGHTH}, {NOTE_D5, N_QUARTER},
  // กระตุก!
  {NOTE_C5, N_EIGHTH}, {NOTE_C5, N_HALF}
};

// 🌕 1.6 เพลงลอยกระทง (เพลงประเพณีไทยชื่อก้องโลก)
// "วันเพ็ญเดือนสิบสอง น้ำก็นองเต็มตลิ่ง เราทั้งหลายชายหญิง สนุกกันจริงวันลอยกระทง"
const Note SONG_LOYKRATHONG[] PROGMEM = {
  // วัน-เพ็ญ-เดือน-สิบ-สอง / น้ำ-ก็-นอง-เต็ม-ตลิ่ง
  {NOTE_A4, N_EIGHTH}, {NOTE_A4, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_A4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_E4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_A4, N_QUARTER},
  // เรา-ทั้ง-หลาย-ชาย-หญิง
  {NOTE_C5, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_A4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_A4, N_QUARTER},
  // ส-นุก-กัน-จริง-วัน-ลอย-กระ-ทง
  {NOTE_D5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_A4, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_D5, N_QUARTER},
  // ลอย ลอย กระทง (บุญจะส่งให้เราสุขใจ)
  {NOTE_C5, N_EIGHTH}, {NOTE_A4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_E4, N_EIGHTH}, {NOTE_D4, N_EIGHTH}, {NOTE_E4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_A4, N_HALF}
};

// ============================================================================
// 2. หมวดเพลงระดับโลก & ไวรัล (WORLD FAMOUS THEMES & HOOKS)
// ============================================================================

// 🍄 2.1 Super Mario Bros Ground Theme (Nintendo Original)
const Note SONG_MARIO[] PROGMEM = {
  // อินโทรเปิดตัวอันโด่งดัง
  {NOTE_E5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {REST, N_EIGHTH}, {NOTE_E5, N_EIGHTH},
  {REST, N_EIGHTH},    {NOTE_C5, N_EIGHTH}, {NOTE_E5, N_QUARTER},
  {NOTE_G5, N_QUARTER}, {REST, N_QUARTER}, {NOTE_G4, N_HALF},

  // ท่อนฮุคหลัก (โด - ซอล - มี)
  {NOTE_C5, N_DOTTED_QUARTER}, {NOTE_G4, N_DOTTED_QUARTER}, {NOTE_E4, N_DOTTED_QUARTER},
  {NOTE_A4, N_QUARTER}, {NOTE_B4, N_QUARTER}, {NOTE_AS4, N_EIGHTH}, {NOTE_A4, N_QUARTER},
  {NOTE_G4, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_G5, N_EIGHTH}, {NOTE_A5, N_QUARTER}
};

// 🕺 2.2 Coffin Dance (Astronomia - ไวรัลแดนซ์ 126 BPM)
const Note SONG_COFFIN[] PROGMEM = {
  // ท่อนสับอินโทร
  {NOTE_A4, N_EIGHTH}, {NOTE_A4, N_EIGHTH}, {NOTE_A4, N_EIGHTH}, {NOTE_A4, N_EIGHTH},
  {NOTE_C5, N_EIGHTH}, {NOTE_B4, N_EIGHTH}, {NOTE_A4, N_EIGHTH}, {NOTE_G4, N_EIGHTH},
  {NOTE_A4, N_EIGHTH}, {NOTE_A4, N_EIGHTH}, {NOTE_A4, N_EIGHTH}, {NOTE_A4, N_EIGHTH},
  {NOTE_C5, N_EIGHTH}, {NOTE_B4, N_EIGHTH}, {NOTE_A4, N_EIGHTH}, {NOTE_G4, N_EIGHTH},

  // ท่อนดรอป EDM
  {NOTE_C5, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_C5, N_EIGHTH},
  {NOTE_E5, N_EIGHTH}, {NOTE_D5, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_B4, N_EIGHTH},
  {NOTE_C5, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_C5, N_EIGHTH},
  {NOTE_D5, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_B4, N_EIGHTH}, {NOTE_A4, N_HALF}
};

// 🦈 2.3 Baby Shark (เบบี้ชาร์ค ดู ดู๊ ดู ดู ดู ดู๊)
const Note SONG_BABYSHARK[] PROGMEM = {
  {NOTE_D4, N_QUARTER}, {NOTE_E4, N_QUARTER},
  {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {REST, N_SIXTEENTH},
  {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_SIXTEENTH}, {NOTE_G4, N_SIXTEENTH},

  {NOTE_D4, N_QUARTER}, {NOTE_E4, N_QUARTER},
  {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {REST, N_SIXTEENTH},
  {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_SIXTEENTH}, {NOTE_G4, N_SIXTEENTH},

  {NOTE_D4, N_QUARTER}, {NOTE_E4, N_QUARTER},
  {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {REST, N_SIXTEENTH},
  {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_G4, N_SIXTEENTH}, {NOTE_G4, N_SIXTEENTH},

  // เบบี้ชาร์ค!
  {NOTE_G4, N_QUARTER}, {NOTE_G4, N_QUARTER}, {NOTE_FS4, N_HALF}
};

// 🎂 2.4 Happy Birthday (สุขสันต์วันเกิด ครบ 4 วรรคสมบูรณ์แบบ)
const Note SONG_HBD[] PROGMEM = {
  // แฮปปี้เบิร์ธเดย์ทูยู
  {NOTE_C4, N_DOTTED_EIGHTH}, {NOTE_C4, N_SIXTEENTH}, {NOTE_D4, N_QUARTER}, {NOTE_C4, N_QUARTER}, {NOTE_F4, N_QUARTER}, {NOTE_E4, N_HALF},
  // แฮปปี้เบิร์ธเดย์ทูยู
  {NOTE_C4, N_DOTTED_EIGHTH}, {NOTE_C4, N_SIXTEENTH}, {NOTE_D4, N_QUARTER}, {NOTE_C4, N_QUARTER}, {NOTE_G4, N_QUARTER}, {NOTE_F4, N_HALF},
  // แฮปปี้เบิร์ธเดย์ แดร์...
  {NOTE_C4, N_DOTTED_EIGHTH}, {NOTE_C4, N_SIXTEENTH}, {NOTE_C5, N_QUARTER}, {NOTE_A4, N_QUARTER}, {NOTE_F4, N_QUARTER}, {NOTE_E4, N_QUARTER}, {NOTE_D4, N_HALF},
  // แฮปปี้เบิร์ธเดย์ทูยู
  {NOTE_AS4, N_DOTTED_EIGHTH}, {NOTE_AS4, N_SIXTEENTH}, {NOTE_A4, N_QUARTER}, {NOTE_F4, N_QUARTER}, {NOTE_G4, N_QUARTER}, {NOTE_F4, N_WHOLE}
};

// ⚔️ 2.5 Star Wars - The Imperial March (ธีม Darth Vader ทรงพลัง)
const Note SONG_STARWARS[] PROGMEM = {
  {NOTE_A4, N_QUARTER}, {NOTE_A4, N_QUARTER}, {NOTE_A4, N_QUARTER},
  {NOTE_F4, N_DOTTED_EIGHTH}, {NOTE_C5, N_SIXTEENTH}, {NOTE_A4, N_QUARTER},
  {NOTE_F4, N_DOTTED_EIGHTH}, {NOTE_C5, N_SIXTEENTH}, {NOTE_A4, N_HALF},

  {NOTE_E5, N_QUARTER}, {NOTE_E5, N_QUARTER}, {NOTE_E5, N_QUARTER},
  {NOTE_F5, N_DOTTED_EIGHTH}, {NOTE_C5, N_SIXTEENTH}, {NOTE_GS4, N_QUARTER},
  {NOTE_F4, N_DOTTED_EIGHTH}, {NOTE_C5, N_SIXTEENTH}, {NOTE_A4, N_HALF}
};

// 🔔 2.6 โดราเอมอน (Doraemon - อัง อัง อัง โตเดโมะ ดาอิซูคิ โดราเอมอน)
const Note SONG_DORAEMON[] PROGMEM = {
  {NOTE_C5, N_QUARTER}, {NOTE_D5, N_QUARTER}, {NOTE_E5, N_QUARTER}, {NOTE_G5, N_QUARTER},
  {NOTE_A5, N_DOTTED_QUARTER}, {NOTE_G5, N_QUARTER}, {NOTE_E5, N_HALF},
  {NOTE_C5, N_QUARTER}, {NOTE_D5, N_QUARTER}, {NOTE_E5, N_QUARTER}, {NOTE_D5, N_HALF}
};

// 🎸 2.7 Seven Nation Army (The White Stripes - ร็อคริฟฟ์ก้องโลก)
const Note SONG_SEVEN[] PROGMEM = {
  {NOTE_E4, N_DOTTED_QUARTER}, {NOTE_E4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_E4, N_EIGHTH}, {NOTE_D4, N_EIGHTH}, {NOTE_C4, N_HALF}, {NOTE_B3, N_HALF},
  {NOTE_E4, N_DOTTED_QUARTER}, {NOTE_E4, N_EIGHTH}, {NOTE_G4, N_EIGHTH}, {NOTE_E4, N_EIGHTH}, {NOTE_D4, N_EIGHTH}, {NOTE_C4, N_HALF}, {NOTE_B3, N_HALF}
};

// ⭐ 2.8 Twinkle Twinkle Little Star (ดาวดวงน้อย)
const Note SONG_TWINKLE[] PROGMEM = {
  {NOTE_C5, N_QUARTER}, {NOTE_C5, N_QUARTER}, {NOTE_G5, N_QUARTER}, {NOTE_G5, N_QUARTER},
  {NOTE_A5, N_QUARTER}, {NOTE_A5, N_QUARTER}, {NOTE_G5, N_HALF},
  {NOTE_F5, N_QUARTER}, {NOTE_F5, N_QUARTER}, {NOTE_E5, N_QUARTER}, {NOTE_E5, N_QUARTER},
  {NOTE_D5, N_QUARTER}, {NOTE_D5, N_QUARTER}, {NOTE_C5, N_HALF}
};

// ============================================================================
// 3. หมวดเสียงเอฟเฟกต์ (SOUND EFFECTS & CUES)
// ============================================================================
const Note SFX_BEEP[] PROGMEM = {
  {NOTE_E5, N_QUARTER}
};

const Note SFX_HAPPY[] PROGMEM = {
  {NOTE_C5, N_EIGHTH}, {NOTE_G5, N_QUARTER}
};

const Note SFX_FANFARE[] PROGMEM = {
  {NOTE_G4, N_EIGHTH}, {NOTE_C5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_G5, N_HALF}
};

const Note SFX_ALARM[] PROGMEM = {
  {NOTE_A5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}, {NOTE_A5, N_EIGHTH}, {NOTE_E5, N_EIGHTH}
};

// ============================================================================
// ตารางทะเบียนเพลง (SONG REGISTRY) - สไตล์ CS เพิ่มเพลงใหม่ง่ายเพียง 1 บรรทัด!
// ============================================================================
const SongItem SONG_REGISTRY[] = {
  {"numalee",     "หนูมาลีมีลูกแมว", "recreation", "🐱", 135, 0.82f, SONG_NUMALEE,    sizeof(SONG_NUMALEE)    / sizeof(Note)},
  {"chang",       "เพลงช้าง",       "recreation", "🐘", 125, 0.82f, SONG_CHANG,      sizeof(SONG_CHANG)      / sizeof(Note)},
  {"kaiyang",     "ไก่ย่างถูกเผา",   "recreation", "🍗", 125, 0.82f, SONG_KAIYANG,    sizeof(SONG_KAIYANG)    / sizeof(Note)},
  {"kingkanbai",  "กิ่งก้านใบ",     "recreation", "🍃", 128, 0.82f, SONG_KINGKANBAI, sizeof(SONG_KINGKANBAI) / sizeof(Note)},
  {"tuktuk",      "รถตุ๊กตุ๊ก",       "recreation", "🛺", 130, 0.82f, SONG_TUKTUK,     sizeof(SONG_TUKTUK)     / sizeof(Note)},
  {"loykrathong", "เพลงลอยกระทง",   "recreation", "🌕", 120, 0.84f, SONG_LOYKRATHONG,sizeof(SONG_LOYKRATHONG)/ sizeof(Note)},
  {"mario",       "Super Mario",    "game",       "🍄", 180, 0.80f, SONG_MARIO,      sizeof(SONG_MARIO)      / sizeof(Note)},
  {"coffin",      "Coffin Dance",   "game",       "🕺", 126, 0.80f, SONG_COFFIN,     sizeof(SONG_COFFIN)     / sizeof(Note)},
  {"babyshark",   "Baby Shark",     "pop",        "🦈", 116, 0.82f, SONG_BABYSHARK,  sizeof(SONG_BABYSHARK)  / sizeof(Note)},
  {"hbd",         "Happy Birthday", "pop",        "🎂", 120, 0.85f, SONG_HBD,        sizeof(SONG_HBD)        / sizeof(Note)},
  {"starwars",    "Imperial March", "pop",        "⚔️", 104, 0.85f, SONG_STARWARS,   sizeof(SONG_STARWARS)   / sizeof(Note)},
  {"doraemon",    "Doraemon Theme", "pop",        "🔔", 130, 0.84f, SONG_DORAEMON,   sizeof(SONG_DORAEMON)   / sizeof(Note)},
  {"seven",       "Seven Nation",   "pop",        "🎸", 124, 0.84f, SONG_SEVEN,      sizeof(SONG_SEVEN)      / sizeof(Note)},
  {"twinkle",     "ดาวดวงน้อย",     "pop",        "⭐", 115, 0.82f, SONG_TWINKLE,    sizeof(SONG_TWINKLE)    / sizeof(Note)},
  {"sfx_happy",   "ทักทายสดใส",     "sfx",        "✨", 120, 0.80f, SFX_HAPPY,       sizeof(SFX_HAPPY)       / sizeof(Note)},
  {"sfx_fanfare", "ไชโยเฉลิมฉลอง",  "sfx",        "🎉", 120, 0.80f, SFX_FANFARE,     sizeof(SFX_FANFARE)     / sizeof(Note)},
  {"sfx_alarm",   "เตือนภัยไซเรน",  "sfx",        "🚨", 140, 0.80f, SFX_ALARM,       sizeof(SFX_ALARM)       / sizeof(Note)}
};

const size_t SONG_REGISTRY_COUNT = sizeof(SONG_REGISTRY) / sizeof(SongItem);

// ============================================================================
// NON-BLOCKING MELODY PLAYER ENGINE (Computer Science State Machine)
// คำนวณเวลาอัตโนมัติจาก BPM และ Gate Ratio พร้อม Articulation Gap ขั้นต่ำ 30ms
// ทำให้เสียงบัซเซอร์ทุกพยางค์ขับออกมาชัดเจน ไม่มีการกลืนเสียงกัน 100%
// ============================================================================
class SongPlayer {
public:
  enum State { IDLE, SOUNDING, PAUSING };

  SongPlayer(uint8_t buzzerPin) :
    _pin(buzzerPin),
    _state(IDLE),
    _song(nullptr),
    _noteIdx(0),
    _timer(0),
    _currentSoundMs(0),
    _currentPauseMs(0) {}

  void begin() {
    pinMode(_pin, OUTPUT);
    noTone(_pin);
  }

  bool playById(const char* id) {
    for (size_t i = 0; i < SONG_REGISTRY_COUNT; i++) {
      if (strcmp(SONG_REGISTRY[i].id, id) == 0) {
        return playSong(&SONG_REGISTRY[i]);
      }
    }
    return false;
  }

  bool playSong(const SongItem* song) {
    if (!song || song->length == 0) return false;
    _song = song;
    _noteIdx = 0;
    _state = SOUNDING;
    _startCurrentNote();
    return true;
  }

  void stop() {
    noTone(_pin);
    _state = IDLE;
    _song = nullptr;
    _noteIdx = 0;
  }

  void update() {
    if (_state == IDLE || !_song) return;

    unsigned long now = millis();
    if (_state == SOUNDING) {
      if (now - _timer >= _currentSoundMs) {
        noTone(_pin);
        if (_currentPauseMs > 0) {
          _state = PAUSING;
          _timer = now;
        } else {
          _advanceNote();
        }
      }
    } else if (_state == PAUSING) {
      if (now - _timer >= _currentPauseMs) {
        _advanceNote();
      }
    }
  }

  bool isPlaying() const { return _state != IDLE; }
  const SongItem* currentSong() const { return _song; }
  size_t currentNoteIndex() const { return _noteIdx; }

  void toneBeep(uint16_t freq, uint16_t durationMs) {
    stop();
    tone(_pin, freq, durationMs);
  }

private:
  uint8_t _pin;
  State _state;
  const SongItem* _song;
  size_t _noteIdx;
  unsigned long _timer;
  uint32_t _currentSoundMs;
  uint32_t _currentPauseMs;

  void _startCurrentNote() {
    if (!_song || _noteIdx >= _song->length) {
      stop();
      return;
    }

    Note note;
    memcpy_P(&note, &_song->notes[_noteIdx], sizeof(Note));

    // คำนวณเวลาจาก BPM และสัดส่วนตัวโน้ต
    // 1 Beat = 60000 / BPM (ms)
    float oneBeatMs = 60000.0f / (float)_song->bpm;
    float totalNoteMs = note.beats * oneBeatMs;

    if (note.freq == REST) {
      _currentSoundMs = 0;
      _currentPauseMs = (uint32_t)totalNoteMs;
      noTone(_pin);
      _state = PAUSING;
      _timer = millis();
      return;
    }

    // คำนวณ Gate Ratio เพื่อสร้างช่องไฟระหว่างตัวโน้ต (Articulation Gap)
    // รับประกันว่าช่องว่างหยุดเสียงจะไม้น้อยกว่า 30ms เสมอ เพื่อให้แยกพยางค์ได้ชัด
    float soundFraction = constrain(_song->gateRatio, 0.50f, 0.90f);
    _currentSoundMs = (uint32_t)(totalNoteMs * soundFraction);
    _currentPauseMs = (uint32_t)(totalNoteMs - _currentSoundMs);
    if (_currentPauseMs < 30) {
      _currentPauseMs = 30;
      if (_currentSoundMs > 30) _currentSoundMs -= 30;
    }

    _timer = millis();
    tone(_pin, note.freq);
  }

  void _advanceNote() {
    _noteIdx++;
    if (_noteIdx < _song->length) {
      _state = SOUNDING;
      _startCurrentNote();
    } else {
      stop();
    }
  }
};

#endif // SONG_SYSTEM_H
