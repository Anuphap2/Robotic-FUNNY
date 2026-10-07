# Verification — firmware 2.3.2

Checked on 7 October 2026 (Asia/Bangkok). No firmware was uploaded and no
physical robot was driven. Hardware observations remain supplied by the user.

## Manual control regressions

Node runs the actual dashboard functions extracted from WebUI.h:

- 13 existing configuration/telemetry tests: 51 fields, bounds, types, servo
  limits, distances, unknown/GPIO fields, blank values, v1/v2 import compatibility,
  and Interactive controls/status.
- 9 manual-control tests: stop while a forward request is outstanding, release
  abort propagation, heartbeat timing including network wait, no parallel drive
  queue, direction change waiting for stop acknowledgement, late old completion,
  old button release, failure without automatic movement retry, idle/active page
  exit, telemetry cancellation, offline stop, reboot/clock ordering, and mode
  change waiting for the outstanding stop.
- Dashboard and preview JavaScript syntax checked.

Command: node --test tests/config-ui.test.mjs tests/manual-drive-ui.test.mjs
Result: 22 passed, 0 failed.

Ten compile-time assertions in manual-drive-policy.cpp execute the production
ManualDriveGuard.h functions. Checked with the installed ESP32 C++ compiler,
C++17, -fsyntax-only. Covers lease age/expiry/future timestamps, millis rollover,
late and duplicate sequences after a stop, global cancellation, bounded four-page
capacity, slot reuse, invalid packets not poisoning a sequence, and cancellation
across rollover. Passed. No test buffers are linked into the firmware.

## ESP32 build

Final source compiled successfully with esp32:esp32:esp32, ESP32 core 3.3.12,
--warnings all, and installed libraries. All changed production files match the
build copy after excluding Arduino-generated declarations and line markers.

- Program: 1,086,406 / 1,310,720 bytes (82%).
- Static RAM: 52,824 / 327,680 bytes (16%); 274,856 remain before runtime allocation.
- No project-source diagnostics. Earlier clean builds reported only the existing
  ESP32Servo unused-variable warnings; the final incremental build passed.
- The verified working tree includes the user's uncommitted GPIO refresh,
  obstacle threshold and debounce changes. Those changes were preserved and
  intentionally not added to this fix's commit.

## Scope and hardware limits

MotorController's command handler now reports rejected movement truthfully and
passes the web's remaining lease to the independent motor timer. The user's
existing direct digital drive and repeated GPIO writes were preserved. No PWM
speed calibration or physical wheel feedback was introduced. setSpeed/setTrim
remain no-ops in this baseline; equal command voltage cannot guarantee equal RPM.

All 13 pin assignments match the starting revision; Config.h and concurrent
RecreationEngine.h edits are outside this fix and are left uncommitted. The motor
file's pre-existing GPIO refresh changes are also left uncommitted; only the new
command handler is included in this commit. Existing config keys, order, numeric
bounds/defaults and NVS schema remain compatible. No reset or config erase.

HTTP is still served cooperatively through the ESP32 core parser. Idle connection
wait is shortened to 150 ms, stream idle timeout to 100 ms, and response sending
has a 350 ms budget. This is not a hard total parse deadline for all possible
requests. The motor timer and timestamp-based lease, rather than browser abort,
provide the software cutoff during a stalled request. Wi-Fi modem sleep is off.

A real-board check is still required: press/release/stop under weak Wi-Fi,
changing direction/mode, page closure, router loss, actual lease cutoff and
braking distance, root/config/catalog loading, input/output voltages under load,
and the powered-off motor-output swap described in README. Motor readiness is
software readiness; it does not measure RPM, motor current, or driver voltage.

tests/preview.mjs is a UI fixture only, with compatible boot/time metadata. It
does not execute the production guard, confirm physical movement, or measure
ESP32 network latency. Older autonomy verification is historical in Git; it is
not evidence for the user's subsequently changed autonomy engine.
