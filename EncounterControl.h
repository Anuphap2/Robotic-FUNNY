#ifndef ENCOUNTER_CONTROL_H
#define ENCOUNTER_CONTROL_H
#include <stdint.h>

constexpr int16_t encounterClearCm(int16_t pathCm, int16_t greetingCm, int16_t followCm) {
  int16_t clear = greetingCm + 20;
  if (clear < followCm + 10) clear = followCm + 10;
  return clear < pathCm ? pathCm : clear;
}

// One encounter survives echo noise, brief loss and our own turns. Re-arm only
// after a sustained usable departure, never from a single far reading.
class EncounterControl {
public:
  static constexpr uint32_t GREETING_LIMIT_MS = 2500;
  static constexpr uint32_t HOLD_LIMIT_MS = 2500;
  static constexpr uint32_t DEPARTURE_MS = 3000;
  static constexpr uint32_t REARM_MS = 10000;
  constexpr void reset() {
    _greeted = _active = _seeking = _departing = _holding = false;
    _greetedAt = _departureAt = _heldAt = _lastFarAt = 0; _farReadings = 0;
  }
  constexpr bool canGreet(uint32_t now, uint32_t cooldown) const {
    return !_greeted && !_active && now - _greetedAt >= cooldown;
  }
  constexpr void greet(uint32_t now) {
    _greeted = _active = true; _greetedAt = now;
    _departing = _holding = false; _farReadings = 0;
  }
  constexpr bool finishGreeting(uint32_t now, bool gestureMoving) {
    if (!_active || (gestureMoving && now - _greetedAt < GREETING_LIMIT_MS)) return false;
    _active = false; _seeking = true; _holding = false; return true;
  }
  constexpr bool seekingPath() const { return _seeking; }
  constexpr bool greeted() const { return _greeted; }
  constexpr void pathOpened() { _seeking = _holding = false; }
  constexpr void clearHold() { _holding = false; }
  constexpr bool holdExpired(uint32_t now, bool fresh) {
    if (!_holding) { _holding = true; _heldAt = now; }
    if (!fresh || now - _heldAt < HOLD_LIMIT_MS) return false;
    _holding = false; _seeking = true; return true;
  }
  constexpr void observeDeparture(uint32_t now, bool fresh, bool usable,
      bool departureAllowed, bool far, uint32_t cooldown) {
    if (!_greeted) return;
    if (!usable || !departureAllowed || !far) {
      _departing = false; _farReadings = 0; return;
    }
    if (!fresh) return;
    if (_departing && now - _lastFarAt > 600) { _departing = false; _farReadings = 0; }
    if (!_departing) { _departing = true; _departureAt = now; }
    _lastFarAt = now;
    if (_farReadings < 3) ++_farReadings;
    uint32_t rearm = cooldown > REARM_MS ? cooldown : REARM_MS;
    if (_farReadings >= 3 && now - _departureAt >= DEPARTURE_MS && now - _greetedAt >= rearm) {
      _greeted = false; _departing = false; _farReadings = 0;
    }
  }
private:
  bool _greeted = false, _active = false, _seeking = false;
  bool _departing = false, _holding = false;
  uint8_t _farReadings = 0;
  uint32_t _greetedAt = 0, _departureAt = 0, _heldAt = 0, _lastFarAt = 0;
};

namespace encounterChecks {
constexpr bool jitterNeverRearms() {
  EncounterControl encounter; encounter.greet(2000);
  encounter.finishGreeting(2001,false);
  for (uint32_t now=2100;now<30000;now+=100) {
    int cm = now%1000==0 ? 200 : now%200==0 ? 58 : 40;
    encounter.observeDeparture(now,true,true,true,cm>=75,1500);
    if (encounter.canGreet(now,1500)) return false;
  }
  return encounter.greeted();
}
constexpr bool departureRearmsOnce() {
  EncounterControl encounter; encounter.greet(2000);
  encounter.finishGreeting(2001,false);
  for (uint32_t now=2100;now<12000;now+=100) {
    encounter.observeDeparture(now,true,true,true,true,1500);
    if (encounter.canGreet(now,1500)) return false;
  }
  encounter.observeDeparture(12000,true,true,true,true,1500);
  if (!encounter.canGreet(12000,1500)) return false;
  encounter.greet(12000);
  return !encounter.canGreet(12001,1500) && encounter.greeted();
}
constexpr bool unreliableDeparture() {
  EncounterControl encounter; encounter.greet(2000);
  encounter.finishGreeting(2001,false);
  for (uint32_t now=13000;now<18000;now+=100)
    encounter.observeDeparture(now,true,true,false,true,1500); // Turning/gesturing/following.
  if (!encounter.greeted()) return false;
  encounter.observeDeparture(18000,true,true,true,true,1500);
  encounter.observeDeparture(20000,false,false,true,true,1500); // No echo breaks continuity.
  encounter.observeDeparture(22000,true,true,true,true,1500);
  if (!encounter.greeted()) return false;
  for (uint32_t now=22001;now<27000;++now)
    encounter.observeDeparture(now,false,true,true,true,1500); // Same echo, no extra votes.
  if (!encounter.greeted()) return false;
  encounter.observeDeparture(27000,true,true,true,false,1500); // Near again breaks continuity.
  encounter.observeDeparture(28000,true,true,true,true,1500);
  encounter.observeDeparture(28100,true,true,true,true,1500);
  encounter.observeDeparture(32000,true,true,true,true,1500); // A stalled loop cannot prove continuous departure.
  return encounter.greeted() && !encounter.canGreet(27000,1500);
}
constexpr bool greetingEndsAndRoutes() {
  EncounterControl encounter; encounter.greet(2000);
  if (encounter.finishGreeting(2001,true)) return false;
  if (!encounter.finishGreeting(2990,false) || !encounter.seekingPath()) return false;
  if (encounter.finishGreeting(3000,false) || encounter.canGreet(3000,500)) return false;
  encounter.pathOpened();
  if (encounter.seekingPath() || !encounter.greeted()) return false;
  encounter.reset(); encounter.greet(4000);
  if (encounter.finishGreeting(6499,true)) return false;
  return encounter.finishGreeting(6500,true) && encounter.seekingPath();
}
constexpr bool finiteHold() {
  EncounterControl encounter;
  if (encounter.holdExpired(1000,true) || encounter.holdExpired(3499,true)) return false;
  if (encounter.holdExpired(4000,false)) return false;
  if (!encounter.holdExpired(4000,true) || !encounter.seekingPath()) return false;
  encounter.pathOpened();
  if (encounter.holdExpired(5000,true)) return false;
  encounter.clearHold(); // Progress/invalid echo clears idle timing, not greeting suppression.
  return !encounter.holdExpired(7500,true);
}
constexpr bool clockWrap() {
  EncounterControl encounter; encounter.greet(UINT32_MAX-999);
  if (encounter.finishGreeting(1499,true)) return false;
  return encounter.finishGreeting(1500,true) && !encounter.canGreet(2000,1500);
}
static_assert(encounterClearCm(60,45,65)==75 && encounterClearCm(100,45,65)==100,
  "New path must clear both encounter and normal driving zones");
static_assert(jitterNeverRearms(), "Near/far jitter and isolated spikes cannot repeat greetings");
static_assert(departureRearmsOnce(), "Sustained departure plus minimum quiet time permits a new encounter");
static_assert(unreliableDeparture(), "Turning, following, no echo and duplicate frames do not prove departure");
static_assert(greetingEndsAndRoutes(), "Gesture completion or timeout requests routing exactly once");
static_assert(finiteHold(), "Near-target holding must eventually request a new path on fresh evidence");
static_assert(clockWrap(), "Greeting completion survives millis rollover");
}
#endif
