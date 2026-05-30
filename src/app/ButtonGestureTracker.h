#pragma once

#include <stdint.h>

namespace assclets {

enum class GestureMode : uint8_t {
  Idle,
  PushToTalk,
  Locked,
};

enum class GestureAction : uint8_t {
  None,
  EnterPushToTalk,
  EnterLocked,
  EnterIdle,
};

class ButtonGestureTracker {
 public:
  GestureAction update(GestureMode mode, uint32_t now, bool wasPressed, bool isPressed, bool wasReleased,
                       uint32_t holdMs, uint32_t doubleTapMs);

  void clearPendingTap() {
    pendingTap_ = false;
  }

 private:
  bool pendingTap_ = false;
  uint32_t pressStartMs_ = 0;
  uint32_t lastTapMs_ = 0;
};

}  // namespace assclets
