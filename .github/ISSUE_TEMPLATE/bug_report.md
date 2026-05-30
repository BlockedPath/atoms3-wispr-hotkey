---
name: Bug report
about: Report firmware behavior that is broken or unexpected
title: "[Bug] "
labels: bug
assignees: ""
---

## What happened?

Describe the broken behavior and what you expected instead.

## Hardware

- Board: M5Stack AtomS3
- Power source: USB / external supply
- Optional battery gauge enabled: yes / no
- Firmware commit:

## Steps to reproduce

1.
2.
3.

## Observed device state

- Display text:
- BLE connection state:
- Wispr Flow shortcut configured as Ctrl+Option: yes / no
- OTA dot state, if relevant:

## Logs or screenshots

Paste serial logs from `pio device monitor` if available.

## Validation tried

- [ ] Rebuilt with `pio run`
- [ ] Uploaded over USB with `pio run -t upload`
- [ ] Uploaded over OTA
- [ ] Re-paired the BLE keyboard in macOS
