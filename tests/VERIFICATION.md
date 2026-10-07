# Verification — firmware 2.3.4

Checked on 8 October 2026 (Asia/Bangkok). No firmware was uploaded and no
physical robot was driven. The user reports that the web stays connected while
all servos and motors remain still. The user subsequently reports firmware
2.3.3, HIGH/LOW motor GPIO and failed servo initialization. No measured rail
voltage or serial boot log has been supplied, so the physical cause remains
unconfirmed.

## Software checks

Node executes the actual functions extracted from WebUI.h:

- 13 configuration/telemetry tests: 51 settings, bounds, types, servo limits,
  distances, unknown/GPIO fields, blank values, v1/v2 imports and Interactive UI.
- 9 manual-control regressions: outstanding request/stop/abort ordering,
  heartbeat timing, no parallel drive queue, direction/mode changes, late
  responses, old button release, stalled requests without automatic movement
  retry, page exit, telemetry cancellation, offline stop and reboot clocks.
- 7 hardware UI tests: live enable flags override stale config; each servo's
  status is independent; watchdog errors and panic/brownout differ; old telemetry
  does not invent readiness; GPIO polarity/low outputs/pending transitions are
  reported; individual servo tests use only the chosen slider and reject drafts;
  detailed servo/audio error messages include the firmware version.

Command: node --test tests/config-ui.test.mjs tests/manual-drive-ui.test.mjs
  tests/hardware-ui.test.mjs
Result: 29 passed, 0 failed. Dashboard and preview JavaScript syntax passed.

The installed ESP32 C++ compiler, C++17, -fsyntax-only, also passed:

- Ten assertions executing production ManualDriveGuard.h, covering expiry,
  future timestamps, rollover, stop/cancellation ordering, duplicate sequences,
  four-page capacity, slot reuse and invalid packets.
- Thirteen assertions per compilation executing production ServoOutput.h and
  ServoSignal.h, in four peripheral modes: normal attachment, attachment failure,
  wrong frequency and write failure. Covers 50 Hz / 20 ms frames, saved pulse
  calibration, angle clamping, 16-bit duty, invalid settings, independent
  channels, detach/retry and error propagation. Only the peripheral calls are
  stubbed; these checks do not prove actual electrical output. Test stubs and
  assertions are not linked into the firmware.

Telemetry format fits its fixed 1,536-byte buffer: a conservative 1,271-byte
estimate uses 32-byte quoted values, full uint32/32-bit integer widths, full
boolean literals and a wider distance than this sensor reports. The runtime
snprintf truncation check remains. Diagnostics use fixed stack buffers; response
transmission retains the web core's existing allocations.
Git diff --check passed.

## Confirmed servo startup regression

The installed core 3.3.12 ledcReadFreq checks ledcRead (duty) and returns zero
when duty is zero. Firmware 2.3.3 checked that helper immediately after attach,
before writing the saved rest pose, and detached successful channels. This is a
confirmed software defect that matches the user's failed-servo status.

Firmware 2.3.4 reads the assigned timer's frequency via ledc_get_freq, using the
core peripheral manager's channel/timer metadata. It verifies the expected
channel and 16-bit resolution as well as 50 Hz. Readiness no longer depends on
duty or the first frame's latch timing. No temporary test pose or delay was added.
The stubs now model the pin-level helper returning zero; a negative control
compiled the previous committed driver against those stubs and failed the
startup assertion as expected. The corrected production driver passes all four
modes. This regression was missed by the earlier stub which always returned 50.

Manual three-axis positions and motion frames propagate write failures instead
of reporting success. Motion freezes on a failed frame. Typed showcase/motion
and axis commands check enabled/individual servo readiness, and distinguish
unknown IDs, disabled audio, audio initialization and servo write failures.
No motor signal level, drive speed or lease was altered in this follow-up.

## ESP32 build

Final source compiled successfully for esp32:esp32:esp32, ESP32 core 3.3.12,
--warnings all, using installed libraries. No project-source diagnostics.

- Program: 1,088,098 / 1,310,720 bytes (83%).
- Static RAM: 52,512 / 327,680 bytes (16%); 275,168 before runtime allocation.
- ESP32Servo is no longer a firmware dependency. The new servo driver uses the
  core's attachment result and duty-independent timer readback, channels 0/1/2,
  50 Hz/16-bit.
  Buzzer remains channel 6/10-bit; core 3.3.12 assigns timers dynamically according
  to compatible frequency and resolution, not a fixed channel-to-timer mapping.
- Ten source files match the final build snapshot: sketch, servo output/signal,
  motion, motor, song, dashboard, RuntimeConfig, Config and RecreationEngine.
  Arduino-generated prototypes/line markers are excluded from this comparison.

The installed ESP32PWM attachPin path sets its attachedState before checking
ledcAttachChannel and retains that state on failure. This is a verified false
readiness path, not evidence that it failed on the user's board. The currently
installed ESP32Servo setTimerWidth guards a zero width difference, so a division
by zero is not established as the cause of this incident. External libraries
were only read, not modified by this fix.

## Preserved state and hardware limits

All 13 GPIO assignments match starting revision 0192c93. Existing settings,
key order, bounds/defaults and NVS v2 schema remain compatible. No reset or
config erase. Config.h and RecreationEngine.h user edits are left uncommitted.
The user's motor GPIO refresh/digitalWrite changes are preserved in the tested
working tree and left uncommitted. This follow-up does not edit the motor file;
the previous fix committed only watchdog lifecycle, diagnostics and a stale-drive
fallback in that file.

Web commands now distinguish disabled devices from driver initialization errors.
A retry first stops motors/audio/motion/autonomy, cancels manual drive, reuses
an existing timer handle and reattaches enabled servos at their saved rest angles.
It respects saved disable flags. Single-axis tests can operate a healthy servo
without requiring the other two outputs to be ready.

Motor GPIO levels come from gpio_get_level on the original output pins. This
core's OUTPUT mode also enables input readback. The snapshot holds the motor
lock; it reports logic levels, pending direction, command age and lease. Both
telemetry and action responses include it, so manual driving can display the
acknowledged signal state while telemetry is paused. It cannot measure driver
voltage, current, encoder/RPM or confirm physical wheel rotation.

Existing direct digital drive, 5 ms direction dead time, independent 2 ms motor
timer, 400 ms timestamp-based manual lease, 100 ms heartbeat and 350 ms web-drive
request timeout remain. setSpeed/setTrim are still no-ops in the user's baseline.
No physical wheel speed calibration is claimed.

Real-board checks remain: upload and reload the dashboard; confirm saved enables
and per-axis status; with wheels lifted test stop/release/direction changes and
individual servos; measure power and driver outputs under load if GPIO changes
but motors stay still. The user reports a 5V 3A stepdown; its label alone does
not prove the voltage during motor/servo startup. README contains the powered-off
motor-output swap check. tests/preview.mjs is a browser fixture only, not the
production hardware lifecycle or a measurement of ESP32 latency.
