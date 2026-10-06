#ifndef RECREATION_ENGINE_H
#define RECREATION_ENGINE_H

#include <Arduino.h>
#include "FaceSystem.h"
#include "SongSystem.h"
#include "MotionSystem.h"
#include "MotorController.h"
#include "SensorManager.h"
#include "DecisionTree.h"

// ============================================================================
// RECREATION ENGINE (Coordinates Entertainment Modes & Companion Interactions)
// ============================================================================
enum RecreationMode {
  MODE_MANUAL = 0,        // Direct Web Control
  MODE_PARTY_BEAT = 1,    // Sound-Reactive Dancing (Clap/Music follower)
  MODE_INTERACTIVE = 2,   // Autonomous greeting, exploration and front-target following
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
    _lastProximityReaction(0) {}

  void begin() {
    _mode = MODE_MANUAL;
  }

  void setMode(RecreationMode mode, bool center = true) {
    _motor.stop();
    _motion.freeze();
    _song.stop();
    _targetLocked = false; _moving.clear(); _greetedNearby = false;
    _window.clear(); _cruise.stop(); _pulseActive = false; _turns = 0;
    _retry.reset(); _heldFarReadings = 0;
    _target = TargetClass::NO_DATA; _reason = "inactive";
    _sequence = _sensor.distanceSequence(); _observeAfter = millis();
    _behavior = "observe";
    _mode = mode;
    _showcaseActive = false;

    if (_mode == MODE_MANUAL) {
      _face.setFace("normal");
      if (center) _motion.toCenter();
      _motor.stop();
    } else if (_mode == MODE_PARTY_BEAT) {
      _lastPartyMove = millis();
      _partyRested = false;
      _face.setFace("music");
      _face.setTemporaryFace("happy", 1200);
      _song.playById("sfx_happy");
    } else if (_mode == MODE_INTERACTIVE) {
      _face.setFace("normal");
      _song.toneBeep(660, 100);
    }
  }

  bool startInteractive() {
    if (!robotSettings.get(SONAR_ENABLED) || !robotSettings.get(MOTOR_ENABLED) || !_motor.ready()) return false;
    setMode(MODE_INTERACTIVE);
    _reason = "collecting";
    return true;
  }
  const char* interactiveString() const { return "auto"; }
  const char* behaviorString() const { return _behavior; }
  const char* decisionClass() const { return targetClassName(_target); }
  const char* decisionReason() const { return _reason; }
  bool targetLocked() const { return _targetLocked; }
  uint8_t movingVotes() const { return _moving.votes(); }

  RecreationMode mode() const { return _mode; }

  const char* modeString() const {
    switch (_mode) {
      case MODE_PARTY_BEAT:   return "party";
      case MODE_INTERACTIVE:  return "interactive";
      case MODE_SHOWCASE:     return "showcase";
      default:                return "manual";
    }
  }

  // Start synchronized song & dance show
  bool startShowcase(const char* songId, const char* motionId) {
    _motor.stop();
    _targetLocked = false; _moving.clear(); _cruise.stop(); _pulseActive = false; _window.clear(); _reason = "inactive";
    _song.stop(); _motion.freeze();
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
    _song.stop(); _motion.freeze();
    _mode = MODE_MANUAL; _showcaseActive = false;
    return false;
  }

  void update() {
    // 1. Showcase Mode: check if song ended
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

    // 3. Proximity Interactive Mode
    if (_mode == MODE_INTERACTIVE) {
      _updateInteractiveMode();
      return;
    }
  }

private:
  FaceEngine& _face;
  SongPlayer& _song;
  MotionPlayer& _motion;
  MotorController& _motor;
  SensorManager& _sensor;

  RecreationMode _mode;
  int _partyStep;
  unsigned long _lastPartyMove;
  bool _showcaseActive;
  unsigned long _lastProximityReaction;
  bool _partyRested = false;
  RangeWindow _window;
  MovingEvidence _moving;
  CruiseController _cruise;
  ScanRetry _retry;
  TargetClass _target = TargetClass::NO_DATA;
  const char* _reason = "inactive";
  const char* _behavior = "observe";
  bool _targetLocked = false, _pulseActive = false, _greetedNearby = false;
  uint8_t _turns = 0;
  uint8_t _heldFarReadings = 0;
  uint32_t _sequence = 0, _observeAfter = 0, _pulseEnd = 0;

  void _updatePartyMode() {
    unsigned long now = millis();

    // Beat / Clap Event Detected
    if (_sensor.consumeBeat()) {
      _partyStep = (_partyStep + 1) % 4;
      _lastPartyMove = now;
      _partyRested = false;

      // Dynamic reactive body gestures
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
      // Quiet down if no beat for 1.2 seconds
      if (_motion.isMoving() == false) {
        _motion.toCenter();
        _face.setFace("normal");
        _partyRested = true;
      }
    }
  }

  void _invalidate(const char* reason, TargetClass target) {
    _beginObservation(millis()); _targetLocked = false; _heldFarReadings = 0;
    _behavior = "stop";
    _window.clear(); _target = target; _reason = reason;
  }
  uint32_t _settleDuration() const {
    return sonarSettleMs(robotSettings.get(SONAR_INTERVAL));
  }
  void _beginObservation(uint32_t now) {
    _motor.stop(); _cruise.stop(); _pulseActive = false;
    _window.clear(); _moving.clear();
    _sequence = _sensor.distanceSequence(); _observeAfter = now + _settleDuration();
  }
  uint32_t _driveLease() const {
    return autonomousLeaseMs(robotSettings.get(SONAR_INTERVAL),
      robotSettings.get(SONAR_TIMEOUT), robotSettings.get(AUTO_PULSE));
  }
  CruiseLimits _cruiseLimits() const {
    return {static_cast<int16_t>(robotSettings.get(AUTO_STOP)),
      roamStartCm(robotSettings.get(AUTO_STOP), robotSettings.get(AUTO_CLEAR)),
      static_cast<int16_t>(robotSettings.get(FOLLOW_DISTANCE)),
      static_cast<int16_t>(robotSettings.get(FOLLOW_MAX)),
      cruiseScanMs(robotSettings.get(AUTO_OBSERVE))};
  }
  void _startCruise(CruiseMode mode, uint32_t now, int16_t cm) {
    if (!_motor.cruise(_driveLease())) { _invalidate("hardware_disabled", TargetClass::NO_DATA); return; }
    _cruise.start(mode, now, cm); _window.clear(); _moving.clear(); _heldFarReadings = 0;
    _behavior = mode == CruiseMode::FOLLOW ? "follow" : "roam";
    _reason = mode == CruiseMode::FOLLOW ? "follow_cruise" : "roam_cruise";
  }
  void _startPulse(MotorController::Direction direction, uint32_t duration, const char* reason) {
    if (!_motor.pulse(direction, duration)) { _invalidate("hardware_disabled", TargetClass::NO_DATA); return; }
    _pulseActive = true; _pulseEnd = millis() + duration;
    _observeAfter = _pulseEnd + _settleDuration();
    _window.clear(); _moving.clear(); _reason = reason;
  }
  void _updateInteractiveMode() {
    uint32_t now = millis();
    if (!robotSettings.get(MOTOR_ENABLED) || !robotSettings.get(SONAR_ENABLED) || !_motor.ready()) {
      _invalidate("hardware_disabled", TargetClass::NO_DATA); return;
    }
    if (!_sensor.distanceValid() || !autonomousRangeFresh(_sensor.distanceAge(),
        robotSettings.get(SONAR_INTERVAL), _driveLease())) {
      _invalidate("no_echo_stop", TargetClass::NO_DATA); return;
    }
    int16_t cm = static_cast<int16_t>(_sensor.distance());
    if (cm > robotSettings.get(PROX_GREETING) + 10) _greetedNearby = false;
    if (cm <= robotSettings.get(AUTO_STOP)) {
      _invalidate("too_close_stop", TargetClass::TOO_CLOSE);
      _updateGreeting(true); return;
    }
    if (_cruise.active()) {
      bool fresh = _sequence != _sensor.distanceSequence();
      if (fresh) _sequence = _sensor.distanceSequence();
      auto action = _cruise.update(now, true, fresh, cm, _cruiseLimits());
      switch (action) {
        case CruiseAction::KEEP:
          if (fresh && _targetLocked && cm > robotSettings.get(FOLLOW_MAX)) _reason = "checking_target";
          return; // Same echo never renews the motor lease.
        case CruiseAction::RENEW:
          if (!_motor.cruise(_driveLease())) _invalidate("hardware_disabled", TargetClass::NO_DATA);
          else _reason = _targetLocked ? "follow_cruise" : "roam_cruise";
          return;
        case CruiseAction::LOST: _invalidate("target_lost", TargetClass::CLEAR); return;
        case CruiseAction::FAULT: _invalidate("no_echo_stop", TargetClass::NO_DATA); return;
        case CruiseAction::OBSERVE:
          _beginObservation(now); _behavior = "observe"; _reason = "periodic_scan"; return;
        case CruiseAction::BRAKE:
          _beginObservation(now); _behavior = "avoid"; _reason = "obstacle_brake"; return;
        case CruiseAction::HOLD:
          _beginObservation(now); _behavior = "follow"; _reason = "follow_hold"; return;
      }
    }
    if (_targetLocked && cm > robotSettings.get(FOLLOW_MAX)) {
      _motor.stop(); _behavior = "follow"; _reason = "checking_target";
      if (_sequence != _sensor.distanceSequence()) {
        _sequence = _sensor.distanceSequence(); ++_heldFarReadings;
        _window.clear(); _moving.clear();
      }
      if (_heldFarReadings >= 2) _invalidate("target_lost", TargetClass::CLEAR);
      return;
    }
    _heldFarReadings = 0;
    if (_pulseActive) {
      if (static_cast<int32_t>(now - _pulseEnd) < 0) return;
      _motor.stop(); _pulseActive = false; _window.clear();
    }
    if (static_cast<int32_t>(now - _observeAfter) < 0) { _reason = "settling"; _behavior = "observe"; return; }
    if (_motion.isMoving()) {
      _sequence = _sensor.distanceSequence(); _observeAfter = now + _settleDuration();
      _reason = "greeting"; _behavior = "greet"; return;
    }
    bool fresh = false;
    if (_sequence != _sensor.distanceSequence()) {
      _sequence = _sensor.distanceSequence();
      // Measurements during body motion or the stationary settling wait are discarded.
      if (static_cast<int32_t>((now - _sensor.distanceAge()) - _observeAfter) >= 0) { _window.add(cm); fresh = true; }
    }
    auto evidence = _window.evidence(true, cm);
    _target = decideTarget(evidence, robotSettings.get(AUTO_STOP), robotSettings.get(FOLLOW_MAX), robotSettings.get(TARGET_DELTA));
    if (fresh) _moving.add(_target, evidence.deltaCm);
    bool greetingDue = !_greetedNearby && cm <= robotSettings.get(PROX_GREETING) &&
      now - _lastProximityReaction >= static_cast<uint32_t>(robotSettings.get(PROX_COOLDOWN));
    auto action = decideAutomatic({_target, _targetLocked, _moving.votes(), cm, greetingDue,
      _turns, static_cast<uint8_t>(robotSettings.get(AUTO_TURN_LIMIT))},
      robotSettings.get(FOLLOW_DISTANCE), roamStartCm(robotSettings.get(AUTO_STOP), robotSettings.get(AUTO_CLEAR)));
    switch (action) {
      case AutoAction::STOP:
        _invalidate("ambiguous_stop", _target); return;
      case AutoAction::WAIT:
        _behavior = "observe";
        _reason = _target == TargetClass::MOVING ? "checking_movement" : "collecting"; return;
      case AutoAction::RELEASE:
        _invalidate("target_lost", _target); return;
      case AutoAction::ACQUIRE:
        // This is an automatic range target lock, never a claim of human identity.
        _targetLocked = true; _turns = 0; _retry.clearWait();
        _behavior = "follow"; _reason = "target_acquired";
        _face.setTemporaryFace("happy", 800); return;
      case AutoAction::FOLLOW:
        _startCruise(CruiseMode::FOLLOW, now, cm); return;
      case AutoAction::HOLD:
        _behavior = "follow"; _reason = "follow_hold";
        if (greetingDue) _updateGreeting(false); return;
      case AutoAction::ROAM:
        _turns = 0; _retry.clearWait();
        _startCruise(CruiseMode::ROAM, now, cm); return;
      case AutoAction::TURN:
        _behavior = "avoid"; ++_turns;
        _startPulse(_retry.right() ? MotorController::RIGHT : MotorController::LEFT,
          robotSettings.get(AUTO_TURN), _retry.right() ? "scan_right" : "scan_turn"); return;
      case AutoAction::BLOCKED:
        // Bound each scan batch, then try the other direction after a rest.
        // Still requires fresh stationary evidence and clearance above AUTO_STOP.
        _behavior = "avoid"; _reason = "avoid_retry";
        if (_retry.ready(now, fresh, robotSettings.get(AUTO_OBSERVE))) {
          _turns = 1;
          _startPulse(_retry.right() ? MotorController::RIGHT : MotorController::LEFT,
            robotSettings.get(AUTO_TURN), _retry.right() ? "scan_right" : "scan_turn");
        }
        return;
    }
  }

  void _updateGreeting(bool tooClose) {
    unsigned long now = millis();
    if (now - _lastProximityReaction < (uint32_t)robotSettings.get(PROX_COOLDOWN)) return;
    if (!_sensor.distanceValid() || _greetedNearby) return;

    float dist = _sensor.distance();
    if (tooClose || (dist > 0.0f && dist < robotSettings.get(PROX_CLOSE))) {
      // Too close! Playful surprise reaction
      _lastProximityReaction = now; _greetedNearby = true;
      _face.setTemporaryFace("surprise", 1600);
      _song.playById("sfx_alarm");
      _motion.playById("head_shake");
    } else if (dist >= robotSettings.get(PROX_CLOSE) && dist <= robotSettings.get(PROX_GREETING)) {
      // Friendly greeting zone!
      _lastProximityReaction = now; _greetedNearby = true;
      _face.setTemporaryFace("happy", 1800);
      _song.playById("sfx_happy");
      _motion.playById("wave");
    }
  }
};

#endif // RECREATION_ENGINE_H
