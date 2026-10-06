# Verification — firmware 2.3.0

Checked on 7 October 2026 (Asia/Bangkok). No firmware was uploaded and no
physical robot was driven during these checks.

## Final ESP32 build

Final source compiled successfully with `--warnings all`, FQBN
`esp32:esp32:esp32`, ESP32 core 3.3.12 and the libraries listed in README.
The final build copy matches all changed production sources (excluding the
`#line` markers inserted by Arduino).

- Program storage: 1,080,874 / 1,310,720 bytes (82%).
- Static RAM: 52,768 / 327,680 bytes (16%); 274,912 bytes remain before runtime
  allocations. This is not a measurement of the running board's heap.
- Static RAM increased by 32 bytes from 2.2.0. Cruise/filter/retry storage is fixed
  capacity, with no additional song buffers or unbounded history.
- Full initial build reported existing ESP32Servo unused-variable warnings;
  final incremental build completed without diagnostics from project sources.
- All 39 production assertions also passed an independent syntax-only compile
  with the ESP32 C++ compiler.
- `Config.h` matches original commit `0192c93`; GPIO assignments are unchanged.
- `SongSystem.h` matches `a20d12c`; 443 notes and all 19 tracks retain the earlier
  reduction to four bytes per note (1,772 bytes of note tables in Flash).

## Production movement and decision regressions

The firmware contains 39 static assertions executing its production functions.
These checks compile away; they do not allocate runtime test buffers or tasks.

`AutonomousDrive.h` verifies:

- Leases derived from sonar cadence, the configured minimum and the 600 ms cap.
- Three seconds of uninterrupted clear-path drive with 29 new-echo renewals;
  repeated loop ticks do not renew the lease or stop the drive.
- Timer expiration on schedule when fresh echoes cease, even if the last range
  value itself still looks clear.
- Continuous following without exploration's periodic observation stops.
- Median filtering and a follow stop threshold distinct from the start threshold.
- One far outlier withholds renewal without losing the target; two distinct far
  readings release it. Repeated ticks over the same echo are not extra votes.
- Immediate raw-close, invalid-echo and obstacle braking, bypassing filtering.
- Explicit cruise cancellation prevents subsequent echo-triggered renewal.
- Avoidance cooldown, alternate-side retry and a requirement for fresh evidence.
  A retry direction is kept for three bounded batches instead of undoing the
  previous turn at every batch boundary and revisiting only the same sector.
- Cruise, watchdog and retry deadlines across millis() rollover.
- Old capture ages cannot authorize renewal, including long sonar intervals.
- Narrow saved clearance configurations still retain an 8 cm start/stop gap.

`DecisionTree.h` verifies the existing range classifier, stationary movement
votes, fixed six-reading window, automatic acquisition/follow/loss/recovery,
priority stops and bounded turn batches. An untracked stationary obstacle is
avoided instead of receiving a long greeting gesture.

Its additional integrated 12-second clear-path trace executes the actual
stationary window, classifier, policy, cruise controller and watchdog predicate.
At the default 100 ms echo cadence it starts cruising four times and spends at
least 85% of the trace walking. Moving samples never enter the stationary target
window. This is an ideal software trace, not a measurement on a physical robot.

## Dashboard and compatibility

`node --test tests/config-ui.test.mjs`: 13 tests covering actual dashboard code:
field bounds/types, servo/proximity/autonomy constraints, invalid/unknown/GPIO
fields, blank inputs, v1 import preserving 42 old values, bounded v2 imports,
one-command Interactive start, idle page closure, manual drive exit stop,
automatic telemetry and the new continuous-drive/observation/avoidance labels.

Dashboard JavaScript syntax and `git diff --check` passed.
The 51 saved field keys, order, numeric bounds/defaults and NVS schema v2 remain
compatible with 2.2.0. Labels/hints now explain the new timing semantics:
`auto_pulse_ms` is a minimum forward lease, `auto_observe_ms` controls periodic
observation cadence and avoidance retry cooldown, and `auto_turn_limit` bounds
each scan batch. No saved configuration is erased or rewritten automatically.

`tests/preview.mjs` remains a UI-only fixture with timed example telemetry. It
does not execute production decision logic or motors. Browser checks from 2.2.0
are historical; the movement checks above are independent of that fixture.

## Hardware verification still required

Test with the actual circuit on flat ground: continuous drive and actual braking
distance, front-echo quality while moving, pulse/lease cutoff during HTTP stalls,
old ISR captures after a stall, sonar timeout/no echo, follow deadband and single
far spikes, repeated far readings and reacquisition, periodic stationary scans,
alternate-side avoidance, close-stop priority, manual override, emergency stop,
greetings and sustained heap use. Existing NVS migration/persistence requires
reboot checks. Defaults retain a 100 ms sonar interval; use 60–200 ms for smooth
cruising. Very long saved intervals can still cause watchdog stops between echoes.

Range-only following cannot prove human identity or see side/rear/floor edges.
Continuous motion remains subject to usable new echoes and the independent
watchdog; it does not authorize blind movement when sensor data is missing.
