#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H
#include <Arduino.h>
#include "Config.h"
#include "RuntimeConfig.h"
#include "SonarReading.h"

class SensorManager {
public:
  void begin() {
    pinMode(PIN_TRIG, OUTPUT); pinMode(PIN_ECHO, INPUT); pinMode(PIN_MIC_AO, INPUT);
    digitalWrite(PIN_TRIG, LOW); analogReadResolution(12);
    attachInterruptArg(digitalPinToInterrupt(PIN_ECHO), echoEdge, this, CHANGE);
    startCalibration();
  }
  void startCalibration() {
    _calibrating = robotSettings.get(MIC_ENABLED);
    _calibrationCount = 0; _calibrationSum = 0;
    _volume = 0; _peak = 0; _beat = false;
  }
  void applyConfig() {
    portENTER_CRITICAL(&_echoMux);
    _armed = false; _complete = false; _rose = false;
    _echoTimeoutUs = sonarTimeoutUs(robotSettings.get(SONAR_TIMEOUT));
    portEXIT_CRITICAL(&_echoMux);
    _reading.kind = SonarKind::Invalid;
    _distance = -1; _beat = false; _volume = 0; _peak = 0;
    if (!robotSettings.get(MIC_ENABLED)) _calibrating = false;
  }
  void update() {
    uint32_t now = millis();
    if (robotSettings.get(MIC_ENABLED)) {
      if (_calibrating && now - _lastMic >= 4) {
        _lastMic = now; _calibrationSum += analogRead(PIN_MIC_AO);
        if (++_calibrationCount >= 60) {
          _baseline = _calibrationSum / _calibrationCount; _calibrating = false;
        }
      } else if (!_calibrating && now - _lastMic >= (uint32_t)robotSettings.get(MIC_INTERVAL)) {
        _lastMic = now;
        int instant = abs(analogRead(PIN_MIC_AO) - _baseline);
        int weight = robotSettings.get(MIC_SMOOTH);
        _volume = (_volume * (100 - weight) + instant * weight) / 100;
        _peak = max(instant, max(0, _peak - 4));
        if (_volume > robotSettings.get(CLAP_THRESHOLD) && now - _lastBeat >= (uint32_t)robotSettings.get(BEAT_COOLDOWN)) {
          _lastBeat = now; _beat = true;
        }
      }
    }
    updateSonar(now);
  }
  int volume() const { return _volume; }
  int peakVolume() const { return _peak; }
  int baseline() const { return _baseline; }
  bool calibrating() const { return _calibrating; }
  float distance() const { return _distance; }
  uint32_t distanceSequence() const { return _distanceSequence; }
  const SonarReading& reading() const { return _reading; }
  uint32_t distanceAge() const { return millis() - _lastDistance; }
  uint32_t distanceMaxAge() const { return sonarIntervalMs(robotSettings.get(SONAR_INTERVAL)) * 4; }
  bool distanceValid() const { return _reading.kind == SonarKind::Echo && _reading.usable(millis(), distanceMaxAge()); }
  bool distanceNoEcho() const { return _reading.kind == SonarKind::NoEcho && _reading.usable(millis(), distanceMaxAge()); }
  bool consumeBeat() {
    bool found = _beat && millis() - _lastBeat < 500;
    _beat = false; return found;
  }
  bool isObstacleNear(float thresholdCm = 20) const { return distanceValid() && _distance < thresholdCm; }
private:
  int _baseline = 2000, _volume = 0, _peak = 0;
  float _distance = -1;
  SonarReading _reading;
  bool _beat = false, _calibrating = false;
  uint32_t _lastMic = 0, _lastBeat = 0, _lastSonar = 0, _lastDistance = 0;
  uint32_t _distanceSequence = 0;
  uint32_t _calibrationSum = 0;
  uint16_t _calibrationCount = 0;
  portMUX_TYPE _echoMux = portMUX_INITIALIZER_UNLOCKED;
  volatile bool _armed = false, _rose = false, _complete = false;
  volatile uint32_t _riseUs = 0, _pulseUs = 0;
  volatile uint32_t _completedMs = 0;
  volatile uint32_t _triggerUs = 0, _triggerMs = 0, _echoTimeoutUs = 40000;
  static void echoEdge(void* arg);
  void updateSonar(uint32_t now) {
    if (!robotSettings.get(SONAR_ENABLED)) { _reading.kind = SonarKind::Invalid; _distance = -1; return; }
    uint32_t pulse = 0, completedMs = 0;
    uint32_t startedMs = 0;
    SonarKind kind = SonarKind::Invalid;
    bool done, waiting;
    portENTER_CRITICAL(&_echoMux);
    done = _complete; waiting = _armed;
    if (done) {
      pulse = _pulseUs; completedMs = _completedMs; startedMs = _triggerMs;
      kind = sonarPulseKind(pulse, _echoTimeoutUs); _complete = false;
    }
    if (waiting && micros() - _triggerUs >= _echoTimeoutUs) {
      kind = sonarTimeoutKind(_rose, digitalRead(PIN_ECHO) == HIGH);
      // Stamp the actual deadline, not a later loop resumed after an HTTP stall.
      startedMs = _triggerMs; completedMs = _triggerMs + (_echoTimeoutUs + 999) / 1000;
      _armed = false; waiting = false; done = true;
    }
    portEXIT_CRITICAL(&_echoMux);
    if (done) {
      float cm = pulse * 0.01715f + robotSettings.get(DISTANCE_OFFSET) / 10.0f;
      if (kind == SonarKind::Echo && cm <= 0) kind = SonarKind::Invalid;
      _distance = kind == SonarKind::Echo ? cm : -1;
      // An echo received before an HTTP stall is NOT a new measurement now.
      _lastDistance = completedMs;
      ++_distanceSequence;
      _reading = {kind, _distance, _distanceSequence, startedMs, completedMs};
    }
    if (!waiting && now - _lastSonar >= sonarIntervalMs(robotSettings.get(SONAR_INTERVAL))) {
      _lastSonar = now;
      portENTER_CRITICAL(&_echoMux);
      if (digitalRead(PIN_ECHO) == HIGH) {
        // A stuck high pin must not be interpreted as open space.
        _reading = {SonarKind::Invalid, -1, ++_distanceSequence, now, now};
        _distance = -1; _lastDistance = now;
        portEXIT_CRITICAL(&_echoMux); return;
      }
      _triggerUs = micros(); _triggerMs = now;
      _echoTimeoutUs = sonarTimeoutUs(robotSettings.get(SONAR_TIMEOUT));
      _rose = false; _complete = false; _armed = true;
      portEXIT_CRITICAL(&_echoMux);
      digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10); digitalWrite(PIN_TRIG, LOW);
    }
  }
};
#endif
