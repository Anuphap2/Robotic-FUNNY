# Verification — firmware 2.3.5

Checked on 8 October 2026 (Asia/Bangkok). The sensor model supplied by the user
is HC-SR05. No firmware was uploaded and no physical robot was driven.

## Navigation and measurement checks

The installed ESP32 compiler evaluates the actual constexpr production
ObstacleAvoidance and SonarReading code with -std=c++17 -Wall -Wextra
-fsyntax-only (tests/obstacle-policy.cpp). Nineteen static assertions cover:

- Startup centering and waiting for a trigger after servo settling; stale
  completions never permit forward motion. Long valid ranges remain usable.
- A roughly 2 cm echo triggers a stop, brake, fixed 650 ms reverse, 200 ms rest.
- Both left/right choices, no return on either side, repeated old measurements,
  and triggers issued just before the 500 ms settling deadline.
- Wheels stopped during each scan, head centered during reverse and turn,
  fixed 600 ms turn, and a fresh front measurement after turning.
- Both sides blocked: finite 1100 ms reverse, 200 ms rest, 1200 ms turn.
  Equal blocked/clear sides deterministically select right.
- Invalid/noise/missing-edge/stuck-high results are never considered open space.
  A newly invalid front result stops a previous forward drive.
- No-return pulses and silent LOW timeout are distinct from invalid samples;
  saved short trigger intervals/timeouts are clamped to HC-SR05 timing guards.
- Maneuver leases shrink to 1 ms at their deadline; repeated use of one forward
  sample also reduces its remaining lease rather than extending its lifetime.
- Cancellation and millis rollover do not resume an earlier maneuver.

No replacement navigation implementation is used in these checks. Sensor GPIO
and interrupt behavior are built with the real ESP32 core; physical echo timing
and servo movement still require board measurements.

## Dashboard and existing regressions

Node executes actual functions extracted from WebUI.h:

- 12 configuration/navigation UI checks including v1/v2 imports, 51 stable
  persisted slots, bounds/types, physical servo homes, obsolete distances
  unable to restrict obstacle settings, legacy fields hidden during search,
  scanning distinct from turning, and no target-follow presentation.
- 9 existing manual-control ordering, stop, abort, heartbeat, page-exit,
  telemetry cancellation, and reboot checks.
- 7 existing hardware UI checks for enable flags, independent servo readiness,
  motor initialization, GPIO readback and detailed action errors.

Result: 28 passed, 0 failed.
Command: node --test tests/config-ui.test.mjs tests/manual-drive-ui.test.mjs
  tests/hardware-ui.test.mjs

The ESP32 compiler also passes tests/manual-drive-policy.cpp and all four
SERVO_TEST_MODE variants of tests/servo-output-policy.cpp (only peripheral calls
stubbed). These do not prove electrical servo signals or wheel rotation.

## ESP32 build

Final source compiles successfully for esp32:esp32:esp32 with ESP32 core 3.3.12
and installed libraries, --warnings all. No project-source diagnostics.

- Program: 1,083,586 / 1,310,720 bytes (82%).
- Static RAM: 52,512 / 327,680 bytes (16%); 275,168 bytes before runtime use.
- All 17 firmware source files match the staged source snapshot byte for
  byte. Compiled headers and SensorManager.cpp also match workspace source
  after removing Arduino-generated line markers and normalizing line endings.
  Generated main contains Firmware 2.3.5; its added prototypes are excluded
  from source-hash comparison.
- Dashboard JavaScript syntax and git diff --check pass.

Build: arduino-cli --config-file .build/cli.yaml compile
  --fqbn esp32:esp32:esp32 --warnings all
  --build-path .build/obstacle-2.3.5 .build/source/sketch_oct2a
The staged source includes only firmware files; no test code is linked.
An initial build also passed before final motor-lease/timing updates; figures
above belong to the final source, not the initial snapshot.

## Compatibility and limits

All original GPIO assignments remain. NVS v2 key order and 51-slot record layout
remain; no reset or storage write was performed. Legacy behavior fields remain
in records/imports/exports but have no effect on navigation and are hidden.
Sonar defaults become 60 ms / 40000 us, timeout maximum widens to 60000 us.
Saved shorter settings are preserved in storage and guarded at runtime.

HC-SR05 real-module timing has not been measured. SRF05 manufacturer documentation
reports approximately 30 ms no-return pulses and at least 50 ms between triggers:
https://www.robot-electronics.co.uk/htm/srf05tech.htm
Firmware uses 60 ms minimum trigger spacing / 40 ms minimum wait as conservative
operating choices, accepts long out-of-range/timeout pulses as NoEcho, and keeps
stuck HIGH or malformed edges Invalid. Silent LOW NoEcho cannot distinguish an
open area from a disconnected/dead sensor. Forward motion on NoEcho is the
requested behavior, not a claim that empty space was positively measured.

The previous obstacle code read the latest echo after a fixed delay regardless
of when it was triggered, repeatedly filtered the same measurement, could discard
near/far valid readings, randomly chose turn duration, and immediately resumed
front driving after a turn. New code uses measurement start times, distinct result
kinds, fixed phases and front reacquisition. Motor driveFor uses the existing
independent 2 ms watchdog and direction dead time with an explicit remaining
lease. Manual-drive signals, speed and timestamp ordering are unchanged.

No encoder, rear/edge sensor or physical motion test exists. Turn/reverse duration
is a command time, not a measured angle or travel distance. Test lifted wheels,
clear front, each side blocked, both sides blocked, no return, and cancellation
on the actual robot before evaluating navigation on the floor.
