# Feature Ideas & Hardware Notes — AtomS3 Hotkey

A backlog of things this device *could* do, plus what the hardware is actually
capable of and where the limits are. The current firmware is a BLE HID
push-to-talk button for Wispr Flow (hold = talk, double-tap = lock, tap = stop)
with Wi-Fi OTA. Everything below builds on that base.

> **The one constraint that shapes everything:** the screen is **not a
> touchscreen**. It's a 128×128 IPS LCD with a single physical button behind the
> glass (GPIO41). Every "menu" / "macropad" idea needs a *selector* — button
> tap-count / hold, **IMU tilt/flick**, or extra buttons/encoder via the Grove
> port.

## Feature backlog (ranked by value-to-effort)

| # | Feature | What it does | HW used | Effort | Reuse of current code |
|---|---------|--------------|---------|--------|-----------------------|
| 1 | **Video-call mute toggle** | Press sends Zoom/Meet/Slack mute shortcut; LCD shows big `MUTED`/`LIVE` | BLE HID, LCD, button | Easy | Same model as PTT — one chord + state flag + redraw |
| 2 | **Profile / mode switcher** | Long-press cycles modes (Dictation / Mute / Media / Macros); LCD shows current; button action depends on mode | LCD, button | Easy | PTT becomes one mode; turns the button into a multi-tool |
| 3 | **Media / Consumer-Control keys** | Play-pause, vol ±, next/prev via HID Consumer Control page | BLE HID, button, IMU opt. | Easy | Adds a HID usage page alongside the keyboard |
| 4 | **IMU flick/tilt gestures** | Flick to switch layer / lock-unlock recording; "knock" to fire an action hands-free | MPU6886, LCD | Medium | A second input axis next to the button; pairs with #2 |
| 5 | **Multi-key macropad (button-driven)** | Cycle stored hotkeys on the LCD (short-press), fire (long-press); IMU tilt to scroll | LCD, button, BLE HID | Medium | Same HID send path, a selectable list instead of one chord |
| 6 | **Macropad via Grove expansion** | Plug a Grove key/dual-button/rotary-encoder unit for real keys + a knob (volume/scrub) | Grove I²C/GPIO, BLE HID, LCD labels | Medium | Solves the no-touch problem; LCD shows per-key labels |
| 7 | **BLE ↔ USB-HID output toggle** | Switch keystroke output between BLE and the USB-C cable (USB = no pairing, works at boot/login) | USB native HID, BLE, button | Medium | Duplicates the HID send path to USB + a flag |
| 8 | **USB-CDC status HUD** | A Mac script pushes data (build status, next calendar event, now-playing, unread count) over USB serial; LCD displays it | USB CDC serial, LCD | Medium | Always-USB-powered → a serial pipe is free; no Wi-Fi/RAM cost |
| 9 | **Now-playing / media HUD** | Show current track + transport; combine with #3 media keys | USB CDC in + BLE HID out, LCD | Medium | Sending keys is trivial; *receiving* metadata needs a Mac helper |
| 10 | **Wi-Fi status display** | Pull a simple HTTP/MQTT endpoint (CI, home-assistant, weather) and render | Wi-Fi, LCD | Medium–Hard | Independent of HID; heaviest RAM user — prefer plain HTTP over OAuth |
| 11 | **Display dim + wake-on-motion** | Backlight dims/sleeps when idle; IMU motion or button wakes it | MPU6886 IRQ, LCD, button | Easy–Medium | UX/desk-glare polish (not power — there's no battery) |
| 12 | **IR universal remote / desk macro** | Blast IR codes (TV, AC, projector) on a button or mode | IR transmit LED, button, LCD | Medium | A non-HID "desk gadget" mode that slots into #2 |

**Recommended next builds:** #1 (mute toggle) and #2 (mode switcher) are the
highest utility-per-hour and reuse the current code almost verbatim. #4 (IMU
gestures) is the most novel payoff for medium effort.

## Hardware capabilities

- **MCU:** ESP32-S3FN8, dual-core Xtensa LX7 @ 240 MHz, 8 MB flash, **512 KB
  SRAM, no PSRAM** on this module.
- **Display:** 128×128 IPS LCD, GC9107 over SPI. Great for text, icons, bars,
  simple sprites via M5GFX/LovyanGFX. **Non-touch.**
- **Input:** single physical button = the whole screen pressing GPIO41
  (distinguish via press duration / tap count / IMU).
- **IMU:** MPU6886 6-axis (accel + gyro), I²C 0x68 internal — tilt, flick,
  knock-detection, orientation, motion-wake interrupt.
- **Radios:** Wi-Fi 802.11 b/g/n (2.4 GHz) **+** BLE 5.0, concurrent
  (time-sliced single radio).
- **USB:** native USB-C (OTG, fixed GPIO19/20) — USB HID device **and/or** USB
  CDC serial to the Mac. Always USB-powered (no battery).
- **Expansion:** HY2.0-4P Grove port = 5V/GND/G1/G2 (I²C-capable → encoders, key
  units, sensors). Exposed GPIO: **G5, G6, G7, G8, G38, G39**.
- **IR:** infrared transmit LED present on this screen variant — usable for
  remote tricks.
- **Toolchain:** Arduino + PlatformIO (board `m5stack-atoms3`), M5Unified/M5GFX;
  BLE HID via T-vK ESP32-BLE-Keyboard (NimBLE saves RAM); USB HID via Arduino
  `USBHIDKeyboard`/TinyUSB.

## Limitations / gotchas

- **Screen is not a touchscreen** — one physical button only. Real multi-key
  input needs IMU gestures or external buttons/encoder on Grove/GPIO.
- **BLE only — no Bluetooth Classic / A2DP** on ESP32-S3. Can't be a Bluetooth
  audio sink/source. HID-over-BLE is fine.
- **No battery** — "deep sleep for power" is moot; the meaningful version is
  backlight-off + wake-on-motion for desk glare/UX.
- **512 KB SRAM, no PSRAM** — BLE + Wi-Fi + TLS + display buffers together is
  genuinely tight. They *can* coexist; lean on NimBLE, avoid heavy TLS/JSON if
  you also want a live display.
- **USB is host OR device, not both at once** (single OTG port,
  software-switchable) — "read a real keyboard and re-emit over BLE" needs
  mode-switching, not concurrency.
- **Status displays need a data source.** Pulling Google Calendar/OAuth directly
  on-device is painful at this RAM. Clean path: a tiny **Mac-side script pushing
  data over USB CDC serial** — no Wi-Fi, no OAuth, coexists with BLE HID.
- **HID asymmetry:** *sending* media/Consumer-Control keys is trivial;
  *receiving* now-playing metadata or app state always needs a Mac-side helper.
- **No addressable RGB LED** on the screen variant (the LCD replaces it — that's
  the Lite/U feature).

## Synergies with the dictation use case

- **Mode switcher (#2) makes PTT one mode among several** — dictation stays
  highest-value, but mute toggle / media / macros share the device with zero
  context-switching.
- **Mute toggle (#1)** is the same mental model as PTT (hold-to-talk vs.
  toggle-to-mute), same HID path — arguably the best desk-utility for anyone on
  calls all day.
- **LCD as live status:** the double-tap-to-lock state is already tracked — keep
  rendering it big so you can glance and know whether Wispr is hot.
- **IMU gestures (#4)** let you lock/unlock or switch to mute mode without
  lifting your hand to press.
- **USB-HID output mode (#7):** push-to-talk over the USB cable is more reliable
  than BLE (no pairing dropouts, works before login) — a nice fallback for the
  same Ctrl+Option chord.

## Multi-device / fleet ideas (using owned hardware)

Bringing in the rest of the kit (see [`INVENTORY.md`](INVENTORY.md): more ESP32s,
2× Pi Zero W, 2× Arduinos, a Pico W). The unlocking insight: a mixed fleet has
**Wi-Fi as the only universal bus**, so MQTT/UDP/HTTP is the common language;
ESP-NOW is a fast shortcut *between ESP chips only*; and the **Pi Zero W can be a
USB gadget**, which is the keystone for several of these.

| # | Build | What it does | Devices | Key enabler | Effort |
|---|-------|--------------|---------|-------------|--------|
| F1 | **USB-HID bridge + fleet hub** ⭐ | Pi Zero W plugs into the Mac as ONE rock-solid USB keyboard/mouse; every wireless button (AtomS3, ESP32s, Pico W) sends events to the Pi, which re-emits them as USB HID. Kills BLE pairing flakiness and scales to unlimited buttons. | Pi Zero W (bridge) + AtomS3 + ESP32s + Pico W | Pi USB gadget (`libcomposite` HID) + MQTT/UDP | Med–Hard |
| F2 | **OTA / fleet manager** | The Pi hosts every device's firmware and a web dashboard; one click pushes Wi-Fi OTA to all ESP32s and shows who's online. Extends the OTA we already built. | Pi Zero W + all ESP32s | HTTP firmware server + ArduinoOTA/`espota` | Med |
| F3 | **ESP-NOW desk-button network** | Scatter ESP32 buttons (mute / dictation / scene) around the desk/room; all hit one receiver with near-zero latency, no router. Receiver bridges to the Mac (via F1 or its own USB/BLE). | ESP32s + 1 receiver (AtomS3 or ESP32) | ESP-NOW | Med |
| F4 | **Desk-automation hub** | Pi runs MQTT + Node-RED/Home-Assistant-lite; ESP32 sensor/button nodes publish; AtomS3 shows status + triggers scenes. The "brain" that ties everything together. | Pi Zero W + ESP32s + AtomS3 | MQTT broker on the Pi | Med |
| F5 | **Pico W macropad (KMK)** | A dedicated, ultra-reliable native-USB macro keyboard running KMK/CircuitPython, sitting next to the dictation button. | Pico W + key switches | KMK firmware | Easy–Med |
| F6 | **Wireless status displays** | ESP32 + small screens around the room showing CI/build status, next calendar event, now-playing — fed by the Pi hub. | ESP32s (+ screens) + Pi | MQTT/HTTP from F4 | Med |
| F7 | **Presence/context auto-switch** | An ESP32 with a presence/light sensor tells the hub when you're at the desk → auto-switch Wispr profile, dim displays, set mute state. | ESP32 + sensor + Pi/AtomS3 | ESP-NOW/MQTT | Med |
| F8 | **Analog control surface** | Arduino with pots/encoders/faders → volume, scrub, brush-size; sent as serial to the Mac, or routed through the F1 bridge as HID/scroll. | Arduino (+ pots/encoders) | USB serial (→ HID via Leonardo or the Pi bridge) | Easy–Med |
| F9 | **BT-audio tricks** | A *classic* ESP32 (not the S3) can do Bluetooth Classic / A2DP — e.g. a BT audio receiver or a BT-HID device the S3 can't be. | classic ESP32 | Bluetooth Classic / A2DP | Med |
| F10 | **Conference command center** | Combine: AtomS3 mute/PTT + a Pico/ESP32 macropad for Zoom/OBS scenes + a Pi web panel = a full call/stream control deck. | AtomS3 + Pico W/ESP32 + Pi | F1/F4 + HID | Med–Hard |

**Where to start:** **F1 (Pi Zero W USB-HID bridge)** is the keystone — it makes
every other wireless device reliable and removes the BLE pairing pain from the
current dictation button, then F2/F3/F4 build naturally on top of it.

**Honest constraints for the fleet:**
- **Pi Zero W is slow** (single-core ARM11) — perfect as a bridge / MQTT broker /
  light web server, but *not* for on-device Whisper or LLMs. Use it to route and
  serve, not to compute.
- **ESP-NOW is ESP-only** — Pico W and Pi can't join it. For a truly mixed fleet,
  standardize on **MQTT/UDP over Wi-Fi** and use ESP-NOW only as a fast lane
  among the ESPs (with a bridge node).
- **Arduino USB HID depends on the model** — Uno/Nano (ATmega328) can't be a
  native USB keyboard without reflashing the USB chip; a Leonardo/Micro/Pro-Micro
  can. Either way it can always talk serial and route through the F1 bridge.
- **ESP32-S3 (AtomS3) has no Bluetooth Classic** — for A2DP/BT-audio ideas you
  need one of the *classic* ESP32 boards.

## Sources

- AtomS3 docs — <https://docs.m5stack.com/en/core/AtomS3>
- AtomS3 product page (spec list, IR, screen) — <https://shop.m5stack.com/products/atoms3-dev-kit-w-0-85-inch-screen>
- PlatformIO board `m5stack-atoms3` — <https://docs.platformio.org/en/latest/boards/espressif32/m5stack-atoms3.html>
- ESP32-S3 BLE-only (no Classic/A2DP) — <https://github.com/pschatzmann/ESP32-A2DP/issues/420>
- ESP32-S3 USB OTG host-or-device — <https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/peripherals/usb.html>
- Memory (512 KB SRAM, no PSRAM) — <https://devices.esphome.io/devices/m5stack-atoms3-lite/>
- ESP32-S3 native USB HID (Arduino/TinyUSB) — <https://philippkueng.ch/2025-06-16-creating-a-virtual-usb-keyboard-with-the-esp32-s3.html>
- BLE HID keyboard library (T-vK, NimBLE) — <https://github.com/T-vK/ESP32-BLE-Keyboard>
