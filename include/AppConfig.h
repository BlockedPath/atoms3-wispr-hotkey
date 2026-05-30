#pragma once

#include <Arduino.h>

// Optional ADC battery gauge. Enable with:
//   build_flags = -DASSCLETS_BATTERY_GAUGE=1
// after wiring the 9V divider described in README.md.
#ifndef ASSCLETS_BATTERY_GAUGE
#define ASSCLETS_BATTERY_GAUGE 0
#endif

namespace assclets {
namespace config {

constexpr const char* kBleDeviceName = "AtomS3 Hotkey";
constexpr const char* kBleManufacturer = "M5Stack";
constexpr uint8_t kBleBatteryLevel = 100;

constexpr uint32_t kHoldMs = 250;
constexpr uint32_t kDoubleTapMs = 400;
constexpr uint32_t kRecordingFrameMs = 33;
constexpr uint32_t kBatterySampleMs = 5000;
constexpr uint32_t kWifiRetryMs = 10000;

constexpr int kBatteryAdcPin = 8;        // G8 / GPIO8 (ADC1_CH7)
constexpr float kBatteryDivider = 4.3f;  // (R1+R2)/R2 for R1=330k, R2=100k
constexpr int kBatteryFullMv = 9000;     // ~100% (alkaline 9V under light load)
constexpr int kBatteryEmptyMv = 6000;    // ~0%   (effectively flat)
constexpr int kBatteryPresentMv = 4500;  // below this = no pack attached (on USB)

}  // namespace config
}  // namespace assclets
