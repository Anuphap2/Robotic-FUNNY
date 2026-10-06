#ifndef SONG_SYSTEM_H
#define SONG_SYSTEM_H

#include <Arduino.h>
#include "RuntimeConfig.h"
#include "esp32-hal-ledc.h"
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
// DURATION & DYNAMICS NOTATION
// ============================================================================
#define N_WHOLE          4.00f  // 1 ห้องเต็ม (ตัวกลม)
#define N_DOTTED_HALF    3.00f  // ตัวขาวประจุด
#define N_HALF           2.00f  // 2 จังหวะ (ตัวขาว)
#define N_DOTTED_QUARTER 1.50f  // ตัวดำประจุด
#define N_QUARTER        1.00f  // 1 จังหวะ (ตัวดำ)
#define N_DOTTED_EIGHTH  0.75f  // เขบ็ตหนึ่งชั้นประจุด
#define N_EIGHTH         0.50f  // ครึ่งจังหวะ (เขบ็ต 1 ชั้น)
#define N_SIXTEENTH      0.25f  // เสี้ยวจังหวะ (เขบ็ต 2 ชั้น)

// ระดับความดัง (Musical Dynamics)
enum DynamicLevel {
  DYN_AUTO = 0, // อัตโนมัติ: จังหวะตก/โน้ตยาว = หนักแน่น, โน้ตสั้น = เบานุ่ม
  DYN_P    = 1, // Piano (นุ่มนวล)
  DYN_MP   = 2, // Mezzo-piano (ปานกลาง)
  DYN_MF   = 3, // Mezzo-forte (ชัดเจน)
  DYN_F    = 4  // Forte (หนักแน่น มีพลัง)
};

// สไตล์เสียงดนตรี (Instrument Sound Timbres)
enum InstrumentTimbre {
  TIMBRE_MUSIC_BOX = 0, // 🔔 กล่องดนตรี / กระดิ่งแก้ว (Natural decaying chime envelope - ไพเราะที่สุด)
  TIMBRE_VIBRATO   = 1, // 🎻 เสียงร้องมีลูกคอ (Expressive human vibrato on sustained notes)
  TIMBRE_ACOUSTIC  = 2, // 🎹 เปียโน / คาลิมบา (Pluck attack with warm body resonance)
  TIMBRE_CHIPTUNE  = 3  // 👾 เรโทร 8-บิต (Classic arcade punchy square wave)
};

struct Note {
  uint16_t freq;  // ความถี่เสียง เช่น NOTE_C5, REST
  float beats;    // ความยาวจังหวะ (เช่น N_QUARTER, N_EIGHTH)
  uint8_t dyn;    // ไดนามิก (DYN_AUTO, DYN_P, DYN_MF, DYN_F)
};

struct SongItem {
  const char* id;          // Machine ID สำหรับเรียกผ่าน Web API
  const char* title;       // ชื่อเพลงภาษาไทย
  const char* category;    // หมวดหมู่: "recreation", "pop", "game", "sfx"
  const char* icon;        // ไอคอนแสดงบนแดชบอร์ด
  uint16_t bpm;            // ความเร็วเพลง (Beats Per Minute)
  float gateRatio;         // อัตราส่วนความกว้างตัวโน้ต (0.75-0.85)
  const Note* notes;       // ตัวโน้ตใน PROGMEM flash
  size_t length;           // จำนวนตัวโน้ต
};

// ============================================================================
// 1. หมวดเพลงสันทนาการและเพลงไวรัล (AUTHENTIC RECREATION & VIRAL MELODIES)
// ============================================================================

// 🍦 1.1 เพลงมี่เสวี่ย (Mixue Ice Cream Theme Song - MuseScore 9369592)
// "หนี่ อ้าย หว่อ, หว่อ อ้าย หนี่... มี่ เสวี่ย ปิง เฉิง เถียน มี่ มี่"
const Note SONG_MIXUE[] PROGMEM = {
  // วรรค 1: หนี่-อ้าย-หว่อ, หว่อ-อ้าย-หนี่ (Sol La Sol Mi | Sol La Sol Mi)
  {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_A5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP},
  {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_A5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_QUARTER, DYN_F},

  // วรรค 2: มี่-เสวี่ย-ปิง-เฉิง-เถียน-มี่-มี่ (Do Do Re Mi | Re - - -)
  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_C5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_F},
  {NOTE_D5, N_HALF,    DYN_F}, {REST, N_QUARTER, DYN_AUTO},

  // วรรค 3: หนี่-อ้าย-หว่อ, หว่อ-อ้าย-หนี่ (Sol La Sol Mi | Sol La Sol Mi)
  {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_A5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP},
  {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_A5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_QUARTER, DYN_F},

  // วรรค 4: มี่-เสวี่ย-ปิง-เฉิง-เถียน-มี่-มี่ (Re Mi Re Do | Do - - -)
  {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_F},  {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_C5, N_EIGHTH, DYN_MP},
  {NOTE_C5, N_HALF,    DYN_F}, {REST, N_QUARTER, DYN_AUTO},

  // วรรค 5: จังหวะเด้งดึ๋ง Interlude (ซอล ซอล ซอล ซอล)
  {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_EIGHTH, DYN_F},

  // วรรค 6 (ร้องท่อนฮุคซ้ำแบบจัดเต็ม)
  {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_A5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP},
  {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_A5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_QUARTER, DYN_F},
  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_C5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_F},
  {NOTE_D5, N_HALF,    DYN_F}, {REST, N_QUARTER, DYN_AUTO},

  // วรรค 7: ท่อนจบประทับใจ
  {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_F},  {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_C5, N_EIGHTH, DYN_MP},
  {NOTE_C5, N_WHOLE,   DYN_F}
};

// 🐱 1.2 หนูมาลีมีลูกแมวเหมียว (Mary Had a Little Lamb)
const Note SONG_NUMALEE[] PROGMEM = {
  // วรรค 1: หนู-มา-ลี-มี / ลูก-แมว-เหมียว
  {NOTE_E5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MP}, {NOTE_C5, N_QUARTER, DYN_MP}, {NOTE_D5, N_QUARTER, DYN_MP},
  {NOTE_E5, N_QUARTER, DYN_F},  {NOTE_E5, N_QUARTER, DYN_MF}, {NOTE_E5, N_HALF,    DYN_F},

  // วรรค 2: ลูก-แมว-เหมียว
  {NOTE_D5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MP}, {NOTE_D5, N_HALF,    DYN_MF},

  // วรรค 3: ลูก-แมว-เหมียว
  {NOTE_E5, N_QUARTER, DYN_MF}, {NOTE_G5, N_QUARTER, DYN_F},  {NOTE_G5, N_HALF,    DYN_F},

  // วรรค 4: หนู-มา-ลี-มี / ลูก-แมว-เหมียว
  {NOTE_E5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MP}, {NOTE_C5, N_QUARTER, DYN_MP}, {NOTE_D5, N_QUARTER, DYN_MP},
  {NOTE_E5, N_QUARTER, DYN_F},  {NOTE_E5, N_QUARTER, DYN_MF}, {NOTE_E5, N_QUARTER, DYN_MF}, {NOTE_E5, N_QUARTER, DYN_MF},

  // วรรค 5: ขน-มัน-ดู-คล้าย / สำ-ลี
  {NOTE_D5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MP}, {NOTE_E5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MP},
  {NOTE_C5, N_DOTTED_HALF, DYN_F}, {REST, N_QUARTER, DYN_AUTO}
};

// 🐘 1.3 เพลงช้าง (เพลงเด็กและสันทนาการอันดับ 1 ของคนไทย)
const Note SONG_CHANG[] PROGMEM = {
  // ช้าง-ช้าง-ช้าง
  {NOTE_G5, N_QUARTER, DYN_F},  {NOTE_G5, N_QUARTER, DYN_MF}, {NOTE_G5, N_HALF, DYN_F},
  // น้อง-เคย-เห็น-ช้าง-หรือ-เปล่า
  {NOTE_G5, N_EIGHTH, DYN_MF},  {NOTE_A5, N_EIGHTH, DYN_F},  {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_EIGHTH, DYN_MP}, {NOTE_C5, N_DOTTED_QUARTER, DYN_MF},
  // ช้าง-มัน-ตัว-โต-ไม่-เบา
  {NOTE_E5, N_EIGHTH, DYN_MF},  {NOTE_E5, N_EIGHTH, DYN_MP}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_A5, N_EIGHTH, DYN_F},  {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_DOTTED_QUARTER, DYN_MF},
  // จมูก-ยาว-ยาว-เรียก-ว่า-งวง
  {NOTE_G5, N_EIGHTH, DYN_MF},  {NOTE_A5, N_EIGHTH, DYN_F},  {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_EIGHTH, DYN_MP}, {NOTE_C5, N_DOTTED_QUARTER, DYN_MF},
  // มี-เขี้ยว-ใต้-งวง-เรียก-ว่า-งา
  {NOTE_E5, N_EIGHTH, DYN_MF},  {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_A5, N_EIGHTH, DYN_F},  {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_EIGHTH, DYN_MP}, {NOTE_E5, N_QUARTER, DYN_MF},
  // มี-หู-มี-ตา-หาง-ยาว
  {NOTE_G5, N_EIGHTH, DYN_MF},  {NOTE_A5, N_EIGHTH, DYN_F},  {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_QUARTER, DYN_MF}, {NOTE_C5, N_HALF, DYN_F}
};

// 🍗 1.4 เพลงไก่ย่างถูกเผา
const Note SONG_KAIYANG[] PROGMEM = {
  // ไก่-ย่าง-ถูก-เผา
  {NOTE_G5, N_EIGHTH, DYN_F}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP}, {NOTE_G5, N_QUARTER, DYN_F},
  // ไก่-ย่าง-ถูก-เผา
  {NOTE_G5, N_EIGHTH, DYN_F}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP}, {NOTE_G5, N_QUARTER, DYN_F},

  // มัน-จะ-โดน-ไม้-เสียบ
  {NOTE_A5, N_EIGHTH, DYN_F}, {NOTE_A5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_QUARTER, DYN_MF},
  // มัน-จะ-โดน-ไม้-เสียบ
  {NOTE_A5, N_EIGHTH, DYN_F}, {NOTE_A5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_QUARTER, DYN_MF},

  // เสียบ-ตูด-ซ้าย
  {NOTE_C5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_QUARTER, DYN_F},
  // เสียบ-ตูด-ขวา
  {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_E5, N_EIGHTH, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MF},

  // ร้อน-จริง-จริง
  {NOTE_C5, N_EIGHTH, DYN_MP}, {NOTE_C5, N_EIGHTH, DYN_MP}, {NOTE_C5, N_QUARTER, DYN_MF},
  // ร้อน-จริง-จริง
  {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_F},
  // ร้อน-จริง-จริง
  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_D5, N_EIGHTH, DYN_F},  {NOTE_C5, N_HALF,    DYN_F}
};

// 🍃 1.5 เพลงกิ่งก้านใบ
const Note SONG_KINGKANBAI[] PROGMEM = {
  {NOTE_C5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MF}, {NOTE_E5, N_HALF, DYN_F},
  {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_G5, N_QUARTER, DYN_F},
  {NOTE_E5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MP}, {NOTE_C5, N_HALF, DYN_MF},

  {NOTE_C5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MF}, {NOTE_E5, N_HALF, DYN_F},
  {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_G5, N_QUARTER, DYN_F},
  {NOTE_E5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MP}, {NOTE_C5, N_HALF, DYN_MF},

  {NOTE_G5, N_EIGHTH, DYN_F}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_A5, N_EIGHTH, DYN_F}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP}, {NOTE_G5, N_QUARTER, DYN_F},
  {NOTE_G5, N_EIGHTH, DYN_F}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_A5, N_EIGHTH, DYN_F}, {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP}, {NOTE_C5, N_QUARTER, DYN_MF},

  {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_G5, N_QUARTER, DYN_F},
  {NOTE_C5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MF}, {NOTE_C5, N_HALF, DYN_F}
};

// 🛺 1.6 เพลงรถตุ๊กตุ๊ก
const Note SONG_TUKTUK[] PROGMEM = {
  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_QUARTER, DYN_F},
  {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_E5, N_EIGHTH, DYN_MF}, {NOTE_C5, N_QUARTER, DYN_MF},
  {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_G5, N_EIGHTH, DYN_MF}, {NOTE_A5, N_EIGHTH, DYN_F},  {NOTE_G5, N_QUARTER, DYN_F},
  {NOTE_E5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_MP}, {NOTE_F5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_QUARTER, DYN_MF},
  {NOTE_C5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_QUARTER, DYN_F},
  {NOTE_E5, N_EIGHTH, DYN_MF}, {NOTE_D5, N_EIGHTH, DYN_MP}, {NOTE_C5, N_QUARTER, DYN_MF},
  {NOTE_C5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_QUARTER, DYN_MF},
  {NOTE_C5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_QUARTER, DYN_MF},
  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_C5, N_HALF,    DYN_F}
};

// 🌕 1.7 เพลงลอยกระทง
const Note SONG_LOYKRATHONG[] PROGMEM = {
  {NOTE_A4, N_EIGHTH, DYN_MF}, {NOTE_A4, N_EIGHTH, DYN_MP}, {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_A4, N_EIGHTH, DYN_MP}, {NOTE_G4, N_EIGHTH, DYN_MP}, {NOTE_E4, N_EIGHTH, DYN_MP}, {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_A4, N_QUARTER, DYN_F},
  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_C5, N_EIGHTH, DYN_MP}, {NOTE_D5, N_EIGHTH, DYN_F},  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_A4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_EIGHTH, DYN_MP}, {NOTE_A4, N_QUARTER, DYN_F},
  {NOTE_D5, N_EIGHTH, DYN_F},  {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_F},  {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_A4, N_EIGHTH, DYN_MF}, {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_F},
  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_A4, N_EIGHTH, DYN_MP}, {NOTE_G4, N_EIGHTH, DYN_MP}, {NOTE_E4, N_EIGHTH, DYN_MP}, {NOTE_D4, N_EIGHTH, DYN_MP}, {NOTE_E4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_A4, N_HALF,    DYN_F}
};

// ============================================================================
// 2. หมวดเพลงระดับโลก & ไวรัล (WORLD THEMES)
// ============================================================================

// 🍄 2.1 Super Mario Bros Ground Theme
const Note SONG_MARIO[] PROGMEM = {
  {NOTE_E5, N_EIGHTH, DYN_F},  {NOTE_E5, N_EIGHTH, DYN_F}, {REST, N_EIGHTH, DYN_AUTO}, {NOTE_E5, N_EIGHTH, DYN_F},
  {REST, N_EIGHTH, DYN_AUTO},  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_QUARTER, DYN_F},
  {NOTE_G5, N_QUARTER, DYN_F}, {REST, N_QUARTER, DYN_AUTO}, {NOTE_G4, N_HALF, DYN_MF},

  {NOTE_C5, N_DOTTED_QUARTER, DYN_F}, {NOTE_G4, N_DOTTED_QUARTER, DYN_MF}, {NOTE_E4, N_DOTTED_QUARTER, DYN_MF},
  {NOTE_A4, N_QUARTER, DYN_F},  {NOTE_B4, N_QUARTER, DYN_F}, {NOTE_AS4, N_EIGHTH, DYN_MF}, {NOTE_A4, N_QUARTER, DYN_F},
  {NOTE_G4, N_EIGHTH, DYN_MF},  {NOTE_E5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_EIGHTH, DYN_F},  {NOTE_A5, N_QUARTER, DYN_F}
};

// 🕺 2.2 Coffin Dance (Astronomia)
const Note SONG_COFFIN[] PROGMEM = {
  {NOTE_A4, N_EIGHTH, DYN_MF}, {NOTE_A4, N_EIGHTH, DYN_MP}, {NOTE_A4, N_EIGHTH, DYN_MF}, {NOTE_A4, N_EIGHTH, DYN_MP},
  {NOTE_C5, N_EIGHTH, DYN_F},  {NOTE_B4, N_EIGHTH, DYN_MF}, {NOTE_A4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_EIGHTH, DYN_MP},
  {NOTE_A4, N_EIGHTH, DYN_MF}, {NOTE_A4, N_EIGHTH, DYN_MP}, {NOTE_A4, N_EIGHTH, DYN_MF}, {NOTE_A4, N_EIGHTH, DYN_MP},
  {NOTE_C5, N_EIGHTH, DYN_F},  {NOTE_B4, N_EIGHTH, DYN_MF}, {NOTE_A4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_EIGHTH, DYN_MP},

  {NOTE_C5, N_EIGHTH, DYN_F},  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_C5, N_EIGHTH, DYN_F},  {NOTE_C5, N_EIGHTH, DYN_MF},
  {NOTE_E5, N_EIGHTH, DYN_F},  {NOTE_D5, N_EIGHTH, DYN_MF}, {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_B4, N_EIGHTH, DYN_MP},
  {NOTE_C5, N_EIGHTH, DYN_F},  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_C5, N_EIGHTH, DYN_F},  {NOTE_C5, N_EIGHTH, DYN_MF},
  {NOTE_D5, N_EIGHTH, DYN_F},  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_B4, N_EIGHTH, DYN_MP}, {NOTE_A4, N_HALF,    DYN_F}
};

// 🦈 2.3 Baby Shark
const Note SONG_BABYSHARK[] PROGMEM = {
  {NOTE_D4, N_QUARTER, DYN_MF}, {NOTE_E4, N_QUARTER, DYN_MF},
  {NOTE_G4, N_EIGHTH, DYN_F},  {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_EIGHTH, DYN_MF}, {REST, N_SIXTEENTH, DYN_AUTO},
  {NOTE_G4, N_EIGHTH, DYN_F},  {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_SIXTEENTH, DYN_MP}, {NOTE_G4, N_SIXTEENTH, DYN_MP},

  {NOTE_D4, N_QUARTER, DYN_MF}, {NOTE_E4, N_QUARTER, DYN_MF},
  {NOTE_G4, N_EIGHTH, DYN_F},  {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_EIGHTH, DYN_MF}, {REST, N_SIXTEENTH, DYN_AUTO},
  {NOTE_G4, N_EIGHTH, DYN_F},  {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_SIXTEENTH, DYN_MP}, {NOTE_G4, N_SIXTEENTH, DYN_MP},

  {NOTE_D4, N_QUARTER, DYN_MF}, {NOTE_E4, N_QUARTER, DYN_MF},
  {NOTE_G4, N_EIGHTH, DYN_F},  {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_EIGHTH, DYN_MF}, {REST, N_SIXTEENTH, DYN_AUTO},
  {NOTE_G4, N_EIGHTH, DYN_F},  {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_G4, N_SIXTEENTH, DYN_MP}, {NOTE_G4, N_SIXTEENTH, DYN_MP},

  {NOTE_G4, N_QUARTER, DYN_F}, {NOTE_G4, N_QUARTER, DYN_F}, {NOTE_FS4, N_HALF, DYN_F}
};

// 🎂 2.4 Happy Birthday
const Note SONG_HBD[] PROGMEM = {
  {NOTE_C4, N_DOTTED_EIGHTH, DYN_MP}, {NOTE_C4, N_SIXTEENTH, DYN_P},  {NOTE_D4, N_QUARTER, DYN_MF}, {NOTE_C4, N_QUARTER, DYN_MP}, {NOTE_F4, N_QUARTER, DYN_F},  {NOTE_E4, N_HALF, DYN_F},
  {NOTE_C4, N_DOTTED_EIGHTH, DYN_MP}, {NOTE_C4, N_SIXTEENTH, DYN_P},  {NOTE_D4, N_QUARTER, DYN_MF}, {NOTE_C4, N_QUARTER, DYN_MP}, {NOTE_G4, N_QUARTER, DYN_F},  {NOTE_F4, N_HALF, DYN_F},
  {NOTE_C4, N_DOTTED_EIGHTH, DYN_MP}, {NOTE_C4, N_SIXTEENTH, DYN_P},  {NOTE_C5, N_QUARTER, DYN_F},  {NOTE_A4, N_QUARTER, DYN_F},  {NOTE_F4, N_QUARTER, DYN_MF}, {NOTE_E4, N_QUARTER, DYN_MF}, {NOTE_D4, N_HALF, DYN_MF},
  {NOTE_AS4, N_DOTTED_EIGHTH, DYN_MF},{NOTE_AS4, N_SIXTEENTH, DYN_MP}, {NOTE_A4, N_QUARTER, DYN_F},  {NOTE_F4, N_QUARTER, DYN_MF}, {NOTE_G4, N_QUARTER, DYN_F},  {NOTE_F4, N_WHOLE, DYN_F}
};

// ⚔️ 2.5 Star Wars - The Imperial March
const Note SONG_STARWARS[] PROGMEM = {
  {NOTE_A4, N_QUARTER, DYN_F}, {NOTE_A4, N_QUARTER, DYN_F}, {NOTE_A4, N_QUARTER, DYN_F},
  {NOTE_F4, N_DOTTED_EIGHTH, DYN_MF}, {NOTE_C5, N_SIXTEENTH, DYN_MP}, {NOTE_A4, N_QUARTER, DYN_F},
  {NOTE_F4, N_DOTTED_EIGHTH, DYN_MF}, {NOTE_C5, N_SIXTEENTH, DYN_MP}, {NOTE_A4, N_HALF, DYN_F},

  {NOTE_E5, N_QUARTER, DYN_F}, {NOTE_E5, N_QUARTER, DYN_F}, {NOTE_E5, N_QUARTER, DYN_F},
  {NOTE_F5, N_DOTTED_EIGHTH, DYN_MF}, {NOTE_C5, N_SIXTEENTH, DYN_MP}, {NOTE_GS4, N_QUARTER, DYN_F},
  {NOTE_F4, N_DOTTED_EIGHTH, DYN_MF}, {NOTE_C5, N_SIXTEENTH, DYN_MP}, {NOTE_A4, N_HALF, DYN_F}
};

// 🔔 2.6 โดราเอมอน
const Note SONG_DORAEMON[] PROGMEM = {
  {NOTE_C5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MF}, {NOTE_E5, N_QUARTER, DYN_F},  {NOTE_G5, N_QUARTER, DYN_F},
  {NOTE_A5, N_DOTTED_QUARTER, DYN_F}, {NOTE_G5, N_QUARTER, DYN_MF}, {NOTE_E5, N_HALF, DYN_F},
  {NOTE_C5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MF}, {NOTE_E5, N_QUARTER, DYN_F},  {NOTE_D5, N_HALF, DYN_F}
};

// 🎸 2.7 Seven Nation Army
const Note SONG_SEVEN[] PROGMEM = {
  {NOTE_E4, N_DOTTED_QUARTER, DYN_F}, {NOTE_E4, N_EIGHTH, DYN_MP}, {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_E4, N_EIGHTH, DYN_MP}, {NOTE_D4, N_EIGHTH, DYN_MP}, {NOTE_C4, N_HALF, DYN_F}, {NOTE_B3, N_HALF, DYN_F},
  {NOTE_E4, N_DOTTED_QUARTER, DYN_F}, {NOTE_E4, N_EIGHTH, DYN_MP}, {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_E4, N_EIGHTH, DYN_MP}, {NOTE_D4, N_EIGHTH, DYN_MP}, {NOTE_C4, N_HALF, DYN_F}, {NOTE_B3, N_HALF, DYN_F}
};

// ⭐ 2.8 Twinkle Twinkle Little Star
const Note SONG_TWINKLE[] PROGMEM = {
  {NOTE_C5, N_QUARTER, DYN_MF}, {NOTE_C5, N_QUARTER, DYN_MP}, {NOTE_G5, N_QUARTER, DYN_F},  {NOTE_G5, N_QUARTER, DYN_MF},
  {NOTE_A5, N_QUARTER, DYN_F},  {NOTE_A5, N_QUARTER, DYN_MF}, {NOTE_G5, N_HALF,    DYN_F},
  {NOTE_F5, N_QUARTER, DYN_MF}, {NOTE_F5, N_QUARTER, DYN_MP}, {NOTE_E5, N_QUARTER, DYN_MF}, {NOTE_E5, N_QUARTER, DYN_MP},
  {NOTE_D5, N_QUARTER, DYN_MF}, {NOTE_D5, N_QUARTER, DYN_MP}, {NOTE_C5, N_HALF,    DYN_F}
};

// 🤙 2.9 ท่าไวรัล SIX-SEVEN (Doot Doot 6-7 Drill Meme Theme)
const Note SONG_SIX_SEVEN[] PROGMEM = {
  // Bar 1: Intro drill staccato beat
  {NOTE_F4, N_SIXTEENTH, DYN_F}, {NOTE_F4, N_SIXTEENTH, DYN_MF}, {REST, N_EIGHTH, DYN_P},
  {NOTE_F4, N_SIXTEENTH, DYN_F}, {NOTE_F4, N_SIXTEENTH, DYN_MF}, {REST, N_EIGHTH, DYN_P},
  {NOTE_GS4, N_EIGHTH, DYN_F},   {NOTE_G4, N_EIGHTH, DYN_MF},

  // Bar 2: "SIX! ... SEVEN!"
  {NOTE_C5, N_QUARTER, DYN_F},        // "SIX!"
  {REST, N_EIGHTH, DYN_P},
  {NOTE_CS5, N_DOTTED_QUARTER, DYN_F}, // "SEVEN!"
  {REST, N_EIGHTH, DYN_P},

  // Bar 3: Drill rhythm groove
  {NOTE_F4, N_EIGHTH, DYN_MF}, {NOTE_GS4, N_EIGHTH, DYN_F}, {NOTE_AS4, N_EIGHTH, DYN_F}, {NOTE_C5, N_EIGHTH, DYN_F},
  {NOTE_AS4, N_EIGHTH, DYN_MF}, {NOTE_GS4, N_EIGHTH, DYN_MF}, {NOTE_F4, N_QUARTER, DYN_F},

  // Bar 4: "SIX! SEVEN! SIX! SEVEN!"
  {NOTE_C5, N_EIGHTH, DYN_F},  {REST, N_EIGHTH, DYN_P},       // "SIX!"
  {NOTE_CS5, N_QUARTER, DYN_F},                               // "SEVEN!"
  {NOTE_C5, N_EIGHTH, DYN_F},  {REST, N_EIGHTH, DYN_P},       // "SIX!"
  {NOTE_DS5, N_DOTTED_QUARTER, DYN_F},                        // "SEVEN!"
  {REST, N_QUARTER, DYN_P}
};

// ============================================================================
// 3. หมวดเสียงเอฟเฟกต์ (SOUND EFFECTS)
// ============================================================================
const Note SFX_BEEP[] PROGMEM = {
  {NOTE_E5, N_QUARTER, DYN_F}
};

const Note SFX_HAPPY[] PROGMEM = {
  {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_G5, N_QUARTER, DYN_F}
};

const Note SFX_FANFARE[] PROGMEM = {
  {NOTE_G4, N_EIGHTH, DYN_MF}, {NOTE_C5, N_EIGHTH, DYN_MF}, {NOTE_E5, N_EIGHTH, DYN_F}, {NOTE_G5, N_HALF, DYN_F}
};

const Note SFX_ALARM[] PROGMEM = {
  {NOTE_A5, N_EIGHTH, DYN_F}, {NOTE_E5, N_EIGHTH, DYN_F}, {NOTE_A5, N_EIGHTH, DYN_F}, {NOTE_E5, N_EIGHTH, DYN_F}
};

// ============================================================================
// ตารางทะเบียนเพลง (SONG REGISTRY)
// ============================================================================
const SongItem SONG_REGISTRY[] = {
  {"mixue",       "เพลงมี่เสวี่ย (Mixue)", "recreation", "🍦", 118, 0.82f, SONG_MIXUE,      sizeof(SONG_MIXUE)      / sizeof(Note)},
  {"six_seven",   "SIX-SEVEN (6-7)", "pop",        "🤙", 132, 0.80f, SONG_SIX_SEVEN,  sizeof(SONG_SIX_SEVEN)  / sizeof(Note)},
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
// EXPRESSIVE ACOUSTIC MELODY SYNTHESIZER
// สถาปัตยกรรมสังเคราะห์เสียงพร้อม Dynamic Velocity & Timbre Envelope Modulation
// ออกแบบระบบปิดเสียงแบบ Hardware Detach & Pull LOW ป้องกันเสียงแช่ค้าง 100%
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
    _lastModTime(0),
    _currentSoundMs(0),
    _currentPauseMs(0),
    _currentFreq(0),
    _baseDuty(450),
    _timbre(TIMBRE_MUSIC_BOX) {}

  void begin() {
    _silence();
  }

  void setTimbre(InstrumentTimbre t) {
    _timbre = t;
  }

  InstrumentTimbre timbre() const { return _timbre; }

  const char* timbreName() const {
    switch (_timbre) {
      case TIMBRE_MUSIC_BOX: return "music_box";
      case TIMBRE_VIBRATO:   return "vibrato";
      case TIMBRE_ACOUSTIC:  return "acoustic";
      case TIMBRE_CHIPTUNE:  return "chiptune";
      default:               return "music_box";
    }
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
    if (!robotSettings.get(AUDIO_ENABLED) || !robotSettings.get(AUDIO_VOLUME) || !song || song->length == 0) return false;
    stop(); // Reset and silence any ongoing playback first
    _song = song;
    _noteIdx = 0;
    _state = SOUNDING;
    _startCurrentNote();
    return _state != IDLE;
  }

  void stop() {
    _state = IDLE;
    _standaloneBeep = false;
    _song = nullptr;
    _noteIdx = 0;
    _currentFreq = 0;
    _silence();
  }

  void update() {
    if (_state == IDLE) return;
    if (!robotSettings.get(AUDIO_ENABLED) || !robotSettings.get(AUDIO_VOLUME)) { stop(); return; }
    if (_standaloneBeep) {
      if (millis() - _timer >= _currentSoundMs) stop();
      return;
    }
    if (!_song) return;

    unsigned long now = millis();
    if (_state == SOUNDING) {
      unsigned long elapsed = now - _timer;

      if (elapsed >= _currentSoundMs) {
        _silence(); // Silent gap between notes - guaranteed 0V
        if (_currentPauseMs > 0) {
          _state = PAUSING;
          _timer = now;
        } else {
          _advanceNote();
        }
      } else {
        // Real-time Dynamic Envelope & Timbre Modulation (Rate-limited to avoid bus flood)
        _modulateEnvelope(elapsed);
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
    if (freq > 0 && robotSettings.get(AUDIO_ENABLED) && robotSettings.get(AUDIO_VOLUME)) {
      if (!_attachTone(freq)) return;
      _currentFreq = freq; _baseDuty = scaledDuty(400);
      ledcWrite(_pin, _baseDuty);
      _currentSoundMs = durationMs; _currentPauseMs = 0;
      _timer = millis(); _lastModTime = _timer;
      _standaloneBeep = true; _state = SOUNDING;
    }
  }
  bool hardwareOK() const { return _hardwareOK; }

private:
  uint8_t _pin;
  State _state;
  const SongItem* _song;
  size_t _noteIdx;
  unsigned long _timer;
  unsigned long _lastModTime;
  uint32_t _currentSoundMs;
  uint32_t _currentPauseMs;
  uint16_t _currentFreq;
  uint32_t _baseDuty;
  InstrumentTimbre _timbre;
  bool _attached = false, _standaloneBeep = false, _hardwareOK = true;
  uint32_t scaledDuty(uint32_t duty) const { return duty * robotSettings.get(AUDIO_VOLUME) / 100; }
  bool _attachTone(uint16_t frequency) {
    // Classic ESP32: channel 6 / timer 3 cannot change the servo timer 0.
    _attached = ledcAttachChannel(_pin, frequency, 10, 6);
    _hardwareOK = _attached;
    if (!_attached) { stop(); return false; }
    return true;
  }

  // ตัดวงจรสัญญาณเสียงอย่างสมบูรณ์แบบ: ปลดปล่อย Pin จาก LEDC และสั่ง LOW 0V ป้องกันเสียงค้าง 100%
  void _silence() {
    if (_attached) {
      ledcWrite(_pin, 0);
      ledcDetach(_pin);
      _attached = false;
    }
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
  }

  void _startCurrentNote() {
    if (!_song || _noteIdx >= _song->length) {
      stop();
      return;
    }

    Note note;
    memcpy_P(&note, &_song->notes[_noteIdx], sizeof(Note));
    _currentFreq = note.freq;

    // คำนวณความยาวเวลาจาก BPM (1 Beat = 60000 / BPM ms)
    float oneBeatMs = 60000.0f / ((float)_song->bpm * robotSettings.get(AUDIO_TEMPO) / 100.0f);
    float totalNoteMs = note.beats * oneBeatMs;

    if (note.freq == REST) {
      _currentSoundMs = 0;
      _currentPauseMs = (uint32_t)totalNoteMs;
      _silence();
      _state = PAUSING;
      _timer = millis();
      return;
    }

    // คำนวณความดังเริ่มต้น (Dynamic Velocity)
    uint8_t dyn = note.dyn;
    if (dyn == DYN_AUTO) {
      if (note.beats >= 2.0f)      dyn = DYN_F;  // โน้ตยาว / ท้ายประโยค = ก้องกังวาน (Forte)
      else if (note.beats >= 1.0f) dyn = DYN_MF; // ตัวดำ = ชัดเจน (Mezzo-forte)
      else                         dyn = DYN_MP; // โน้ตสั้น = นุ่มนวล (Mezzo-piano)
    }

    switch (dyn) {
      case DYN_P:  _baseDuty = 160; break; // เบา นุ่ม
      case DYN_MP: _baseDuty = 270; break; // ปานกลางค่อนข้างเบา
      case DYN_MF: _baseDuty = 380; break; // ชัดเจนกำลังดี
      case DYN_F:
      default:     _baseDuty = 490; break; // หนักแน่น พุ่งกังวาน (Max 511)
    }

    _baseDuty = scaledDuty(_baseDuty);
    // Articulation Gate Ratio (เว้นวรรคช่องไฟขั้นต่ำ 35ms เสมอ)
    float soundFraction = constrain(_song->gateRatio, 0.60f, 0.88f);
    _currentSoundMs = (uint32_t)(totalNoteMs * soundFraction);
    _currentPauseMs = (uint32_t)(totalNoteMs - _currentSoundMs);
    if (_currentPauseMs < 35) {
      _currentPauseMs = 35;
      if (_currentSoundMs > 35) _currentSoundMs -= 35;
    }

    _timer = millis();
    _lastModTime = _timer;

    // ติดตั้งและเริ่มส่งสัญญาณความถี่โน้ต
    if (!_attachTone(_currentFreq)) return;
    ledcWrite(_pin, _baseDuty);
  }

  // ปรับแต่งหางเสียง (Envelope) แบบจำกัดความถี่การอัปเดต (Rate-limited 25ms) ป้องกันการล็อกของ LEDC Timer
  void _modulateEnvelope(unsigned long elapsed) {
    if (_currentFreq == REST || _currentSoundMs == 0) return;

    unsigned long now = millis();
    if (now - _lastModTime < 25) return; // ทำงานไม่เกิน 40 ครั้ง/วินาที ป้องกัน Timer Hardware Crash
    _lastModTime = now;

    float progress = (float)elapsed / (float)_currentSoundMs;
    if (progress > 1.0f) progress = 1.0f;

    switch (_timbre) {
      case TIMBRE_MUSIC_BOX: {
        // 🔔 กล่องดนตรี: เสียงเคาะกระดิ่งแก้ว แล้วค่อยๆ กังวานหรี่ลงอย่างนุ่มนวล
        float decay = 1.0f - (progress * 0.70f);
        uint32_t duty = (uint32_t)(_baseDuty * decay);
        if (duty < scaledDuty(50)) duty = scaledDuty(50);
        ledcWrite(_pin, duty);
        break;
      }

      case TIMBRE_VIBRATO: {
        // 🎻 เสียงร้องมีลูกคอ: โน้ตที่ลากยาวเกิน 350ms จะมีลูกคอพริ้วหวาน (LFO 5.5 Hz)
        if (_currentSoundMs >= 350 && elapsed >= 180) {
          float lfoPhase = (float)(elapsed - 180) * 0.035f;
          float pitchOffset = sin(lfoPhase) * (_currentFreq * 0.012f);
          ledcChangeFrequency(_pin, (uint32_t)(_currentFreq + pitchOffset), 10);
        }
        ledcWrite(_pin, _baseDuty);
        break;
      }

      case TIMBRE_ACOUSTIC: {
        // 🎹 เปียโน/คาลิมบา: ดีดหัวโน้ตคมชัด แล้วคงเสียง sustain อุ่นๆ
        uint32_t duty = _baseDuty;
        if (progress > 0.15f) {
          float decay = 1.0f - ((progress - 0.15f) * 0.50f);
          duty = (uint32_t)(_baseDuty * decay);
        }
        if (duty < scaledDuty(60)) duty = scaledDuty(60);
        ledcWrite(_pin, duty);
        break;
      }

      case TIMBRE_CHIPTUNE:
      default: {
        // 👾 เกม 8-บิต: ยึดตาม Dynamic Velocity หนัก-เบา
        ledcWrite(_pin, _baseDuty);
        break;
      }
    }
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
