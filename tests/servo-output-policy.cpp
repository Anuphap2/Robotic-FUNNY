// Compile using -I tests/servo-stubs and SERVO_TEST_MODE 0, 1, 2, 3.
// Evaluates the actual production class; only the peripheral API is substituted.
#include "../ServoOutput.h"
static_assert(SERVO_FREQUENCY_HZ == 50 && SERVO_PERIOD_US == 20000, "20 ms servo frames");
static_assert(servoPulseUs(0, 500, 2400) == 500, "minimum pulse");
static_assert(servoPulseUs(90, 500, 2400) == 1450, "center pulse");
static_assert(servoPulseUs(180, 500, 2400) == 2400, "maximum pulse");
static_assert(servoPulseUs(-10, 500, 2400) == 500, "low angle clamp");
static_assert(servoPulseUs(200, 500, 2400) == 2400, "high angle clamp");
static_assert(servoPulseUs(90, 1000, 2000) == 1500, "saved pulse calibration");
static_assert(servoPulseDuty(500) == 1638, "minimum duty");
static_assert(servoPulseDuty(1450) == 4751, "center duty");
static_assert(servoPulseDuty(2400) == 7864, "maximum duty");
constexpr bool invalidConfiguration() {
  ServoOutput head(0);
  return !head.attached() && !head.write(90) &&
    !head.attach(18, 1000, 1000) && !head.attach(18, 2400, 500) &&
    !head.attach(18, 500, 20000) && !head.attached();
}
static_assert(invalidConfiguration(), "invalid settings never enable output");
constexpr bool peripheralLifecycle() {
  ServoOutput head(0), left(1), right(2);
  const bool headAttached = head.attach(18, 500, 2400);
#if SERVO_TEST_MODE == 1 || SERVO_TEST_MODE == 2
  return !headAttached && !head.attached() && !head.write(90);
#else
  if (!headAttached || !head.attached() || !left.attach(19, 500, 2400) || !right.attach(23, 500, 2400)) return false;
#if SERVO_TEST_MODE == 3
  return !head.write(90) && !left.write(90) && !right.write(90);
#else
  if (!head.write(0) || !left.write(90) || !right.write(180)) return false;
  left.detach();
  if (left.attached() || left.write(90) || !head.attached() || !right.attached()) return false;
  if (!left.attach(19, 1000, 2000) || !left.write(90)) return false;
  head.detach(); right.detach(); left.detach();
  return !head.attached() && !left.attached() && !right.attached();
#endif
#endif
}
static_assert(peripheralLifecycle(), "attach failure, wrong frequency, write failure, independent outputs and retry");
