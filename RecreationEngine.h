#ifndef RECREATION_ENGINE_H
#define RECREATION_ENGINE_H

#include <Arduino.h>
#include "FaceSystem.h"
#include "SongSystem.h"
#include "MotionSystem.h"
#include "MotorController.h"
#include "SensorManager.h"

// ============================================================================
// RECREATION ENGINE (Coordinates Entertainment Modes & Interactive Follower)
// ============================================================================
// ตัด Decision Tree ที่ซับซ้อนออกตามคำขอ ใช้ Fast if-else Architecture
// ตัดสินใจทันทีในลูป (~30-50ms) ตอบสนองฉับไว ไม่หน่วง ไม่ค้าง และล้อวิ่ง 100% เท่ากัน
// ============================================================================

enum RecreationMode {
  MODE_MANUAL = 0,        // Direct Web Control
  MODE_PARTY_BEAT = 1,    // Sound-Reactive Dancing (Clap/Music follower)
  MODE_INTERACTIVE = 2,   // Fast Human Tracking & Interactive Follower (Direct if-else)
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
    _targetLocked(false),
    _filteredDist(-1.0f),
    _filterCount(0),
    _dropoutCount(0),
    _lostStartTime(0),
    _searchSweepTimer(0),
    _searchPhase(0),
    _searchTurnRight(true),
    _lastGreetTime(0),
    _lastLogTime(0),
    _refDist(-1.0f),
    _refDistTimer(0),
    _movingConfirmCount(0),
    _lastTargetMotionTime(0),
    _pauseObserveTimer(0),
    _isObservingObstacle(false),
    _avoidTurnTimer(0),
    _isAvoiding(false),
    _avoidTurnRight(true),
    _behavior("observe"),
    _reason("inactive") {}

  void begin() {
    _mode = MODE_MANUAL;
  }

  void setMode(RecreationMode mode, bool center = true) {
    _motor.stopMotors();
    _motion.freeze();
    _song.stop();
    _targetLocked = false;
    _filteredDist = -1.0f;
    _filterCount = 0;
    _dropoutCount = 0;
    _lostStartTime = 0;
    _searchPhase = 0;
    _searchTurnRight = true;
    _refDist = -1.0f;
    _refDistTimer = 0;
    _movingConfirmCount = 0;
    _lastTargetMotionTime = 0;
    _pauseObserveTimer = 0;
    _isObservingObstacle = false;
    _avoidTurnTimer = 0;
    _isAvoiding = false;
    _avoidTurnRight = true;
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
      _face.setFace("normal");
      _song.toneBeep(880, 80);
      _behavior = "roam";
      _reason = "roam_cruise";
      _lastGreetTime = millis();
      _refDistTimer = millis();
      Serial.println("\n[INTERACTIVE] Roam Forward & Moving Target Tracking Started");
    }
  }

  bool startInteractive(bool lockFront = false) {
    if (!robotSettings.get(SONAR_ENABLED) || !robotSettings.get(MOTOR_ENABLED) || !_motor.ready()) return false;
    setMode(MODE_INTERACTIVE);
    _targetLocked = lockFront;
    return true;
  }

  const char* interactiveString() const { return "auto"; }
  const char* behaviorString() const { return _behavior; }
  const char* decisionClass() const { return _targetLocked ? "moving_target" : (_isObservingObstacle ? "static_object" : "clear"); }
  const char* decisionReason() const { return _reason; }
  bool targetLocked() const { return _targetLocked; }
  uint8_t movingVotes() const { return _targetLocked ? 2 : 0; }

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
    _targetLocked = false;
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

    // 3. Proximity Interactive Follower Mode (Fast if-else)
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

  // Interactive Follower State Variables
  bool             _targetLocked;
  float            _filteredDist;
  float            _filterBuf[5];
  uint8_t          _filterCount;
  uint8_t          _dropoutCount;

  uint32_t         _lostStartTime;
  uint32_t         _searchSweepTimer;
  uint8_t          _searchPhase;
  bool             _searchTurnRight;
  uint32_t         _lastGreetTime;
  uint32_t         _lastLogTime;

  float            _refDist;
  uint32_t         _refDistTimer;
  uint8_t          _movingConfirmCount;
  uint32_t         _lastTargetMotionTime;
  uint32_t         _pauseObserveTimer;
  bool             _isObservingObstacle;
  uint32_t         _avoidTurnTimer;
  bool             _isAvoiding;
  bool             _avoidTurnRight;

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
  // หมุนกวาดค้นหาเป้าหมาย (Sweep Search) เมื่อเป้าหมายหลุดสายตา
  // --------------------------------------------------------------------------
  void _doSweepSearch(uint32_t now) {
    uint32_t elapsed = now - _searchSweepTimer;

    switch (_searchPhase) {
      case 0:
        // กวาดทิศทางแรก (550 ms)
        if (_searchTurnRight) _motor.turnRight(SPEED_TURN_SEARCH);
        else _motor.turnLeft(SPEED_TURN_SEARCH);
        _face.setFace(_searchTurnRight ? "look_right" : "look_left");
        if (elapsed >= 550) {
          _motor.stopMotors();
          _searchPhase = 1;
          _searchSweepTimer = now;
        }
        break;

      case 1:
        // หยุดอ่านเซนเซอร์ชั่วขณะ (220 ms)
        _motor.stopMotors();
        if (elapsed >= 220) {
          _searchTurnRight = !_searchTurnRight;
          _searchPhase = 2;
          _searchSweepTimer = now;
        }
        break;

      case 2:
        // กวาดทิศทางตรงข้าม (1100 ms)
        if (_searchTurnRight) _motor.turnRight(SPEED_TURN_SEARCH);
        else _motor.turnLeft(SPEED_TURN_SEARCH);
        _face.setFace(_searchTurnRight ? "look_right" : "look_left");
        if (elapsed >= 1100) {
          _motor.stopMotors();
          _searchPhase = 3;
          _searchSweepTimer = now;
        }
        break;

      case 3:
        // หยุดอ่านเซนเซอร์อีกครั้ง (220 ms)
        _motor.stopMotors();
        if (elapsed >= 220) {
          _searchTurnRight = !_searchTurnRight;
          _searchPhase = 0;
          _searchSweepTimer = now;
        }
        break;
    }
  }

  // --------------------------------------------------------------------------
  // Interactive Engine: Roaming ("เดินๆ ไป") + Moving Target Tracking (วัตถุขยับ)
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

    // 3. จัดการกรณีที่กำลังเล่นท่าทางทักทาย (Wave motion)
    if (_motion.isMoving()) {
      _motor.stopMotors();
      _behavior = "greet";
      _reason = "greeting";
      return;
    }

    // ========================================================================
    // STATE A: กำลังหมุนเลี้ยวหลบสิ่งกีดขวาง (Obstacle Avoidance Turn)
    // ========================================================================
    if (_isAvoiding) {
      if (now - _avoidTurnTimer < 380) {
        if (_avoidTurnRight) _motor.turnRight(SPEED_TURN_SEARCH);
        else _motor.turnLeft(SPEED_TURN_SEARCH);
        _behavior = "avoid";
        _reason = "avoid_turn";
        return;
      } else {
        _motor.stopMotors();
        _isAvoiding = false;
        _isObservingObstacle = false;
        _refDist = -1.0f;
        _refDistTimer = now;
      }
    }

    // ========================================================================
    // STATE B: ถ้ายังไม่ได้ล็อคเป้าหมาย (ROAMING: เดินๆ ไปสำรวจหาวัตถุที่ขยับ)
    // ========================================================================
    if (!_targetLocked) {
      // B.1 ใกล้สิ่งกีดขวางฉุกเฉิน (< 32 cm) -> หลบทันที
      if (dist > 0 && dist < 32.0f) {
        _isAvoiding = true;
        _avoidTurnTimer = now;
        _avoidTurnRight = !_avoidTurnRight;
        _face.setFace("surprise");
        _motor.stopMotors();
        return;
      }

      // B.2 ตรวจจับ "วัตถุที่ขยับระยะห่างไปเรื่อยๆ"
      if (_refDist < 0 || (now - _refDistTimer >= 150)) {
        if (_refDist > 0 && dist > 0) {
          float delta = dist - _refDist;

          // ถ้ามีวัตถุด้านหน้า (45 - 160 cm) และระยะกำลัง "ขยับห่างออกไป" (delta >= 3.5 cm)
          // ขณะที่หุ่นกำลังเดินหน้าแต่ระยะกลับเพิ่มขึ้น = วัตถุข้างหน้ากำลังเดินหนี/เคลื่อนที่นำหน้าแน่นอน!
          if (dist >= 45.0f && dist <= 160.0f && delta >= 3.5f) {
            _movingConfirmCount++;
            if (_movingConfirmCount >= 2) {
              _targetLocked = true;
              _movingConfirmCount = 0;
              _lastTargetMotionTime = now;
              _isObservingObstacle = false;
              _face.setFace("happy");
              _song.toneBeep(880, 80);
              Serial.println("[INTERACTIVE] Moving Target Detected Ahead! Locked & Following!");
              return;
            }
          } else {
            if (_movingConfirmCount > 0) _movingConfirmCount--;
          }
        }
        _refDist = dist;
        _refDistTimer = now;
      }

      // B.3 มีสิ่งกีดขวางอยู่ตรงหน้าปานกลาง (32 - 50 cm)
      // หุ่นยนต์จะหยุดดูชั่วขณะ (250 ms) เพื่อแยกแยะว่าเป็นกำแพงหรือคน
      if (dist >= 32.0f && dist <= 50.0f) {
        if (!_isObservingObstacle) {
          _isObservingObstacle = true;
          _pauseObserveTimer = now;
          _motor.stopMotors();
          _behavior = "observe";
          _reason = "checking_object";
          return;
        }

        _motor.stopMotors();
        // ถ้าหยุดดูแล้วระยะขยับ (|dist - _refDist| >= 3.0 cm) -> คนกำลังขยับตัวอยู่ตรงหน้า!
        if (fabs(dist - _refDist) >= 3.0f) {
          _targetLocked = true;
          _isObservingObstacle = false;
          _face.setFace("love");
          _song.toneBeep(988, 80);
          return;
        }

        // ถ้าหยุดครบ 250 ms แล้วระยะยังนิ่งสนิท -> สิ่งของนิ่ง/กำแพง! ให้เลี้ยวหลบ
        if (now - _pauseObserveTimer >= 250) {
          _isAvoiding = true;
          _avoidTurnTimer = now;
          _avoidTurnRight = !_avoidTurnRight;
          _isObservingObstacle = false;
          return;
        }
        return;
      }

      // B.4 ทางโล่ง (> 50 cm หรือไม่มี echo) -> เดินหน้าสำรวจไปเรื่อยๆ (เดินๆ ไป)
      _isObservingObstacle = false;
      _motor.moveForward(SPEED_FOLLOW_SLOW); // 85% รักษาแรงบิดสำหรับตัวถังหนัก
      _behavior = "roam";
      _reason = "roam_cruise";
      _face.setFace("normal");

      // Serial Debug Monitor (Rate-limited 400ms)
      if (now - _lastLogTime >= 400) {
        _lastLogTime = now;
        Serial.printf("[ROAM] Dist: %.1f cm | Walking Forward...\n", dist);
      }
      return;
    }

    // ========================================================================
    // STATE C: ล็อคเป้าหมายแล้ว (FOLLOWING: เดินตามวัตถุที่ขยับด้วย Fast if-else)
    // ========================================================================
    // C.1 เป้าหมายหลุดระยะ (> 180 cm หรือขาด echo)
    if (dist <= 0 || dist > 180.0f) {
      if (_lostStartTime == 0) {
        _lostStartTime = now;
        _motor.stopMotors();
        _behavior = "stop";
        _reason = "target_lost";
        _face.setFace("surprise");
      } else if (now - _lostStartTime < 300) {
        _motor.stopMotors();
      } else if (now - _lostStartTime < 3500) {
        // กวาดค้นหา 3.5 วินาที
        _behavior = "avoid";
        _reason = "searching";
        _doSweepSearch(now);
      } else {
        // หาไม่พบเกิน 3.5 วินาที -> ปลดล็อค แล้วกลับไป "เดินๆ ไป (Roam)" หาคนใหม่ต่อ!
        _targetLocked = false;
        _lostStartTime = 0;
        _motor.stopMotors();
        _face.setFace("normal");
        Serial.println("[INTERACTIVE] Target lost timeout -> Returning to Roam mode.");
      }
      return;
    }

    // พบเป้าหมายชัดเจนในระยะ (8 - 180 cm)
    _lostStartTime = 0;

    // C.2 ทักทาย (Greeting): ยืนตรงหน้า 50 - 65 cm และเว้นช่วง 10 วินาที
    if (dist >= 50.0f && dist <= 65.0f && (now - _lastGreetTime >= 10000)) {
      _lastGreetTime = now;
      _motor.stopMotors();
      _behavior = "greet";
      _reason = "greeting";
      _face.setTemporaryFace("love", 1600);
      _song.playById("sfx_happy");
      _motion.playById("wave");
      return;
    }

    // C.3 Fast if-else Distance Following (ความถี่สูง 30ms ตอบสนองทันใจ)
    if (dist < 35.0f) {
      // < 35 cm: ใกล้เกินไป อันตราย -> เบรกหยุดทันที
      _motor.stopMotors();
      _behavior = "stop";
      _reason = "too_close_stop";
      _face.setFace("surprise");
    } else if (dist < 50.0f) {
      // 35 - 50 cm: ชิดเกินไป -> ถอยหลังเว้นระยะ (85% รักษาแรงบิด)
      _motor.moveBackward(SPEED_REVERSE_PCT);
      _behavior = "avoid";
      _reason = "escape_reverse";
      _face.setFace("surprise");
    } else if (dist <= 65.0f) {
      // 50 - 65 cm: TARGET ZONE (60 cm) -> หยุดรักษาระยะ สบตา
      _motor.stopMotors();
      _behavior = "follow";
      _reason = "follow_hold";
      _face.setFace("love");
    } else if (dist <= 85.0f) {
      // 65 - 85 cm: ขยับห่างออกไป -> เดินตามชิด (85%)
      _motor.moveForward(SPEED_FOLLOW_SLOW);
      _behavior = "follow";
      _reason = "follow_cruise";
      _face.setFace("normal");
    } else if (dist <= 110.0f) {
      // 85 - 110 cm: ขยับห่างออกไปอีก -> เดินตามปกติ (95%)
      _motor.moveForward(SPEED_FOLLOW_MID);
      _behavior = "follow";
      _reason = "follow_cruise";
      _face.setFace("happy");
    } else {
      // 110 - 180 cm: วัตถุอยู่ไกล -> เดินตามเร็วเต็มกำลัง (100%)
      _motor.moveForward(SPEED_FOLLOW_FAST);
      _behavior = "follow";
      _reason = "follow_cruise";
      _face.setFace("happy");
    }

    // Serial Debug Monitor (Rate-limited 300ms)
    if (now - _lastLogTime >= 300) {
      _lastLogTime = now;
      Serial.printf("[FOLLOW] Dist: %.1f cm | %s | %s\n", dist, _behavior, _reason);
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
