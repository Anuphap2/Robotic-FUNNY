#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H
#include <Arduino.h>
#include "Config.h"
#include "RuntimeConfig.h"
#include <esp_timer.h>
#include <driver/gpio.h>

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
      _timerReady = esp_timer_start_periodic(_watchdog, 5000) == ESP_OK;
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
  void stop() {
    portENTER_CRITICAL(&_mux); stopLocked(); portEXIT_CRITICAL(&_mux);
  }
  void moveForward() { request(FORWARD); }
  void moveBackward() { request(BACKWARD); }
  void turnLeft() { request(LEFT); }
  void turnRight() { request(RIGHT); }
  void spinDance() { request(SPIN); }
  // Autonomous pulses have their own short lease, enforced by the timer even
  // if the web server stalls. A pulse is issued once, never renewed in flight.
  bool pulse(Direction direction, uint32_t durationMs) {
    if (direction != FORWARD && direction != LEFT && direction != RIGHT) return false;
    return request(direction, constrain(durationMs, 60UL, 300UL));
  }
  bool handleCommand(const String& cmd) {
    if (cmd == "stop") { stop(); return true; }
    if (cmd == "forward") return request(FORWARD);
    if (cmd == "backward") return request(BACKWARD);
    if (cmd == "left") return request(LEFT);
    if (cmd == "right") return request(RIGHT);
    if (cmd == "spin") return request(SPIN);
    return false;
  }
  void update() {
    // Runs in the ESP timer task as well as the main loop. HTTP parsing or slow
    // clients cannot postpone the motor watchdog. All shared state is locked.
    portENTER_CRITICAL(&_mux);
    if (!_enabled ||
        ((_dir != STOPPED || _pending != STOPPED) &&
         millis() - _lastCommandTime >= _lease)) {
      stopLocked(); portEXIT_CRITICAL(&_mux); return;
    }
    if (_pending != STOPPED && millis() - _switchTime >= 5) {
      Direction next = _pending; _pending = STOPPED; drive(next);
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
  esp_timer_handle_t _watchdog = nullptr;
  mutable portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED;
  static void timerTick(void* arg) { static_cast<MotorController*>(arg)->update(); }
  void stopLocked() { outputsLow(); _dir = STOPPED; _pending = STOPPED; }
  void writePin(int pin, bool high) { gpio_set_level(static_cast<gpio_num_t>(pin), high ? 1 : 0); }
  void outputsLow() {
    writePin(PIN_A1A, false); writePin(PIN_A1B, false);
    writePin(PIN_B1A, false); writePin(PIN_B1B, false);
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
  void drive(Direction next) {
    bool leftForward = next == FORWARD || next == RIGHT || next == SPIN;
    bool rightForward = next == FORWARD || next == LEFT;
    leftForward ^= _leftReverse; rightForward ^= _rightReverse;
    writePin(PIN_A1A, leftForward); writePin(PIN_A1B, !leftForward);
    writePin(PIN_B1A, rightForward); writePin(PIN_B1B, !rightForward);
    _dir = next;
  }
};
#endif
