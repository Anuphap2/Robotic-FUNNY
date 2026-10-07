#ifndef SERVO_OUTPUT_H
#define SERVO_OUTPUT_H
#include <Arduino.h>
#include "esp32-hal-ledc.h"
#include "ServoSignal.h"

// Explicit channels use the core's actual attachment result instead of an
// independent allocator's bookkeeping. Core 3.x selects compatible timers.
// All three outputs share 50 Hz / 16 bits; buzzer channel 6 stays separate.
class ServoOutput {
public:
  constexpr explicit ServoOutput(uint8_t channel) : _channel(channel) {}
  constexpr bool attach(uint8_t pin, uint32_t minimum, uint32_t maximum) {
    if (minimum >= maximum || maximum >= SERVO_PERIOD_US) return false;
    if (_attached) detach();
    _pin = pin; _minimum = minimum; _maximum = maximum;
    _attached = ledcAttachChannel(pin, SERVO_FREQUENCY_HZ, SERVO_RESOLUTION_BITS, _channel);
    if (_attached && ledcReadFreq(pin) != SERVO_FREQUENCY_HZ) { detach(); return false; }
    return _attached;
  }
  constexpr bool attached() const { return _attached && ledcReadFreq(_pin) == SERVO_FREQUENCY_HZ; }
  constexpr void detach() {
    if (!_attached) return;
    ledcWrite(_pin, 0); ledcDetach(_pin); _attached = false;
    pinMode(_pin, OUTPUT); digitalWrite(_pin, LOW);
  }
  constexpr bool write(int angle) {
    if (!attached()) return false;
    const uint32_t duty = servoPulseDuty(servoPulseUs(angle, _minimum, _maximum));
    return ledcWrite(_pin, duty); // Duty is latched at a PWM boundary; no immediate readback comparison.
  }
private:
  const uint8_t _channel;
  uint8_t _pin = 0;
  bool _attached = false;
  uint32_t _minimum = 500, _maximum = 2400;
};
#endif
