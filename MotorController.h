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

  // ไม่ใช้การซอย % PWM แล้ว เพื่อป้องกันการเบรกสลับ (L9110S Dynamic Braking) ขับตรงเต็มแรงดัน Direct Drive
  void setTrim(uint8_t leftPct, uint8_t rightPct) {
    (void)leftPct;
    (void)rightPct;
  }
  uint8_t leftTrim() const { return 100; }
  uint8_t rightTrim() const { return 100; }

  void stop() {
    portENTER_CRITICAL(&_mux); stopLocked(); portEXIT_CRITICAL(&_mux);
  }

  void moveForward() { request(FORWARD); }
  void moveBackward() { request(BACKWARD); }
  void turnLeft() { request(LEFT); }
  void turnRight() { request(RIGHT); }
  void spinDance() { request(SPIN); }
  void stopMotors() { stop(); }

  void setSpeed(uint8_t pct = 100) {
    (void)pct; // Direct Digital Drive เต็มกำลัง 100% เสมอ
  }

  void moveForward(uint8_t speed) { (void)speed; request(FORWARD); }
  void moveBackward(uint8_t speed) { (void)speed; request(BACKWARD); }
  void turnLeft(uint8_t speed) { (void)speed; request(LEFT); }
  void turnRight(uint8_t speed) { (void)speed; request(RIGHT); }

  void setMotor(int16_t leftSpeed, int16_t rightSpeed) {
    if (leftSpeed == 0 && rightSpeed == 0) {
      stop();
    } else if (leftSpeed > 0 && rightSpeed > 0) {
      request(FORWARD);
    } else if (leftSpeed < 0 && rightSpeed < 0) {
      request(BACKWARD);
    } else if (leftSpeed < 0 && rightSpeed > 0) {
      request(LEFT);
    } else if (leftSpeed > 0 && rightSpeed < 0) {
      request(RIGHT);
    }
  }

  // Autonomous pulses have their own short lease, enforced by the timer even
  // if the web server stalls. A pulse is issued once, never renewed in flight.
  bool pulse(Direction direction, uint32_t durationMs) {
    if (direction != FORWARD && direction != BACKWARD && direction != LEFT && direction != RIGHT) return false;
    return request(direction, constrain(durationMs, 60UL, 300UL));
  }

  // Continuous forward drive: extending the SAME direction does not drop GPIOs
  // or repeat direction-switch dead time.
  bool cruise(uint32_t leaseMs) {
    return request(FORWARD, constrain(leaseMs, 150UL, 600UL));
  }

  bool handleCommand(const String& cmd) {
    if (cmd == "stop") { stop(); return true; }
    if (cmd == "forward") { moveForward(); return true; }
    if (cmd == "backward") { moveBackward(); return true; }
    if (cmd == "left") { turnLeft(); return true; }
    if (cmd == "right") { turnRight(); return true; }
    if (cmd == "spin") { spinDance(); return true; }
    return false;
  }

  void update() {
    // Watchdog check and dead-time transition in loop context
    portENTER_CRITICAL(&_mux);
    if (!_enabled ||
        ((_dir != STOPPED || _pending != STOPPED) &&
         motorLeaseExpired(millis(), _lastCommandTime, _lease))) {
      stopLocked(); portEXIT_CRITICAL(&_mux); return;
    }
    if (_pending != STOPPED && millis() - _switchTime >= 5) {
      Direction next = _pending; _pending = STOPPED; _dir = next;
      applyOutputsLocked();
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
      applyOutputsLocked();
      portEXIT_CRITICAL(&_mux);
      return;
    }
    if (_dir == STOPPED) {
      outputsLow();
    }
    portEXIT_CRITICAL(&_mux);
  }

  void stopLocked() {
    outputsLow();
    _dir = STOPPED;
    _pending = STOPPED;
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

  void applyOutputsLocked() {
    if (_dir == STOPPED) {
      outputsLow();
      return;
    }

    bool leftForward = _dir == FORWARD || _dir == RIGHT || _dir == SPIN;
    bool rightForward = _dir == FORWARD || _dir == LEFT;
    leftForward ^= _leftReverse;
    rightForward ^= _rightReverse;

    // Direct Digital Saturation Drive: Continuous 100% full rail voltage
    setPinCached(PIN_A1A, _pinA1A, leftForward);
    setPinCached(PIN_A1B, _pinA1B, !leftForward);
    setPinCached(PIN_B1A, _pinB1A, rightForward);
    setPinCached(PIN_B1B, _pinB1B, !rightForward);
  }
};
#endif
