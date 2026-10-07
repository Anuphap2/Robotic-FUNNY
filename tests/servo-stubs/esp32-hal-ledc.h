#pragma once
#include <stdint.h>
#ifndef SERVO_TEST_MODE
#define SERVO_TEST_MODE 0
#endif
constexpr bool ledcAttachChannel(uint8_t pin, uint32_t frequency, uint8_t bits, uint8_t channel) {
  return SERVO_TEST_MODE != 1 && frequency == 50 && bits == 16 &&
    ((pin == 18 && channel == 0) || (pin == 19 && channel == 1) || (pin == 23 && channel == 2));
}
constexpr uint32_t ledcReadFreq(uint8_t) { return SERVO_TEST_MODE == 2 ? 1000 : 50; }
constexpr bool ledcWrite(uint8_t, uint32_t duty) { return SERVO_TEST_MODE != 3 && duty < 65536; }
constexpr bool ledcDetach(uint8_t) { return true; }
