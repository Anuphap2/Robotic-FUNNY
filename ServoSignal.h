#ifndef SERVO_SIGNAL_H
#define SERVO_SIGNAL_H
#include <stdint.h>

constexpr uint32_t SERVO_FREQUENCY_HZ = 50;
constexpr uint8_t SERVO_RESOLUTION_BITS = 16;
constexpr uint32_t SERVO_PERIOD_US = 1000000 / SERVO_FREQUENCY_HZ;
constexpr uint32_t servoPulseUs(int angle, uint32_t minimum, uint32_t maximum) {
  return minimum + static_cast<uint32_t>(angle < 0 ? 0 : angle > 180 ? 180 : angle) * (maximum - minimum) / 180;
}
constexpr uint32_t servoPulseDuty(uint32_t pulseUs) {
  return static_cast<uint32_t>((static_cast<uint64_t>(pulseUs) * (1UL << SERVO_RESOLUTION_BITS) + SERVO_PERIOD_US / 2) / SERVO_PERIOD_US);
}
#endif
