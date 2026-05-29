# assclets — AtomS3 Wispr Flow push-to-talk button

Turns an **M5Stack AtomS3** into a hardware **push-to-talk** button for
[Wispr Flow](https://wisprflow.ai) on macOS.

Hold the screen → the AtomS3 holds **Ctrl+Option** down over Bluetooth → Wispr
Flow records the mic. Release → the keys let go → Wispr transcribes and inserts
the text where your cursor is.

The whole front LCD of the AtomS3 is a button (GPIO41), so you literally press
the screen in to talk.

## Hardware

- **M5Stack AtomS3** (ESP32-S3, 0.85" LCD). USB-C for power/flashing.
- A Mac with Bluetooth and Wispr Flow installed.

> The AtomS3 has no battery, so it stays plugged in for power. It talks to the
> Mac over **Bluetooth LE** (the ESP32-S3 has no Bluetooth Classic, which is
> fine — macOS supports BLE HID keyboards).

## Build & flash

Requires [PlatformIO](https://platformio.org/install) (`pip install platformio`
or the VS Code extension). All library/core versions are pinned in
`platformio.ini`, so the build is reproducible.

First, set up your Wi-Fi/OTA credentials (git-ignored, never committed):

```bash
cp src/secrets.h.example src/secrets.h   # then edit src/secrets.h
```

Then build and flash over USB-C:

```bash
pio run                 # compile
pio run -t upload       # flash over USB-C
pio device monitor      # (optional) view boot/serial logs at 115200
```

### Wireless updates (Wi-Fi OTA)

Once the firmware is on the device and it has joined your Wi-Fi, you can flash
**without a cable** — the small dot in the top-right of the screen turns green
when OTA is ready:

```bash
pio run -e atoms3-ota -t upload     # flashes to atom-hotkey.local over Wi-Fi
```

The hostname (`atom-hotkey`) is set by `OTA_HOSTNAME` in `secrets.h` and matched
by `upload_port` in `platformio.ini`. Set `OTA_PASSWORD` (and `--auth` in the OTA
env) if you want it password-protected.

## Pair with macOS

1. After flashing, the screen shows **PAIR**.
2. macOS **System Settings → Bluetooth** → connect **"AtomS3 Hotkey"**.
   - If the *Keyboard Setup Assistant* pops up, just close it.
3. Once connected the screen shows **READY**.

## Configure Wispr Flow

**Wispr Flow → Settings → General → Shortcuts → Push-to-talk → set to
`Ctrl+Option`** (hold both, the firmware sends exactly this).

> Wispr's push-to-talk requires a *modifier* combo — lone keys and bare F-keys
> are blocked. `Ctrl+Option` is Wispr's own recommended combo.

## Use

Focus any text field, then:

| Gesture | Action |
|---------|--------|
| **Press & hold** | Talk while held (push-to-talk); release to transcribe |
| **Double-tap** | **Lock** recording on (hands-free) — keeps Ctrl+Option held |
| **Tap** (while locked) | Stop & transcribe |

Wispr needs about a 1-second minimum, so quick taps don't dictate.

Screen states: **PAIR** (waiting for Bluetooth) · **READY** (connected, idle) ·
**REC** "listening" (held) · **REC** "tap to stop" on a dark-red background
(locked / hands-free). A green dot top-right means Wi-Fi OTA is ready.

> A press must last ~250 ms to count as a hold (vs. a tap) — this is how the
> firmware tells push-to-talk from a double-tap. Tune `HOLD_MS` / `DTAP_MS` in
> `src/main.cpp` to taste.

## Notes & troubleshooting

- **~1 second minimum hold** — quick taps do nothing (this is Wispr's behavior).
- **Verify the raw hotkey** (before wiring up Wispr): open Karabiner-Elements →
  EventViewer, or the macOS *Accessibility Keyboard*
  (System Settings → Accessibility → Keyboard), and confirm `⌃` + `⌥` light up
  while held and clear on release.
- **Stuck modifiers:** the firmware clears Ctrl+Option on every fresh BLE
  connect, so a hold interrupted by sleep/disconnect can't leave them latched.
- **Won't reconnect after sleep/reboot:** toggle Bluetooth, or remove the
  device in Bluetooth settings and re-pair.
- **Build errors about NimBLE:** make sure `NimBLE-Arduino` resolved to a
  **1.4.x** (not 2.x) and that the platform is `espressif32@6.9.0` (ESP32 core
  2.0.x). See comments in `platformio.ini`.

## Ideas / roadmap

See [`IDEAS.md`](IDEAS.md) for a ranked backlog of features to add (mute toggle,
mode switcher, IMU gestures, media keys, macropad, etc.), plus the hardware
capabilities and constraints.

## Rename the device

Change the name string in `src/main.cpp`:
`BleKeyboard bleKeyboard("AtomS3 Hotkey", "M5Stack", 100);`
