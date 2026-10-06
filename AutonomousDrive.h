#ifndef AUTONOMOUS_DRIVE_H
#define AUTONOMOUS_DRIVE_H
#include <stdint.h>

// A lease is renewed only by a NEW usable echo, never by another loop tick.
// MotorController also caps this lease by the user's motor timeout.
constexpr uint32_t autonomousLeaseMs(uint32_t interval, uint32_t echoUs, uint32_t minimum) {
  uint32_t lease = interval * 2 + (echoUs + 999) / 1000 + 40;
  if (lease < minimum) lease = minimum;
  return lease < 150 ? 150 : lease > 600 ? 600 : lease;
}
constexpr uint32_t cruiseScanMs(uint32_t observationMs) {
  return observationMs * 4 < 3000 ? 3000 : observationMs * 4;
}
constexpr uint32_t sonarSettleMs(uint32_t interval) {
  return interval < 60 ? 60 : interval > 120 ? 120 : interval;
}
constexpr int16_t roamStartCm(int16_t stopCm, int16_t clearCm) {
  // Preserve an 8 cm start/stop gap even for a very narrow saved configuration.
  return clearCm < stopCm + 13 ? stopCm + 13 : clearCm;
}
constexpr bool motorLeaseExpired(uint32_t now, uint32_t issued, uint32_t lease) {
  return now - issued >= lease;
}
constexpr bool autonomousRangeFresh(uint32_t age, uint32_t interval, uint32_t lease) {
  uint32_t maximumAge = interval * 2 < lease ? interval * 2 : lease;
  return age < maximumAge;
}
enum class CruiseMode { IDLE, ROAM, FOLLOW };
enum class CruiseAction { KEEP, RENEW, OBSERVE, BRAKE, HOLD, LOST, FAULT };
struct CruiseLimits { int16_t stopCm, clearCm, followCm, maxCm; uint32_t scanMs; };

class ScanRetry {
public:
  constexpr void reset() { _waiting = false; _right = false; _batches = 0; }
  constexpr void clearWait() { _waiting = false; _batches = 0; }
  constexpr bool right() const { return _right; }
  constexpr bool ready(uint32_t now, bool fresh, uint32_t cooldown) {
    if (!_waiting) { _waiting = true; _deadline = now + cooldown; }
    if (!fresh || static_cast<int32_t>(now - _deadline) < 0) return false;
    // Keep the new direction for three bounded batches; alternating every batch
    // would simply undo the previous turn and revisit the same blocked sector.
    _waiting = false;
    if (!_batches) { _right = !_right; _batches = 3; }
    --_batches; return true;
  }
private:
  bool _waiting = false, _right = false;
  uint32_t _deadline = 0;
  uint8_t _batches = 0;
};

class CruiseController {
public:
  constexpr void stop() { _mode = CruiseMode::IDLE; _farReadings = 0; }
  constexpr bool active() const { return _mode != CruiseMode::IDLE; }
  constexpr CruiseMode mode() const { return _mode; }
  constexpr void start(CruiseMode mode, uint32_t now, int16_t cm) {
    _mode = mode; _started = now; _farReadings = 0; _index = 0;
    for (auto& value : _cm) value = cm;
  }
  constexpr CruiseAction update(uint32_t now, bool valid, bool fresh, int16_t cm, CruiseLimits limits) {
    if (!active()) return CruiseAction::KEEP;
    // Raw close readings bypass filtering. A lost/stale echo cannot renew drive.
    if (!valid || cm <= limits.stopCm) return finish(CruiseAction::FAULT);
    if (!fresh) return CruiseAction::KEEP;
    if (_mode == CruiseMode::ROAM) {
      int brakeCm = limits.clearCm - 8;
      if (brakeCm < limits.stopCm + 5) brakeCm = limits.stopCm + 5;
      if (cm <= brakeCm) return finish(CruiseAction::BRAKE);
      if (now - _started >= limits.scanMs) return finish(CruiseAction::OBSERVE);
    } else {
      // One far spike withholds renewal; two distinct far echoes release a lock.
      if (cm > limits.maxCm) {
        if (++_farReadings >= 2) return finish(CruiseAction::LOST);
        return CruiseAction::KEEP;
      }
      _farReadings = 0;
    }
    _cm[_index] = cm; _index = (_index + 1) % 3;
    int16_t a = _cm[0], b = _cm[1], c = _cm[2];
    int16_t median = a < b ? (b < c ? b : a < c ? c : a) : (a < c ? a : b < c ? c : b);
    // Start at follow + 5 (DecisionTree), stop at follow: a real deadband.
    if (_mode == CruiseMode::FOLLOW && median <= limits.followCm) return finish(CruiseAction::HOLD);
    return CruiseAction::RENEW;
  }
private:
  CruiseMode _mode = CruiseMode::IDLE;
  uint32_t _started = 0;
  int16_t _cm[3] = {};
  uint8_t _index = 0, _farReadings = 0;
  constexpr CruiseAction finish(CruiseAction action) { stop(); return action; }
};
// Compile-time regressions execute the SAME controller and lease predicate used
// by the firmware; they allocate no runtime buffers or test tasks.
namespace autonomousChecks {
constexpr CruiseLimits limits{25,60,65,180,3000};
constexpr bool continuousDrive() {
  CruiseController drive; drive.start(CruiseMode::ROAM,0,150);
  uint32_t issued = 0; int renewals = 0;
  for (uint32_t now=10; now<3000; now+=10) {
    auto action = drive.update(now,true,now%100==0,150,limits);
    if (action==CruiseAction::RENEW) { issued=now; ++renewals; }
    else if (action!=CruiseAction::KEEP) return false;
    if (!drive.active() || motorLeaseExpired(now,issued,autonomousLeaseMs(100,22000,120))) return false;
  }
  return renewals==29 && drive.update(3000,true,true,150,limits)==CruiseAction::OBSERVE && !drive.active();
}
constexpr bool stallCutoff() {
  CruiseController drive; drive.start(CruiseMode::FOLLOW,0,100);
  if (drive.update(100,true,true,100,limits)!=CruiseAction::RENEW) return false;
  for (uint32_t now=101;now<=362;++now)
    if (drive.update(now,true,false,100,limits)!=CruiseAction::KEEP) return false;
  return !motorLeaseExpired(361,100,262) && motorLeaseExpired(362,100,262) &&
    drive.update(363,false,false,100,limits)==CruiseAction::FAULT && !drive.active();
}
constexpr bool followFiltering() {
  CruiseController drive; drive.start(CruiseMode::FOLLOW,0,80);
  // A single low outlier does not cause a walk-stop-walk cycle.
  if (drive.update(100,true,true,64,limits)!=CruiseAction::RENEW) return false;
  if (drive.update(200,true,true,75,limits)!=CruiseAction::RENEW) return false;
  if (drive.update(300,true,true,64,limits)!=CruiseAction::HOLD || drive.active()) return false;
  drive.start(CruiseMode::FOLLOW,400,80);
  // Unlike exploration, following has no mandatory periodic stop.
  for (uint32_t now=500;now<12000;now+=100)
    if (drive.update(now,true,true,80,limits)!=CruiseAction::RENEW) return false;
  drive.stop();
  return drive.update(12000,true,true,80,limits)==CruiseAction::KEEP && !drive.active();
}
constexpr bool farEchoFiltering() {
  CruiseController drive; drive.start(CruiseMode::FOLLOW,0,100);
  if (drive.update(100,true,true,250,limits)!=CruiseAction::KEEP) return false;
  // Loop ticks over the same echo must not become a second lost-target vote.
  for (uint32_t now=101;now<200;++now)
    if (drive.update(now,true,false,250,limits)!=CruiseAction::KEEP || !drive.active()) return false;
  if (drive.update(200,true,true,100,limits)!=CruiseAction::RENEW) return false;
  if (drive.update(300,true,true,250,limits)!=CruiseAction::KEEP) return false;
  return drive.update(400,true,true,250,limits)==CruiseAction::LOST && !drive.active();
}
constexpr bool immediateBraking() {
  CruiseController drive; drive.start(CruiseMode::ROAM,0,100);
  if (drive.update(100,true,true,52,limits)!=CruiseAction::BRAKE || drive.active()) return false;
  drive.start(CruiseMode::FOLLOW,200,100);
  if (drive.update(201,true,false,25,limits)!=CruiseAction::FAULT || drive.active()) return false;
  drive.start(CruiseMode::ROAM,300,100);
  if (drive.update(301,false,false,100,limits)!=CruiseAction::FAULT || drive.active()) return false;
  drive.start(CruiseMode::ROAM,400,100);
  // Even an unusually narrow configured clearance respects stop + 5.
  return drive.update(500,true,true,30,{25,30,65,180,3000})==CruiseAction::BRAKE;
}
constexpr bool retryOtherSide() {
  ScanRetry retry;
  if (retry.ready(100,true,600) || retry.ready(699,true,600)) return false;
  if (retry.ready(900,false,600) || retry.right()) return false;
  if (!retry.ready(900,true,600) || !retry.right()) return false;
  if (retry.ready(1000,true,600)) return false;
  if (!retry.ready(1600,true,600) || !retry.right()) return false;
  if (retry.ready(1700,true,600) || !retry.ready(2300,true,600) || !retry.right()) return false;
  if (retry.ready(2400,true,600) || !retry.ready(3000,true,600) || retry.right()) return false;
  retry.reset();
  // The deadline may be zero after millis() wrap; zero is not a sentinel.
  if (retry.ready(UINT32_MAX-599,true,600)) return false;
  return retry.ready(0,true,600) && retry.right();
}
constexpr bool wrapSafe() {
  CruiseController drive; const uint32_t started=UINT32_MAX-999;
  drive.start(CruiseMode::ROAM,started,150);
  if (drive.update(1999,true,true,150,limits)!=CruiseAction::RENEW) return false;
  return drive.update(2000,true,true,150,limits)==CruiseAction::OBSERVE &&
    !motorLeaseExpired(49,UINT32_MAX-99,150) && motorLeaseExpired(50,UINT32_MAX-99,150);
}
static_assert(autonomousLeaseMs(100,22000,120)==262, "Default lease bridges sonar frames");
static_assert(autonomousLeaseMs(60,2000,60)==162, "Fast sonar lease retains processing margin");
static_assert(autonomousLeaseMs(60,2000,300)==300, "Honor configured minimum lease");
static_assert(autonomousLeaseMs(1000,30000,120)==600, "Never coast on a long sonar interval");
static_assert(sonarSettleMs(60)==60 && sonarSettleMs(100)==100 && sonarSettleMs(1000)==120, "Settling is short and bounded");
static_assert(cruiseScanMs(600)==3000 && cruiseScanMs(3000)==12000, "Observe periodically, not after each lease");
static_assert(continuousDrive(), "Cruise continuously for three seconds, renewing only fresh frames");
static_assert(stallCutoff(), "Same echo cannot renew drive; timer predicate expires on schedule");
static_assert(followFiltering(), "Follow filtering, continuous tracking and explicit cancellation");
static_assert(farEchoFiltering(), "Far outlier is not an immediate target loss");
static_assert(immediateBraking(), "Raw close/invalid echoes override filtering and renewal");
static_assert(retryOtherSide(), "Bounded avoidance retries alternate directions without web input");
static_assert(wrapSafe(), "Cruise and lease arithmetic survive clock rollover");
static_assert(autonomousRangeFresh(199,100,262) && !autonomousRangeFresh(200,100,262) &&
  !autonomousRangeFresh(1000,100,262) && !autonomousRangeFresh(600,1000,600),
  "Delayed echoes keep their capture age and cannot renew autonomous drive");
static_assert(roamStartCm(25,60)==60 && roamStartCm(25,30)==38 && roamStartCm(59,60)==72,
  "Narrow saved clearance settings must not collapse the exploration deadband");
}
#endif
