#pragma once

#include <algorithm>

namespace assclets {

inline int mapBatteryMvToPercent(int batteryMv, int presentMv, int emptyMv, int fullMv) {
  if (batteryMv < presentMv) {
    return -1;
  }

  if (fullMv <= emptyMv) {
    return 100;
  }

  const int scaled = (batteryMv - emptyMv) * 100 / (fullMv - emptyMv);
  return std::clamp(scaled, 0, 100);
}

}  // namespace assclets
