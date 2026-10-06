# Verification — firmware 2.2.0

Checked on 7 October 2026 (Asia/Bangkok). No firmware was uploaded and no
physical robot was driven during these checks.

## Final ESP32 build

Compiled the final source with `--warnings all`, FQBN `esp32:esp32:esp32`,
ESP32 core 3.3.12 and the library versions listed in README. Final incremental
build completed successfully without warnings/errors.

- Program storage: 1,078,366 / 1,310,720 bytes (82%).
- Static RAM: 52,736 / 327,680 bytes (16%).
- Remaining RAM before runtime allocations: 274,944 bytes. This is not a measured
  running-board heap; Wi-Fi/HTTP/OLED allocate memory at runtime.
- Static RAM is unchanged from 2.1.0. All new target evidence storage is fixed
  capacity; no dynamic history or additional song buffers are allocated.
- `Config.h` is identical to original commit `0192c93`; no GPIO assignments changed.
- Saved-settings count (51), order, numeric defaults and NVS schema v2 remain
  compatible with 2.1.0; only two explanatory hints changed.
- `SongSystem.h` is unchanged from `d64557f`: 443 notes, all 19 registered tracks,
  four bytes per note, 1,772 bytes of note tables in Flash. The previously verified
  3,544-byte score reduction is retained.

## Production decision logic checked at compile time

`DecisionTree.h` contains 23 static assertions executing the production functions:

- Eight range-classifier checks: missing echo, close boundary, insufficient
  readings, discontinuity, far echo, approaching/receding candidates and noise.
- Twelve automatic-policy checks: wait for repeated movement evidence, acquire
  without a web command, follow a stopped target, hold the distance boundary,
  release a lost target, greet, explore a newly clear path, scan, limit turning,
  and give close/unstable/missing readings priority over motion.
- A movement-vote regression: votes require consistent direction and clear on
  stationary readings, reversed direction or explicit reset.
- A six-reading-window regression: reset, fixed capacity and outlier detection.
- An integrated sequence using the actual window, voting, classifier and policy:
  observe -> automatically acquire -> follow -> release -> resume exploration.
  It supplies no human-confirmation or behavior-selection commands.

These assertions passed in the final firmware build. They verify the pure
production decision logic, not ESP32 timing, NVS behavior or real actuator motion.

## Actual dashboard code tested with Node

`node --test tests/config-ui.test.mjs`: 12 tests passed. Tests execute dashboard
functions extracted from WebUI.h using the RuntimeConfig.h schema, covering:

- Every field's bounds and invalid types, servo transforms, proximity and
  autonomous distance relationships, missing/unknown/GPIO keys and blank inputs.
- v1 JSON migration preserves all 42 old values and supplies only new defaults;
  v2 validates the full draft, and maximum form size fits the 2,048-byte body cap.
- Idle page exit does not cancel autonomy; active/in-flight manual driving sends
  a keepalive stop.
- One click sends one `mode=interactive` start command, with no behavior selector
  or target-confirmation control; the stop button sends the explicit stop command.
- Telemetry renders all six automatic behavior states and range target lock
  without a confirmation step.

Dashboard JavaScript syntax and `git diff --check` passed.

## Browser checks with a local UI fixture

`tests/preview.mjs` serves the actual dashboard and schema with timed example
telemetry. This fixture does not execute the firmware decision tree or motors.

- The dashboard starts with only start/stop controls for Interactive. There is
  no behavior selector or target-confirmation button.
- After one start click, observed automatic avoidance and exploration statuses
  with no additional behavior commands. Timed sample telemetry supplies these
  changes; this proves UI rendering, not physical autonomy.
- Stop Interactive returns the simulated system to Manual with motor stop.
- Closing the idle dashboard and opening it again preserves the simulated
  Interactive mode; no extra start command is needed.
- At 320, 375 and the current desktop viewport (920 pixels), the document scroll
  width does not exceed its client width.
- No browser warnings/errors were reported.

## Hardware verification still required

Test on the actual circuit: independent pulse cutoff during an HTTP stall,
sonar freshness/timeout and sensor placement, braking distance, turn clearance,
automatic acquisition of moving targets, close/stale/outlier stops, following
and loss/reacquisition, greeting without repeated motion, return to exploration,
manual override, emergency stop, operation after browser closure and sustained
heap use. Existing v1/v2 NVS migration/persistence still needs reboot checks.

Range-only automatic following cannot prove that a target is human, distinguish
an unmoving person from a wall/object, retain visual identity, see lateral or
rear targets, or detect floor edges. It follows a consistent front echo by
heuristic, automatically, without a human-confirmation gate. Loss of usable
measurements stops motion; exploration resumes only after fresh usable evidence.
