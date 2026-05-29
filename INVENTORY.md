# Hardware Inventory

Stuff I own that could extend this project or seed new ones. Fleet-level project
ideas that combine these live in [`IDEAS.md`](IDEAS.md) → *Multi-device / fleet
ideas*.

> Some models below are unconfirmed — fill in the exact board so the ideas can be
> pinned down (it matters: e.g. an Arduino Uno can't be a native USB keyboard but
> a Leonardo/Micro can; a classic ESP32 has Bluetooth Classic/audio but the
> ESP32-S3 doesn't).

| Device | Qty | Key capabilities | Notes / confirm |
|--------|-----|------------------|-----------------|
| **M5Stack AtomS3** (ESP32-S3) | 1 | Wi-Fi + BLE, native USB HID/CDC, 128×128 LCD button, MPU6886 IMU, IR TX | The current dictation device |
| **ESP32 boards** | "multiple" | Wi-Fi + BLE **+ Bluetooth Classic** (A2DP/BT-HID), ESP-NOW, lots of GPIO/ADC | ❓ which models? (classic ESP32 / WROOM / S2 / C3 …) — classic ESP32 adds BT audio that the S3 lacks |
| **Raspberry Pi Zero W** | 2 | Full Linux, Wi-Fi + BT, **USB gadget mode** (HID/Ethernet/serial), can run servers | ARM11 single-core — great hub/bridge, too weak for real-time Whisper/LLM |
| **Arduino** | 2 | MCU + USB serial, analog in, lots of GPIO | ❓ which models? Uno/Nano (no native USB HID — needs reflash) vs Leonardo/Micro/Pro-Micro (native USB HID) |
| **Raspberry Pi Pico W** | 1 | RP2040 dual-core, Wi-Fi (CYW43), **native USB HID**, PIO | Excellent reliable USB macropad (KMK/CircuitPython or C) |

## Common interconnects available across the fleet

- **ESP-NOW** — ultra-low-latency, router-free peer messaging **between ESP chips only** (ESP32 ↔ ESP32/AtomS3). Pico W and Pi don't speak it.
- **Wi-Fi (the universal bus)** — everything here has it. MQTT / HTTP / UDP works across *all* devices, so it's the natural common protocol for a mixed fleet.
- **BLE** — AtomS3, ESP32s, Pico W, Pi Zero W can all do it.
- **Bluetooth Classic** — classic ESP32 + Pi Zero W only (audio, BT-HID).
- **USB** — AtomS3 / Pico W (native HID device), Pi Zero W (HID *gadget*), Leonardo-class Arduinos (native HID). Uno/Nano = serial only without a reflash.

## To add later

- Exact ESP32 model(s) and how many.
- Exact Arduino model(s).
- Any sensors, screens, encoders, Grove units, breadboards, batteries.
