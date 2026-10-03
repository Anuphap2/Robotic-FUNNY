#ifndef SONGS_H
#define SONGS_H

#include <Arduino.h>

// ขา Buzzer อ้างอิงจากโค้ดหลัก
extern const int PIN_BUZZER;

// ================= ตารางความถี่ตัวโน้ตมาตรฐาน (Hz) =================
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
#define REST     0    // เว้นวรรค / หยุดเสียง

// ================= ฟังก์ชันพื้นฐานสำหรับบรรเลงโน้ต =================
inline void playTone(int freq, int durationMs, int pauseMs = 25) {
  if (freq > 0) {
    tone(PIN_BUZZER, freq);
  } else {
    noTone(PIN_BUZZER);
  }
  delay(durationMs);
  noTone(PIN_BUZZER);
  delay(pauseMs);
}

// ================= รายการเพลง (Jukebox) =================

// 1. เพลงไก่ย่างถูกเผา
inline void playKaiYang() {
  int notes[] = { NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4, NOTE_C4 };
  int times[] = { 180,     180,     180,     180,     250,     180,     250,     180,     180,     180,     350 };
  int len = sizeof(notes) / sizeof(notes[0]);
  for (int i = 0; i < len; i++) playTone(notes[i], times[i]);
}

// 2. Super Mario Theme (ท่อนฮิต)
inline void playMario() {
  int notes[] = { NOTE_E5, NOTE_E5, REST,    NOTE_E5, REST,    NOTE_C5, NOTE_E5, NOTE_G5, REST,    NOTE_G4 };
  int times[] = { 90,      90,      90,      90,      90,      90,      90,      140,     100,     140 };
  int len = sizeof(notes) / sizeof(notes[0]);
  for (int i = 0; i < len; i++) playTone(notes[i], times[i], 20);
}

// 3. Coffin Dance (Astronomia)
inline void playCoffin() {
  int notes[] = { NOTE_A4, NOTE_A4, NOTE_A4, NOTE_C5, NOTE_E5, NOTE_D5, NOTE_C5, NOTE_A4 };
  int times[] = { 120,     120,     120,     120,     140,     140,     140,     200 };
  int len = sizeof(notes) / sizeof(notes[0]);
  for (int i = 0; i < len; i++) playTone(notes[i], times[i]);
}

// 4. Star Wars - The Imperial March
inline void playStarWars() {
  int notes[] = { NOTE_A4, NOTE_A4, NOTE_A4, NOTE_F4, NOTE_C5, NOTE_A4 };
  int times[] = { 350,     350,     350,     250,     120,     400 };
  int len = sizeof(notes) / sizeof(notes[0]);
  for (int i = 0; i < len; i++) playTone(notes[i], times[i]);
}

// 5. Baby Shark (ท่อนฮุค)
inline void playBabyShark() {
  int notes[] = { NOTE_D4, NOTE_E4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_G4 };
  int times[] = { 140,     140,     110,     110,     110,     70,      110,     110,     180 };
  int len = sizeof(notes) / sizeof(notes[0]);
  for (int i = 0; i < len; i++) playTone(notes[i], times[i]);
}

// 6. เพลงวันเกิด (Happy Birthday)
inline void playHBD() {
  int notes[] = { NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_F4, NOTE_E4 };
  int times[] = { 180,     100,     300,     300,     300,     500 };
  int len = sizeof(notes) / sizeof(notes[0]);
  for (int i = 0; i < len; i++) playTone(notes[i], times[i]);
}

// 7. Seven Nation Army
inline void playSevenNation() {
  int notes[] = { NOTE_E4, NOTE_E4, NOTE_G4, NOTE_E4, NOTE_D4, NOTE_C4, NOTE_B4 };
  int times[] = { 260,     140,     180,     180,     140,     260,     350 };
  int len = sizeof(notes) / sizeof(notes[0]);
  for (int i = 0; i < len; i++) playTone(notes[i], times[i]);
}

// ================= เสียง Sound Effects พิเศษ =================
inline void playBeepShort() {
  playTone(NOTE_E5, 80, 0);
}

inline void playHappyChirp() {
  playTone(NOTE_C5, 60, 20);
  playTone(NOTE_G5, 120, 0);
}

#endif