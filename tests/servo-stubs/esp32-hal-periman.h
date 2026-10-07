#pragma once
#include "esp32-hal-ledc.h"
constexpr int ESP32_BUS_TYPE_LEDC = 1;
inline constexpr ledc_channel_handle_t servoTestBus[] = {{0,16,0},{1,16,0},{2,16,0}};
constexpr const ledc_channel_handle_t* perimanGetPinBus(uint8_t pin, int) {
  return pin == 18 ? &servoTestBus[0] : pin == 19 ? &servoTestBus[1] : pin == 23 ? &servoTestBus[2] : nullptr;
}
