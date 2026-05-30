#pragma once

#include <Arduino.h>

#include "hardware/DisplayView.h"

namespace assclets {

class OtaService {
 public:
  explicit OtaService(DisplayView& display) : display_(display) {}

  void begin();
  bool update(uint32_t now);

  bool ready() const {
    return ready_;
  }

 private:
  DisplayView& display_;
  bool ready_ = false;
  uint32_t lastWifiTryMs_ = 0;

  bool setReady(bool ready);
};

}  // namespace assclets
