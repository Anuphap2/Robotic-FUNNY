#ifndef DECISION_TREE_H
#define DECISION_TREE_H
#include <stdint.h>
#include "AutonomousDrive.h"

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

// Automatic behavior selection, independent of the web UI. A moving candidate
// needs two successive fresh, consistent decisions while the body is stopped.
class MovingEvidence {
public:
  constexpr void clear() { _votes = 0; _direction = 0; }
  constexpr void add(TargetClass target, int16_t delta) {
    if (target != TargetClass::MOVING) { clear(); return; }
    int8_t direction = delta > 0 ? 1 : -1;
    _votes = direction == _direction ? (_votes < 2 ? _votes + 1 : 2) : 1;
    _direction = direction;
  }
  constexpr uint8_t votes() const { return _votes; }
private:
  uint8_t _votes = 0;
  int8_t _direction = 0;
};
enum class AutoAction { STOP, WAIT, RELEASE, ACQUIRE, FOLLOW, HOLD, ROAM, TURN, BLOCKED };
struct AutoObservation {
  TargetClass target;
  bool locked;
  uint8_t movingVotes;
  int16_t distanceCm;
  bool greetingDue;
  uint8_t turns, turnLimit;
};
constexpr AutoAction decideAutomatic(AutoObservation o, int followCm, int clearCm) {
  return o.target == TargetClass::NO_DATA || o.target == TargetClass::TOO_CLOSE || o.target == TargetClass::UNSTABLE ? AutoAction::STOP
    : o.target == TargetClass::COLLECTING ? AutoAction::WAIT
    : o.locked && o.target == TargetClass::CLEAR ? AutoAction::RELEASE
    : !o.locked && o.target == TargetClass::MOVING ? (o.movingVotes >= 2 ? AutoAction::ACQUIRE : AutoAction::WAIT)
    : o.locked ? (o.distanceCm > followCm + 5 ? AutoAction::FOLLOW : AutoAction::HOLD)
    : o.distanceCm >= clearCm ? AutoAction::ROAM
    : o.turns >= o.turnLimit ? AutoAction::BLOCKED : AutoAction::TURN;
}
static_assert(decideAutomatic({TargetClass::MOVING,false,1,100,false,0,4},65,60) == AutoAction::WAIT, "Do not follow a single movement decision");
static_assert(decideAutomatic({TargetClass::MOVING,false,2,100,false,0,4},65,60) == AutoAction::ACQUIRE, "Acquire automatically after consistent movement");
static_assert(decideAutomatic({TargetClass::STATIC_OBJECT,true,0,100,false,0,4},65,60) == AutoAction::FOLLOW, "Maintain acquired front target without human input");
static_assert(decideAutomatic({TargetClass::STATIC_OBJECT,true,0,70,false,0,4},65,60) == AutoAction::HOLD, "Hold follow deadband boundary");
static_assert(decideAutomatic({TargetClass::CLEAR,true,0,181,false,0,4},65,60) == AutoAction::RELEASE, "Lose target before exploring again");
static_assert(decideAutomatic({TargetClass::STATIC_OBJECT,false,0,40,true,0,4},65,60) == AutoAction::TURN, "Do not pause to greet an untracked obstacle");
static_assert(decideAutomatic({TargetClass::STATIC_OBJECT,false,0,60,false,4,4},65,60) == AutoAction::ROAM, "Clear path resumes exploration automatically");
static_assert(decideAutomatic({TargetClass::STATIC_OBJECT,false,0,50,false,0,4},65,60) == AutoAction::TURN, "Scan when front path is obstructed");
static_assert(decideAutomatic({TargetClass::STATIC_OBJECT,false,0,50,false,4,4},65,60) == AutoAction::BLOCKED, "Bound blind turning");
static_assert(decideAutomatic({TargetClass::TOO_CLOSE,true,2,20,true,0,4},65,60) == AutoAction::STOP, "Safety overrides acquired target and greeting");
static_assert(decideAutomatic({TargetClass::UNSTABLE,true,2,100,false,0,4},65,60) == AutoAction::STOP, "Ambiguous evidence overrides following");
static_assert(decideAutomatic({TargetClass::NO_DATA,false,0,300,false,0,4},65,60) == AutoAction::STOP, "No echo never starts roaming");
constexpr bool movingEvidenceRegression() {
  MovingEvidence evidence;
  evidence.add(TargetClass::MOVING,8);
  if (evidence.votes()!=1) return false;
  evidence.add(TargetClass::MOVING,12);
  if (evidence.votes()!=2) return false;
  evidence.add(TargetClass::MOVING,-8);
  if (evidence.votes()!=1) return false;
  evidence.add(TargetClass::STATIC_OBJECT,0);
  if (evidence.votes()!=0) return false;
  evidence.add(TargetClass::MOVING,9); evidence.clear();
  return evidence.votes()==0;
}
static_assert(movingEvidenceRegression(), "Moving evidence must reset on noise, direction reversal and target loss");
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
constexpr bool automaticSequenceRegression() {
  RangeWindow window;
  MovingEvidence moving;
  bool locked = false;
  AutoAction action = AutoAction::WAIT;
  const int16_t approach[] = {100,104,108,112};
  for (int16_t cm : approach) {
    window.add(cm);
    auto e = window.evidence(true,cm);
    auto target = decideTarget(e,25,180,8);
    moving.add(target,e.deltaCm);
    action = decideAutomatic({target,locked,moving.votes(),cm,false,0,4},65,60);
    if (cm < 112 && action != AutoAction::WAIT) return false;
    if (action == AutoAction::ACQUIRE) locked = true;
  }
  if (!locked || action != AutoAction::ACQUIRE) return false;
  // Clear movement evidence after our own motor pulse, then follow the target
  // using fresh stationary readings even if it has now stopped moving.
  window.clear(); moving.clear();
  window.add(95); window.add(95); window.add(95);
  auto target = decideTarget(window.evidence(true,95),25,180,8);
  if (decideAutomatic({target,locked,0,95,false,0,4},65,60) != AutoAction::FOLLOW) return false;
  window.clear();
  window.add(220); window.add(220); window.add(220);
  target = decideTarget(window.evidence(true,220),25,180,8);
  if (decideAutomatic({target,locked,0,220,false,0,4},65,60) != AutoAction::RELEASE) return false;
  locked = false;
  // The next stationary observation resumes exploration without a web command.
  return decideAutomatic({target,locked,0,220,false,0,4},65,60) == AutoAction::ROAM;
}
static_assert(automaticSequenceRegression(), "Observe, acquire, follow, lose target and resume exploration autonomously");
constexpr bool clearPathCruiseRegression() {
  CruiseController drive; RangeWindow window; MovingEvidence moving;
  const CruiseLimits limits{25,60,65,180,cruiseScanMs(600)};
  uint32_t observeAfter=0, issued=0; int drivingFrames=0, starts=0;
  // 12 seconds of real classifier/controller decisions at 100 ms sonar cadence.
  // Motion samples do not enter the stationary target-classification window.
  for (uint32_t now=0;now<12000;now+=100) {
    if (drive.active()) {
      auto action=drive.update(now,true,true,220,limits);
      if (action==CruiseAction::RENEW) { issued=now; ++drivingFrames; }
      else if (action==CruiseAction::OBSERVE) {
        window.clear(); moving.clear(); observeAfter=now+sonarSettleMs(100);
      } else return false;
    } else if (static_cast<int32_t>(now-observeAfter)>=0) {
      window.add(220); auto e=window.evidence(true,220);
      auto target=decideTarget(e,25,180,8); moving.add(target,e.deltaCm);
      auto action=decideAutomatic({target,false,moving.votes(),220,false,0,4},65,60);
      if (action==AutoAction::ROAM) {
        drive.start(CruiseMode::ROAM,now,220); issued=now; ++starts; ++drivingFrames;
        window.clear(); moving.clear();
      } else if (action!=AutoAction::WAIT) return false;
    }
    if (drive.active() && motorLeaseExpired(now,issued,autonomousLeaseMs(100,22000,120))) return false;
  }
  return starts==4 && drivingFrames>=102; // >=85% walking in this ideal echo trace.
}
static_assert(clearPathCruiseRegression(), "Stable clear path spends at least 85 percent of time cruising, not collecting");
#endif
