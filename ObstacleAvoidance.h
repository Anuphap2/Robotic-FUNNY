#pragma once
#include "SonarReading.h"

// No target acquisition or random scan/turn decisions. Time arithmetic wraps.
class ObstacleAvoidance {
public:
  enum Stage { Front, Brake, Reverse, Rest, LeftScan, RightScan, Center,
               DeadReverse, TurnRest, Turn, AfterTurn };
  enum Drive { Stop, Forward, Backward, Left, Right };
  struct Output { Drive drive; int head; const char* reason; bool avoiding; uint32_t leaseMs = 0; };
  struct Settings {
    int center = 90, left = 150, right = 30;
    float stopCm = 35;
    uint32_t settleMs = 500, reverseMs = 650, turnMs = 600, deadReverseMs = 1100;
    uint32_t maxAgeMs = 240;
  };
  constexpr void reset(uint32_t now) {
    _stage = AfterTurn; _since = now; _lastSequence = 0;
    _left = _right = -1; _turnRight = true; _deadEnd = false;
  }
  constexpr Stage stage() const { return _stage; }
  constexpr float leftCm() const { return _left; }
  constexpr float rightCm() const { return _right; }
  constexpr Output update(uint32_t now, const SonarReading& sample, const Settings& cfg) {
    uint32_t elapsed = now - _since;
    switch (_stage) {
      case AfterTurn:
      case Front:
        if (_stage == AfterTurn && elapsed < cfg.settleMs) return output(now, cfg);
        if (!sample.usable(now, cfg.maxAgeMs) ||
            now - sample.startedMs > elapsed - (_stage == AfterTurn ? cfg.settleMs : 0))
          return {Stop, cfg.center, "measurement_wait", false};
        // Readings made during a turn or before centering cannot start driving.
        if (sample.clearance() <= cfg.stopCm) {
          _lastSequence = sample.sequence;
          enter(Brake, now);
          return output(now, cfg);
        }
        _lastSequence = sample.sequence;
        _stage = Front; // Keep the original settled-front measurement boundary.
        return {Forward, cfg.center, sample.kind == SonarKind::NoEcho ? "no_echo_clear" : "roam_cruise", false,
                cfg.maxAgeMs - (now - sample.completedMs)};
      case Brake: if (elapsed >= 200) enter(Reverse, now); break;
      case Reverse: if (elapsed >= cfg.reverseMs) enter(Rest, now); break;
      case Rest: if (elapsed >= 200) enter(LeftScan, now); break;
      case LeftScan:
      case RightScan:
        // The trigger itself must be AFTER the servo settling interval.
        if (elapsed >= cfg.settleMs && sample.sequence != _lastSequence &&
            sample.usable(now, cfg.maxAgeMs) && now - sample.startedMs <= elapsed - cfg.settleMs) {
          _lastSequence = sample.sequence;
          if (_stage == LeftScan) { _left = sample.clearance(); enter(RightScan, now); }
          else {
            _right = sample.clearance(); _turnRight = _right >= _left;
            _deadEnd = _left <= cfg.stopCm && _right <= cfg.stopCm;
            enter(Center, now);
          }
        }
        break;
      case Center:
        if (elapsed >= cfg.settleMs) enter(_deadEnd ? DeadReverse : Turn, now);
        break;
      case DeadReverse:
        if (elapsed >= cfg.deadReverseMs) enter(TurnRest, now);
        break;
      case TurnRest: if (elapsed >= 200) enter(Turn, now); break;
      case Turn:
        if (elapsed >= (_deadEnd ? cfg.turnMs * 2 : cfg.turnMs)) enter(AfterTurn, now);
        break;
    }
    return output(now, cfg);
  }
private:
  Stage _stage = AfterTurn;
  uint32_t _since = 0, _lastSequence = 0;
  float _left = -1, _right = -1;
  bool _turnRight = true, _deadEnd = false;
  constexpr void enter(Stage stage, uint32_t now) { _stage = stage; _since = now; }
  constexpr Output output(uint32_t now, const Settings& cfg) const {
    const uint32_t elapsed = now - _since;
    switch (_stage) {
      case Brake: return {Stop, cfg.center, "obstacle_brake", true};
      case Reverse: return {Backward, cfg.center, "escape_reverse", true, cfg.reverseMs - elapsed};
      case DeadReverse: return {Backward, cfg.center, "escape_reverse", true, cfg.deadReverseMs - elapsed};
      case Rest: case TurnRest: return {Stop, cfg.center, "reverse_rest", true};
      case LeftScan: return {Stop, cfg.left, "scan_left", true};
      case RightScan: return {Stop, cfg.right, "scan_right", true};
      case Center: return {Stop, cfg.center, "scan_center", true};
      case Turn: return {_turnRight ? Right : Left, cfg.center, _turnRight ? "avoid_right" : "avoid_left", true,
                         (_deadEnd ? cfg.turnMs * 2 : cfg.turnMs) - elapsed};
      default: return {Stop, cfg.center, "measurement_wait", false};
    }
  }
};
