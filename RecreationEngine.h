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
    _obstacleDetectCount(0),
    _lastAvoidClearTime(0),
    _distLeft(-1.0f),
    _distRight(-1.0f),
    _frontDistAtTrigger(0.0f),
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
    _obstacleDetectCount = 0;
    _lastAvoidClearTime = 0;
    _distLeft = -1.0f;
    _distRight = -1.0f;
    _frontDistAtTrigger = 0.0f;
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
      _motion.toCenter();
      _face.setFace("happy");
      _song.toneBeep(880, 80);
      _behavior = "roam";
      _reason = "roam_cruise";
      _nextEntertainmentTime = millis() + 1500; // เริ่มสุ่มเพลงแรกหลังเปิดโหมด 1.5 วินาที
      Serial.println("\n[INTERACTIVE] Autonomous Roam & Obstacle Avoidance Started! (Arms stationary)");
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
  uint8_t          _obstacleDetectCount;
  uint32_t         _lastAvoidClearTime;

  float            _distLeft;
  float            _distRight;
  float            _frontDistAtTrigger;

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
  // --------------------------------------------------------------------------
  // สุ่มเลือกเพลงสร้างความบันเทิงระหว่างเดินสำรวจ (เล่นเฉพาะเพลง/SFX และเปลี่ยนสีหน้า แขนอยู่นิ่ง)
  // --------------------------------------------------------------------------
  void _triggerRandomEntertainment(uint32_t now) {
    // 20% โอกาส: เล่นเสียง SFX ทักทายสดใส (ไม่ขยับแขน)
    if (random(0, 100) < 20) {
      _face.setTemporaryFace("love", 2000);
      _song.playById("sfx_happy");
      _nextEntertainmentTime = now + random(10000, 18000);
      Serial.println("[ENTERTAINMENT] Random Sound Greeting (Arms stationary)");
      return;
    }

    // 80% โอกาส: สุ่มเล่นเพลงยอดนิยม พร้อมเปลี่ยนสีหน้า OLED (แขนอยู่นิ่ง)
    static const char* const FUN_SONGS[] = {
      "mixue", "six_seven", "numalee", "chang", "kaiyang",
      "kingkanbai", "tuktuk", "mario", "coffin", "babyshark",
      "doraemon", "twinkle"
    };
    static const size_t FUN_SONG_COUNT = sizeof(FUN_SONGS) / sizeof(FUN_SONGS[0]);

    size_t songIdx = random(0, FUN_SONG_COUNT);
    const char* chosenSong = FUN_SONGS[songIdx];

    _song.playById(chosenSong);

    // เลือกสีหน้าที่เข้ากับเพลง
    if (strcmp(chosenSong, "six_seven") == 0) {
      _face.setFace("six_seven");
    } else {
      static const char* const FUN_FACES[] = {
        "happy", "love", "wink", "cool", "music"
      };
      size_t fIdx = random(0, sizeof(FUN_FACES) / sizeof(FUN_FACES[0]));
      _face.setFace(FUN_FACES[fIdx]);
    }

    // กำหนดเวลาสุ่มความบันเทิงรอบถัดไป
    _nextEntertainmentTime = now + random(18000, 32000);
    Serial.printf("[ENTERTAINMENT] Playing random song: %s (Arms stationary)\n", chosenSong);
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

    // ดึงระยะตั้งค่าสำหรับเริ่มหลบสิ่งกีดขวาง (auto_stop_cm จาก Config)
    const float avoidThreshold = (float)max((int32_t)15, robotSettings.get(AUTO_STOP));
    const int centerDeg = robotSettings.get(HEAD_CENTER);
    const int minDeg = robotSettings.get(HEAD_MIN);
    const int maxDeg = robotSettings.get(HEAD_MAX);
    const int scanLeftDeg = constrain(centerDeg + 55, minDeg, maxDeg);
    const int scanRightDeg = constrain(centerDeg - 55, minDeg, maxDeg);

    // ========================================================================
    // PART A: หลบสิ่งกีดขวาง & หาทางใหม่ด้วย Servo Scan (แทนการหมุนตัวทั้งคัน)
    // ========================================================================
    if (_isAvoiding) {
      uint32_t elapsed = now - _avoidTimer;

      if (_avoidStage == 1) {
        // ขั้นตอนที่ 1: ถอยหลังระยะสั้น (Micro-reverse) เมื่อประชิดกำแพงมากเกินไป
        // เพื่อเว้นระยะปลอดภัยให้หัวหมุนกวาดได้โดยไม่ชนสิ่งกีดขวาง
        _motor.moveBackward(SPEED_REVERSE_PCT);
        _behavior = "avoid";
        _reason = "escape_reverse";

        if (elapsed >= _avoidReverseDuration) {
          _motor.stopMotors();
          // สลับเข้าสู่การกวาดหัวสแกนซ้าย
          _avoidStage = 2;
          _avoidTimer = now;
          _motion.setHeadAngle(scanLeftDeg);
          _behavior = "avoid";
          _reason = "scan_turn";
        }
        return;
      } else if (_avoidStage == 2) {
        // ขั้นตอนที่ 2: เซอร์โวหัวหันมองซ้าย (scanLeftDeg)
        _motor.stopMotors();
        _behavior = "avoid";
        _reason = "scan_turn";

        if (elapsed >= HEAD_SCAN_SETTLE_MS) {
          // อ่านระยะด้านซ้าย
          _distLeft = _sensor.distanceValid() ? _sensor.distance() : -1.0f;
          Serial.printf("[SCAN] Left (%d deg) -> Dist: %.1f cm\n", scanLeftDeg, _distLeft);

          // สั่งเซอร์โวหัวหันกวาดไปทางขวา (scanRightDeg)
          _avoidStage = 3;
          _avoidTimer = now;
          _motion.setHeadAngle(scanRightDeg);
          _reason = "scan_right";
        }
        return;
      } else if (_avoidStage == 3) {
        // ขั้นตอนที่ 3: เซอร์โวหัวหันมองขวา (scanRightDeg)
        _motor.stopMotors();
        _behavior = "avoid";
        _reason = "scan_right";

        // หมุนกวาดมุมกว้างจากซ้ายสุดมาขวาสุด (+40ms settle time)
        if (elapsed >= (HEAD_SCAN_SETTLE_MS + 40)) {
          // อ่านระยะด้านขวา
          _distRight = _sensor.distanceValid() ? _sensor.distance() : -1.0f;
          Serial.printf("[SCAN] Right (%d deg) -> Dist: %.1f cm\n", scanRightDeg, _distRight);

          // หมุนหัวกลับมาหน้าตรง (centerDeg)
          _avoidStage = 4;
          _avoidTimer = now;
          _motion.setHeadAngle(centerDeg);
        }
        return;
      } else if (_avoidStage == 4) {
        // ขั้นตอนที่ 4: คืนหัวหน้าตรง และตัดสินใจเลือกทิศทาง
        _motor.stopMotors();

        if (elapsed >= 120) {
          // คำนวณคะแนนพื้นที่ว่าง (Clearance Score):
          // ถ้าเซนเซอร์วัดไม่พบ (<=0 หรือ > 200 cm) แสดงว่าเปิดโล่งไม่มีสิ่งกีดขวางในระยะ ให้คะแนน 999 cm
          float scoreL = (_distLeft <= 0.0f || _distLeft > 200.0f) ? 999.0f : _distLeft;
          float scoreR = (_distRight <= 0.0f || _distRight > 200.0f) ? 999.0f : _distRight;

          Serial.printf("[SCAN DECISION] Left score: %.1f cm | Right score: %.1f cm (Avoid Threshold: %.1f cm)\n",
                        scoreL, scoreR, avoidThreshold);

          // กรณีเจอทางตัน/มุมอับ (Dead-end): ทั้งสองด้านมีสิ่งกีดขวางขวางอยู่ (<= avoidThreshold)
          if (scoreL <= avoidThreshold && scoreR <= avoidThreshold) {
            _consecutiveBlocks++;
            _avoidStage = 5; // Long Reverse Hold
            _avoidTimer = now;
            _avoidReverseDuration = AVOID_DEADEND_HOLD_MS;
            _avoidTurnDuration = random(750, 950); // หมุนตัวกลับหลังหัน (U-turn) ออกจากซอยตัน
            _avoidTurnRight = (scoreR >= scoreL);
            _behavior = "avoid";
            _reason = "escape_reverse";
            _face.setTemporaryFace("surprise", 1500);
            _song.toneBeep(660, 120);
            Serial.printf("[SCAN] DEAD END TRAP! Both sides blocked (L:%.1f, R:%.1f). Reversing %lu ms -> U-turn\n",
                          scoreL, scoreR, (unsigned long)_avoidReverseDuration);
          } else {
            // มีทางออกเปิดโล่ง: เลือกเลี้ยวไปด้านที่มีช่องว่างมากกว่า
            _consecutiveBlocks = 0;
            if (scoreL > scoreR) {
              _avoidTurnRight = false; // เลือกเลี้ยวซ้าย
              _reason = "scan_turn";
              Serial.println("[SCAN] Selected LEFT (Clearer path)");
            } else {
              _avoidTurnRight = true;  // เลือกเลี้ยวขวา
              _reason = "scan_right";
              Serial.println("[SCAN] Selected RIGHT (Clearer path)");
            }

            _avoidTurnDuration = random(AVOID_TURN_MIN_MS, AVOID_TURN_MAX_MS);
            _avoidStage = 6; // เริ่มหมุนเลี้ยวตัวถังทันที
            _avoidTimer = now;
            _behavior = "avoid";
          }
        }
        return;
      } else if (_avoidStage == 5) {
        // ขั้นตอนที่ 5: ถอยหลังยาวกรณีติดในมุมอับ/ทางตัน
        _motor.moveBackward(SPEED_REVERSE_PCT);
        _behavior = "avoid";
        _reason = "escape_reverse";

        if (elapsed >= _avoidReverseDuration) {
          _motor.stopMotors();
          _avoidStage = 6; // หมุน U-turn
          _avoidTimer = now;
        }
        return;
      } else if (_avoidStage == 6) {
        // ขั้นตอนที่ 6: หมุนตัวถังเลี้ยวไปทางที่เปิดโล่ง (High Torque Pivot Turn 100%)
        if (_avoidTurnRight) {
          _motor.turnRight(SPEED_ROAM_TURN);
          _reason = "scan_right";
        } else {
          _motor.turnLeft(SPEED_ROAM_TURN);
          _reason = "scan_turn";
        }
        _behavior = "avoid";

        if (elapsed >= _avoidTurnDuration) {
          // เลี้ยวเสร็จสิ้น! คืนสู่การเดินหน้าสำรวจต่อทันที
          _motor.stopMotors();
          _isAvoiding = false;
          _avoidStage = 0;
          _lastAvoidClearTime = now;
          _filterCount = 0;
          _dropoutCount = 0;
          _obstacleDetectCount = 0;
          _motion.setHeadAngle(centerDeg);
          Serial.println("[SCAN] Avoidance complete. Resuming forward roam!");
        }
        return;
      }
    }

    // ตรวจพบสิ่งกีดขวางหรือกำแพง (<= avoidThreshold)
    // กรองยืนยัน 2 ครั้งติดกัน (Debounce) ป้องกันคลื่นสะท้อนพื้นหลอกให้หุ่นเบรกหมุนตัวซ้ำซาก
    if (dist > 0 && dist <= avoidThreshold) {
      _obstacleDetectCount++;
    } else {
      _obstacleDetectCount = 0;
    }

    if (_obstacleDetectCount >= 2) {
      _obstacleDetectCount = 0;
      _motor.stopMotors();
      _frontDistAtTrigger = dist;
      _isAvoiding = true;
      _avoidTimer = now;

      if (!robotSettings.get(SERVO_ENABLED)) {
        // กรณีปิดเซอร์โวในระบบ: ถอยหลังแล้วสุ่มเลี้ยวแบบดั้งเดิม
        _avoidStage = 5;
        _avoidReverseDuration = AVOID_REVERSE_HOLD_MS;
        _avoidTurnDuration = random(AVOID_TURN_MIN_MS, AVOID_TURN_MAX_MS);
        _avoidTurnRight = (random(0, 2) == 0);
        _behavior = "avoid";
        _reason = "escape_reverse";
      } else if (_frontDistAtTrigger > 0 && _frontDistAtTrigger < (avoidThreshold * 0.55f)) {
        // ถ้าชิดสิ่งกีดขวางมากเกินไป ถอยระยะสั้นๆ ก่อน เพื่อเปิดช่องว่างให้หัวหมุนกวาดได้
        _avoidStage = 1;
        _avoidReverseDuration = 260;
        _motor.moveBackward(SPEED_REVERSE_PCT);
        _behavior = "avoid";
        _reason = "escape_reverse";
      } else {
        // เริ่มกวาดหัวสแกนซ้ายทันที
        _avoidStage = 2;
        _avoidTimer = now;
        _motion.setHeadAngle(scanLeftDeg);
        _behavior = "avoid";
        _reason = "scan_turn";
      }
      _face.setTemporaryFace("surprise", 1200);
      return;
    }

    // ถ้าเดินหน้าต่อได้ราบรื่นเกิน 3 วินาที ให้รีเซ็ตประวัติทางตัน
    if (now - _lastAvoidClearTime > 3000 && _consecutiveBlocks > 0) {
      _consecutiveBlocks = 0;
    }

    // ทางข้างหน้าโล่ง (> avoidThreshold หรือไม่มีสิ่งกีดขวาง) -> เดินหน้าสำรวจเต็มกำลัง Direct Drive
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

    // Serial Debug Monitor (Rate-limited 400ms)
    if (now - _lastLogTime >= 400) {
      _lastLogTime = now;
      Serial.printf("[ROAM & FUN] Dist: %.1f cm (Thresh: %.0f) | Move: %s | Song: %s\n",
                    dist, avoidThreshold, _behavior, _song.isPlaying() ? "Playing" : "Idle");
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
          _motion.setManualAngles(robotSettings.get(HEAD_CENTER), 140, 40);
          _face.setFace("music");
          break;
        case 1:
          _motion.setManualAngles(robotSettings.get(HEAD_CENTER), 40, 140);
          _face.setFace("happy");
          break;
        case 2:
          _motion.setManualAngles(robotSettings.get(HEAD_CENTER), 150, 30);
          _face.setFace("wink");
          break;
        case 3:
          _motion.setManualAngles(robotSettings.get(HEAD_CENTER), 50, 130);
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
