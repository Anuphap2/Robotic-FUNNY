# Verification — firmware 2.0.0

Checked on 7 October 2026. These checks do not involve flashing or running a
physical robot.

## Firmware build

Final source compiled successfully with `--warnings all` for
`esp32:esp32:esp32`, ESP32 core 3.3.12 and the library versions listed in README.

- Program storage: 1,068,614 / 1,310,720 bytes (81%).
- Static RAM: 52,648 / 327,680 bytes (16%).
- Remaining RAM before runtime allocations: 275,032 bytes. This is not the
  free-heap measurement of a running robot; Wi-Fi, HTTP and the OLED allocate
  additional memory at runtime.
- `Config.h` has no changes compared with the original commit `0192c93`.
- JavaScript syntax check and `git diff --check` passed.

## Automated validation

`node --test tests/config-ui.test.mjs`: 6 tests passed. Tests execute the actual
dashboard validator extracted from WebUI.h and use the firmware's field schema.
They cover all 42 fields' ranges and invalid types, transformed servo homes,
overlapping proximity zones, unknown/pin keys, missing fields and blank numeric
inputs. They do not execute ESP32 C++ or NVS.

## Browser checks with simulated data

The local `tests/preview.mjs` fixture provides schema and catalogs from the source
and simulates the robot API. Checked in the in-app browser:

- 320, 375, 768, 1200 and 1280 pixel widths: no horizontal overflow inside the
  dashboard. Configuration uses one, two or three columns at the intended
  breakpoints.
- All 42 settings appear; GPIO table has no editable fields.
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
