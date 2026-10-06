# Verification — firmware 2.1.0

Checked on 7 October 2026. These checks do not involve flashing or running a
physical robot.

## Firmware build

Final source compiled successfully with `--warnings all` for
`esp32:esp32:esp32`, ESP32 core 3.3.12 and the library versions listed in README.

The full rebuild reported four unused-variable warnings in the installed
ESP32Servo library; no project-source warnings or errors were reported. The
final incremental build completed without diagnostic output.

- Program storage: 1,078,454 / 1,310,720 bytes (82%).
- Static RAM: 52,736 / 327,680 bytes (16%).
- Remaining RAM before runtime allocations: 274,944 bytes. This is not the
  free-heap measurement of a running robot; Wi-Fi, HTTP and the OLED allocate
  additional memory at runtime.
- `Config.h` has no changes compared with the original commit `0192c93`.
- JavaScript syntax check and `git diff --check` passed.

## Automated validation

`node --test tests/config-ui.test.mjs`: 10 tests passed. Tests execute the actual
dashboard validator extracted from WebUI.h and use the firmware's field schema.
They cover all 51 fields' ranges and invalid types, transformed servo homes,
overlapping proximity zones, unknown/pin keys, missing fields and blank numeric
inputs. New tests cover autonomous distance relationships, v1/v2 JSON migration,
maximum form-body size and the actual page-exit handler: an idle page does not
cancel autonomy, while active/in-flight manual driving sends a keepalive stop.
They do not execute ESP32 peripherals or NVS.

`DecisionTree.h` contains eight compile-time assertions executing the production
classifier (missing echo, close boundary, insufficient samples, discontinuity,
far range, both directions of target motion, and small stationary noise), plus
one regression executing the production six-reading window, reset, capacity and
outlier rejection. These passed in the final ESP32 build. No motor timing or
physical person detection is inferred from these checks.

## Prior browser checks with simulated data (v2.0.0)

These were completed for the previous dashboard and retained as prior evidence.
The specific final v2.1.0 checks are listed in the next section.

The local `tests/preview.mjs` fixture provides schema and catalogs from the source
and simulates the robot API. Checked in the in-app browser:

- 320, 375, 768, 1200 and 1280 pixel widths: no horizontal overflow inside the
  dashboard. Configuration uses one, two or three columns at the intended
  breakpoints.
- All 42 original settings appear; GPIO table has no editable fields.
- Saving updates the revision and clears the dirty count.
- Invalid servo home and proximity zone values are rejected before submission.
- A second browser window with an old revision shows the conflict error and
  preserves its draft.
- Search filters matching groups/fields.
- JSON import fills a draft without automatically saving; importing a file with
  a GPIO key is rejected without changing the draft.
- Ctrl+Home preserves the selected Hardware tab; no JavaScript errors were
  reported on the final page.
- Export creates the Blob/link and success feedback without JavaScript errors;
  the embedded browser did not emit a download event, so the saved file itself
  was not verified in this environment.

The fixture does not verify C++ handlers, Preferences persistence, interrupts,
PWM timing, driver voltages or actual actuator movement. Use the physical-board
checks in README before operation. In particular, verify config across a reboot,
watchdog behavior with a stalled HTTP client, servo limits, simultaneous audio
and movement, sonar timeout and free heap during sustained use.


## Interactive 2.1.0 and score memory checks

- Confirmed in the browser fixture: selecting follow, starting, presenting a
  moving-candidate reason, human confirmation, holding distance, disabling the
  confirmation button after confirmation, emergency stop and selecting roam.
- Closing the final idle dashboard and opening another dashboard retained the
  simulated follow mode and confirmed target. The page-exit regression also
  tests the manual-drive stop cases.
- New autonomous fields appear (51 fields total). Invalid stop/clearance
  relationship is rejected, and saving a valid pulse duration updates revision
  and clears the draft. This is simulated persistence, not NVS verification.
- Checked the final Interactive UI at 320, 375, 768 and 1280 pixels, and Hardware
  config at 320 and 375: document scroll width does not exceed client width.
- Final page reported no browser warnings/errors.
- All 19 registered note tables, 443 notes total, preserve the previous note
  sequences exactly against commit `127de54`. The old ELF table sizes total
  5,316 bytes; the final ELF table sizes total 1,772 bytes, verified with
  `xtensa-esp32-elf-nm -S`. Every table is one-third the original size. The score
  constructor and four-byte Note layout are checked at compile time.
- Saving 3,544 bytes of note data is a Flash reduction. Scores were already in
  PROGMEM; no corresponding free-heap reduction is claimed. Total firmware
  increases because of the new behavior and UI. Static RAM increases by 88 bytes
  over v2.0.0; the new range history has a fixed capacity and no dynamic storage.

The range classifier cannot identify humans, distinguish a stationary human
from a wall/object, detect target identity replacement, see lateral targets or
measure rear/side/floor clearance. Following requires a human-confirmed front
candidate, and loss of echo/range or ambiguous readings clears confirmation.
Autonomous pulses are bounded by an independent timer lease, but physical
stopping distance, turn clearance, timing under HTTP stalls, actual sensor
placement and NVS v1 migration across reboot remain hardware test items.
No firmware was flashed during this task.
