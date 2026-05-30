# Architecture

`assclets` is firmware for one purpose-built device: an M5Stack AtomS3 that acts
as a Wispr Flow push-to-talk button over BLE HID, with Wi-Fi OTA as a supporting
service.

The code is intentionally split by runtime responsibility:

| Path | Responsibility |
|------|----------------|
| `src/main.cpp` | Arduino lifecycle adapter only (`setup` / `loop`) |
| `src/app/HotkeyController.*` | Device state machine, button gestures, BLE key actions |
| `src/hardware/DisplayView.*` | LCD rendering and screen-specific layout |
| `src/hardware/BatteryMonitor.*` | Optional ADC battery gauge sampling |
| `src/hardware/OtaService.*` | Wi-Fi connection retry and ArduinoOTA handling |
| `include/AppConfig.h` | Compile-time device constants and feature flags |
| `src/secrets.h` | Local Wi-Fi / OTA credentials; ignored by Git |

## Runtime Flow

1. `setup()` initializes M5 hardware, display buffers, BLE HID, OTA, and the
   optional battery monitor.
2. `loop()` delegates to `HotkeyController::update()`.
3. The controller polls M5 button state, Wi-Fi OTA, BLE connection edges, and
   low-rate battery updates.
4. All user-visible state is rendered through `DisplayView`; application logic
   never draws directly to the LCD canvas.

## Design Rules

- Keep `main.cpp` free of product behavior. New behavior should live behind a
  focused class or small helper.
- Keep hardware service classes narrow. They may own device APIs, but they
  should not own the push-to-talk state machine.
- Avoid blocking network work. BLE HID is the primary function; OTA must remain
  best-effort and non-blocking.
- Add compile-time flags in `include/AppConfig.h` when a feature depends on
  optional wiring or hardware.
- Keep display strings short enough for the 128x128 LCD and draw through the
  off-screen canvas to avoid flicker.
