#pragma once
#include <stdint.h>

enum class SonarKind : uint8_t { Invalid, Echo, NoEcho };

// Allow HC-SR05 modules to finish their own no-return pulse before retriggering.
constexpr uint32_t sonarIntervalMs(uint32_t configured) { return configured < 60 ? 60 : configured; }
constexpr uint32_t sonarTimeoutUs(uint32_t configured) { return configured < 40000 ? 40000 : configured; }
constexpr SonarKind sonarPulseKind(uint32_t pulse, uint32_t timeoutUs = 40000) {
  return pulse < 100 || pulse > timeoutUs ? SonarKind::Invalid :
    pulse >= 28000 ? SonarKind::NoEcho : SonarKind::Echo;
}
constexpr SonarKind sonarTimeoutKind(bool rose, bool echoHigh) {
  return !rose && !echoHigh ? SonarKind::NoEcho : SonarKind::Invalid;
}
struct SonarReading {
  SonarKind kind = SonarKind::Invalid;
  float cm = -1;
  uint32_t sequence = 0, startedMs = 0, completedMs = 0;
  constexpr bool usable(uint32_t now, uint32_t maxAge) const {
    return kind != SonarKind::Invalid && now - completedMs < maxAge;
  }
  constexpr float clearance() const { return kind == SonarKind::NoEcho ? 999.0f : cm; }
};
