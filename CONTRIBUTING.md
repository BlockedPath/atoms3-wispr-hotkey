# Contributing

Thanks for helping improve `atoms3-wispr-hotkey`.

This is firmware for an M5Stack AtomS3 that acts as a Wispr Flow push-to-talk
button over BLE HID. Keep the BLE hotkey path responsive; Wi-Fi OTA, display
rendering, and optional battery UI support that core loop.

## Development Setup

1. Install PlatformIO.
2. Clone the repo.
3. Copy `src/secrets.h.example` to `src/secrets.h`.
4. Fill in local Wi-Fi and OTA credentials in `src/secrets.h`.

Do not commit `src/secrets.h` or any build outputs.

## Build

Run the default firmware build before opening a pull request:

```bash
pio run
```

Use USB upload for a fresh device:

```bash
pio run -t upload
```

Use OTA only after OTA firmware is already installed and the device is reachable:

```bash
ASSCLETS_OTA_PASSWORD='your-ota-password' pio run -e atoms3-ota -t upload
```

## Code Guidelines

- Keep `src/main.cpp` as Arduino lifecycle glue only.
- Put button gestures, BLE HID behavior, and high-level state in
  `src/app/HotkeyController.*`.
- Draw only through `src/hardware/DisplayView.*`.
- Keep display strings to 12 characters or fewer.
- Keep OTA best-effort and non-blocking.
- Put compile-time feature flags and device constants in `include/AppConfig.h`.
- Prefer small, readable changes over broad refactors.

## Hardware Validation

For changes that touch BLE, gestures, OTA, display, or battery behavior, validate
on hardware when possible:

- Pairing shows `PAIR` then `READY`.
- Holding the screen sends and holds Ctrl+Option.
- Releasing the screen releases the keys.
- Double-tap enters locked recording.
- A later tap exits locked recording.
- The OTA dot reflects readiness without interrupting BLE behavior.
- Battery UI stays hidden unless the gauge is enabled and a valid pack is
  detected.

If hardware validation was not possible, say that in the pull request.
