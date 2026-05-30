# Agent Notes

This repo contains firmware for an M5Stack AtomS3 that acts as a Wispr Flow
push-to-talk button over BLE HID. Treat the BLE hotkey path as the primary
product behavior; Wi-Fi OTA, display rendering, and the optional battery gauge
support that core loop.

## Project Shape

- PlatformIO project targeting `m5stack-atoms3` with the Arduino framework.
- `platformio.ini` pins `espressif32@6.9.0`, ESP32 Arduino core 2.0.x,
  `ESP32 BLE Keyboard`, `M5Unified`, and `NimBLE-Arduino@^1.4.1`.
- Keep those pins intentional. The BLE keyboard library is known not to compile
  cleanly with ESP32 Arduino core 3.x / newer NimBLE combinations.
- Local Wi-Fi and OTA credentials live in `src/secrets.h`; never commit that
  file. Use `src/secrets.h.example` as the template.

## Build And Flash

Common commands:

```bash
pio run
pio run -t upload
pio device monitor
pio run -e atoms3-ota -t upload
```

Use USB upload for a fresh device. Use the `atoms3-ota` environment only after
OTA firmware is already installed and the device is reachable as
`atom-hotkey.local` or whatever `OTA_HOSTNAME` is set to locally.

## Code Boundaries

- `src/main.cpp` should stay as Arduino lifecycle glue only.
- `src/app/HotkeyController.*` owns the button gesture state machine, BLE HID
  key presses, connection transitions, and high-level device state.
- `src/hardware/DisplayView.*` owns all LCD drawing through the off-screen
  canvas. Keep strings short enough for the 128x128 display.
- `src/hardware/OtaService.*` owns Wi-Fi retry and ArduinoOTA handling. OTA
  must remain best-effort and non-blocking so BLE HID stays responsive.
- `src/hardware/BatteryMonitor.*` owns optional ADC sampling for the 9V gauge.
- `include/AppConfig.h` is where compile-time feature flags and device constants
  belong.

Do not add product behavior directly to `loop()`. Prefer a focused class or a
small helper behind `HotkeyController` when adding features.

## Hardware Safety

- The AtomS3 screen button is GPIO41 via `M5.BtnA`.
- The firmware sends `KEY_LEFT_CTRL` + `KEY_LEFT_ALT` for Wispr Flow
  push-to-talk.
- The optional battery badge is disabled by default. Only enable
  `ASSCLETS_BATTERY_GAUGE=1` after the resistor divider is actually wired.
- Battery gauge input is GPIO8 / ADC1. ADC1 is deliberate because ADC2 conflicts
  with Wi-Fi on ESP32.
- Before connecting a 9V divider to GPIO8, the midpoint must be measured with a
  multimeter. It should be about 2.2V from a fresh 9V battery; wiring raw 9V to
  the pin can destroy the device.

## Implementation Guidance

- Keep button handling responsive. Avoid long delays or blocking network work in
  paths reached from `HotkeyController::update()`.
- Release stuck modifiers defensively on disconnect/reconnect paths.
- Render visible state only through `DisplayView`; do not draw directly from
  app logic.
- Add feature flags in `include/AppConfig.h` for behavior that depends on
  optional wiring, hardware variants, or local setup.
- Preserve the existing namespace style: `namespace assclets { ... }`.
- Prefer small, readable C++ changes over broad refactors. This is firmware for
  a tiny device, so memory use, redraw frequency, and blocking calls matter.

## Validation

At minimum, run `pio run` after code changes. For changes that touch BLE,
gestures, OTA, display, or battery behavior, also validate on hardware when
possible:

- Pairing shows `PAIR` then `READY`.
- Holding the screen sends and holds Ctrl+Option, then releases on button
  release.
- Double-tap enters locked recording and a later tap releases the keys.
- The OTA dot reflects readiness without interrupting BLE behavior.
- Battery UI remains hidden unless the gauge is enabled and a valid pack is
  detected.

If hardware validation was not possible, say so clearly in the final response.
