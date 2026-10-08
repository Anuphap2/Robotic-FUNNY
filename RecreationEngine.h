#ifndef RECREATION_ENGINE_H
#define RECREATION_ENGINE_H

#include <Arduino.h>
#include "Config.h"
#include "FaceSystem.h"
#include "SongSystem.h"
#include "MotionSystem.h"
#include "MotorController.h"
#include "SensorManager.h"
#include "ObstacleAvoidance.h"

// ============================================================================
// RECREATION ENGINE (Autonomous Entertainment & Obstacle Avoidance Robot)
// ============================================================================
// ออกแบบตามคำขอ:
// 1. เดินสำรวจอิสระ (Roam) ไม่ชนกำแพง/สิ่งกีดขวาง (Obstacle Avoidance)
// 2. ตัดฟังก์ชันเดินตามคนทิ้งทั้งหมด เพื่อไม่ให้เกิดความสับสนหรือลังเล
// 3. เพลงและสีหน้าทำงานแยกจากการนำทาง หัวและแขนไม่ขยับตามเพลงระหว่างเดิน
// 4. หยุด -> ถอย -> สแกนขณะล้อหยุด -> คืนหัวตรง -> เลี้ยว -> วัดหน้าใหม่
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
    _lastHeadAngle(-1),
    _navigationAvoiding(false),
    _nextEntertainmentTime(0),
    _lastLogTime(0),
    _lastStartleAt(0),
    _hasStartled(false),
    _behavior("observe"),
    _reason("inactive") {}

  void begin() {
    _mode = MODE_MANUAL;
  }

  void setMode(RecreationMode mode, bool center = true) {
    _sensor.consumeLoudSound();
    _motor.stopMotors();
    _motion.freeze();
    _song.stop();
    _navigation.reset(millis());
    _lastHeadAngle = -1;
    _navigationAvoiding = false;
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

  bool startInteractive() {
    if (!robotSettings.get(SONAR_ENABLED) || !robotSettings.get(MOTOR_ENABLED) || !_motor.ready() ||
        !robotSettings.get(SERVO_ENABLED) || !_motion.headReady() || !_scanAnglesValid()) return false;
    setMode(MODE_INTERACTIVE);
    return true;
  }

  const char* interactiveString() const { return "auto"; }
  const char* behaviorString() const { return _behavior; }
  const char* decisionClass() const { return _navigationAvoiding ? "static_object" : strcmp(_behavior, "stop") == 0 ? "no_data" : "clear"; }
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
      _sensor.consumeLoudSound();
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
      _sensor.consumeLoudSound();
      return;
    }

    // 3. Autonomous Entertainment & Obstacle Avoidance (Interactive Mode)
    if (_mode == MODE_INTERACTIVE) {
      _updateInteractiveMode();
      _startleOnLoudSound();
      return;
    }

    // React while idle/manual too; party beat and showcase keep their own sound behavior.
    if (_mode == MODE_MANUAL) _startleOnLoudSound();
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

  ObstacleAvoidance _navigation;
  int _lastHeadAngle;
  bool _navigationAvoiding;

  uint32_t         _nextEntertainmentTime;
  uint32_t         _lastLogTime;
  uint32_t         _lastStartleAt;
  bool             _hasStartled;

  const char*      _behavior;
  const char*      _reason;

  void _startleOnLoudSound() {
    if (!_sensor.consumeLoudSound()) return;
    const uint32_t now = millis();
    if ((_hasStartled && now - _lastStartleAt < 1800) || _song.isPlaying()) return;
    _lastStartleAt = now; _hasStartled = true;
    _face.setTemporaryFace("surprise", 1600);
    _song.toneBeep(1400, 180);
  }

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
  bool _scanAnglesValid() const {
    const int center = robotSettings.get(HEAD_CENTER);
    const int left = constrain(center + HEAD_SCAN_LEFT_DEG - HEAD_SCAN_CENTER_DEG, 0, 180);
    const int right = constrain(center + HEAD_SCAN_RIGHT_DEG - HEAD_SCAN_CENTER_DEG, 0, 180);
    const int home = robotSettings.servoAngle(HEAD_MIN, center);
    const int l = robotSettings.servoAngle(HEAD_MIN, left);
    const int r = robotSettings.servoAngle(HEAD_MIN, right);
    return l != home && r != home && l != r;
  }

  void _updateInteractiveMode() {
    const uint32_t now = millis();
    if (!robotSettings.get(MOTOR_ENABLED) || !robotSettings.get(SONAR_ENABLED) || !_motor.ready() ||
        !robotSettings.get(SERVO_ENABLED) || !_motion.headReady() || !_scanAnglesValid()) {
      _motor.stopMotors(); _behavior = "stop"; _reason = "hardware_disabled";
      _navigation.reset(now); _lastHeadAngle = -1; _navigationAvoiding = false;
      return;
    }

    ObstacleAvoidance::Settings cfg;
    cfg.center = robotSettings.get(HEAD_CENTER);
    // Logical angles: MotionPlayer applies reverse/trim/physical limits once.
    cfg.left = constrain(cfg.center + HEAD_SCAN_LEFT_DEG - HEAD_SCAN_CENTER_DEG, 0, 180);
    cfg.right = constrain(cfg.center + HEAD_SCAN_RIGHT_DEG - HEAD_SCAN_CENTER_DEG, 0, 180);
    cfg.stopCm = robotSettings.get(AUTO_STOP);
    cfg.settleMs = HEAD_SCAN_SETTLE_MS;
    cfg.reverseMs = AVOID_REVERSE_HOLD_MS;
    cfg.deadReverseMs = AVOID_DEADEND_HOLD_MS;
    cfg.turnMs = AVOID_TURN_MS;
    cfg.maxAgeMs = _sensor.distanceMaxAge();
    const auto command = _navigation.update(now, _sensor.reading(), cfg);
    _navigationAvoiding = command.avoiding;
    _reason = command.reason;
    _behavior = command.avoiding ? "avoid" : command.drive == ObstacleAvoidance::Forward ? "roam" : "stop";
    if (command.head != _lastHeadAngle) {
      // Stop before moving the scan head. Never renew the same servo target every loop.
      _motor.stopMotors();
      if (!_motion.setHeadAngle(command.head)) {
        _behavior = "stop"; _reason = "servo_write_failed";
        _navigation.reset(now); _navigationAvoiding = false;
        return;
      }
      _lastHeadAngle = command.head;
    }
    switch (command.drive) {
      case ObstacleAvoidance::Forward: _motor.driveFor(MotorController::FORWARD, command.leaseMs); break;
      case ObstacleAvoidance::Backward: _motor.driveFor(MotorController::BACKWARD, command.leaseMs); break;
      case ObstacleAvoidance::Left: _motor.driveFor(MotorController::LEFT, command.leaseMs); break;
      case ObstacleAvoidance::Right: _motor.driveFor(MotorController::RIGHT, command.leaseMs); break;
      default: _motor.stopMotors(); break;
    }
    if (!command.avoiding && !_song.isPlaying() && static_cast<int32_t>(now - _nextEntertainmentTime) >= 0)
      _triggerRandomEntertainment(now);
    if (now - _lastLogTime >= 400) {
      _lastLogTime = now;
      Serial.printf("[AVOID] stage=%d reason=%s front=%.1f L=%.1f R=%.1f\n",
                    _navigation.stage(), _reason, _sensor.distance(), _navigation.leftCm(), _navigation.rightCm());
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
