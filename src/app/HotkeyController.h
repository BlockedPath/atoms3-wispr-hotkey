#pragma once

#include <Arduino.h>
#include <BleKeyboard.h>

#include "AppConfig.h"
#include "hardware/BatteryMonitor.h"
#include "hardware/DisplayView.h"
#include "hardware/OtaService.h"

namespace assclets {

class HotkeyController {
 public:
  void begin();
  void update();

 private:
  enum class Mode : uint8_t {
    Idle,
    PushToTalk,
    Locked,
  };

  DisplayView display_;
  BatteryMonitor battery_;
  OtaService ota_{display_};
  BleKeyboard keyboard_{config::kBleDeviceName, config::kBleManufacturer, config::kBleBatteryLevel};

  Mode mode_ = Mode::Idle;
  bool wasConnected_ = false;
  bool pendingTap_ = false;
  bool displayDimmed_ = false;

  uint32_t pressStartMs_ = 0;
  uint32_t lastTapMs_ = 0;
  uint32_t recordingStartMs_ = 0;
  uint32_t lastClipMs_ = 0;
  uint32_t lastBatterySampleMs_ = 0;
  uint32_t lastFrameMs_ = 0;
  uint32_t lastDisplayUseMs_ = 0;

  void handleConnection(bool connected, uint32_t now);
  void handleButton(uint32_t now);
  void animateRecording(uint32_t now);
  void sampleBatteryIfDue(uint32_t now);
  void redrawStaticState();
  void markDisplayUsed(uint32_t now);
  void setDisplayDimmed(bool dimmed);
  void updateDisplayDimming(uint32_t now);

  void enterIdle();
  void enterPushToTalk();
  void enterLocked();

  void holdWisprCombo();
  void releaseKeys();
};

}  // namespace assclets
