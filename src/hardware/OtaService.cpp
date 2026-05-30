#include "hardware/OtaService.h"

#include <ArduinoOTA.h>
#include <WiFi.h>

#include "AppConfig.h"
#include "secrets.h"

#ifndef OTA_PASSWORD
#error "OTA_PASSWORD must be set to a non-empty value in src/secrets.h"
#endif

#ifndef __INTELLISENSE__
static_assert(sizeof(OTA_PASSWORD) > 1,
              "OTA_PASSWORD must be set to a non-empty value in src/secrets.h");
#endif

namespace assclets {

void OtaService::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  lastWifiTryMs_ = millis();

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([this]() {
    updating_ = true;
    displayActivity_ = true;
    display_.showUpdating(0, 100);
  });
  ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
    updating_ = true;
    displayActivity_ = true;
    display_.showUpdating(progress, total);
  });
  ArduinoOTA.onEnd([this]() {
    updating_ = false;
    displayActivity_ = true;
    display_.showUpdating(100, 100);
  });
  ArduinoOTA.onError([this](ota_error_t) {
    updating_ = false;
    displayActivity_ = true;
    display_.showDisconnected();
  });
}

bool OtaService::update(uint32_t now) {
  bool changed = false;

  if (WiFi.status() == WL_CONNECTED) {
    if (!ready_) {
      ArduinoOTA.begin();
      changed = setReady(true);
    }
    ArduinoOTA.handle();
    return changed;
  }

  changed = setReady(false);
  if (now - lastWifiTryMs_ > config::kWifiRetryMs) {
    WiFi.reconnect();
    lastWifiTryMs_ = now;
  }
  return changed;
}

bool OtaService::consumeDisplayActivity() {
  const bool activity = displayActivity_;
  displayActivity_ = false;
  return activity;
}

bool OtaService::setReady(bool ready) {
  if (ready_ == ready) {
    return false;
  }

  ready_ = ready;
  display_.setOtaReady(ready_);
  return true;
}

}  // namespace assclets
