#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

// ============================================================================
// L9110S DUAL DC MOTOR CONTROLLER CLASS
// ============================================================================
class MotorController {
public:
  enum Direction { STOPPED, FORWARD, BACKWARD, LEFT, RIGHT, SPIN };

  MotorController() :
    _dir(STOPPED),
    _lastCommandTime(0),
    _autoTimeoutMs(2000),
    _autoTimeoutEnabled(true) {}

  void begin() {
    pinMode(PIN_A1A, OUTPUT);
    pinMode(PIN_A1B, OUTPUT);
    pinMode(PIN_B1A, OUTPUT);
    pinMode(PIN_B1B, OUTPUT);
    stop();
  }

  void stop() {
    digitalWrite(PIN_A1A, LOW);
    digitalWrite(PIN_A1B, LOW);
    digitalWrite(PIN_B1A, LOW);
    digitalWrite(PIN_B1B, LOW);
    _dir = STOPPED;
  }

  void moveForward() {
    digitalWrite(PIN_A1A, HIGH);
    digitalWrite(PIN_A1B, LOW);
    digitalWrite(PIN_B1A, HIGH);
    digitalWrite(PIN_B1B, LOW);
    _dir = FORWARD;
    _lastCommandTime = millis();
  }

  void moveBackward() {
    digitalWrite(PIN_A1A, LOW);
    digitalWrite(PIN_A1B, HIGH);
    digitalWrite(PIN_B1A, LOW);
    digitalWrite(PIN_B1B, HIGH);
    _dir = BACKWARD;
    _lastCommandTime = millis();
  }

  void turnLeft() {
    digitalWrite(PIN_A1A, LOW);
    digitalWrite(PIN_A1B, HIGH);
    digitalWrite(PIN_B1A, HIGH);
    digitalWrite(PIN_B1B, LOW);
    _dir = LEFT;
    _lastCommandTime = millis();
  }

  void turnRight() {
    digitalWrite(PIN_A1A, HIGH);
    digitalWrite(PIN_A1B, LOW);
    digitalWrite(PIN_B1A, LOW);
    digitalWrite(PIN_B1B, HIGH);
    _dir = RIGHT;
    _lastCommandTime = millis();
  }

  void spinDance() {
    turnRight();
    _dir = SPIN;
    _lastCommandTime = millis();
  }

  bool handleCommand(const String& cmd) {
    if (cmd == "forward") { moveForward(); return true; }
    if (cmd == "backward") { moveBackward(); return true; }
    if (cmd == "left") { turnLeft(); return true; }
    if (cmd == "right") { turnRight(); return true; }
    if (cmd == "stop") { stop(); return true; }
    if (cmd == "spin") { spinDance(); return true; }
    return false;
  }

  void update() {
    // Safety watchdog: stop motors if no command received within timeout
    if (_autoTimeoutEnabled && _dir != STOPPED) {
      if (millis() - _lastCommandTime >= _autoTimeoutMs) {
        stop();
      }
    }
  }

  void setAutoTimeout(uint32_t ms) { _autoTimeoutMs = ms; }
  void enableAutoTimeout(bool enable) { _autoTimeoutEnabled = enable; }
  Direction direction() const { return _dir; }

  const char* directionString() const {
    switch (_dir) {
      case FORWARD:  return "forward";
      case BACKWARD: return "backward";
      case LEFT:     return "left";
      case RIGHT:    return "right";
      case SPIN:     return "spin";
      default:       return "stop";
    }
  }

private:
  Direction _dir;
  unsigned long _lastCommandTime;
  uint32_t _autoTimeoutMs;
  bool _autoTimeoutEnabled;
};

#endif // MOTOR_CONTROLLER_H
