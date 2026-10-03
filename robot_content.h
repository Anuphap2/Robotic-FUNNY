#ifndef ROBOT_CONTENT_H
#define ROBOT_CONTENT_H

#include <Arduino.h>
#include <ESP32Servo.h>

// อ้างอิงตัวแปรฮาร์ดแวร์จากไฟล์หลัก sketch_oct2a.ino
extern const int PIN_BUZZER;
extern Servo servoHead;
extern Servo servoArmL;
extern Servo servoArmR;

// ================= 1. โน้ตดนตรีมาตรฐาน (Hz) =================
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define REST     0

struct Note {
  int freq;
  int duration;
};

struct SongItem {
  const char* cmd;
  const Note* notes;
  size_t length;
};

// --- รายการตัวโน้ตของแต่ละเพลง ---
const Note song_kaiyang[] = {
  {NOTE_C4, 180}, {NOTE_D4, 180}, {NOTE_E4, 180}, {NOTE_F4, 180},
  {NOTE_G4, 250}, {NOTE_G4, 180}, {NOTE_G4, 250}, {NOTE_F4, 180},
  {NOTE_E4, 180}, {NOTE_D4, 180}, {NOTE_C4, 350}
};

const Note song_mario[] = {
  {NOTE_E5, 90}, {NOTE_E5, 90}, {REST, 90}, {NOTE_E5, 90},
  {REST, 90},    {NOTE_C5, 90}, {NOTE_E5, 90}, {NOTE_G5, 140},
  {REST, 100},   {NOTE_G4, 140}
};

const Note song_coffin[] = {
  {NOTE_A4, 120}, {NOTE_A4, 120}, {NOTE_A4, 120}, {NOTE_C5, 120},
  {NOTE_E5, 140}, {NOTE_D5, 140}, {NOTE_C5, 140}, {NOTE_A4, 200}
};

const Note song_starwars[] = {
  {NOTE_A4, 350}, {NOTE_A4, 350}, {NOTE_A4, 350},
  {NOTE_F4, 250}, {NOTE_C5, 120}, {NOTE_A4, 400}
};

const Note song_babyshark[] = {
  {NOTE_D4, 140}, {NOTE_E4, 140}, {NOTE_G4, 110}, {NOTE_G4, 110},
  {NOTE_G4, 110}, {NOTE_G4, 70},  {NOTE_G4, 110}, {NOTE_G4, 110}, {NOTE_G4, 180}
};

const Note song_hbd[] = {
  {NOTE_C4, 180}, {NOTE_C4, 100}, {NOTE_D4, 300},
  {NOTE_C4, 300}, {NOTE_F4, 300}, {NOTE_E4, 500}
};

const Note song_seven[] = {
  {NOTE_E4, 260}, {NOTE_E4, 140}, {NOTE_G4, 180},
  {NOTE_E4, 180}, {NOTE_D4, 140}, {NOTE_C4, 260}, {NOTE_B4, 350}
};

const Note snd_beep[] = {
  {NOTE_E5, 80}
};

const Note snd_happy[] = {
  {NOTE_C5, 60}, {NOTE_G5, 120}
};

// ================= ตารางลงทะเบียนเพลง (Song Registry) =================
const SongItem SONG_REGISTRY[] = {
  {"kaiyang",   song_kaiyang,   sizeof(song_kaiyang) / sizeof(Note)},
  {"mario",     song_mario,     sizeof(song_mario) / sizeof(Note)},
  {"coffin",    song_coffin,    sizeof(song_coffin) / sizeof(Note)},
  {"starwars",  song_starwars,  sizeof(song_starwars) / sizeof(Note)},
  {"babyshark", song_babyshark, sizeof(song_babyshark) / sizeof(Note)},
  {"hbd",       song_hbd,       sizeof(song_hbd) / sizeof(Note)},
  {"seven",     song_seven,     sizeof(song_seven) / sizeof(Note)},
  {"beep",      snd_beep,       sizeof(snd_beep) / sizeof(Note)},
  {"snd_happy", snd_happy,      sizeof(snd_happy) / sizeof(Note)}
};

const size_t SONG_COUNT = sizeof(SONG_REGISTRY) / sizeof(SongItem);

// ฟังก์ชันเล่นเพลงตามชื่อ cmd ที่ส่งมาจากหน้าเว็บ
inline bool dispatchSong(const String& cmd) {
  for (size_t i = 0; i < SONG_COUNT; i++) {
    if (cmd == SONG_REGISTRY[i].cmd) {
      for (size_t j = 0; j < SONG_REGISTRY[i].length; j++) {
        int f = SONG_REGISTRY[i].notes[j].freq;
        int d = SONG_REGISTRY[i].notes[j].duration;
        if (f > 0) tone(PIN_BUZZER, f);
        else noTone(PIN_BUZZER);
        delay(d);
        noTone(PIN_BUZZER);
        delay(25);
      }
      return true;
    }
  }
  return false;
}

// ================= 2. ระบบท่าทาง Servo (Motion Registry) =================
struct ServoFrame {
  int head;
  int armL;
  int armR;
  int holdMs;
};

struct MotionItem {
  const char* cmd;
  const ServoFrame* frames;
  size_t length;
};

// เฟรมท่าเต้น (Dance)
const ServoFrame motion_dance[] = {
  {60,  110, 150, 200},
  {120, 30,  70,  200},
  {60,  110, 150, 200},
  {120, 30,  70,  200},
  {90,  30,  150, 200}
};

// เฟรมท่ายืนตรง (Center)
const ServoFrame motion_center[] = {
  {90, 30, 150, 200}
};

// ================= ตารางลงทะเบียนท่าทาง (Motion Registry) =================
const MotionItem MOTION_REGISTRY[] = {
  {"dance",  motion_dance,  sizeof(motion_dance) / sizeof(ServoFrame)},
  {"center", motion_center, sizeof(motion_center) / sizeof(ServoFrame)}
};

const size_t MOTION_COUNT = sizeof(MOTION_REGISTRY) / sizeof(MotionItem);

// ฟังก์ชันขยับท่าทางตามชื่อ cmd ที่ส่งมาจากหน้าเว็บ
inline bool dispatchMotion(const String& cmd) {
  for (size_t i = 0; i < MOTION_COUNT; i++) {
    if (cmd == MOTION_REGISTRY[i].cmd) {
      for (size_t j = 0; j < MOTION_REGISTRY[i].length; j++) {
        servoHead.write(MOTION_REGISTRY[i].frames[j].head);
        servoArmL.write(MOTION_REGISTRY[i].frames[j].armL);
        servoArmR.write(MOTION_REGISTRY[i].frames[j].armR);
        delay(MOTION_REGISTRY[i].frames[j].holdMs);
      }
      return true;
    }
  }
  return false;
}

#endif