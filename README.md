# assclets

Firmware for turning an **M5Stack AtomS3** into a hardware push-to-talk button
for [Wispr Flow](https://wisprflow.ai) on macOS.

This is currently **AtomS3-only firmware**. It is not a generic ESP32 project
yet. Other ESP32 boards need porting for their board target, button/input pin,
display or no-display behavior, and HID setup.

Hold the AtomS3 screen button and the device holds **Ctrl+Option** over
Bluetooth LE HID. Release the button and the keys are released. Wispr Flow uses
that shortcut to record while held, then transcribes into the focused text field.

## What You Need

- M5Stack AtomS3 with the 0.85 inch screen.
- USB-C cable for power and first flash.
- A Mac with Bluetooth and Wispr Flow installed.
- Python 3 and PlatformIO.

Generic ESP32 boards are not supported by this firmware as-is. The code expects
the AtomS3 board definition, M5Unified screen button, and 128x128 display.

The AtomS3 does not have a built-in battery. It should stay plugged into USB for
power unless you add your own external supply.

## Install PlatformIO

Install PlatformIO with one of these options:

```bash
python3 -m pip install --user platformio
```

Or install the PlatformIO extension in VS Code.

Confirm the command works:

```bash
pio --version
```

## Get The Firmware

Clone this repo and enter it:

```bash
git clone https://github.com/BlockedPath/atoms3-wispr-hotkey.git
cd atoms3-wispr-hotkey
```

If you forked the project, use your fork URL instead.

## Create Your Local Secrets File

The firmware includes Wi-Fi OTA support, so it needs local Wi-Fi settings at
build time. Copy the example file:

```bash
cp src/secrets.h.example src/secrets.h
```

Edit `src/secrets.h`:

```cpp
#define WIFI_SSID     "your-wifi-name"
#define WIFI_PASS     "your-wifi-password"

#define OTA_HOSTNAME  "atom-hotkey"
#define OTA_PASSWORD  "choose-a-long-random-ota-password"
```

`src/secrets.h` is intentionally ignored by Git. Do not commit your Wi-Fi
password or OTA password.

`OTA_PASSWORD` must be non-empty. The firmware refuses to compile with an empty
OTA password because wireless updates can replace firmware on a device paired to
your Mac as a Bluetooth keyboard.

## Build And Flash Over USB

Plug the AtomS3 into your Mac with USB-C, then build:

```bash
pio run
```

Flash the firmware:

```bash
pio run -t upload
```

Optional serial logs:

```bash
pio device monitor
```

After flashing, the AtomS3 screen should show `PAIR`.

## Pair With macOS

1. Open macOS System Settings.
2. Go to Bluetooth.
3. Connect to `AtomS3 Hotkey`.
4. If Keyboard Setup Assistant opens, close it.

When pairing succeeds, the AtomS3 screen changes from `PAIR` to `READY`.

## Configure Wispr Flow

In Wispr Flow, set push-to-talk to the same shortcut the firmware sends:

```text
Settings -> General -> Shortcuts -> Push-to-talk -> Ctrl+Option
```

Wispr requires a modifier shortcut for push-to-talk. This firmware sends exactly
left Control plus left Option.

## Use The Button

| Gesture | Result |
| --- | --- |
| Press and hold | Talk while held |
| Release after hold | Stop recording and transcribe |
| Double-tap | Lock recording on for hands-free dictation |
| Tap while locked | Stop locked recording and transcribe |

Screen states:

| State | Meaning |
| --- | --- |
| `PAIR` | Waiting for Bluetooth connection |
| `READY` | Connected and idle |
| `M:SS` with `listening` | Recording while held |
| `M:SS` with `tap to stop` | Locked recording is active |
| `last M:SS` | Length of the most recent recording |

Wispr may ignore very short clips. Hold for about one second or longer.

## Wireless OTA Updates

After the first USB flash, future updates can be sent over Wi-Fi if the AtomS3
is on the same network and the OTA dot on the screen is green.

The default OTA target is `atom-hotkey.local`, matching `OTA_HOSTNAME` in
`src/secrets.h` and `upload_port` in `platformio.ini`.

OTA uploads require the password you set in `src/secrets.h`. Keep it private and
pass the same value to PlatformIO through `ASSCLETS_OTA_PASSWORD`.

Flash over Wi-Fi:

```bash
ASSCLETS_OTA_PASSWORD='your-ota-password' pio run -e atoms3-ota -t upload
```

If you change `OTA_HOSTNAME`, also update `upload_port` in the `atoms3-ota`
environment in `platformio.ini`.

## Customize The Device

Common settings live in `include/AppConfig.h`:

| Setting | Purpose |
| --- | --- |
| `kBleDeviceName` | Bluetooth device name shown in macOS |
| `kHoldMs` | Press duration needed before hold-to-talk starts |
| `kDoubleTapMs` | Max time between taps for locked recording |
| `kRecordingFrameMs` | Display animation update interval |
| `kWifiRetryMs` | Wi-Fi reconnect retry interval |

If you rename `kBleDeviceName`, remove the old Bluetooth device in macOS and
pair again.

## Optional 9V Battery Gauge

The battery badge is disabled by default. Leave it disabled unless you wire a
resistor divider from a 9V battery pack to GPIO8.

Divider:

```text
9V (+) -- R1 330k --+-- R2 100k -- GND
                    |
                    +-- G8 / GPIO8 on AtomS3
```

Important safety checks:

- AtomS3 ground and battery ground must be shared.
- Use GPIO8 because it is on ADC1. ADC2 conflicts with Wi-Fi on ESP32.
- Before connecting GPIO8, measure the divider midpoint with a multimeter.
- A fresh 9V battery should read about 2.2V at the midpoint.
- If the midpoint reads raw battery voltage, do not connect it to GPIO8.

After wiring and verifying the divider, enable the gauge by adding this build
flag in `platformio.ini`:

```ini
build_flags =
    -DUSE_NIMBLE
    -DASSCLETS_BATTERY_GAUGE=1
```

On USB power or a normal USB power bank, keep the gauge disabled. A regulated 5V
USB supply does not expose battery state to the AtomS3.

## What Should Not Be In GitHub

These ignored files are local-only and are not needed in the GitHub repo for
another user to install the firmware:

| Ignored path | Why it should stay out of Git |
| --- | --- |
| `.pio/`, `.pioenvs/`, `.piolibdeps/` | PlatformIO build outputs and downloaded libraries |
| `*.bin`, `*.elf`, `*.map` | Generated firmware artifacts |
| `.vscode/`, `.idea/` | Local editor settings |
| `.DS_Store` | macOS Finder metadata |
| `src/secrets.h` | User-specific Wi-Fi and OTA credentials |

Users do need a `src/secrets.h` locally, but they should generate it from
`src/secrets.h.example` after cloning.

## Troubleshooting

If PlatformIO cannot find the board, update PlatformIO:

```bash
pio upgrade
pio pkg update
```

If macOS will not reconnect, remove `AtomS3 Hotkey` from Bluetooth settings and
pair again.

If Wispr does not start recording, verify the shortcut in Wispr is set to
`Ctrl+Option`. You can also use Karabiner-Elements EventViewer or the macOS
Accessibility Keyboard to confirm Control and Option are held while pressing the
AtomS3 screen.

If OTA upload fails, flash once over USB again and confirm the device is on the
same Wi-Fi network as your Mac.

If you get build errors around BLE or NimBLE, do not upgrade the pinned platform
or libraries casually. `platformio.ini` intentionally uses ESP32 Arduino core
2.0.x with NimBLE 1.4.x because the BLE keyboard library is not compatible with
all newer core/library combinations.

## Project Layout

| Path | Responsibility |
| --- | --- |
| `src/main.cpp` | Arduino `setup()` and `loop()` glue |
| `src/app/HotkeyController.*` | Button gestures, BLE HID state, recording modes |
| `src/hardware/DisplayView.*` | LCD drawing |
| `src/hardware/OtaService.*` | Wi-Fi retry and ArduinoOTA handling |
| `src/hardware/BatteryMonitor.*` | Optional battery gauge sampling |
| `include/AppConfig.h` | Device constants and compile-time flags |
| `src/secrets.h.example` | Template for local Wi-Fi and OTA config |

See `docs/ARCHITECTURE.md` for the internal design rules.
