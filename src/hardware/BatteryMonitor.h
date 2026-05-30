#pragma once

#include <Arduino.h>

namespace assclets {

class BatteryMonitor {
 public:
  void begin();
  bool sample();

  int percent() const {
    return percent_;
  }

 private:
  int percent_ = -1;
};

}  // namespace assclets
