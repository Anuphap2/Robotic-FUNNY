#ifndef RECREATION_ENGINE_H
#define RECREATION_ENGINE_H

#include <Arduino.h>
#include "Config.h"
#include "FaceSystem.h"
#include "SongSystem.h"
#include "MotionSystem.h"
#include "MotorController.h"
#include "SensorManager.h"

// ============================================================================
// RECREATION ENGINE (Autonomous Entertainment & Obstacle Avoidance Robot)
// ============================================================================
// ออกแบบตามคำขอ:
// 1. เดินสำรวจอิสระ (Roam) ไม่ชนกำแพง/สิ่งกีดขวาง (Obstacle Avoidance)
// 2. ตัดฟังก์ชันเดินตามคนทิ้งทั้งหมด เพื่อไม่ให้เกิดความสับสนหรือลังเล
// 3. สุ่มเล่นเพลง สุ่มเต้น สุ่มทักทาย สุ่มเปลี่ยนสีหน้า OLED สร้างความบันเทิงรอบตัว
// 4. มอเตอร์ทำงานด้วยความเร็วสูงคงแรงบิด (85%-90%) ป้องกันล้อติดขัดจากน้ำหนักตัวถัง
// ============================================================================

enum RecreationMode {
  MODE_MANUAL = 0,        // Direct Web Control
  MODE_PARTY_BEAT = 1,    // Sound-Reactive Dancing (Clap/Music follower)
  MODE_INTERACTIVE = 2,   // Autonomous Roam & Smart Entertainment
  MODE_SHOWCASE = 3       // Synchronized Song & Dance Performance
};

class RecreationEngine {
public:
  RecreationEngine(FaceEngine& face, SongPlayer& song, MotionPlayer& motion, MotorController& motor, SensorManager& sensor) :
    _face(face),
    _song(song),
    _motion(motion),
    _motor(motor),
    _sensor(sensor),
    _mode(MODE_MANUAL),
    _partyStep(0),
    _lastPartyMove(0),
    _showcaseActive(false),
    _partyRested(false),
    _filteredDist(-1.0f),
    _filterCount(0),
    _dropoutCount(0),
    _isAvoiding(false),
    _avoidStage(0),
    _avoidTimer(0),
    _avoidReverseDuration(AVOID_REVERSE_HOLD_MS),
    _avoidTurnDuration(500),
    _avoidTurnRight(true),
    _consecutiveBlocks(0),
    _lastAvoidClearTime(0),
    _nextEntertainmentTime(0),
    _lastLogTime(0),
    _behavior("observe"),
    _reason("inactive") {}

  void begin() {
    _mode = MODE_MANUAL;
  }

  void setMode(RecreationMode mode, bool center = true) {
    _motor.stopMotors();
    _motion.freeze();
    _song.stop();
    _filteredDist = -1.0f;
    _filterCount = 0;
    _dropoutCount = 0;
    _isAvoiding = false;
    _avoidStage = 0;
    _avoidTimer = 0;
    _avoidTurnRight = true;
    _consecutiveBlocks = 0;
    _lastAvoidClearTime = 0;
    _behavior = "observe";
    _reason = "inactive";
    _mode = mode;
    _showcaseActive = false;

    if (_mode == MODE_MANUAL) {
      _face.setFace("normal");
      if (center) _motion.toCenter();
      _motor.stopMotors();
    } else if (_mode == MODE_PARTY_BEAT) {
      _lastPartyMove = millis();
      _partyRested = false;
      _face.setFace("music");
      _face.setTemporaryFace("happy", 1200);
      _song.playById("sfx_happy");
    } else if (_mode == MODE_INTERACTIVE) {
      _face.setFace("happy");
      _song.toneBeep(880, 80);
      _behavior = "roam";
      _reason = "roam_cruise";
      _nextEntertainmentTime = millis() + 1500; // เริ่มสุ่มเพลงแรกหลังเปิดโหมด 1.5 วินาที
      Serial.println("\n[INTERACTIVE] Autonomous Entertainment & Obstacle Avoidance Started!");
    }
  }

  bool startInteractive(bool lockFront = false) {
    (void)lockFront;
    if (!robotSettings.get(SONAR_ENABLED) || !robotSettings.get(MOTOR_ENABLED) || !_motor.ready()) return false;
    setMode(MODE_INTERACTIVE);
    return true;
  }

  const char* interactiveString() const { return "auto"; }
  const char* behaviorString() const { return _behavior; }
  const char* decisionClass() const { return _isAvoiding ? "static_object" : "clear"; }
  const char* decisionReason() const { return _reason; }
  bool targetLocked() const { return false; }
  uint8_t movingVotes() const { return 0; }

  RecreationMode mode() const { return _mode; }

  const char* modeString() const {
    switch (_mode) {
      case MODE_PARTY_BEAT:     return "party";
      case MODE_INTERACTIVE:    return "interactive";
      case MODE_SHOWCASE:       return "showcase";
      default:                  return "manual";
    }
  }

  // Start synchronized song & dance show
  bool startShowcase(const char* songId, const char* motionId) {
    _motor.stopMotors();
    _reason = "inactive";
    _song.stop();
    _motion.freeze();
    bool sOk = _song.playById(songId);
    bool mOk = _motion.playById(motionId, true); // Loop dance while song plays
    if (sOk && mOk) {
      _mode = MODE_SHOWCASE;
      _showcaseActive = true;
      if ((motionId && strcmp(motionId, "six_seven") == 0) || (songId && strcmp(songId, "six_seven") == 0)) {
        _face.setFace("six_seven");
      } else {
        _face.setFace("music");
      }
      return true;
    }
    _song.stop();
    _motion.freeze();
    _mode = MODE_MANUAL;
    _showcaseActive = false;
    return false;
  }

  void update() {
    // 1. Showcase Mode: ตรวจสอบเพลงจบ
    if (_mode == MODE_SHOWCASE && _showcaseActive) {
      if (!_song.isPlaying()) {
        _motion.toCenter();
        _face.setFace("happy");
        _face.setTemporaryFace("love", 1500);
        _showcaseActive = false;
        _mode = MODE_MANUAL;
      }
      return;
    }

    // 2. Sound-Reactive Party Mode
    if (_mode == MODE_PARTY_BEAT) {
      _updatePartyMode();
      return;
    }

    // 3. Autonomous Entertainment & Obstacle Avoidance (Interactive Mode)
    if (_mode == MODE_INTERACTIVE) {
      _updateInteractiveMode();
      return;
    }
  }

private:
  FaceEngine&      _face;
  SongPlayer&      _song;
  MotionPlayer&    _motion;
  MotorController& _motor;
  SensorManager&   _sensor;

  RecreationMode   _mode;
  int              _partyStep;
  unsigned long    _lastPartyMove;
  bool             _showcaseActive;
  bool             _partyRested;

  // Obstacle Avoidance & Filtering
  float            _filteredDist;
  float            _filterBuf[5];
  uint8_t          _filterCount;
  uint8_t          _dropoutCount;

  bool             _isAvoiding;
  uint8_t          _avoidStage;
  uint32_t         _avoidTimer;
  uint32_t         _avoidReverseDuration;
  uint32_t         _avoidTurnDuration;
  bool             _avoidTurnRight;
  uint8_t          _consecutiveBlocks;
  uint32_t         _lastAvoidClearTime;

  uint32_t         _nextEntertainmentTime;
  uint32_t         _lastLogTime;

  const char*      _behavior;
  const char*      _reason;

  // --------------------------------------------------------------------------
  // Fast 5-Sample Median Filter (กรอง Outlier 150cm กระโดดหลุดออกทันทีใน <0.5 µs)
  // --------------------------------------------------------------------------
  float _filterDistance(float raw) {
    if (raw < 5.0f || raw > 220.0f) {
      _dropoutCount++;
      if (_dropoutCount >= 3) {
        _filteredDist = -1.0f;
      }
      return _filteredDist;
    }

    _dropoutCount = 0;

    if (_filterCount < 5) {
      _filterBuf[_filterCount++] = raw;
    } else {
      for (int i = 0; i < 4; i++) {
        _filterBuf[i] = _filterBuf[i + 1];
      }
      _filterBuf[4] = raw;
    }

    float sorted[5];
    for (int i = 0; i < _filterCount; i++) sorted[i] = _filterBuf[i];

    for (int i = 1; i < _filterCount; i++) {
      float key = sorted[i];
      int j = i - 1;
      while (j >= 0 && sorted[j] > key) {
        sorted[j + 1] = sorted[j];
        j--;
      }
      sorted[j + 1] = key;
    }

    _filteredDist = sorted[_filterCount / 2];
    return _filteredDist;
  }

  // --------------------------------------------------------------------------
  // สุ่มเลือกเพลง ท่าเต้น หรือการทักทาย สร้างความบันเทิงอย่างมีชีวิตชีวา
  // --------------------------------------------------------------------------
  void _triggerRandomEntertainment(uint32_t now) {
    // 20% โอกาส: ทำท่าโบกมือทักทายพร้อมเสียง SFX สดใส
    if (random(0, 100) < 20) {
      _face.setTemporaryFace("love", 2000);
      _song.playById("sfx_happy");
      _motion.playById("wave");
      _nextEntertainmentTime = now + random(10000, 18000);
      Serial.println("[ENTERTAINMENT] Random Greeting: Waving Hello!");
      return;
    }

    // 80% โอกาส: สุ่มเล่นเพลงยอดนิยม พร้อมเต้นต่อเนื่อง
    static const char* const FUN_SONGS[] = {
      "mixue", "six_seven", "numalee", "chang", "kaiyang",
      "kingkanbai", "tuktuk", "mario", "coffin", "babyshark",
      "doraemon", "twinkle"
    };
    static const size_t FUN_SONG_COUNT = sizeof(FUN_SONGS) / sizeof(FUN_SONGS[0]);

    size_t songIdx = random(0, FUN_SONG_COUNT);
    const char* chosenSong = FUN_SONGS[songIdx];

    _song.playById(chosenSong);

    // เลือกท่าเต้นและสีหน้าที่แมตช์กับเพลง
    if (strcmp(chosenSong, "six_seven") == 0) {
      _motion.playById("six_seven", true);
      _face.setFace("six_seven");
    } else if (strcmp(chosenSong, "kaiyang") == 0) {
      _motion.playById("chicken", true);
      _face.setFace("music");
    } else {
      _triggerRandomDance();
    }

    // กำหนดเวลาสุ่มความบันเทิงรอบถัดไป
    _nextEntertainmentTime = now + random(18000, 32000);
    Serial.printf("[ENTERTAINMENT] Playing random song: %s\n", chosenSong);
  }

  void _triggerRandomDance() {
    static const char* const FUN_DANCES[] = {
      "dance", "disco", "chicken", "cheer", "wave", "hug", "head_shake"
    };
    static const size_t FUN_DANCE_COUNT = sizeof(FUN_DANCES) / sizeof(FUN_DANCES[0]);
    static const char* const FUN_FACES[] = {
      "happy", "love", "wink", "cool", "music"
    };
    static const size_t FUN_FACE_COUNT = sizeof(FUN_FACES) / sizeof(FUN_FACES[0]);

    size_t dIdx = random(0, FUN_DANCE_COUNT);
    size_t fIdx = random(0, FUN_FACE_COUNT);
    _motion.playById(FUN_DANCES[dIdx], true);
    _face.setFace(FUN_FACES[fIdx]);
  }

  // --------------------------------------------------------------------------
  // โหมดเดินสำรวจ & หลบสิ่งกีดขวาง + ความบันเทิง (Interactive Roam & Entertainment)
  // --------------------------------------------------------------------------
  void _updateInteractiveMode() {
    uint32_t now = millis();

    // 1. ตรวจสอบความพร้อมของเซนเซอร์และมอเตอร์
    if (!robotSettings.get(MOTOR_ENABLED) || !robotSettings.get(SONAR_ENABLED) || !_motor.ready()) {
      _behavior = "stop";
      _reason = "hardware_disabled";
      _motor.stopMotors();
      return;
    }

    // 2. อ่านระยะทางผ่าน Fast Median Filter
    float raw = _sensor.distanceValid() ? _sensor.distance() : -1.0f;
    float dist = _filterDistance(raw);

    // ========================================================================
    // PART A: หลบสิ่งกีดขวาง & หาทางใหม่ (ถอยหลังค้าง แล้วค่อยเลี้ยว)
    // ========================================================================
    if (_isAvoiding) {
      uint32_t elapsed = now - _avoidTimer;

      if (_avoidStage == 1) {
        // ขั้นตอนที่ 1: ถอยหลังค้าง (Sustained Reverse Hold)
        // ดึงตัวถังหนักออกจากกำแพงหรือมุมตัน เพื่อเปิดระยะห่างด้านหน้าอย่างปลอดภัย
        _motor.moveBackward(SPEED_REVERSE_PCT);
        _behavior = "avoid";
        _reason = "escape_reverse";

        if (elapsed >= _avoidReverseDuration) {
          _motor.stopMotors();
          _avoidStage = 2;
          _avoidTimer = now;
        }
        return;
      } else if (_avoidStage == 2) {
        // ขั้นตอนที่ 2: หยุดพักสั้นๆ (90 ms) ให้มอเตอร์ตัดแรงเฉื่อยและป้องกันไฟกระชาก
        _motor.stopMotors();
        _behavior = "avoid";
        _reason = "escape_reverse";

        if (elapsed >= 90) {
          _avoidStage = 3;
          _avoidTimer = now;
        }
        return;
      } else if (_avoidStage == 3) {
        // ขั้นตอนที่ 3: หมุนเลี้ยวหาทางใหม่ (High Torque Pivot Turn 100%)
        if (_avoidTurnRight) _motor.turnRight(SPEED_ROAM_TURN);
        else _motor.turnLeft(SPEED_ROAM_TURN);
        _behavior = "avoid";
        _reason = "avoid_turn";

        if (elapsed >= _avoidTurnDuration) {
          _motor.stopMotors();
          _isAvoiding = false;
          _avoidStage = 0;
          _lastAvoidClearTime = now;
        }
        return;
      }
    }

    // ตรวจพบสิ่งกีดขวางหรือกำแพง (< AVOID_DISTANCE_THRESHOLD_CM)
    if (dist > 0 && dist <= AVOID_DISTANCE_THRESHOLD_CM) {
      _motor.stopMotors();
      _isAvoiding = true;
      _avoidTimer = now;
      _avoidStage = 1; // เริ่มขั้นตอนถอยหลังค้างทันที

      // ตรวจสอบว่าเป็นการติดซ้ำซ้อนในมุมอับ/ทางตันหรือไม่
      if (now - _lastAvoidClearTime < 2200) {
        _consecutiveBlocks++;
      } else {
        _consecutiveBlocks = 1;
      }

      if (_consecutiveBlocks >= 2) {
        // เจอทางตัน / เข้ามุม: ถอยหลังค้างยาวพิเศษ (~1000–1250 ms) แล้วสุ่มหมุน U-turn ออก
        _avoidReverseDuration = random(AVOID_DEADEND_HOLD_MS - 100, AVOID_DEADEND_HOLD_MS + 150);
        _avoidTurnDuration = random(750, 950); // หมุนตัวกลับหลังหันออกจากซอยตัน
        _avoidTurnRight = (random(0, 2) == 0);
        _face.setTemporaryFace("surprise", 1500);
        _song.toneBeep(660, 120); // บี๊บเตือนสั้นๆ ว่าพบทางตัน
        Serial.printf("[AVOID] Dead-end trap (Streak %d)! Long reverse: %lu ms -> U-turn: %lu ms\n",
                      _consecutiveBlocks, (unsigned long)_avoidReverseDuration, (unsigned long)_avoidTurnDuration);
      } else {
        // เจอกำแพง/สิ่งกีดขวางทั่วไป: ถอยหลังค้าง (~650–800 ms) แล้วสุ่มเลี้ยวหาทางใหม่
        _avoidReverseDuration = random(AVOID_REVERSE_HOLD_MS, AVOID_REVERSE_HOLD_MS + 150);
        _avoidTurnDuration = random(AVOID_TURN_MIN_MS, AVOID_TURN_MAX_MS);
        _avoidTurnRight = (random(0, 2) == 0);
        _face.setTemporaryFace("surprise", 900);
        Serial.printf("[AVOID] Obstacle ahead (%.1f cm)! Sustained reverse: %lu ms -> Turn: %lu ms\n",
                      dist, (unsigned long)_avoidReverseDuration, (unsigned long)_avoidTurnDuration);
      }
      return;
    }

    // ถ้าเดินหน้าต่อได้ราบรื่นเกิน 3 วินาที ให้รีเซ็ตประวัติทางตัน
    if (now - _lastAvoidClearTime > 3000 && _consecutiveBlocks > 0) {
      _consecutiveBlocks = 0;
    }

    // ทางข้างหน้าโล่ง (> AVOID_DISTANCE_THRESHOLD_CM หรือไม่มีสิ่งกีดขวาง) -> เดินหน้าสำรวจเต็มกำลัง Direct Drive
    _motor.moveForward(); // ขับเคลื่อน Direct Digital Full Drive 100% ต่อเนื่อง
    _behavior = "roam";
    _reason = "roam_cruise";

    // ========================================================================
    // PART B: ระบบสร้างความบันเทิงอัตโนมัติ (Random Entertainment Engine)
    // ========================================================================
    // ถ้ายังไม่ได้เล่นเพลง และถึงเวลาสุ่มความบันเทิง
    if (!_song.isPlaying() && now >= _nextEntertainmentTime) {
      _triggerRandomEntertainment(now);
    }

    // ถ้ากำลังเล่นเพลงอยู่ ให้วนท่าเต้นต่อเนื่อง
    if (_song.isPlaying()) {
      if (_motion.isMoving() == false) {
        _triggerRandomDance();
      }
    }

    // Serial Debug Monitor (Rate-limited 400ms)
    if (now - _lastLogTime >= 400) {
      _lastLogTime = now;
      Serial.printf("[ROAM & FUN] Dist: %.1f cm | Move: %s | Song: %s\n",
                    dist, _behavior, _song.isPlaying() ? "Playing" : "Idle");
    }
  }

  // --------------------------------------------------------------------------
  // โหมดเต้นตามจังหวะเสียงเพลง/ปรบมือ (Party Beat)
  // --------------------------------------------------------------------------
  void _updatePartyMode() {
    unsigned long now = millis();

    if (_sensor.consumeBeat()) {
      _partyStep = (_partyStep + 1) % 4;
      _lastPartyMove = now;
      _partyRested = false;

      switch (_partyStep) {
        case 0:
          _motion.setManualAngles(65, 140, 40);
          _face.setFace("music");
          break;
        case 1:
          _motion.setManualAngles(115, 40, 140);
          _face.setFace("happy");
          break;
        case 2:
          _motion.setManualAngles(90, 150, 30);
          _face.setFace("wink");
          break;
        case 3:
          _motion.setManualAngles(80, 50, 130);
          _face.setFace("cool");
          break;
      }
    } else if (!_partyRested && now - _lastPartyMove >= 1200) {
      if (_motion.isMoving() == false) {
        _motion.toCenter();
        _face.setFace("normal");
        _partyRested = true;
      }
    }
  }
};

#endif // RECREATION_ENGINE_H
