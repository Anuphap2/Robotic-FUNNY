#include "../ManualDriveGuard.h"

static_assert(manualDriveRemaining(1000, 1000) == 400, "fresh lease");
static_assert(manualDriveRemaining(1399, 1000) == 1, "queue time spends lease");
static_assert(manualDriveRemaining(1400, 1000) == 0, "exact expiry");
static_assert(manualDriveRemaining(999, 1000) == 0, "future timestamp rejected");
static_assert(manualDriveRemaining(20, UINT32_MAX - 99) == 280, "millis rollover");

constexpr bool reorderedCommands() {
  ManualDriveGuard guard;
  return guard.accept(1000, 1, 1, 1000, false) &&
         guard.accept(1020, 1, 3, 1020, true) &&
         !guard.accept(1040, 1, 2, 1010, false) &&
         !guard.accept(1040, 1, 3, 1040, false) &&
         guard.accept(1060, 1, 4, 1060, false);
}
static_assert(reorderedCommands(), "stop rejects late and duplicate movement");
constexpr bool globalCancellation() {
  ManualDriveGuard guard;
  guard.cancelAll(1200);
  return !guard.accept(1210, 1, 1, 1190, false) &&
         !guard.accept(1210, 2, 1, 1200, false) &&
         guard.accept(1220, 2, 2, 1210, false) &&
         guard.accept(5000, 3, 1, 100, true);
}
static_assert(globalCancellation(), "all-stop cancels queued movement from every page");
constexpr bool capacity() {
  ManualDriveGuard guard;
  for (uint32_t i = 1; i <= 4; ++i) if (!guard.accept(1000, i, 1, 1000, false)) return false;
  return !guard.accept(1000, 5, 1, 1000, false) &&
         guard.accept(3001, 5, 1, 3001, false) &&
         !guard.accept(3001, 1, 2, 1000, false);
}
static_assert(capacity(), "fixed capacity never evicts recent stop sequence");
constexpr bool invalidDoesNotPoison() {
  ManualDriveGuard guard;
  return !guard.accept(2000, 0, 1, 2000, false) &&
         !guard.accept(2000, 1, 0, 2000, false) &&
         !guard.accept(2000, 1, 99, 1000, false) &&
         guard.accept(2000, 1, 1, 2000, false);
}
static_assert(invalidDoesNotPoison(), "expired commands cannot suppress a fresh command");
constexpr bool cancellationRollover() {
  ManualDriveGuard guard;
  guard.cancelAll(UINT32_MAX - 10);
  return !guard.accept(20, 1, 1, UINT32_MAX - 20, false) &&
         guard.accept(20, 1, 2, 10, false);
}
static_assert(cancellationRollover(), "stop barrier survives millis rollover");
