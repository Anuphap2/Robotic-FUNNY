#ifndef SONG_SYSTEM_H
#define SONG_SYSTEM_H

#include <Arduino.h>
#include "Config.h"

// ============================================================================
// STANDARD MUSICAL PITCH FREQUENCIES (Hz) - Scientific Pitch Notation
// ============================================================================
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62

#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123

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
// DATA STRUCTURES
// ============================================================================
struct Note {
  uint16_t freq;      // Frequency in Hz (REST = 0)
  uint16_t duration;  // Active sounding time in ms
  uint16_t pause;     // Silent gap after note for crisp articulation (25-50ms)
};

struct SongItem {
  const char* id;        // Machine ID for API / dispatch
  const char* title;     // Human title
  const char* category;  // "recreation", "pop", "game", "sfx"
  const char* icon;      // Emoji icon
  const Note* notes;     // Sequence in PROGMEM flash
  size_t length;         // Note count
};

// ============================================================================
// ACCURATE MUSICAL REPERTOIRE (ท่วงทำนองถูกต้อง ฟังออก 100%)
// ============================================================================

// 1. หนูมาลี (Mary Had a Little Lamb) - มีลูกแมวเหมียว ขนมันดูคล้ายสำลี
// ทำนอง: E D C D | E E E - | D D D - | E G G - | E D C D | E E E E | D D E D | C - - -
const Note MELODY_NUMALEE[] PROGMEM = {
  // วรรค 1: หนู-มา-ลี-มี / ลูก-แมว-เหมียว
  {NOTE_E4, 300, 40}, {NOTE_D4, 300, 40}, {NOTE_C4, 300, 40}, {NOTE_D4, 300, 40},
  {NOTE_E4, 300, 40}, {NOTE_E4, 300, 40}, {NOTE_E4, 600, 80},

  // วรรค 2: ลูก-แมว-เหมียว (D D D)
  {NOTE_D4, 300, 40}, {NOTE_D4, 300, 40}, {NOTE_D4, 600, 80},

  // วรรค 3: ลูก-แมว-เหมียว (E G G)
  {NOTE_E4, 300, 40}, {NOTE_G4, 300, 40}, {NOTE_G4, 600, 80},

  // วรรค 4: หนู-มา-ลี-มี / ลูก-แมว-เหมียว
  {NOTE_E4, 300, 40}, {NOTE_D4, 300, 40}, {NOTE_C4, 300, 40}, {NOTE_D4, 300, 40},
  {NOTE_E4, 300, 40}, {NOTE_E4, 300, 40}, {NOTE_E4, 300, 40}, {NOTE_E4, 300, 40},

  // วรรค 5: ขน-มัน-ดู-คล้าย / สำ-ลี
  {NOTE_D4, 300, 40}, {NOTE_D4, 300, 40}, {NOTE_E4, 300, 40}, {NOTE_D4, 300, 40},
  {NOTE_C4, 850, 100}
};

// 2. ไก่ย่างถูกเผา (เพลงสันทนาการระดับตำนาน)
// ทำนองจริง: ซอล ซอล มี ซอล (ไก่ย่างถูกเผา) | ซอล ซอล มี ซอล (ไก่ย่างถูกเผา)
// ลา ลา ซอล มี เร (มันจะโดนไม้เสียบ) | ลา ลา ซอล มี เร (มันจะโดนไม้เสียบ)
// โด เร มี (เสียบตูดซ้าย) | ซอล มี เร (เสียบตูดขวา)
// โด โด โด (ร้อนจริงจริง) | เร เร เร (ร้อนจริงจริง) | โด เร โด (ร้อนจริงจริง)
const Note MELODY_KAIYANG[] PROGMEM = {
  // ไก่-ย่าง-ถูก-เผา
  {NOTE_G4, 180, 30}, {NOTE_G4, 180, 30}, {NOTE_E4, 180, 30}, {NOTE_G4, 320, 60},
  // ไก่-ย่าง-ถูก-เผา
  {NOTE_G4, 180, 30}, {NOTE_G4, 180, 30}, {NOTE_E4, 180, 30}, {NOTE_G4, 320, 60},

  // มัน-จะ-โดน-ไม้-เสียบ
  {NOTE_A4, 170, 25}, {NOTE_A4, 170, 25}, {NOTE_G4, 170, 25}, {NOTE_E4, 170, 25}, {NOTE_D4, 320, 60},
  // มัน-จะ-โดน-ไม้-เสียบ
  {NOTE_A4, 170, 25}, {NOTE_A4, 170, 25}, {NOTE_G4, 170, 25}, {NOTE_E4, 170, 25}, {NOTE_D4, 320, 60},

  // เสียบ-ตูด-ซ้าย
  {NOTE_C4, 200, 30}, {NOTE_D4, 200, 30}, {NOTE_E4, 350, 50},
  // เสียบ-ตูด-ขวา
  {NOTE_G4, 200, 30}, {NOTE_E4, 200, 30}, {NOTE_D4, 350, 50},

  // ร้อน-จริง-จริง (โด โด โด)
  {NOTE_C4, 160, 30}, {NOTE_C4, 160, 30}, {NOTE_C4, 280, 50},
  // ร้อน-จริง-จริง (เร เร เร)
  {NOTE_D4, 160, 30}, {NOTE_D4, 160, 30}, {NOTE_D4, 280, 50},
  // ร้อน-จริง-จริง (โด เร โด)
  {NOTE_C4, 180, 30}, {NOTE_D4, 180, 30}, {NOTE_C4, 450, 80}
};

// 3. กิ่งก้านใบ (สันทนาการค่ายลูกเสือ/รับน้อง)
// กิ่ง(โด) ก้าน(เร) ใบ(มี) ชะ ชะ(ซอล ซอล) ใบ(มี) ก้าน(เร) กิ่ง(โด)
// ฝนตกลมแรงจริงจริง (ซอล ซอล ลา ซอล มี ซอล)
const Note MELODY_KINGKANBAI[] PROGMEM = {
  // กิ่ง ก้าน ใบ
  {NOTE_C4, 240, 30}, {NOTE_D4, 240, 30}, {NOTE_E4, 420, 50},
  // ชะ ชะ
  {NOTE_G4, 140, 25}, {NOTE_G4, 140, 50},
  // ใบ ก้าน กิ่ง
  {NOTE_E4, 240, 30}, {NOTE_D4, 240, 30}, {NOTE_C4, 420, 80},

  // กิ่ง ก้าน ใบ
  {NOTE_C4, 240, 30}, {NOTE_D4, 240, 30}, {NOTE_E4, 420, 50},
  // ชะ ชะ
  {NOTE_G4, 140, 25}, {NOTE_G4, 140, 50},
  // ใบ ก้าน กิ่ง
  {NOTE_E4, 240, 30}, {NOTE_D4, 240, 30}, {NOTE_C4, 420, 80},

  // ฝนตกลมแรงจริงจริง
  {NOTE_G4, 160, 25}, {NOTE_G4, 160, 25}, {NOTE_A4, 160, 25}, {NOTE_G4, 160, 25}, {NOTE_E4, 160, 25}, {NOTE_G4, 350, 50},
  // ฝนตกลมแรงจริงจริง
  {NOTE_G4, 160, 25}, {NOTE_G4, 160, 25}, {NOTE_A4, 160, 25}, {NOTE_G4, 160, 25}, {NOTE_E4, 160, 25}, {NOTE_C4, 350, 50},

  // ชะ ชะ
  {NOTE_G4, 140, 25}, {NOTE_G4, 140, 50},
  // กิ่ง ก้าน ใบ
  {NOTE_C4, 240, 30}, {NOTE_D4, 240, 30}, {NOTE_C4, 550, 100}
};

// 4. รถตุ๊กตุ๊ก (ขับรถตุ๊กตุ๊ก บรรทุกถ่าน รถขึ้นสะพาน รถลงสะพาน กระตุก กระตุก กระตุก)
const Note MELODY_TUKTUK[] PROGMEM = {
  // ขับรถตุ๊กตุ๊ก
  {NOTE_C4, 180, 25}, {NOTE_D4, 180, 25}, {NOTE_E4, 180, 25}, {NOTE_G4, 320, 50},
  // บรรทุกถ่าน
  {NOTE_G4, 180, 25}, {NOTE_E4, 180, 25}, {NOTE_C4, 320, 50},
  // รถขึ้นสะพาน
  {NOTE_G4, 180, 25}, {NOTE_G4, 180, 25}, {NOTE_A4, 180, 25}, {NOTE_G4, 320, 50},
  // รถลงสะพาน
  {NOTE_E4, 180, 25}, {NOTE_E4, 180, 25}, {NOTE_F4, 180, 25}, {NOTE_E4, 320, 50},
  // รถเลี้ยวซ้าย
  {NOTE_C4, 180, 25}, {NOTE_D4, 180, 25}, {NOTE_E4, 300, 40},
  // รถเลี้ยวขวา
  {NOTE_E4, 180, 25}, {NOTE_D4, 180, 25}, {NOTE_C4, 300, 40},
  // กระตุก
  {NOTE_C4, 140, 25}, {NOTE_D4, 240, 40},
  // กระตุก
  {NOTE_C4, 140, 25}, {NOTE_D4, 240, 40},
  // กระตุก!
  {NOTE_C4, 160, 25}, {NOTE_C4, 450, 80}
};

// 5. ปรบมือ 5 ครั้ง (สันทนาการปลุกพลัง แปะ แปะ แปะ แปะ แปะ!)
const Note MELODY_CLAP5[] PROGMEM = {
  {NOTE_G4, 120, 40}, {NOTE_G4, 120, 40}, {NOTE_G4, 120, 40}, {NOTE_G4, 120, 40}, {NOTE_G4, 250, 300},
  {NOTE_C5, 120, 40}, {NOTE_C5, 120, 40}, {NOTE_C5, 120, 40}, {NOTE_C5, 120, 40}, {NOTE_C5, 350, 100}
};

// 6. Super Mario Theme (เพลงเปิดระดับตำนานที่ทุกคนจำได้ทันที)
const Note MELODY_MARIO[] PROGMEM = {
  {NOTE_E5, 90, 40}, {NOTE_E5, 90, 90}, {NOTE_E5, 90, 90}, {NOTE_C5, 90, 30},
  {NOTE_E5, 140, 60}, {NOTE_G5, 260, 120}, {NOTE_G4, 260, 150},

  // ท่อนฮิตต่อมา (โด ซอล มี)
  {NOTE_C5, 200, 80}, {NOTE_G4, 200, 80}, {NOTE_E4, 200, 80},
  {NOTE_A4, 180, 40}, {NOTE_B4, 180, 40}, {NOTE_AS4, 120, 30}, {NOTE_A4, 180, 50},
  {NOTE_G4, 140, 30}, {NOTE_E5, 160, 30}, {NOTE_G5, 160, 30}, {NOTE_A5, 220, 50}
};

// 7. Coffin Dance (Astronomia - ไวรัลแดนซ์แบกโลง)
const Note MELODY_COFFIN[] PROGMEM = {
  // อินโทรสับโน้ต
  {NOTE_A4, 110, 25}, {NOTE_A4, 110, 25}, {NOTE_A4, 110, 25}, {NOTE_A4, 110, 25},
  {NOTE_C5, 110, 25}, {NOTE_B4, 110, 25}, {NOTE_A4, 110, 25}, {NOTE_G4, 110, 25},
  {NOTE_A4, 110, 25}, {NOTE_A4, 110, 25}, {NOTE_A4, 110, 25}, {NOTE_A4, 110, 25},
  {NOTE_C5, 110, 25}, {NOTE_B4, 110, 25}, {NOTE_A4, 110, 25}, {NOTE_G4, 110, 25},

  // ท่อนฮุคดรอปแดนซ์
  {NOTE_C5, 120, 25}, {NOTE_C5, 120, 25}, {NOTE_C5, 120, 25}, {NOTE_C5, 120, 25},
  {NOTE_E5, 140, 25}, {NOTE_D5, 140, 25}, {NOTE_C5, 140, 25}, {NOTE_B4, 140, 25},
  {NOTE_C5, 120, 25}, {NOTE_C5, 120, 25}, {NOTE_C5, 120, 25}, {NOTE_C5, 120, 25},
  {NOTE_D5, 140, 25}, {NOTE_C5, 140, 25}, {NOTE_B4, 140, 25}, {NOTE_A4, 350, 80}
};

// 8. Star Wars - The Imperial March (Darth Vader's Theme)
const Note MELODY_STARWARS[] PROGMEM = {
  {NOTE_A4, 460, 50}, {NOTE_A4, 460, 50}, {NOTE_A4, 460, 50},
  {NOTE_F4, 320, 30}, {NOTE_C5, 140, 30}, {NOTE_A4, 460, 50},
  {NOTE_F4, 320, 30}, {NOTE_C5, 140, 30}, {NOTE_A4, 900, 150},

  {NOTE_E5, 460, 50}, {NOTE_E5, 460, 50}, {NOTE_E5, 460, 50},
  {NOTE_F5, 320, 30}, {NOTE_C5, 140, 30}, {NOTE_GS4, 460, 50},
  {NOTE_F4, 320, 30}, {NOTE_C5, 140, 30}, {NOTE_A4, 900, 150}
};

// 9. Baby Shark (ท่อนฮิต ดู ดู๊ ดู ดู ดู ดู๊)
const Note MELODY_BABYSHARK[] PROGMEM = {
  {NOTE_D4, 220, 30}, {NOTE_E4, 220, 40},
  {NOTE_G4, 130, 25}, {NOTE_G4, 130, 25}, {NOTE_G4, 130, 25}, {REST, 30, 10},
  {NOTE_G4, 130, 25}, {NOTE_G4, 130, 25}, {NOTE_G4, 130, 25}, {NOTE_G4, 80, 20}, {NOTE_G4, 80, 40},

  {NOTE_D4, 220, 30}, {NOTE_E4, 220, 40},
  {NOTE_G4, 130, 25}, {NOTE_G4, 130, 25}, {NOTE_G4, 130, 25}, {REST, 30, 10},
  {NOTE_G4, 130, 25}, {NOTE_G4, 130, 25}, {NOTE_G4, 130, 25}, {NOTE_G4, 80, 20}, {NOTE_G4, 80, 40},

  {NOTE_D4, 220, 30}, {NOTE_E4, 220, 40},
  {NOTE_G4, 130, 25}, {NOTE_G4, 130, 25}, {NOTE_G4, 130, 25}, {REST, 30, 10},
  {NOTE_G4, 130, 25}, {NOTE_G4, 130, 25}, {NOTE_G4, 130, 25}, {NOTE_G4, 80, 20}, {NOTE_G4, 80, 40},

  // เบบี้ชาร์ค!
  {NOTE_G4, 220, 30}, {NOTE_G4, 220, 30}, {NOTE_FS4, 500, 80}
};

// 10. Happy Birthday (สุขสันต์วันเกิด ครบทุกวรรคตรงตามโน้ตสากล)
const Note MELODY_HBD[] PROGMEM = {
  // แฮป-ปี้-เบิร์ธ-เดย์-ทู-ยู
  {NOTE_C4, 300, 30}, {NOTE_C4, 120, 25}, {NOTE_D4, 400, 35}, {NOTE_C4, 400, 35}, {NOTE_F4, 400, 35}, {NOTE_E4, 750, 80},
  // แฮป-ปี้-เบิร์ธ-เดย์-ทู-ยู
  {NOTE_C4, 300, 30}, {NOTE_C4, 120, 25}, {NOTE_D4, 400, 35}, {NOTE_C4, 400, 35}, {NOTE_G4, 400, 35}, {NOTE_F4, 750, 80},
  // แฮป-ปี้-เบิร์ธ-เดย์ แดร์...
  {NOTE_C4, 300, 30}, {NOTE_C4, 120, 25}, {NOTE_C5, 450, 35}, {NOTE_A4, 450, 35}, {NOTE_F4, 400, 35}, {NOTE_E4, 400, 35}, {NOTE_D4, 750, 80},
  // แฮป-ปี้-เบิร์ธ-เดย์-ทู-ยู
  {NOTE_AS4, 300, 30}, {NOTE_AS4, 120, 25}, {NOTE_A4, 400, 35}, {NOTE_F4, 400, 35}, {NOTE_G4, 450, 35}, {NOTE_F4, 900, 120}
};

// 11. Seven Nation Army (The White Stripes)
const Note MELODY_SEVEN[] PROGMEM = {
  {NOTE_E4, 420, 40}, {NOTE_E4, 220, 30}, {NOTE_G4, 280, 30}, {NOTE_E4, 280, 30}, {NOTE_D4, 220, 30}, {NOTE_C4, 550, 50}, {NOTE_B3, 650, 80},
  {NOTE_E4, 420, 40}, {NOTE_E4, 220, 30}, {NOTE_G4, 280, 30}, {NOTE_E4, 280, 30}, {NOTE_D4, 220, 30}, {NOTE_C4, 550, 50}, {NOTE_B3, 650, 80}
};

// 12. Doraemon Theme (โดราเอมอน: อัง อัง อัง โตเดโมะ ดาอิซูคิ โดราเอมอนนน)
const Note MELODY_DORAEMON[] PROGMEM = {
  {NOTE_C5, 200, 30}, {NOTE_D5, 200, 30}, {NOTE_E5, 280, 30}, {NOTE_G5, 280, 40},
  {NOTE_A5, 360, 30}, {NOTE_G5, 220, 30}, {NOTE_E5, 380, 50},
  {NOTE_C5, 200, 30}, {NOTE_D5, 200, 30}, {NOTE_E5, 280, 30}, {NOTE_D5, 500, 80}
};

// 13. Sound Effects (เสียงเอฟเฟกต์สั้น คมชัด)
const Note SFX_BEEP[] PROGMEM = {
  {NOTE_E5, 80, 0}
};

const Note SFX_HAPPY[] PROGMEM = {
  {NOTE_C5, 60, 20}, {NOTE_G5, 140, 0}
};

const Note SFX_FANFARE[] PROGMEM = {
  {NOTE_G4, 90, 20}, {NOTE_C5, 90, 20}, {NOTE_E5, 90, 20}, {NOTE_G5, 300, 0}
};

const Note SFX_ALARM[] PROGMEM = {
  {NOTE_A5, 90, 20}, {NOTE_E5, 90, 20}, {NOTE_A5, 90, 20}, {NOTE_E5, 90, 0}
};

// ============================================================================
// SONG REGISTRY (Open-Closed Extensible Table)
// ============================================================================
const SongItem SONG_REGISTRY[] = {
  {"numalee",    "หนูมาลีมีลูกแมว", "recreation", "🐱", MELODY_NUMALEE,   sizeof(MELODY_NUMALEE) / sizeof(Note)},
  {"kaiyang",    "ไก่ย่างถูกเผา",   "recreation", "🍗", MELODY_KAIYANG,   sizeof(MELODY_KAIYANG) / sizeof(Note)},
  {"kingkanbai", "กิ่งก้านใบ",     "recreation", "🍃", MELODY_KINGKANBAI,sizeof(MELODY_KINGKANBAI) / sizeof(Note)},
  {"tuktuk",     "รถตุ๊กตุ๊ก",       "recreation", "🛺", MELODY_TUKTUK,    sizeof(MELODY_TUKTUK) / sizeof(Note)},
  {"clap5",      "ตบมือ 5 ครั้ง",   "recreation", "👏", MELODY_CLAP5,     sizeof(MELODY_CLAP5) / sizeof(Note)},
  {"mario",      "Super Mario",    "game",       "🍄", MELODY_MARIO,     sizeof(MELODY_MARIO) / sizeof(Note)},
  {"coffin",     "Coffin Dance",   "game",       "🕺", MELODY_COFFIN,    sizeof(MELODY_COFFIN) / sizeof(Note)},
  {"starwars",   "Imperial March", "pop",        "⚔️", MELODY_STARWARS,  sizeof(MELODY_STARWARS) / sizeof(Note)},
  {"babyshark",  "Baby Shark",     "pop",        "🦈", MELODY_BABYSHARK, sizeof(MELODY_BABYSHARK) / sizeof(Note)},
  {"hbd",        "Happy Birthday", "pop",        "🎂", MELODY_HBD,       sizeof(MELODY_HBD) / sizeof(Note)},
  {"seven",      "Seven Nation",   "pop",        "🔥", MELODY_SEVEN,     sizeof(MELODY_SEVEN) / sizeof(Note)},
  {"doraemon",   "Doraemon Theme", "pop",        "🔔", MELODY_DORAEMON,  sizeof(MELODY_DORAEMON) / sizeof(Note)},
  {"sfx_beep",   "บี๊บแจ้งเตือน",   "sfx",        "🔊", SFX_BEEP,         sizeof(SFX_BEEP) / sizeof(Note)},
  {"sfx_happy",  "ทักทายสดใส",     "sfx",        "✨", SFX_HAPPY,        sizeof(SFX_HAPPY) / sizeof(Note)},
  {"sfx_fanfare","ไชโยเฉลิมฉลอง",  "sfx",        "🎉", SFX_FANFARE,      sizeof(SFX_FANFARE) / sizeof(Note)},
  {"sfx_alarm",  "เตือนภัยไซเรน",  "sfx",        "🚨", SFX_ALARM,        sizeof(SFX_ALARM) / sizeof(Note)}
};

const size_t SONG_REGISTRY_COUNT = sizeof(SONG_REGISTRY) / sizeof(SongItem);

// ============================================================================
// NON-BLOCKING SONG PLAYER (State Machine with Clear Articulation)
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
    _currentDuration(0),
    _currentPause(0) {}

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
      if (now - _timer >= _currentDuration) {
        noTone(_pin);
        if (_currentPause > 0) {
          _state = PAUSING;
          _timer = now;
        } else {
          _advanceNote();
        }
      }
    } else if (_state == PAUSING) {
      if (now - _timer >= _currentPause) {
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
  uint16_t _currentDuration;
  uint16_t _currentPause;

  void _startCurrentNote() {
    if (!_song || _noteIdx >= _song->length) {
      stop();
      return;
    }
    Note note;
    memcpy_P(&note, &_song->notes[_noteIdx], sizeof(Note));
    _currentDuration = note.duration;
    _currentPause = note.pause;
    _timer = millis();

    if (note.freq > 0) {
      tone(_pin, note.freq);
    } else {
      noTone(_pin);
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
