#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H
#include <Arduino.h>
#include "Config.h"
#include "RuntimeConfig.h"
#include <esp_timer.h>
#include <driver/gpio.h>

inline bool motorLeaseExpired(uint32_t now, uint32_t issued, uint32_t lease) {
  return (now - issued) >= lease;
}

class MotorController {
public:
  enum Direction { STOPPED, FORWARD, BACKWARD, LEFT, RIGHT, SPIN };
  void begin() {
    pinMode(PIN_A1A, OUTPUT); pinMode(PIN_A1B, OUTPUT);
    pinMode(PIN_B1A, OUTPUT); pinMode(PIN_B1B, OUTPUT);
    stop();
    applyConfig();
    esp_timer_create_args_t args = {};
    args.callback = timerTick;
    args.arg = this;
    args.dispatch_method = ESP_TIMER_TASK;
    args.name = "motor-guard";
    args.skip_unhandled_events = true;
    if (esp_timer_create(&args, &_watchdog) == ESP_OK) {
      _timerReady = esp_timer_start_periodic(_watchdog, 2000) == ESP_OK;
    }
  }
  bool ready() const { return _timerReady; }
  void applyConfig() {
    portENTER_CRITICAL(&_mux);
    stopLocked();
    _enabled = robotSettings.get(MOTOR_ENABLED);
    _timeout = robotSettings.get(MOTOR_TIMEOUT);
    _leftReverse = robotSettings.get(MOTOR_LEFT_REVERSE);
    _rightReverse = robotSettings.get(MOTOR_RIGHT_REVERSE);
    portEXIT_CRITICAL(&_mux);
  }
  void setTrim(uint8_t leftPct, uint8_t rightPct) {
    portENTER_CRITICAL(&_mux);
    _leftTrim = constrain(leftPct, MOTOR_MIN_SPEED_PCT, 100);
    _rightTrim = constrain(rightPct, MOTOR_MIN_SPEED_PCT, 100);
    portEXIT_CRITICAL(&_mux);
  }
  uint8_t leftTrim() const { return _leftTrim; }
  uint8_t rightTrim() const { return _rightTrim; }
  void stop() {
    portENTER_CRITICAL(&_mux); stopLocked(); portEXIT_CRITICAL(&_mux);
  }
  void moveForward() { moveForward(100); }
  void moveBackward() { moveBackward(SPEED_REVERSE_PCT); }
  void turnLeft() { turnLeft(SPEED_ROAM_TURN); }
  void turnRight() { turnRight(SPEED_ROAM_TURN); }
  void spinDance() { setSpeed(100); request(SPIN); }
  void stopMotors() { stop(); }
  void setSpeed(uint8_t pct) {
    // บอดี้หุ่นยนต์มีน้ำหนักมาก ล็อกแรงขับไว้ที่ 80% - 100% เท่านั้น ห้ามต่ำกว่า 80%
    uint8_t s = constrain(pct, MOTOR_MIN_SPEED_PCT, 100);
    setTrim(s, s);
  }
  void moveForward(uint8_t speed) { setSpeed(speed); request(FORWARD); }
  void moveBackward(uint8_t speed) { setSpeed(speed); request(BACKWARD); }
  void turnLeft(uint8_t speed) { setSpeed(speed); request(LEFT); }
  void turnRight(uint8_t speed) { setSpeed(speed); request(RIGHT); }
  void setMotor(int16_t leftSpeed, int16_t rightSpeed) {
    if (leftSpeed == 0 && rightSpeed == 0) {
      stop();
    } else if (leftSpeed > 0 && rightSpeed > 0) {
      setSpeed(constrain(max(leftSpeed, rightSpeed), (int16_t)MOTOR_MIN_SPEED_PCT, (int16_t)100));
      request(FORWARD);
    } else if (leftSpeed < 0 && rightSpeed < 0) {
      setSpeed(constrain(max((int16_t)(-leftSpeed), (int16_t)(-rightSpeed)), (int16_t)MOTOR_MIN_SPEED_PCT, (int16_t)100));
      request(BACKWARD);
    } else if (leftSpeed < 0 && rightSpeed > 0) {
      setSpeed(constrain((int16_t)rightSpeed, (int16_t)MOTOR_MIN_SPEED_PCT, (int16_t)100));
      request(LEFT);
    } else if (leftSpeed > 0 && rightSpeed < 0) {
      setSpeed(constrain((int16_t)leftSpeed, (int16_t)MOTOR_MIN_SPEED_PCT, (int16_t)100));
      request(RIGHT);
    }
  }
  // Autonomous pulses have their own short lease, enforced by the timer even
  // if the web server stalls. A pulse is issued once, never renewed in flight.
  bool pulse(Direction direction, uint32_t durationMs) {
    if (direction != FORWARD && direction != BACKWARD && direction != LEFT && direction != RIGHT) return false;
    setSpeed(100);
    return request(direction, constrain(durationMs, 60UL, 300UL));
  }
  // Continuous forward drive: extending the SAME direction does not drop GPIOs
  // or repeat direction-switch dead time. Call only for a distinct usable echo.
  bool cruise(uint32_t leaseMs) {
    setSpeed(100);
    return request(FORWARD, constrain(leaseMs, 150UL, 600UL));
  }
  bool handleCommand(const String& cmd) {
    if (cmd == "stop") { stop(); return true; }
    if (cmd == "forward") { moveForward(100); return true; }
    if (cmd == "backward") { moveBackward(SPEED_REVERSE_PCT); return true; }
    if (cmd == "left") { turnLeft(SPEED_ROAM_TURN); return true; }
    if (cmd == "right") { turnRight(SPEED_ROAM_TURN); return true; }
    if (cmd == "spin") { spinDance(); return true; }
    return false;
  }
  void update() {
    // Runs in the ESP timer task as well as the main loop. HTTP parsing or slow
    // clients cannot postpone the motor watchdog. All shared state is locked.
    portENTER_CRITICAL(&_mux);
    if (!_enabled ||
        ((_dir != STOPPED || _pending != STOPPED) &&
         motorLeaseExpired(millis(), _lastCommandTime, _lease))) {
      stopLocked(); portEXIT_CRITICAL(&_mux); return;
    }
    if (_pending != STOPPED && millis() - _switchTime >= 5) {
      Direction next = _pending; _pending = STOPPED; _dir = next;
      _accumL = 100; _accumR = 100;
      applyOutputsLocked(true, true);
    }
    portEXIT_CRITICAL(&_mux);
  }
  Direction direction() const {
    portENTER_CRITICAL(&_mux); Direction dir = _dir; portEXIT_CRITICAL(&_mux); return dir;
  }
  const char* directionString() const {
    switch (direction()) {
      case FORWARD: return "forward"; case BACKWARD: return "backward";
      case LEFT: return "left"; case RIGHT: return "right"; case SPIN: return "spin";
      default: return "stop";
    }
  }
private:
  Direction _dir = STOPPED, _pending = STOPPED;
  uint32_t _lastCommandTime = 0, _switchTime = 0;
  uint32_t _timeout = 2000;
  uint32_t _lease = 2000;
  bool _enabled = false, _leftReverse = false, _rightReverse = false, _timerReady = false;
  uint8_t _leftTrim = constrain(MOTOR_LEFT_TRIM_PCT, MOTOR_MIN_SPEED_PCT, 100);
  uint8_t _rightTrim = constrain(MOTOR_RIGHT_TRIM_PCT, MOTOR_MIN_SPEED_PCT, 100);
  uint8_t _accumL = 100, _accumR = 100;
  int8_t _pinA1A = -1, _pinA1B = -1, _pinB1A = -1, _pinB1B = -1;
  esp_timer_handle_t _watchdog = nullptr;
  mutable portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED;

  static void timerTick(void* arg) { static_cast<MotorController*>(arg)->onTimerTick(); }

  void onTimerTick() {
    portENTER_CRITICAL(&_mux);
    if (!_enabled ||
        ((_dir != STOPPED || _pending != STOPPED) &&
         motorLeaseExpired(millis(), _lastCommandTime, _lease))) {
      stopLocked(); portEXIT_CRITICAL(&_mux); return;
    }
    if (_pending != STOPPED && millis() - _switchTime >= 5) {
      Direction next = _pending; _pending = STOPPED; _dir = next;
      _accumL = 100; _accumR = 100;
      applyOutputsLocked(true, true);
      portEXIT_CRITICAL(&_mux);
      return;
    }
    if (_dir == STOPPED) {
      outputsLow();
      portEXIT_CRITICAL(&_mux);
      return;
    }

    bool activeL = true;
    if (_leftTrim < 100) {
      _accumL += _leftTrim;
      if (_accumL >= 100) { _accumL -= 100; activeL = true; }
      else { activeL = false; }
    }

    bool activeR = true;
    if (_rightTrim < 100) {
      _accumR += _rightTrim;
      if (_accumR >= 100) { _accumR -= 100; activeR = true; }
      else { activeR = false; }
    }

    applyOutputsLocked(activeL, activeR);
    portEXIT_CRITICAL(&_mux);
  }

  void stopLocked() {
    outputsLow();
    _dir = STOPPED;
    _pending = STOPPED;
    _accumL = 100;
    _accumR = 100;
  }

  void setPinCached(int pin, int8_t& current, bool high) {
    int8_t desired = high ? 1 : 0;
    if (current != desired) {
      current = desired;
      gpio_set_level(static_cast<gpio_num_t>(pin), desired);
    }
  }

  void outputsLow() {
    setPinCached(PIN_A1A, _pinA1A, false);
    setPinCached(PIN_A1B, _pinA1B, false);
    setPinCached(PIN_B1A, _pinB1A, false);
    setPinCached(PIN_B1B, _pinB1B, false);
  }

  bool request(Direction next, uint32_t lease = 0) {
    portENTER_CRITICAL(&_mux);
    if (!_enabled || !_timerReady) { portEXIT_CRITICAL(&_mux); return false; }
    _lastCommandTime = millis();
    _lease = lease ? min(lease, _timeout) : _timeout;
    if (next == _dir || next == _pending) { portEXIT_CRITICAL(&_mux); return true; }
    outputsLow(); _dir = STOPPED; _pending = next; _switchTime = millis();
    portEXIT_CRITICAL(&_mux);
    return true;
  }

  void applyOutputsLocked(bool activeL, bool activeR) {
    bool leftForward = _dir == FORWARD || _dir == RIGHT || _dir == SPIN;
    bool rightForward = _dir == FORWARD || _dir == LEFT;
    leftForward ^= _leftReverse;
    rightForward ^= _rightReverse;

    if (activeL) {
      setPinCached(PIN_A1A, _pinA1A, leftForward);
      setPinCached(PIN_A1B, _pinA1B, !leftForward);
    } else {
      setPinCached(PIN_A1A, _pinA1A, false);
      setPinCached(PIN_A1B, _pinA1B, false);
    }

    if (activeR) {
      setPinCached(PIN_B1A, _pinB1A, rightForward);
      setPinCached(PIN_B1B, _pinB1B, !rightForward);
    } else {
      setPinCached(PIN_B1A, _pinB1A, false);
      setPinCached(PIN_B1B, _pinB1B, false);
    }
  }
};
#endif
