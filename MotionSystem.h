#ifndef MOTION_SYSTEM_H
#define MOTION_SYSTEM_H

#include <Arduino.h>
#include "ServoOutput.h"
#include "Config.h"
#include "RuntimeConfig.h"

// ============================================================================
// SERVO KEYFRAME DATA STRUCTURES
// ============================================================================
struct ServoKeyframe {
  int16_t head;       // Head angle (0 - 180 deg)
  int16_t armL;       // Left Arm angle (0 - 180 deg)
  int16_t armR;       // Right Arm angle (0 - 180 deg)
  uint16_t holdMs;    // Time to hold this pose
};

struct MotionRoutine {
  const char* id;
  const char* title;
  const char* icon;
  const ServoKeyframe* frames;
  size_t length;
  bool isLooping;
};

// ============================================================================
// KEYFRAME ANIMATION SEQUENCES
// ============================================================================

// 1. Center / Ready Pose
const ServoKeyframe FRAMES_CENTER[] = {
  {90, 30, 150, 250}
};

// 2. Party Dance (สันทนาการแดนซ์ ส่ายหัว สลับแขน)
const ServoKeyframe FRAMES_DANCE[] = {
  {60,  120, 150, 180},
  {120, 30,  60,  180},
  {60,  120, 150, 180},
  {120, 30,  60,  180},
  {90,  140, 40,  200},
  {90,  30,  150, 200}
};

// 3. Chicken Wing Flap (เต้นไก่ย่างถูกเผา กระพือปีกไก่)
const ServoKeyframe FRAMES_CHICKEN[] = {
  {75,  130, 50,  150},
  {105, 30,  150, 150},
  {75,  130, 50,  150},
  {105, 30,  150, 150},
  {90,  140, 40,  180},
  {90,  30,  150, 180}
};

// 4. Wave Hello (โบกมือทักทาย)
const ServoKeyframe FRAMES_WAVE[] = {
  {90,  150, 150, 200},
  {80,  110, 150, 180},
  {100, 150, 150, 180},
  {80,  110, 150, 180},
  {90,  30,  150, 250}
};

// 5. Cheer & Hooray (ยกสองแขนไชโยเฉลิมฉลอง)
const ServoKeyframe FRAMES_CHEER[] = {
  {90,  150, 30,  250},
  {80,  160, 20,  200},
  {100, 160, 20,  200},
  {90,  160, 20,  250},
  {90,  30,  150, 250}
};

// 6. Hug / Open Arms (อ้าแขนขอกอด)
const ServoKeyframe FRAMES_HUG[] = {
  {90,  110, 70,  600},
  {90,  30,  150, 300}
};

// 7. Disco Fever (ชูแขนเฉียงขึ้นลงสไตล์ดิสโก้)
const ServoKeyframe FRAMES_DISCO[] = {
  {60,  160, 150, 220},
  {90,  30,  150, 150},
  {120, 30,  20,  220},
  {90,  30,  150, 150}
};

// 8. Head Shake / Wonder (ส่ายหัวสงสัย)
const ServoKeyframe FRAMES_HEAD_SHAKE[] = {
  {55,  40,  140, 180},
  {125, 40,  140, 180},
  {65,  40,  140, 180},
  {115, 40,  140, 180},
  {90,  30,  150, 200}
};

// 9. SIX-SEVEN Meme Dance (ท่าไวรัล 6-7 ชั่งน้ำหนักมือสลับขึ้นลงพร้อมส่ายหัวสุดกวน)
const ServoKeyframe FRAMES_SIX_SEVEN[] = {
  // Prep / Ready
  {90,  70,  110, 180},

  // Hit 1: "SIX!" (Left hand up, Right hand down, head tilt left)
  {65,  140, 155, 230},
  {72,  125, 145, 100}, // Micro-bounce recoil

  // Hit 1: "SEVEN!" (Left hand down, Right hand up, head tilt right)
  {115, 35,  45,  230},
  {108, 48,  58,  100}, // Micro-bounce recoil

  // Hit 2: "SIX!"
  {65,  140, 155, 230},
  {72,  125, 145, 100},

  // Hit 2: "SEVEN!"
  {115, 35,  45,  230},
  {108, 48,  58,  100},

  // Hit 3 (Fast drill combo: 6 - 7 - 6 - 7!)
  {60,  145, 160, 170}, // 6!
  {120, 30,  40,  170}, // 7!
  {60,  145, 160, 170}, // 6!
  {120, 30,  40,  170}, // 7!

  // Signature balanced scale freeze
  {90,  95,  85,  450},

  // Return to ready center
  {90,  30,  150, 200}
};

// ============================================================================
// MOTION REGISTRY (Extensible Table)
// ============================================================================
const MotionRoutine MOTION_REGISTRY[] = {
  {"dance",      "สันทนาการแดนซ์", "💃", FRAMES_DANCE,      sizeof(FRAMES_DANCE) / sizeof(ServoKeyframe),      false},
  {"six_seven",  "ท่าไวรัล SIX-SEVEN (6-7)", "⚖️", FRAMES_SIX_SEVEN, sizeof(FRAMES_SIX_SEVEN) / sizeof(ServoKeyframe), false},
  {"chicken",    "ท่าไก่ย่างถูกเผา", "🍗", FRAMES_CHICKEN,    sizeof(FRAMES_CHICKEN) / sizeof(ServoKeyframe),    false},
  {"cheer",      "ไชโยเฉลิมฉลอง",  "🙌", FRAMES_CHEER,      sizeof(FRAMES_CHEER) / sizeof(ServoKeyframe),      false},
  {"wave",       "โบกมือทักทาย",   "👋", FRAMES_WAVE,       sizeof(FRAMES_WAVE) / sizeof(ServoKeyframe),       false},
  {"disco",      "ดิสโก้ฟีเวอร์",   "🕺", FRAMES_DISCO,      sizeof(FRAMES_DISCO) / sizeof(ServoKeyframe),      false},
  {"hug",        "อ้าแขนขอกอด",    "🤗", FRAMES_HUG,        sizeof(FRAMES_HUG) / sizeof(ServoKeyframe),        false},
  {"head_shake", "ส่ายหัวดุ๊กดิ๊ก",  "🤔", FRAMES_HEAD_SHAKE, sizeof(FRAMES_HEAD_SHAKE) / sizeof(ServoKeyframe), false},
  {"center",     "ท่ายืนตรงพร้อม",  "🎯", FRAMES_CENTER,     sizeof(FRAMES_CENTER) / sizeof(ServoKeyframe),     false}
};

const size_t MOTION_REGISTRY_COUNT = sizeof(MOTION_REGISTRY) / sizeof(MotionRoutine);

// ============================================================================
// NON-BLOCKING MOTION PLAYER (Computer Science State Machine)
// ============================================================================
class MotionPlayer {
public:
  MotionPlayer() :
    _activeRoutine(nullptr),
    _frameIdx(0),
    _timer(0),
    _isMoving(false),
    _loop(false) {}

  void begin() {
    applyConfig();
  }

  void applyConfig() {
    freeze();
    int low = robotSettings.get(SERVO_MIN_US_CFG), high = robotSettings.get(SERVO_MAX_US_CFG);
    _servoHead.detach(); _servoArmL.detach(); _servoArmR.detach();
    _pulseLow = low; _pulseHigh = high;
    if (!robotSettings.get(SERVO_ENABLED)) return;
    attachServo(_servoHead, PIN_SERVO_HEAD);
    attachServo(_servoArmL, PIN_SERVO_ARM_L);
    attachServo(_servoArmR, PIN_SERVO_ARM_R);
    Serial.printf("[SERVO] Attach status -> Head(pin %d): %d | ArmL(pin %d): %d | ArmR(pin %d): %d\n",
                  PIN_SERVO_HEAD, _servoHead.attached(),
                  PIN_SERVO_ARM_L, _servoArmL.attached(),
                  PIN_SERVO_ARM_R, _servoArmR.attached());
    toCenter();
  }

  void freeze() { _isMoving = false; _activeRoutine = nullptr; }
  bool ready() const { return headReady() && leftReady() && rightReady(); }
  bool headReady() const { return _servoHead.attached(); }
  bool leftReady() const { return _servoArmL.attached(); }
  bool rightReady() const { return _servoArmR.attached(); }

  void toCenter() {
    freeze();
    writeAngles(robotSettings.get(HEAD_CENTER), robotSettings.get(LEFT_CENTER), robotSettings.get(RIGHT_CENTER));
  }

  bool playById(const char* id, bool loop = false) {
    for (size_t i = 0; i < MOTION_REGISTRY_COUNT; i++) {
      if (strcmp(MOTION_REGISTRY[i].id, id) == 0) {
        return playRoutine(&MOTION_REGISTRY[i], loop);
      }
    }
    return false;
  }

  bool playRoutine(const MotionRoutine* routine, bool loop = false) {
    if (!robotSettings.get(SERVO_ENABLED) || !ready() || !routine || routine->length == 0) return false;
    _activeRoutine = routine;
    _loop = loop;
    _frameIdx = 0;
    _isMoving = true;
    if (!_applyFrame(_activeRoutine->frames[0])) { freeze(); return false; }
    _timer = millis();
    return true;
  }

  void stop() {
    toCenter();
  }

  void update() {
    if (!_isMoving || !_activeRoutine) return;

    unsigned long now = millis();
    const ServoKeyframe& current = _activeRoutine->frames[_frameIdx];

    if (now - _timer >= (uint32_t)current.holdMs * 100 / robotSettings.get(MOTION_SPEED)) {
      _frameIdx++;
      if (_frameIdx < _activeRoutine->length) {
        if (!_applyFrame(_activeRoutine->frames[_frameIdx])) { freeze(); return; }
        _timer = now;
      } else {
        if (_loop) {
          _frameIdx = 0;
          if (!_applyFrame(_activeRoutine->frames[0])) { freeze(); return; }
          _timer = now;
        } else {
          _isMoving = false;
          _activeRoutine = nullptr;
          toCenter();
        }
      }
    }
  }

  bool isMoving() const { return _isMoving; }
  const MotionRoutine* currentRoutine() const { return _activeRoutine; }

  bool setManualAngles(int head, int armL, int armR) {
    _isMoving = false;
    _activeRoutine = nullptr;
    return writeAngles(head, armL, armR);
  }
  bool setManualAxis(const String& axis, int angle) {
    if (!robotSettings.get(SERVO_ENABLED)) return false;
    freeze();
    if (axis == "head") return _servoHead.write(robotSettings.servoAngle(HEAD_MIN, angle));
    if (axis == "left") return _servoArmL.write(robotSettings.servoAngle(LEFT_MIN, angle));
    if (axis == "right") return _servoArmR.write(robotSettings.servoAngle(RIGHT_MIN, angle));
    return false;
  }
  bool setHeadAngle(int angle) {
    return setManualAxis("head", angle);
  }

private:
  ServoOutput _servoHead{0};
  ServoOutput _servoArmL{1};
  ServoOutput _servoArmR{2};

  const MotionRoutine* _activeRoutine;
  size_t _frameIdx;
  unsigned long _timer;
  bool _isMoving;
  bool _loop;

  int _pulseLow = 0, _pulseHigh = 0;
  void attachServo(ServoOutput& servo, int pin) {
    if (!servo.attach(pin, _pulseLow, _pulseHigh)) Serial.printf("[SERVO] PWM attach failed on GPIO %d\n", pin);
  }
  bool writeAngles(int head, int left, int right) {
    if (!robotSettings.get(SERVO_ENABLED)) return false;
    const bool headOK = _servoHead.write(robotSettings.servoAngle(HEAD_MIN, head));
    const bool leftOK = _servoArmL.write(robotSettings.servoAngle(LEFT_MIN, left));
    const bool rightOK = _servoArmR.write(robotSettings.servoAngle(RIGHT_MIN, right));
    return headOK && leftOK && rightOK;
  }
  bool _applyFrame(const ServoKeyframe& frame) { return writeAngles(frame.head, frame.armL, frame.armR); }
};

#endif // MOTION_SYSTEM_H
