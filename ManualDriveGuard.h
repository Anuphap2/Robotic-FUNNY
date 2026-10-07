#ifndef MANUAL_DRIVE_GUARD_H
#define MANUAL_DRIVE_GUARD_H
#include <stdint.h>

// Bounded storage, no heap. A delayed packet spends its lease in the queue;
// receiving it does not give it a fresh 400 ms of movement.
constexpr uint32_t MANUAL_DRIVE_LEASE_MS = 400;
constexpr uint32_t manualDriveRemaining(uint32_t now, uint32_t issued) {
  return now - issued < MANUAL_DRIVE_LEASE_MS ? MANUAL_DRIVE_LEASE_MS - (now - issued) : 0;
}
class ManualDriveGuard {
public:
  constexpr void cancelAll(uint32_t now) { _barrier = now; _cancelled = true; }
  constexpr bool accept(uint32_t now, uint32_t client, uint32_t sequence,
                        uint32_t issued, bool stop) {
    if (!client || !sequence) return false;
    if (!stop && (!manualDriveRemaining(now, issued) ||
        (_cancelled && static_cast<int32_t>(issued - _barrier) <= 0))) return false;
    Slot* slot = nullptr;
    for (auto& candidate : _slots) if (candidate.client == client) { slot = &candidate; break; }
    if (!slot) for (auto& candidate : _slots) {
      if (!candidate.client || now - candidate.seen > 2000) { slot = &candidate; break; }
    }
    if (!slot || (slot->client == client && sequence <= slot->sequence)) return false;
    *slot = {client, sequence, now};
    return true;
  }
private:
  struct Slot { uint32_t client = 0, sequence = 0, seen = 0; };
  Slot _slots[4] = {};
  uint32_t _barrier = 0;
  bool _cancelled = false;
};
#endif
