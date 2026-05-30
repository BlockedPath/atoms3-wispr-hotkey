#include <iostream>

#include "hardware/BatteryMath.h"

namespace {

int failures = 0;

void expectEq(int actual, int expected, const char* message) {
  if (actual != expected) {
    ++failures;
    std::cerr << "FAIL: " << message << " (expected " << expected << ", got " << actual << ")\n";
  }
}

void testBatteryMapping() {
  expectEq(assclets::mapBatteryMvToPercent(4400, 4500, 6000, 9000), -1,
           "below presence threshold should hide battery");
  expectEq(assclets::mapBatteryMvToPercent(4500, 4500, 6000, 9000), 0,
           "presence threshold should clamp to 0%");
  expectEq(assclets::mapBatteryMvToPercent(6000, 4500, 6000, 9000), 0, "empty voltage should be 0%");
  expectEq(assclets::mapBatteryMvToPercent(7500, 4500, 6000, 9000), 50,
           "midpoint voltage should be 50%");
  expectEq(assclets::mapBatteryMvToPercent(9000, 4500, 6000, 9000), 100,
           "full voltage should be 100%");
  expectEq(assclets::mapBatteryMvToPercent(9800, 4500, 6000, 9000), 100,
           "above full voltage should clamp to 100%");
  expectEq(assclets::mapBatteryMvToPercent(7500, 4500, 9000, 6000), -1,
           "invalid range should return hidden state");
}

}  // namespace

int main() {
  testBatteryMapping();

  if (failures == 0) {
    std::cout << "All battery math tests passed\n";
    return 0;
  }

  std::cerr << failures << " battery math tests failed\n";
  return 1;
}
