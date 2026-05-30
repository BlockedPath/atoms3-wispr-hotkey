#include "app/HotkeyController.h"

#include <M5Unified.h>

namespace assclets {

void HotkeyController::begin() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(0);

  display_.begin();
  battery_.begin();

  keyboard_.begin();
  ota_.begin();

  battery_.sample();
  display_.setBatteryPercent(battery_.percent());
  display_.setOtaReady(ota_.ready());
  display_.showDisconnected();
}

void HotkeyController::update() {
  M5.update();

  const uint32_t now = millis();
  if (ota_.update(now)) {
    redrawStaticState();
  }
  sampleBatteryIfDue(now);

  const bool connected = keyboard_.isConnected();
  handleConnection(connected);

  if (!connected) {
    delay(20);
    return;
  }

  handleButton(now);
  animateRecording(now);
  delay(5);
}

void HotkeyController::handleConnection(bool connected) {
  if (connected && !wasConnected_) {
    enterIdle();
  } else if (!connected && wasConnected_) {
    releaseKeys();
    mode_ = Mode::Idle;
    gesture_.clearPendingTap();
    display_.showDisconnected();
  }

  wasConnected_ = connected;
}

void HotkeyController::handleButton(uint32_t now) {
  GestureMode gestureMode = GestureMode::Idle;
  switch (mode_) {
    case Mode::Idle:
      gestureMode = GestureMode::Idle;
      break;
    case Mode::PushToTalk:
      gestureMode = GestureMode::PushToTalk;
      break;
    case Mode::Locked:
      gestureMode = GestureMode::Locked;
      break;
  }

  const GestureAction action =
      gesture_.update(gestureMode, now, M5.BtnA.wasPressed(), M5.BtnA.isPressed(),
                      M5.BtnA.wasReleased(), config::kHoldMs, config::kDoubleTapMs);

  switch (action) {
    case GestureAction::EnterPushToTalk:
      enterPushToTalk();
      break;
    case GestureAction::EnterLocked:
      enterLocked();
      break;
    case GestureAction::EnterIdle:
      enterIdle();
      break;
    case GestureAction::None:
      break;
  }
}

void HotkeyController::animateRecording(uint32_t now) {
  if (mode_ != Mode::PushToTalk && mode_ != Mode::Locked) {
    return;
  }

  if (now - lastFrameMs_ < config::kRecordingFrameMs) {
    return;
  }

  lastFrameMs_ = now;
  const uint32_t elapsed = now - recordingStartMs_;
  if (mode_ == Mode::PushToTalk) {
    display_.showPushToTalk(elapsed);
  } else {
    display_.showLocked(elapsed);
  }
}

void HotkeyController::sampleBatteryIfDue(uint32_t now) {
  if (now - lastBatterySampleMs_ < config::kBatterySampleMs) {
    return;
  }

  lastBatterySampleMs_ = now;
  if (!battery_.sample()) {
    return;
  }

  display_.setBatteryPercent(battery_.percent());
  redrawStaticState();
}

void HotkeyController::redrawStaticState() {
  if (mode_ != Mode::Idle) {
    return;
  }

  if (keyboard_.isConnected()) {
    display_.showIdle(lastClipMs_);
  } else {
    display_.showDisconnected();
  }
}

void HotkeyController::enterIdle() {
  if (mode_ == Mode::PushToTalk || mode_ == Mode::Locked) {
    lastClipMs_ = millis() - recordingStartMs_;
  }

  releaseKeys();
  mode_ = Mode::Idle;
  gesture_.clearPendingTap();
  display_.showIdle(lastClipMs_);
}

void HotkeyController::enterPushToTalk() {
  recordingStartMs_ = millis();
  holdWisprCombo();
  mode_ = Mode::PushToTalk;
  display_.showPushToTalk(0);
}

void HotkeyController::enterLocked() {
  recordingStartMs_ = millis();
  holdWisprCombo();
  mode_ = Mode::Locked;
  gesture_.clearPendingTap();
  display_.showLocked(0);
}

void HotkeyController::holdWisprCombo() {
  keyboard_.press(KEY_LEFT_CTRL);
  keyboard_.press(KEY_LEFT_ALT);
}

void HotkeyController::releaseKeys() {
  keyboard_.releaseAll();
}

}  // namespace assclets
