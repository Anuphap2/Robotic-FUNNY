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
      _behavior = "follow";
      _reason = "target_acquired";
      _lastGreetTime = millis(); // Cooldown กันทักทายซ้ำทันทีที่เปิดโหมด
      Serial.println("\n[INTERACTIVE] Mode Started: Fast if-else Human Tracking & Companion");
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
  const char* decisionClass() const { return _targetLocked ? "moving_target" : "clear"; }
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
        if (_searchTurnRight) _motor.turnRight(60);
        else _motor.turnLeft(60);
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
        if (_searchTurnRight) _motor.turnRight(60);
        else _motor.turnLeft(60);
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
  // Interactive Follower Engine: ทำงานด้วย Direct if-else ตัดสินใจทันที!
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

    // 3. ตรวจสอบกรณีไม่มีเป้าหมาย / เป้าหมายหลุดระยะ (> 180 cm หรือไม่มี echo)
    if (dist <= 0 || dist > 180.0f) {
      _targetLocked = false;
      if (_lostStartTime == 0) {
        _lostStartTime = now;
        _motor.stopMotors();
        _behavior = "stop";
        _reason = "target_lost";
        _face.setFace("surprise");
        Serial.println("[INTERACTIVE] Target LOST. Pausing & Debouncing...");
      } else if (now - _lostStartTime < 350) {
        // หยุดรอยืนยัน 350ms เผื่อคนขยับนิดหน่อย
        _motor.stopMotors();
        _behavior = "stop";
        _reason = "target_lost";
      } else if (now - _lostStartTime < 6000) {
        // หลุดเกิน 350ms -> กวาดค้นหา (Sweep Search)
        _behavior = "avoid";
        _reason = "searching";
        _doSweepSearch(now);
      } else {
        // ค้นหาเกิน 6 วินาที -> หยุดรอเพื่อความปลอดภัย
        _motor.stopMotors();
        _behavior = "stop";
        _reason = "no_echo_stop";
        _face.setFace("sleepy");
      }
      return;
    }

    // มีเป้าหมายชัดเจนในระยะ (8 - 180 cm)
    _lostStartTime = 0;
    _targetLocked = true;

    // 4. ทักทาย (Greeting): คนยืนอยู่ในระยะ 50 - 65 cm และเว้นช่วง cooldown 12 วินาที
    if (dist >= 50.0f && dist <= 65.0f && (now - _lastGreetTime >= 12000)) {
      _lastGreetTime = now;
      _motor.stopMotors();
      _behavior = "greet";
      _reason = "greeting";
      _face.setTemporaryFace("love", 1600);
      _song.playById("sfx_happy");
      _motion.playById("wave");
      Serial.println("[INTERACTIVE] Person in front! Waving and greeting.");
      return;
    }

    // หากกำลังทำท่าทางทักทายอยู่ ให้รอจนท่าจบก่อน
    if (_motion.isMoving()) {
      _motor.stopMotors();
      _behavior = "greet";
      _reason = "greeting";
      return;
    }

    // ------------------------------------------------------------------------
    // 5. DIRECT if-else DISTANCE FOLLOWING (ตอบสนองฉับไวทันที!)
    // ------------------------------------------------------------------------
    if (dist < 35.0f) {
      // < 35 cm: ใกล้เกินไป อันตราย -> หยุดสนิททันที!
      _motor.stopMotors();
      _behavior = "stop";
      _reason = "too_close_stop";
      _face.setFace("surprise");
    } else if (dist < 50.0f) {
      // 35 - 50 cm: คนเดินเข้ามาชิดเกินไป -> ถอยหลังช้า ๆ (45%)
      _motor.moveBackward(45);
      _behavior = "avoid";
      _reason = "escape_reverse";
      _face.setFace("surprise");
    } else if (dist <= 65.0f) {
      // 50 - 65 cm: TARGET ZONE (เป้าหมาย 60 cm) -> หยุดรักษาระยะ สบตา
      _motor.stopMotors();
      _behavior = "follow";
      _reason = "follow_hold";
      _face.setFace("love");
    } else if (dist <= 80.0f) {
      // 65 - 80 cm: เดินหน้าช้า (50%)
      _motor.moveForward(50);
      _behavior = "follow";
      _reason = "follow_cruise";
      _face.setFace("normal");
    } else if (dist <= 100.0f) {
      // 80 - 100 cm: เดินหน้าปกติ (75%)
      _motor.moveForward(75);
      _behavior = "follow";
      _reason = "follow_cruise";
      _face.setFace("happy");
    } else {
      // 100 - 180 cm: เป้าหมายอยู่ไกล -> เดินหน้าเร็วเต็มกำลัง (100%)
      _motor.moveForward(100);
      _behavior = "follow";
      _reason = "follow_cruise";
      _face.setFace("happy");
    }

    // Serial Debug Monitor (Rate-limited 300ms)
    if (now - _lastLogTime >= 300) {
      _lastLogTime = now;
      Serial.printf("[INTERACTIVE] Dist: %.1f cm | %s | %s\n", dist, _behavior, _reason);
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
