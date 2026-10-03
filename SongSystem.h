#ifndef SONG_SYSTEM_H
#define SONG_SYSTEM_H

#include <Arduino.h>
#include "Config.h"

// ============================================================================
// STANDARD PITCH DEFINITIONS (Hz)
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
  uint16_t duration;  // Active sound duration in ms
  uint16_t pause;     // Silence duration before next note in ms
};

struct SongItem {
  const char* id;        // Machine identifier for API / dispatching
  const char* title;     // Human-readable title
  const char* category;  // "recreation", "pop", "game", "sfx"
  const char* icon;      // Emoji icon for modern web UI
  const Note* notes;     // Note sequence in flash memory
  size_t length;         // Note count
};

// ============================================================================
// MELODY DEFINITIONS (PROGMEM flash storage to optimize RAM)
// ============================================================================

// 1. เพลงไก่ย่างถูกเผา (สันทนาการ)
const Note MELODY_KAIYANG[] PROGMEM = {
  {NOTE_C4, 180, 20}, {NOTE_D4, 180, 20}, {NOTE_E4, 180, 20}, {NOTE_F4, 180, 20},
  {NOTE_G4, 240, 20}, {NOTE_G4, 180, 20}, {NOTE_G4, 240, 20}, {NOTE_F4, 180, 20},
  {NOTE_E4, 180, 20}, {NOTE_D4, 180, 20}, {NOTE_C4, 320, 40},
  {NOTE_C4, 180, 20}, {NOTE_D4, 180, 20}, {NOTE_E4, 180, 20}, {NOTE_F4, 180, 20},
  {NOTE_G4, 240, 20}, {NOTE_G4, 180, 20}, {NOTE_G4, 240, 20}, {NOTE_F4, 180, 20},
  {NOTE_E4, 180, 20}, {NOTE_D4, 180, 20}, {NOTE_C4, 400, 50}
};

// 2. เพลงกิ่งก้านใบ (สันทนาการยอดนิยม)
const Note MELODY_KINGKANBAI[] PROGMEM = {
  {NOTE_G4, 220, 25}, {NOTE_E4, 220, 25}, {NOTE_C4, 260, 40}, // กิ่ง ก้าน ใบ
  {NOTE_G4, 140, 20}, {NOTE_G4, 140, 40},                      // ชะ ชะ
  {NOTE_C4, 220, 25}, {NOTE_E4, 220, 25}, {NOTE_G4, 260, 50}, // ใบ ก้าน กิ่ง
  {NOTE_G4, 160, 20}, {NOTE_A4, 160, 20}, {NOTE_G4, 160, 20}, {NOTE_E4, 260, 30}, // ฝนตกลมแรง
  {NOTE_G4, 160, 20}, {NOTE_A4, 160, 20}, {NOTE_G4, 160, 20}, {NOTE_E4, 260, 40},
  {NOTE_G4, 140, 20}, {NOTE_G4, 140, 30},                      // ชะ ชะ
  {NOTE_G4, 220, 20}, {NOTE_E4, 220, 20}, {NOTE_C4, 380, 50}  // กิ่ง ก้าน ใบ
};

// 3. เพลงรถตุ๊กตุ๊ก (สันทนาการ)
const Note MELODY_TUKTUK[] PROGMEM = {
  {NOTE_C4, 180, 20}, {NOTE_C4, 180, 20}, {NOTE_E4, 180, 20}, {NOTE_G4, 260, 30}, // ขับรถตุ๊กตุ๊ก
  {NOTE_G4, 180, 20}, {NOTE_E4, 180, 20}, {NOTE_C4, 260, 30},                      // บรรทุกถ่าน
  {NOTE_G4, 180, 20}, {NOTE_A4, 180, 20}, {NOTE_G4, 260, 30},                      // รถขึ้นสะพาน
  {NOTE_E4, 180, 20}, {NOTE_F4, 180, 20}, {NOTE_E4, 260, 30},                      // รถลงสะพาน
  {NOTE_C4, 160, 20}, {NOTE_D4, 160, 20}, {NOTE_E4, 260, 20},                      // กระตุก
  {NOTE_D4, 160, 20}, {NOTE_C4, 260, 20}, {NOTE_C4, 320, 50}                       // กระตุก กระตุก
};

// 4. จังหวะตบมือ 5 ครั้ง (สันทนาการปลุกใจ)
const Note MELODY_CLAP5[] PROGMEM = {
  {NOTE_G4, 120, 30}, {NOTE_G4, 120, 30}, {NOTE_G4, 120, 30}, {NOTE_G4, 120, 30}, {NOTE_G4, 180, 180},
  {NOTE_C5, 120, 30}, {NOTE_C5, 120, 30}, {NOTE_C5, 120, 30}, {NOTE_C5, 120, 30}, {NOTE_C5, 240, 50}
};

// 5. Super Mario Theme
const Note MELODY_MARIO[] PROGMEM = {
  {NOTE_E5, 90, 20}, {NOTE_E5, 90, 20}, {REST, 80, 10}, {NOTE_E5, 90, 20},
  {REST, 80, 10},    {NOTE_C5, 90, 20}, {NOTE_E5, 90, 20}, {NOTE_G5, 150, 40},
  {REST, 90, 10},    {NOTE_G4, 150, 40}
};

// 6. Coffin Dance (Astronomia)
const Note MELODY_COFFIN[] PROGMEM = {
  {NOTE_A4, 120, 20}, {NOTE_A4, 120, 20}, {NOTE_A4, 120, 20}, {NOTE_C5, 120, 20},
  {NOTE_E5, 140, 20}, {NOTE_D5, 140, 20}, {NOTE_C5, 140, 20}, {NOTE_A4, 200, 30},
  {NOTE_A4, 120, 20}, {NOTE_C5, 120, 20}, {NOTE_D5, 140, 20}, {NOTE_E5, 240, 40}
};

// 7. Star Wars - The Imperial March
const Note MELODY_STARWARS[] PROGMEM = {
  {NOTE_A4, 350, 30}, {NOTE_A4, 350, 30}, {NOTE_A4, 350, 30},
  {NOTE_F4, 250, 20}, {NOTE_C5, 120, 20}, {NOTE_A4, 400, 50}
};

// 8. Baby Shark
const Note MELODY_BABYSHARK[] PROGMEM = {
  {NOTE_D4, 140, 20}, {NOTE_E4, 140, 20}, {NOTE_G4, 110, 20}, {NOTE_G4, 110, 20},
  {NOTE_G4, 110, 20}, {NOTE_G4, 70,  20}, {NOTE_G4, 110, 20}, {NOTE_G4, 110, 20},
  {NOTE_G4, 180, 30}, {NOTE_D4, 140, 20}, {NOTE_E4, 140, 20}, {NOTE_G4, 260, 40}
};

// 9. Happy Birthday
const Note MELODY_HBD[] PROGMEM = {
  {NOTE_C4, 180, 20}, {NOTE_C4, 100, 20}, {NOTE_D4, 300, 20},
  {NOTE_C4, 300, 20}, {NOTE_F4, 300, 20}, {NOTE_E4, 500, 40}
};

// 10. Seven Nation Army
const Note MELODY_SEVEN[] PROGMEM = {
  {NOTE_E4, 260, 30}, {NOTE_E4, 140, 20}, {NOTE_G4, 180, 20},
  {NOTE_E4, 180, 20}, {NOTE_D4, 140, 20}, {NOTE_C4, 260, 30}, {NOTE_B4, 350, 40}
};

// 11. Doraemon Theme (ท่อนฮุค)
const Note MELODY_DORAEMON[] PROGMEM = {
  {NOTE_C5, 180, 20}, {NOTE_D5, 180, 20}, {NOTE_E5, 240, 20}, {NOTE_G5, 240, 20},
  {NOTE_A5, 300, 20}, {NOTE_G5, 200, 20}, {NOTE_E5, 300, 30},
  {NOTE_C5, 180, 20}, {NOTE_D5, 180, 20}, {NOTE_E5, 240, 20}, {NOTE_D5, 380, 50}
};

// 12. Sound Effects
const Note SFX_BEEP[] PROGMEM = {
  {NOTE_E5, 80, 0}
};

const Note SFX_HAPPY[] PROGMEM = {
  {NOTE_C5, 60, 20}, {NOTE_G5, 120, 0}
};

const Note SFX_FANFARE[] PROGMEM = {
  {NOTE_G4, 100, 20}, {NOTE_C5, 100, 20}, {NOTE_E5, 100, 20}, {NOTE_G5, 250, 0}
};

const Note SFX_ALARM[] PROGMEM = {
  {NOTE_A5, 100, 20}, {NOTE_E5, 100, 20}, {NOTE_A5, 100, 20}, {NOTE_E5, 100, 0}
};

// ============================================================================
// SONG REGISTRY (Scalable, Extensible, Open-Closed Principle)
// ============================================================================
// To add a new song: Simply add one row to this table! The Web UI and API will automatically update!
const SongItem SONG_REGISTRY[] = {
  {"kaiyang",    "ไก่ย่างถูกเผา",   "recreation", "🍗", MELODY_KAIYANG,    sizeof(MELODY_KAIYANG) / sizeof(Note)},
  {"kingkanbai", "กิ่งก้านใบ",     "recreation", "🍃", MELODY_KINGKANBAI, sizeof(MELODY_KINGKANBAI) / sizeof(Note)},
  {"tuktuk",     "รถตุ๊กตุ๊ก",       "recreation", "🛺", MELODY_TUKTUK,     sizeof(MELODY_TUKTUK) / sizeof(Note)},
  {"clap5",      "ตบมือ 5 ครั้ง",   "recreation", "👏", MELODY_CLAP5,      sizeof(MELODY_CLAP5) / sizeof(Note)},
  {"mario",      "Super Mario",    "game",       "🍄", MELODY_MARIO,      sizeof(MELODY_MARIO) / sizeof(Note)},
  {"coffin",     "Coffin Dance",   "game",       "🕺", MELODY_COFFIN,     sizeof(MELODY_COFFIN) / sizeof(Note)},
  {"starwars",   "Imperial March", "pop",        "⚔️", MELODY_STARWARS,   sizeof(MELODY_STARWARS) / sizeof(Note)},
  {"babyshark",  "Baby Shark",     "pop",        "🦈", MELODY_BABYSHARK,  sizeof(MELODY_BABYSHARK) / sizeof(Note)},
  {"hbd",        "Happy Birthday", "pop",        "🎂", MELODY_HBD,        sizeof(MELODY_HBD) / sizeof(Note)},
  {"seven",      "Seven Nation",   "pop",        "🔥", MELODY_SEVEN,      sizeof(MELODY_SEVEN) / sizeof(Note)},
  {"doraemon",   "Doraemon Theme", "pop",        "🐱", MELODY_DORAEMON,   sizeof(MELODY_DORAEMON) / sizeof(Note)},
  {"sfx_beep",   "บี๊บแจ้งเตือน",   "sfx",        "🔊", SFX_BEEP,          sizeof(SFX_BEEP) / sizeof(Note)},
  {"sfx_happy",  "ทักทายสดใส",     "sfx",        "✨", SFX_HAPPY,         sizeof(SFX_HAPPY) / sizeof(Note)},
  {"sfx_fanfare","ไชโยเฉลิมฉลอง",  "sfx",        "🎉", SFX_FANFARE,       sizeof(SFX_FANFARE) / sizeof(Note)},
  {"sfx_alarm",  "เตือนภัยไซเรน",  "sfx",        "🚨", SFX_ALARM,         sizeof(SFX_ALARM) / sizeof(Note)}
};

const size_t SONG_REGISTRY_COUNT = sizeof(SONG_REGISTRY) / sizeof(SongItem);

// ============================================================================
// NON-BLOCKING SONG PLAYER (Computer Science State Machine)
// ============================================================================
class SongPlayer {
public:
  enum State { IDLE, SOUNDING, PAUSING };

  SongPlayer(uint8_t buzzerPin) : _pin(buzzerPin), _state(IDLE), _song(nullptr), _noteIdx(0), _timer(0) {}

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

  // Quick tone helper for instant audio cues
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
