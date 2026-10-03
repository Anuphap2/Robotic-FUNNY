#ifndef FACE_SYSTEM_H
#define FACE_SYSTEM_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "Config.h"

// ============================================================================
// FACE RENDERING FUNCTIONS (Modular Computer Science Vector Graphics)
// ============================================================================
typedef void (*FaceRenderFn)(Adafruit_SH1106G& d, int phase);

// 1. Normal (ตาแป๋วสดใส มีไฮไลท์ประกายตา)
inline void drawFaceNormal(Adafruit_SH1106G& d, int phase) {
  // Left Eye
  d.fillRoundRect(22, 16, 28, 32, 14, SH110X_WHITE);
  d.fillCircle(28, 24, 4, SH110X_BLACK); // Catchlight inside
  d.fillCircle(40, 36, 2, SH110X_BLACK);

  // Right Eye
  d.fillRoundRect(78, 16, 28, 32, 14, SH110X_WHITE);
  d.fillCircle(84, 24, 4, SH110X_BLACK);
  d.fillCircle(96, 36, 2, SH110X_BLACK);

  // Cute Little Smile
  d.drawPixel(63, 44, SH110X_WHITE);
  d.drawPixel(64, 44, SH110X_WHITE);
  d.drawPixel(62, 43, SH110X_WHITE);
  d.drawPixel(65, 43, SH110X_WHITE);
}

// 2. Happy (ตายิ้มสระอิ มีแก้มชมพู)
inline void drawFaceHappy(Adafruit_SH1106G& d, int phase) {
  // Left Happy Arch
  d.fillRoundRect(20, 22, 32, 14, 7, SH110X_WHITE);
  d.fillRoundRect(20, 28, 32, 14, 7, SH110X_BLACK);

  // Right Happy Arch
  d.fillRoundRect(76, 22, 32, 14, 7, SH110X_WHITE);
  d.fillRoundRect(76, 28, 32, 14, 7, SH110X_BLACK);

  // Blushing Cheeks
  d.drawLine(16, 44, 22, 38, SH110X_WHITE);
  d.drawLine(20, 44, 26, 38, SH110X_WHITE);
  d.drawLine(102, 44, 108, 38, SH110X_WHITE);
  d.drawLine(106, 44, 112, 38, SH110X_WHITE);

  // Big Happy Smile
  d.drawCircle(64, 42, 8, SH110X_WHITE);
  d.fillRect(54, 34, 20, 9, SH110X_BLACK);
}

// 3. Blink (กระพริบตา เส้นเรียวยาว)
inline void drawFaceBlink(Adafruit_SH1106G& d, int phase) {
  d.fillRoundRect(20, 31, 32, 4, 2, SH110X_WHITE);
  d.fillRoundRect(76, 31, 32, 4, 2, SH110X_WHITE);
}

// 4. Wink (วิ้งขยิบตาข้างเดียว ยิ้มหวาน)
inline void drawFaceWink(Adafruit_SH1106G& d, int phase) {
  // Left eye: open Sparkle
  d.fillRoundRect(22, 16, 28, 32, 14, SH110X_WHITE);
  d.fillCircle(28, 24, 4, SH110X_BLACK);
  // Right eye: wink line
  d.fillRoundRect(76, 24, 32, 12, 6, SH110X_WHITE);
  d.fillRoundRect(76, 30, 32, 12, 6, SH110X_BLACK);
  // Wink star sparkle
  d.drawLine(112, 22, 120, 22, SH110X_WHITE);
  d.drawLine(116, 18, 116, 26, SH110X_WHITE);
}

// 5. Love (ตาหัวใจเต้นตึกตัก)
inline void drawFaceLove(Adafruit_SH1106G& d, int phase) {
  int offset = (phase % 2 == 0) ? 0 : 1;
  // Left Heart
  d.fillCircle(28 - offset, 24, 8 + offset, SH110X_WHITE);
  d.fillCircle(44 + offset, 24, 8 + offset, SH110X_WHITE);
  d.fillTriangle(20 - offset*2, 28, 52 + offset*2, 28, 36, 46 + offset*2, SH110X_WHITE);

  // Right Heart
  d.fillCircle(84 - offset, 24, 8 + offset, SH110X_WHITE);
  d.fillCircle(100 + offset, 24, 8 + offset, SH110X_WHITE);
  d.fillTriangle(76 - offset*2, 28, 108 + offset*2, 28, 92, 46 + offset*2, SH110X_WHITE);
}

// 6. Surprise (ตาโต ปากหวอ O_O)
inline void drawFaceSurprise(Adafruit_SH1106G& d, int phase) {
  d.drawCircle(36, 28, 18, SH110X_WHITE);
  d.drawCircle(36, 28, 17, SH110X_WHITE);
  d.fillCircle(36, 28, 6, SH110X_WHITE);

  d.drawCircle(92, 28, 18, SH110X_WHITE);
  d.drawCircle(92, 28, 17, SH110X_WHITE);
  d.fillCircle(92, 28, 6, SH110X_WHITE);

  // Open mouth
  d.fillCircle(64, 48, 7, SH110X_WHITE);
}

// 7. Music (ตาโน้ตดนตรี สันทนาการร้องเพลง)
inline void drawFaceMusic(Adafruit_SH1106G& d, int phase) {
  int yBounce = (phase % 2 == 0) ? 0 : -3;
  // Left Musical Note
  d.fillCircle(30, 36 + yBounce, 6, SH110X_WHITE);
  d.fillRect(34, 18 + yBounce, 4, 18, SH110X_WHITE);
  d.fillRect(34, 18 + yBounce, 12, 4, SH110X_WHITE);

  // Right Double Note
  d.fillCircle(82, 36 - yBounce, 5, SH110X_WHITE);
  d.fillCircle(102, 33 - yBounce, 5, SH110X_WHITE);
  d.fillRect(85, 18 - yBounce, 3, 18, SH110X_WHITE);
  d.fillRect(105, 15 - yBounce, 3, 18, SH110X_WHITE);
  d.fillRect(85, 15 - yBounce, 23, 4, SH110X_WHITE);

  // Singing open mouth
  d.fillCircle(64, 46, 5, SH110X_WHITE);
}

// 8. Cool (ใส่แว่นกันแดดสุดเท่)
inline void drawFaceCool(Adafruit_SH1106G& d, int phase) {
  // Left Glass
  d.fillRoundRect(16, 20, 42, 24, 6, SH110X_WHITE);
  d.drawLine(20, 24, 34, 38, SH110X_BLACK); // Sheen reflection
  d.drawLine(21, 24, 35, 38, SH110X_BLACK);

  // Right Glass
  d.fillRoundRect(70, 20, 42, 24, 6, SH110X_WHITE);
  d.drawLine(74, 24, 88, 38, SH110X_BLACK);
  d.drawLine(75, 24, 89, 38, SH110X_BLACK);

  // Glasses Bridge & Frame Arms
  d.fillRect(58, 24, 12, 4, SH110X_WHITE);
  d.fillRect(6, 22, 10, 3, SH110X_WHITE);
  d.fillRect(112, 22, 10, 3, SH110X_WHITE);

  // Smirk
  d.drawLine(60, 50, 70, 47, SH110X_WHITE);
}

// 9. Angry (คิ้วดุดัน เอาจริงเอาจัง)
inline void drawFaceAngry(Adafruit_SH1106G& d, int phase) {
  // Left Eye & Brow
  d.fillTriangle(16, 16, 52, 28, 16, 42, SH110X_WHITE);
  d.fillCircle(38, 28, 4, SH110X_BLACK);

  // Right Eye & Brow
  d.fillTriangle(112, 16, 76, 28, 112, 42, SH110X_WHITE);
  d.fillCircle(90, 28, 4, SH110X_BLACK);

  // Gritting mouth
  d.drawRect(54, 46, 20, 6, SH110X_WHITE);
  d.drawLine(58, 46, 58, 52, SH110X_WHITE);
  d.drawLine(64, 46, 64, 52, SH110X_WHITE);
  d.drawLine(70, 46, 70, 52, SH110X_WHITE);
}

// 10. Sleepy (ตาง่วงนอน zZZ)
inline void drawFaceSleepy(Adafruit_SH1106G& d, int phase) {
  // Droopy eyes
  d.fillRoundRect(20, 28, 32, 10, 5, SH110X_WHITE);
  d.fillRoundRect(20, 34, 32, 10, 5, SH110X_BLACK);
  d.fillRoundRect(76, 28, 32, 10, 5, SH110X_WHITE);
  d.fillRoundRect(76, 34, 32, 10, 5, SH110X_BLACK);

  // Floating 'z' and 'Z'
  d.setTextSize(1);
  d.setTextColor(SH110X_WHITE);
  d.setCursor(phase % 2 == 0 ? 112 : 110, 12);
  d.print("z");
  d.setTextSize(2);
  d.setCursor(phase % 2 == 0 ? 116 : 118, 0);
  d.print("Z");
}

// 11. Dizzy (ตาหมุนเกลียว วิงเวียน)
inline void drawFaceDizzy(Adafruit_SH1106G& d, int phase) {
  // Left Spiral Cross
  d.drawLine(20, 18, 52, 46, SH110X_WHITE);
  d.drawLine(20, 46, 52, 18, SH110X_WHITE);
  d.drawCircle(36, 32, 14, SH110X_WHITE);

  // Right Spiral Cross
  d.drawLine(76, 18, 108, 46, SH110X_WHITE);
  d.drawLine(76, 46, 108, 18, SH110X_WHITE);
  d.drawCircle(92, 32, 14, SH110X_WHITE);

  // Wobbly mouth
  d.drawPixel(60, 48, SH110X_WHITE);
  d.drawPixel(62, 50, SH110X_WHITE);
  d.drawPixel(64, 48, SH110X_WHITE);
  d.drawPixel(66, 50, SH110X_WHITE);
  d.drawPixel(68, 48, SH110X_WHITE);
}

// 12. Look Left
inline void drawFaceLookLeft(Adafruit_SH1106G& d, int phase) {
  d.drawCircle(36, 30, 16, SH110X_WHITE);
  d.drawCircle(92, 30, 16, SH110X_WHITE);
  d.fillCircle(26, 30, 7, SH110X_WHITE);
  d.fillCircle(82, 30, 7, SH110X_WHITE);
}

// 13. Look Right
inline void drawFaceLookRight(Adafruit_SH1106G& d, int phase) {
  d.drawCircle(36, 30, 16, SH110X_WHITE);
  d.drawCircle(92, 30, 16, SH110X_WHITE);
  d.fillCircle(46, 30, 7, SH110X_WHITE);
  d.fillCircle(102, 30, 7, SH110X_WHITE);
}

// 14. SIX-SEVEN Meme Face (มีมตัวเลข 6 และ 7 บนดวงตาสองข้างสลับเด้งขึ้นลง)
inline void drawFaceSixSeven(Adafruit_SH1106G& d, int phase) {
  int yBounceL = (phase % 2 == 0) ? -3 : 3;
  int yBounceR = (phase % 2 == 0) ? 3 : -3;

  // Left Eye: Badge with "6"
  d.fillRoundRect(18, 14 + yBounceL, 38, 36, 8, SH110X_WHITE);
  d.fillRoundRect(21, 17 + yBounceL, 32, 30, 6, SH110X_BLACK);
  d.setTextSize(3);
  d.setTextColor(SH110X_WHITE);
  d.setCursor(29, 21 + yBounceL);
  d.print("6");

  // Right Eye: Badge with "7"
  d.fillRoundRect(72, 14 + yBounceR, 38, 36, 8, SH110X_WHITE);
  d.fillRoundRect(75, 17 + yBounceR, 32, 30, 6, SH110X_BLACK);
  d.setTextSize(3);
  d.setTextColor(SH110X_WHITE);
  d.setCursor(83, 21 + yBounceR);
  d.print("7");

  // Swag Drill Smirk
  d.drawLine(56, 54, 72, 51, SH110X_WHITE);
  d.drawLine(56, 55, 72, 52, SH110X_WHITE);
}

// ============================================================================
// FACE REGISTRY (Computer Science Extensible Table)
// ============================================================================
// To add a new expression: Simply write a draw function and register it here!
struct FaceItem {
  const char* id;
  const char* title;
  const char* icon;
  FaceRenderFn render;
};

const FaceItem FACE_REGISTRY[] = {
  {"normal",     "ตาปกติสดใส",   "👀", drawFaceNormal},
  {"six_seven",  "มีม 6-7 (Six-Seven)", "🤙", drawFaceSixSeven},
  {"happy",      "ยิ้มมีความสุข",  "😊", drawFaceHappy},
  {"wink",       "วิ้งตาหวาน",    "😉", drawFaceWink},
  {"love",       "หัวใจเต้นตึกตัก", "💖", drawFaceLove},
  {"surprise",   "ตาโตตกใจ",     "😲", drawFaceSurprise},
  {"music",      "ดนตรีสันทนาการ", "🎵", drawFaceMusic},
  {"cool",       "แว่นตาสุดเท่",   "😎", drawFaceCool},
  {"blink",      "กระพริบตา",     "😑", drawFaceBlink},
  {"angry",      "ดุดันจริงจัง",   "😡", drawFaceAngry},
  {"sleepy",     "ง่วงนอน",       "😴", drawFaceSleepy},
  {"dizzy",      "มึนหัวตาลาย",   "🌀", drawFaceDizzy},
  {"look_left",  "มองซ้าย",       "👈", drawFaceLookLeft},
  {"look_right", "มองขวา",       "👉", drawFaceLookRight}
};

const size_t FACE_REGISTRY_COUNT = sizeof(FACE_REGISTRY) / sizeof(FaceItem);

// ============================================================================
// FACE ENGINE CLASS (Handles rendering, smooth animations, auto-blinking)
// ============================================================================
class FaceEngine {
public:
  FaceEngine() :
    _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
    _currentIdx(0),
    _tempIdx(-1),
    _tempUntil(0),
    _animPhase(0),
    _lastAnim(0),
    _nextBlink(0),
    _blinking(false),
    _blinkEnd(0) {}

  bool begin() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    if (!_display.begin(OLED_I2C_ADDR, true)) {
      return false;
    }
    _display.clearDisplay();
    _display.setTextColor(SH110X_WHITE);
    _display.setTextSize(1);
    _display.setCursor(16, 24);
    _display.println("Initializing Robot...");
    _display.display();
    _scheduleNextBlink();
    return true;
  }

  void showBootMessage(const char* line1, const char* line2) {
    _display.clearDisplay();
    _display.setTextColor(SH110X_WHITE);
    _display.setTextSize(1);
    _display.setCursor(10, 20);
    _display.println(line1);
    if (line2) {
      _display.setCursor(10, 36);
      _display.println(line2);
    }
    _display.display();
  }

  bool setFace(const char* id) {
    for (size_t i = 0; i < FACE_REGISTRY_COUNT; i++) {
      if (strcmp(FACE_REGISTRY[i].id, id) == 0) {
        _currentIdx = i;
        _tempIdx = -1; // Clear temporary face
        renderImmediate();
        return true;
      }
    }
    return false;
  }

  void setTemporaryFace(const char* id, unsigned long durationMs) {
    for (size_t i = 0; i < FACE_REGISTRY_COUNT; i++) {
      if (strcmp(FACE_REGISTRY[i].id, id) == 0) {
        _tempIdx = i;
        _tempUntil = millis() + durationMs;
        renderImmediate();
        return;
      }
    }
  }

  const FaceItem& currentFace() const {
    int idx = (_tempIdx >= 0) ? _tempIdx : _currentIdx;
    return FACE_REGISTRY[idx];
  }

  void update() {
    unsigned long now = millis();

    // Check temp face expiration
    if (_tempIdx >= 0 && now >= _tempUntil) {
      _tempIdx = -1;
      renderImmediate();
    }

    // Auto natural blinking when idle on 'normal' face
    if (_tempIdx < 0 && strcmp(FACE_REGISTRY[_currentIdx].id, "normal") == 0) {
      if (!_blinking && now >= _nextBlink) {
        _blinking = true;
        _blinkEnd = now + 150; // 150ms natural eyelid closure
        renderImmediate();
      } else if (_blinking && now >= _blinkEnd) {
        _blinking = false;
        _scheduleNextBlink();
        renderImmediate();
      }
    }

    // Micro-animation phase update (every 300ms for bouncing elements/hearts/Zzz)
    if (now - _lastAnim >= 300) {
      _lastAnim = now;
      _animPhase++;
      // Only re-render if current face has animated elements
      const char* id = currentFace().id;
      if (strcmp(id, "love") == 0 || strcmp(id, "music") == 0 || strcmp(id, "sleepy") == 0) {
        renderImmediate();
      }
    }
  }

  void renderImmediate() {
    _display.clearDisplay();
    _display.setTextColor(SH110X_WHITE);

    if (_blinking) {
      drawFaceBlink(_display, _animPhase);
    } else {
      int active = (_tempIdx >= 0) ? _tempIdx : _currentIdx;
      FACE_REGISTRY[active].render(_display, _animPhase);
    }
    _display.display();
  }

private:
  Adafruit_SH1106G _display;
  int _currentIdx;
  int _tempIdx;
  unsigned long _tempUntil;
  int _animPhase;
  unsigned long _lastAnim;
  unsigned long _nextBlink;
  bool _blinking;
  unsigned long _blinkEnd;

  void _scheduleNextBlink() {
    _nextBlink = millis() + random(3000, 6000);
  }
};

#endif // FACE_SYSTEM_H
