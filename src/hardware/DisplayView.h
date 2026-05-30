#pragma once

#include <Arduino.h>
#include <M5Unified.h>

namespace assclets {

class DisplayView {
 public:
  void begin();

  void setOtaReady(bool ready) {
    otaReady_ = ready;
  }
  void setBatteryPercent(int percent) {
    batteryPercent_ = percent;
  }
  void setDimmed(bool dimmed);

  void showDisconnected();
  void showIdle(uint32_t lastClipMs);
  void showPushToTalk(uint32_t elapsedMs);
  void showLocked(uint32_t elapsedMs);
  void showUpdating(unsigned int progress, unsigned int total);

 private:
  struct Palette {
    uint16_t bg;
    uint16_t bgRecording;
    uint16_t title;
    uint16_t subtitle;
    uint16_t green;
    uint16_t red;
    uint16_t blue;
  };

  M5Canvas canvas_{&M5.Display};
  Palette colors_{};
  bool otaReady_ = false;
  bool dimmed_ = false;
  uint8_t activeBrightness_ = 127;
  int batteryPercent_ = -1;

  void initPalette();
  void renderScreen(const char* title, const char* subtitle, uint16_t accent, uint16_t bg,
                    int iconMode, int iconRadius);
  void renderStatusBadges();

  static String formatDuration(uint32_t ms);
  static int pulseRadius();
};

}  // namespace assclets
