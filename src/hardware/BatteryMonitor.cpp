#include "hardware/BatteryMonitor.h"

#include "AppConfig.h"
#include "hardware/BatteryMath.h"

namespace assclets {

void BatteryMonitor::begin() {
  analogSetPinAttenuation(config::kBatteryAdcPin, ADC_11db);
}

bool BatteryMonitor::sample() {
#if !ASSCLETS_BATTERY_GAUGE
  const int nextPercent = -1;
#else
  uint32_t sum = 0;
  for (int i = 0; i < 8; ++i) {
    sum += analogReadMilliVolts(config::kBatteryAdcPin);
  }

  const int batteryMv = static_cast<int>((sum / 8) * config::kBatteryDivider);
  const int nextPercent = mapBatteryMvToPercent(
      batteryMv, config::kBatteryPresentMv, config::kBatteryEmptyMv, config::kBatteryFullMv);
#endif

  if (nextPercent == percent_) {
    return false;
  }

  percent_ = nextPercent;
  return true;
}

}  // namespace assclets
