#pragma once
#include "esp32-hal-ledc.h"
enum ledc_mode_t { LEDC_HIGH_SPEED_MODE = 0 };
enum ledc_timer_t { LEDC_TIMER_0 = 0 };
constexpr uint32_t ledc_get_freq(ledc_mode_t mode, ledc_timer_t timer) {
  return mode != LEDC_HIGH_SPEED_MODE || timer != LEDC_TIMER_0 ? 0 : SERVO_TEST_MODE == 2 ? 1000 : 50;
}
