#include "hardware/DisplayView.h"

#include <math.h>

#include "AppConfig.h"

namespace assclets {
namespace {

constexpr float kTwoPi = 6.28318530718f;

}  // namespace

void DisplayView::begin() {
  activeBrightness_ = M5.Display.getBrightness();
  canvas_.setColorDepth(16);
  canvas_.createSprite(M5.Display.width(), M5.Display.height());
  canvas_.setTextWrap(false);
  initPalette();
}

void DisplayView::setDimmed(bool dimmed) {
  if (dimmed_ == dimmed) {
    return;
  }

  dimmed_ = dimmed;
  M5.Display.setBrightness(dimmed_ ? config::kDisplayDimBrightness : activeBrightness_);
}

void DisplayView::initPalette() {
  colors_.bg = canvas_.color565(16, 16, 20);
  colors_.bgRecording = canvas_.color565(46, 10, 10);
  colors_.title = canvas_.color565(244, 244, 248);
  colors_.subtitle = canvas_.color565(150, 150, 164);
  colors_.green = canvas_.color565(46, 213, 160);
  colors_.red = canvas_.color565(255, 69, 58);
  colors_.blue = canvas_.color565(90, 162, 255);
}

void DisplayView::showDisconnected() {
  renderScreen("PAIR", "bluetooth", colors_.blue, colors_.bg, 0, 0);
}

void DisplayView::showIdle(uint32_t lastClipMs) {
  const String subtitle =
      lastClipMs ? ("last " + formatDuration(lastClipMs)) : String("hold to talk");
  renderScreen("READY", subtitle.c_str(), colors_.green, colors_.bg, 0, 0);
}

void DisplayView::showPushToTalk(uint32_t elapsedMs) {
  const String title = formatDuration(elapsedMs);
  renderScreen(title.c_str(), "listening", colors_.red, colors_.bg, 1, pulseRadius());
}

void DisplayView::showLocked(uint32_t elapsedMs) {
  const String title = formatDuration(elapsedMs);
  renderScreen(title.c_str(), "tap to stop", colors_.red, colors_.bgRecording, 2, pulseRadius());
}

void DisplayView::showReturnSent() {
  renderScreen("RETURN", "sent", colors_.blue, colors_.bg, 0, 0);
}

void DisplayView::showUpdating(unsigned int progress, unsigned int total) {
  const int cx = canvas_.width() / 2;
  const int pct = total ? static_cast<int>((progress * 100UL) / total) : 0;

  canvas_.fillSprite(colors_.bg);
  canvas_.setTextDatum(middle_center);
  canvas_.setTextColor(colors_.title);
  canvas_.setFont(&fonts::Orbitron_Light_24);
  canvas_.drawString("OTA", cx, 36);

  canvas_.setFont(&fonts::FreeSans9pt7b);
  canvas_.setTextColor(colors_.subtitle);
  canvas_.drawString(String(pct) + "%", cx, 64);

  const int barX = 14;
  const int barY = 86;
  const int barWidth = canvas_.width() - 28;
  const int barHeight = 12;
  canvas_.drawRoundRect(barX, barY, barWidth, barHeight, 4, colors_.subtitle);
  canvas_.fillRoundRect(barX + 2, barY + 2, (barWidth - 4) * pct / 100, barHeight - 4, 2,
                        colors_.green);
  canvas_.pushSprite(0, 0);
}

void DisplayView::renderScreen(const char* title, const char* subtitle, uint16_t accent,
                               uint16_t bg, int iconMode, int iconRadius) {
  const int cx = canvas_.width() / 2;
  const int iconY = 40;
  canvas_.fillSprite(bg);

  if (iconMode == 0) {
    canvas_.drawCircle(cx, iconY, 19, accent);
    canvas_.drawCircle(cx, iconY, 18, accent);
    canvas_.fillCircle(cx, iconY, 4, accent);
  } else {
    canvas_.fillCircle(cx, iconY, iconRadius, accent);
    canvas_.drawCircle(cx, iconY, iconRadius + 6, accent);
    if (iconMode == 2) {
      canvas_.drawCircle(cx, iconY, iconRadius + 10, accent);
    }
  }

  canvas_.setTextDatum(middle_center);
  canvas_.setTextColor(colors_.title);
  canvas_.setFont(&fonts::Orbitron_Light_24);
  if (canvas_.textWidth(title) > canvas_.width() - 8) {
    canvas_.setFont(&fonts::FreeSansBold12pt7b);
  }
  canvas_.drawString(title, cx, 86);

  canvas_.setFont(&fonts::FreeSans9pt7b);
  canvas_.setTextColor(colors_.subtitle);
  canvas_.drawString(subtitle, cx, 110);

  renderStatusBadges();
  canvas_.pushSprite(0, 0);
}

void DisplayView::renderStatusBadges() {
  if (batteryPercent_ >= 0) {
    const int badgeWidth = 18;
    const int badgeHeight = 10;
    const int badgeX = 88;
    const int badgeY = 4;
    const uint16_t fill = batteryPercent_ > 20 ? colors_.green : colors_.red;

    canvas_.drawRect(badgeX, badgeY, badgeWidth, badgeHeight, colors_.subtitle);
    canvas_.fillRect(badgeX + badgeWidth, badgeY + 3, 2, badgeHeight - 6, colors_.subtitle);
    canvas_.fillRect(badgeX + 2, badgeY + 2, (badgeWidth - 4) * batteryPercent_ / 100,
                     badgeHeight - 4, fill);
    canvas_.setFont(&fonts::Font0);
    canvas_.setTextDatum(middle_right);
    canvas_.setTextColor(colors_.subtitle);
    canvas_.drawString((String(batteryPercent_) + "%").c_str(), badgeX - 4,
                       badgeY + badgeHeight / 2);
  }

  canvas_.fillCircle(canvas_.width() - 9, 9, 3, otaReady_ ? colors_.green : colors_.subtitle);
}

String DisplayView::formatDuration(uint32_t ms) {
  const uint32_t seconds = ms / 1000;
  return String(seconds / 60) + ":" + (seconds % 60 < 10 ? "0" : "") + String(seconds % 60);
}

int DisplayView::pulseRadius() {
  const float phase = (millis() % 1400) / 1400.0f;
  const float eased = (sinf(phase * kTwoPi) + 1.0f) * 0.5f;
  return 16 + static_cast<int>(eased * 8.0f);
}

}  // namespace assclets
