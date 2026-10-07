#pragma once
#include <stdint.h>
#ifndef SERVO_TEST_MODE
#define SERVO_TEST_MODE 0
#endif
#define SOC_LEDC_CHANNEL_NUM 8
struct ledc_channel_handle_t { uint8_t channel, channel_resolution, timer_num; };
constexpr bool ledcAttachChannel(uint8_t pin, uint32_t frequency, uint8_t bits, uint8_t channel) {
  return SERVO_TEST_MODE != 1 && frequency == 50 && bits == 16 &&
    ((pin == 18 && channel == 0) || (pin == 19 && channel == 1) || (pin == 23 && channel == 2));
}
// A fresh channel has zero duty: the real core's pin-level helper returns zero.
constexpr uint32_t ledcReadFreq(uint8_t) { return 0; }
constexpr bool ledcWrite(uint8_t, uint32_t duty) { return SERVO_TEST_MODE != 3 && duty < 65536; }
constexpr bool ledcDetach(uint8_t) { return true; }
