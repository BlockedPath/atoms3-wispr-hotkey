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
  markDisplayUsed(millis());
  display_.showDisconnected();
}

void HotkeyController::update() {
  M5.update();

  const uint32_t now = millis();
  const bool otaReadyChanged = ota_.update(now);
  if (ota_.consumeDisplayActivity()) {
    markDisplayUsed(now);
  }
  if (otaReadyChanged) {
    redrawStaticState();
  }
  sampleBatteryIfDue(now);

  const bool connected = keyboard_.isConnected();
  handleConnection(connected, now);

  if (!connected) {
    updateDisplayDimming(now);
    delay(20);
    return;
  }

  handleButton(now);
  finishTapSequence(now);
  finishReturnConfirmation(now);
  animateRecording(now);
  updateDisplayDimming(now);
  delay(5);
}

void HotkeyController::handleConnection(bool connected, uint32_t now) {
  if (connected && !wasConnected_) {
    enterIdle();
  } else if (!connected && wasConnected_) {
    releaseKeys();
    mode_ = Mode::Idle;
    resetTapSequence();
    returnConfirming_ = false;
    markDisplayUsed(now);
    display_.showDisconnected();
  }

  wasConnected_ = connected;
}

void HotkeyController::handleButton(uint32_t now) {
  if (M5.BtnA.wasPressed()) {
    markDisplayUsed(now);
    if (pendingTap_ && now - lastTapMs_ > config::kTapSequenceMs) {
      resetTapSequence();
    }
    pressStartMs_ = now;
  }

  if (mode_ == Mode::Idle && M5.BtnA.isPressed() && now - pressStartMs_ >= config::kHoldMs) {
    resetTapSequence();
    enterPushToTalk();
  }

  if (M5.BtnA.wasReleased()) {
    markDisplayUsed(now);
    const uint32_t pressDuration = now - pressStartMs_;
    if (mode_ == Mode::PushToTalk) {
      enterIdle();
    } else if (mode_ == Mode::Locked) {
      enterIdle();
    } else if (pressDuration < config::kHoldMs) {
      recordShortTap(now);
    }
  }
}

void HotkeyController::recordShortTap(uint32_t now) {
  if (!pendingTap_ || now - lastTapMs_ > config::kTapSequenceMs) {
    tapCount_ = 0;
  }

  pendingTap_ = true;
  lastTapMs_ = now;
  ++tapCount_;

  if (tapCount_ >= config::kReturnTapCount) {
    sendReturnKey();
  }
}

void HotkeyController::finishTapSequence(uint32_t now) {
  if (!pendingTap_ || M5.BtnA.isPressed() || now - lastTapMs_ <= config::kTapSequenceMs) {
    return;
  }

  const uint8_t completedTaps = tapCount_;
  resetTapSequence();
  if (completedTaps == 2) {
    enterLocked();
  }
}

void HotkeyController::resetTapSequence() {
  pendingTap_ = false;
  tapCount_ = 0;
}

void HotkeyController::finishReturnConfirmation(uint32_t now) {
  if (!returnConfirming_ || now - returnSentMs_ < config::kReturnConfirmMs) {
    return;
  }

  returnConfirming_ = false;
  redrawStaticState();
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
  if (mode_ != Mode::Idle || ota_.updating() || returnConfirming_) {
    return;
  }

  markDisplayUsed(millis());
  if (keyboard_.isConnected()) {
    display_.showIdle(lastClipMs_);
  } else {
    display_.showDisconnected();
  }
}

void HotkeyController::markDisplayUsed(uint32_t now) {
  lastDisplayUseMs_ = now;
  setDisplayDimmed(false);
}

void HotkeyController::setDisplayDimmed(bool dimmed) {
  if (displayDimmed_ == dimmed) {
    return;
  }

  displayDimmed_ = dimmed;
  display_.setDimmed(displayDimmed_);
}

void HotkeyController::updateDisplayDimming(uint32_t now) {
  if (mode_ != Mode::Idle) {
    setDisplayDimmed(false);
    return;
  }

  if (ota_.updating()) {
    markDisplayUsed(now);
    return;
  }

  if (!displayDimmed_ && now - lastDisplayUseMs_ >= config::kDisplayIdleDimMs) {
    setDisplayDimmed(true);
  }
}

void HotkeyController::enterIdle() {
  const uint32_t now = millis();
  if (mode_ == Mode::PushToTalk || mode_ == Mode::Locked) {
    lastClipMs_ = now - recordingStartMs_;
  }

  releaseKeys();
  mode_ = Mode::Idle;
  resetTapSequence();
  returnConfirming_ = false;
  markDisplayUsed(now);
  display_.showIdle(lastClipMs_);
}

void HotkeyController::enterPushToTalk() {
  const uint32_t now = millis();
  recordingStartMs_ = now;
  resetTapSequence();
  returnConfirming_ = false;
  holdWisprCombo();
  mode_ = Mode::PushToTalk;
  markDisplayUsed(now);
  display_.showPushToTalk(0);
}

void HotkeyController::enterLocked() {
  const uint32_t now = millis();
  recordingStartMs_ = now;
  returnConfirming_ = false;
  holdWisprCombo();
  mode_ = Mode::Locked;
  resetTapSequence();
  markDisplayUsed(now);
  display_.showLocked(0);
}

void HotkeyController::holdWisprCombo() {
  keyboard_.press(KEY_LEFT_CTRL);
  keyboard_.press(KEY_LEFT_ALT);
}

void HotkeyController::sendReturnKey() {
  const uint32_t now = millis();
  resetTapSequence();
  releaseKeys();
  keyboard_.write(KEY_RETURN);
  returnSentMs_ = now;
  returnConfirming_ = true;
  markDisplayUsed(now);
  display_.showReturnSent();
}

void HotkeyController::releaseKeys() {
  keyboard_.releaseAll();
}

}  // namespace assclets
