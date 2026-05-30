#include <iostream>

#include "app/ButtonGestureTracker.h"

namespace {

using assclets::ButtonGestureTracker;
using assclets::GestureAction;
using assclets::GestureMode;

constexpr uint32_t kHoldMs = 250;
constexpr uint32_t kDoubleTapMs = 400;

int failures = 0;

void expect(bool condition, const char* message) {
  if (!condition) {
    ++failures;
    std::cerr << "FAIL: " << message << "\n";
  }
}

void testHoldStartsPushToTalk() {
  ButtonGestureTracker tracker;
  expect(tracker.update(GestureMode::Idle, 0, true, true, false, kHoldMs, kDoubleTapMs) ==
             GestureAction::None,
         "press edge should not immediately start recording");
  expect(tracker.update(GestureMode::Idle, kHoldMs - 1, false, true, false, kHoldMs,
                        kDoubleTapMs) == GestureAction::None,
         "hold threshold should not trigger early");
  expect(tracker.update(GestureMode::Idle, kHoldMs, false, true, false, kHoldMs, kDoubleTapMs) ==
             GestureAction::EnterPushToTalk,
         "hold threshold should enter push-to-talk");
}

void testReleaseFromPushToTalkReturnsIdle() {
  ButtonGestureTracker tracker;
  expect(tracker.update(GestureMode::PushToTalk, 300, false, false, true, kHoldMs, kDoubleTapMs) ==
             GestureAction::EnterIdle,
         "release in push-to-talk should return idle");
}

void testDoubleTapEntersLocked() {
  ButtonGestureTracker tracker;
  tracker.update(GestureMode::Idle, 0, true, true, false, kHoldMs, kDoubleTapMs);
  expect(tracker.update(GestureMode::Idle, 120, false, false, true, kHoldMs, kDoubleTapMs) ==
             GestureAction::None,
         "first tap should set pending state only");

  tracker.update(GestureMode::Idle, 220, true, true, false, kHoldMs, kDoubleTapMs);
  expect(tracker.update(GestureMode::Idle, 280, false, false, true, kHoldMs, kDoubleTapMs) ==
             GestureAction::EnterLocked,
         "second quick tap should enter locked mode");
}

void testPendingTapTimesOut() {
  ButtonGestureTracker tracker;
  tracker.update(GestureMode::Idle, 0, true, true, false, kHoldMs, kDoubleTapMs);
  tracker.update(GestureMode::Idle, 100, false, false, true, kHoldMs, kDoubleTapMs);

  expect(
      tracker.update(GestureMode::Idle, 600, false, false, false, kHoldMs, kDoubleTapMs) ==
          GestureAction::None,
      "timeout tick should not emit action");

  tracker.update(GestureMode::Idle, 700, true, true, false, kHoldMs, kDoubleTapMs);
  expect(tracker.update(GestureMode::Idle, 760, false, false, true, kHoldMs, kDoubleTapMs) ==
             GestureAction::None,
         "tap after timeout should be treated as a new first tap");
}

void testReleaseFromLockedReturnsIdle() {
  ButtonGestureTracker tracker;
  expect(tracker.update(GestureMode::Locked, 900, false, false, true, kHoldMs, kDoubleTapMs) ==
             GestureAction::EnterIdle,
         "release in locked mode should return idle");
}

}  // namespace

int main() {
  testHoldStartsPushToTalk();
  testReleaseFromPushToTalkReturnsIdle();
  testDoubleTapEntersLocked();
  testPendingTapTimesOut();
  testReleaseFromLockedReturnsIdle();

  if (failures == 0) {
    std::cout << "All button gesture tests passed\n";
    return 0;
  }

  std::cerr << failures << " button gesture tests failed\n";
  return 1;
}
