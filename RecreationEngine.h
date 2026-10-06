#ifndef RECREATION_ENGINE_H
#define RECREATION_ENGINE_H

#include <Arduino.h>
#include "FaceSystem.h"
#include "SongSystem.h"
#include "MotionSystem.h"
#include "MotorController.h"
#include "SensorManager.h"

// ============================================================================
// RECREATION ENGINE (Coordinates Entertainment Modes & Companion Interactions)
// ============================================================================
enum RecreationMode {
  MODE_MANUAL = 0,        // Direct Web Control
  MODE_PARTY_BEAT = 1,    // Sound-Reactive Dancing (Clap/Music follower)
  MODE_INTERACTIVE = 2,   // Proximity Greetings & Distance Reactions
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
      _song.playById("sfx_beep");
    }
  }

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

  void _updateInteractiveMode() {
    unsigned long now = millis();
    if (now - _lastProximityReaction < (uint32_t)robotSettings.get(PROX_COOLDOWN)) return;
    if (!_sensor.distanceValid()) return;

    float dist = _sensor.distance();
    if (dist > 0.0f && dist < robotSettings.get(PROX_CLOSE)) {
      // Too close! Playful surprise reaction
      _lastProximityReaction = now;
      _face.setTemporaryFace("surprise", 1600);
      _song.playById("sfx_alarm");
      _motion.playById("head_shake");
    } else if (dist >= robotSettings.get(PROX_CLOSE) && dist <= robotSettings.get(PROX_GREETING)) {
      // Friendly greeting zone!
      _lastProximityReaction = now;
      _face.setTemporaryFace("happy", 1800);
      _song.playById("sfx_happy");
      _motion.playById("wave");
    }
  }
};

#endif // RECREATION_ENGINE_H
