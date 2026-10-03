#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "Config.h"

// ============================================================================
// SENSOR MANAGER (Non-blocking Sampling, Noise Filtering, Beat Detection)
// ============================================================================
class SensorManager {
public:
  SensorManager() :
    _micBaseline(2000),
    _currentVolume(0),
    _peakVolume(0),
    _currentDistance(-1.0f),
    _lastSensorRead(0),
    _lastBeatTime(0),
    _beatDetected(false),
    _clapThreshold(140) {}

  void begin() {
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    pinMode(PIN_MIC_AO, INPUT);

    digitalWrite(PIN_TRIG, LOW);

    // Baseline calibration for microphone KY-037
    long sum = 0;
    for (int i = 0; i < 60; i++) {
      sum += analogRead(PIN_MIC_AO);
      delay(4);
    }
    _micBaseline = sum / 60;
  }

  void update() {
    unsigned long now = millis();
    if (now - _lastSensorRead < 60) return; // 60ms sampling period (~16 Hz refresh)
    _lastSensorRead = now;

    // 1. Read Microphone (KY-037 Analog)
    int rawAudio = analogRead(PIN_MIC_AO);
    int instantVol = abs(rawAudio - _micBaseline);
    // Smooth EMA filter
    _currentVolume = (_currentVolume * 3 + instantVol * 5) / 8;

    if (instantVol > _peakVolume) {
      _peakVolume = instantVol;
    } else {
      _peakVolume = max(0, _peakVolume - 4); // Peak decay
    }

    // Beat / Clap Event Detection (for Recreational sound-reactive party)
    if (_currentVolume > _clapThreshold && (now - _lastBeatTime >= 280)) {
      _lastBeatTime = now;
      _beatDetected = true;
    }

    // 2. Read Ultrasonic Distance (HC-SR04/05)
    _currentDistance = _measureDistance();
  }

  int volume() const { return _currentVolume; }
  int peakVolume() const { return _peakVolume; }
  float distance() const { return _currentDistance; }

  bool consumeBeat() {
    if (_beatDetected) {
      _beatDetected = false;
      return true;
    }
    return false;
  }

  bool isObstacleNear(float thresholdCm = 20.0f) const {
    return (_currentDistance > 0.0f && _currentDistance < thresholdCm);
  }

  void setClapThreshold(int threshold) { _clapThreshold = threshold; }
  int clapThreshold() const { return _clapThreshold; }

private:
  int _micBaseline;
  int _currentVolume;
  int _peakVolume;
  float _currentDistance;
  unsigned long _lastSensorRead;
  unsigned long _lastBeatTime;
  bool _beatDetected;
  int _clapThreshold;

  float _measureDistance() {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    long duration = pulseIn(PIN_ECHO, HIGH, 22000); // 22ms timeout ~ 3.7 meters
    if (duration <= 0) return -1.0f;
    return (duration * 0.0343f) / 2.0f;
  }
};

#endif // SENSOR_MANAGER_H
