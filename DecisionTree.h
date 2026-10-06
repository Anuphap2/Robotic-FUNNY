#ifndef DECISION_TREE_H
#define DECISION_TREE_H
#include <stdint.h>

// Range-only evidence. No leaf asserts that an echo is a human or a wall.
enum class TargetClass { NO_DATA, TOO_CLOSE, COLLECTING, UNSTABLE, CLEAR, MOVING, STATIC_OBJECT };
struct RangeEvidence {
  bool valid;
  uint8_t samples;
  int16_t distanceCm, deltaCm, maxStepCm;
};
constexpr TargetClass decideTarget(RangeEvidence e, int stopCm, int maxCm, int deltaCm) {
  return !e.valid ? TargetClass::NO_DATA
    : e.distanceCm <= stopCm ? TargetClass::TOO_CLOSE
    : e.samples < 3 ? TargetClass::COLLECTING
    : e.maxStepCm > deltaCm * 4 ? TargetClass::UNSTABLE
    : e.distanceCm > maxCm ? TargetClass::CLEAR
    : (e.deltaCm >= deltaCm || e.deltaCm <= -deltaCm) ? TargetClass::MOVING
    : TargetClass::STATIC_OBJECT;
}
inline const char* targetClassName(TargetClass target) {
  switch (target) {
    case TargetClass::TOO_CLOSE: return "too_close";
    case TargetClass::COLLECTING: return "collecting";
    case TargetClass::UNSTABLE: return "unstable";
    case TargetClass::CLEAR: return "clear";
    case TargetClass::MOVING: return "moving_target";
    case TargetClass::STATIC_OBJECT: return "static_object";
    default: return "no_data";
  }
}
// These exercise the production decision tree during every firmware build.
static_assert(decideTarget({false,6,100,20,10},25,180,8) == TargetClass::NO_DATA, "Missing echo never means clear");
static_assert(decideTarget({true,6,25,20,10},25,180,8) == TargetClass::TOO_CLOSE, "Stop boundary precedes movement");
static_assert(decideTarget({true,2,100,20,10},25,180,8) == TargetClass::COLLECTING, "Require independent samples");
static_assert(decideTarget({true,6,100,50,33},25,180,8) == TargetClass::UNSTABLE, "Discontinuous echo is ambiguous");
static_assert(decideTarget({true,6,181,20,10},25,180,8) == TargetClass::CLEAR, "Distant echo outside tracking range");
static_assert(decideTarget({true,6,100,8,4},25,180,8) == TargetClass::MOVING, "Receding candidate");
static_assert(decideTarget({true,6,100,-8,4},25,180,8) == TargetClass::MOVING, "Approaching candidate");
static_assert(decideTarget({true,6,100,7,4},25,180,8) == TargetClass::STATIC_OBJECT, "Noise is not person evidence");

// Six readings, at most twelve bytes; only readings while the body is stopped
// enter this window. Robot motion must never become apparent target motion.
class RangeWindow {
public:
  constexpr void clear() { _count = 0; }
  constexpr void add(int16_t cm) {
    if (_count == 6) { for (uint8_t i=1;i<6;++i) _cm[i-1]=_cm[i]; --_count; }
    _cm[_count++] = cm;
  }
  constexpr RangeEvidence evidence(bool valid, int16_t distance) const {
    int16_t step = 0;
    for (uint8_t i=1;i<_count;++i) {
      int16_t d = _cm[i] - _cm[i-1]; if (d < 0) d = -d;
      if (d > step) step = d;
    }
    return {valid, _count, distance, static_cast<int16_t>(_count ? _cm[_count-1]-_cm[0] : 0), step};
  }
private:
  int16_t _cm[6] = {};
  uint8_t _count = 0;
};
constexpr bool rangeWindowRegression() {
  RangeWindow window;
  window.add(90); window.add(94); window.add(98);
  auto moving = window.evidence(true, 98);
  if (moving.samples != 3 || moving.deltaCm != 8 || moving.maxStepCm != 4 ||
      decideTarget(moving,25,180,8) != TargetClass::MOVING) return false;
  window.clear();
  if (window.evidence(true,98).samples != 0) return false;
  for (int i=0;i<9;++i) window.add(100);
  auto still = window.evidence(true,100);
  if (still.samples != 6 || still.deltaCm != 0 || still.maxStepCm != 0 ||
      decideTarget(still,25,180,8) != TargetClass::STATIC_OBJECT) return false;
  window.add(140);
  return decideTarget(window.evidence(true,140),25,180,8) == TargetClass::UNSTABLE;
}
static_assert(rangeWindowRegression(), "Range window must clear, cap samples and reject discontinuities");
#endif
