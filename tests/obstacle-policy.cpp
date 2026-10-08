#include "../ObstacleAvoidance.h"

using Nav = ObstacleAvoidance;
constexpr SonarReading echo(float cm, uint32_t seq, uint32_t start) {
  return {SonarKind::Echo, cm, seq, start, start + 5};
}
constexpr SonarReading empty(uint32_t seq, uint32_t start) {
  return {SonarKind::NoEcho, -1, seq, start, start + 40};
}
constexpr bool initialAndStale() {
  Nav n; Nav::Settings c; n.reset(100);
  if (n.update(599, empty(1,500), c).drive != Nav::Stop) return false;
  if (n.update(610, echo(100,2,590), c).drive != Nav::Stop) return false;
  auto clear = n.update(640, empty(3,600), c);
  if (clear.drive != Nav::Forward || clear.leaseMs != 240) return false;
  if (n.update(740, empty(3,600), c).leaseMs != 140) return false;
  if (n.update(881, empty(3,600), c).drive != Nav::Stop) return false;
  return n.update(900, echo(300,4,890), c).drive == Nav::Forward;
}
constexpr bool enterScan(Nav& n, const Nav::Settings& c) {
  n.reset(0);
  if (n.update(510, echo(2,1,500), c).drive != Nav::Stop) return false;
  if (n.update(709, echo(2,1,500), c).drive != Nav::Stop) return false;
  if (n.update(710, {}, c).drive != Nav::Backward) return false;
  auto lastReverse = n.update(1359, {}, c);
  if (lastReverse.drive != Nav::Backward || lastReverse.leaseMs != 1) return false;
  if (n.update(1360, {}, c).drive != Nav::Stop) return false;
  if (n.update(1559, {}, c).head != 90) return false;
  auto left = n.update(1560, {}, c);
  return left.drive == Nav::Stop && left.head == 150 && n.stage() == Nav::LeftScan;
}
constexpr bool scanAndTurn(bool right, bool noReturn) {
  Nav n; Nav::Settings c;
  if (!enterScan(n,c)) return false;
  // A new completion from a trigger sent while the servo was moving is rejected.
  if (n.update(2070, echo(250,2,2059), c).head != 150) return false;
  // Invalid data does not mean open space and cannot select a side.
  if (n.update(2071, {}, c).head != 150) return false;
  auto l = noReturn && !right ? empty(3,2080) : echo(right ? 40 : 150,3,2080);
  if (n.update(2120,l,c).head != 30) return false;
  if (n.update(2620,l,c).head != 30) return false; // same left measurement
  if (n.update(2630,echo(200,4,2619),c).head != 30) return false;
  auto r = noReturn && right ? empty(5,2640) : echo(right ? 150 : 40,5,2640);
  auto center = n.update(2680,r,c);
  if (center.drive != Nav::Stop || center.head != 90) return false;
  if (n.update(3179,{},c).drive != Nav::Stop) return false;
  auto turn = n.update(3180,{},c);
  if (turn.head != 90 || turn.drive != (right ? Nav::Right : Nav::Left)) return false;
  auto lastTurn = n.update(3779,{},c);
  if (lastTurn.drive != turn.drive || lastTurn.leaseMs != 1) return false;
  if (n.update(3780,{},c).drive != Nav::Stop) return false;
  if (n.update(4300,echo(200,6,4279),c).drive != Nav::Stop) return false;
  return n.update(4330,echo(200,7,4320),c).drive == Nav::Forward;
}
constexpr bool deadEndAndTie() {
  Nav n; Nav::Settings c;
  if (!enterScan(n,c)) return false;
  n.update(2070,echo(20,2,2060),c);
  n.update(2580,echo(20,3,2570),c);
  if (n.update(3080,{},c).drive != Nav::Backward) return false;
  if (n.update(4179,{},c).drive != Nav::Backward) return false;
  if (n.update(4180,{},c).drive != Nav::Stop) return false;
  if (n.update(4380,{},c).drive != Nav::Right) return false;
  if (n.update(5579,{},c).drive != Nav::Right) return false;
  return n.update(5580,{},c).drive == Nav::Stop;
}
constexpr bool wrapAndCancel() {
  Nav n; Nav::Settings c;
  n.reset(UINT32_MAX - 200);
  if (n.update(310,echo(10,1,300),c).drive != Nav::Stop) return false;
  if (n.update(510,{},c).drive != Nav::Backward) return false;
  n.reset(511);
  if (n.update(1010,echo(100,2,1000),c).drive != Nav::Stop) return false;
  return n.update(1021,echo(100,3,1011),c).drive == Nav::Forward;
}
constexpr bool clearTieAndInvalidFront() {
  Nav n; Nav::Settings c;
  if (!enterScan(n,c)) return false;
  n.update(2100,empty(2,2060),c);
  n.update(2640,empty(3,2600),c);
  if (n.update(3140,{},c).drive != Nav::Right) return false;
  n.update(3740,{},c);
  if (n.update(4280,empty(4,4240),c).drive != Nav::Forward) return false;
  // A noise/stuck-high result immediately stops a previous continuous drive.
  return n.update(4340,{SonarKind::Invalid,-1,5,4300,4340},c).drive == Nav::Stop;
}
static_assert(initialAndStale(), "Startup, stale results and far echo handling");
static_assert(scanAndTurn(false,false), "Select left from measurements at settled angles");
static_assert(scanAndTurn(true,false), "Select right from measurements at settled angles");
static_assert(scanAndTurn(false,true), "No return on left is clear");
static_assert(scanAndTurn(true,true), "No return on right is clear");
static_assert(deadEndAndTie(), "Both blocked: finite long reverse and fixed U-turn; ties right");
static_assert(wrapAndCancel(), "Rollover and cancel do not resume old maneuver");
static_assert(clearTieAndInvalidFront(), "Both open sides choose right; invalid front stops driving");
static_assert(sonarPulseKind(50) == SonarKind::Invalid, "Noise is not a clear path");
static_assert(sonarPulseKind(117) == SonarKind::Echo, "2cm obstacle must not be filtered out");
static_assert(sonarPulseKind(18000) == SonarKind::Echo, "300cm echo is usable");
static_assert(sonarPulseKind(30000) == SonarKind::NoEcho, "SR05 internal timeout pulse is no return");
static_assert(sonarPulseKind(38000) == SonarKind::NoEcho, "Allow long no-return pulse");
static_assert(sonarPulseKind(41000) == SonarKind::Invalid, "Out-of-window pulse is invalid");
static_assert(sonarTimeoutKind(false,false) == SonarKind::NoEcho, "Silent completed timeout is usable");
static_assert(sonarTimeoutKind(true,true) == SonarKind::Invalid, "Stuck high is invalid");
static_assert(sonarTimeoutKind(true,false) == SonarKind::Invalid, "Missing falling edge is invalid");
static_assert(sonarIntervalMs(30) == 60 && sonarIntervalMs(1000) == 1000, "Saved intervals are guarded");
static_assert(sonarTimeoutUs(12000) == 40000, "Saved short timeouts cannot truncate SR05 no-return pulse");
