#include "app/ButtonGestureTracker.h"

namespace assclets {

GestureAction ButtonGestureTracker::update(GestureMode mode, uint32_t now, bool wasPressed,
                                           bool isPressed, bool wasReleased, uint32_t holdMs,
                                           uint32_t doubleTapMs) {
  if (wasPressed) {
    pressStartMs_ = now;
  }

  if (mode == GestureMode::Idle && isPressed && now - pressStartMs_ >= holdMs) {
    return GestureAction::EnterPushToTalk;
  }

  if (wasReleased) {
    const uint32_t pressDuration = now - pressStartMs_;
    if (mode == GestureMode::PushToTalk || mode == GestureMode::Locked) {
      pendingTap_ = false;
      return GestureAction::EnterIdle;
    }

    if (pressDuration < holdMs) {
      if (pendingTap_ && now - lastTapMs_ <= doubleTapMs) {
        pendingTap_ = false;
        return GestureAction::EnterLocked;
      }

      pendingTap_ = true;
      lastTapMs_ = now;
    }
  }

  if (pendingTap_ && now - lastTapMs_ > doubleTapMs) {
    pendingTap_ = false;
  }

  return GestureAction::None;
}

}  // namespace assclets
