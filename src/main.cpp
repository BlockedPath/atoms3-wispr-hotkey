// AtomS3 -> Wispr Flow push-to-talk button (BLE HID keyboard)
//
// Gestures (the whole front LCD is a button on GPIO41):
//   * Press & hold      -> push-to-talk: holds Ctrl+Option while held (REC).
//   * Double-tap        -> LOCK recording on hands-free (stays held, LOCKED).
//   * Tap while locked  -> stop & transcribe (back to READY).
//
// Set Wispr Flow's push-to-talk shortcut to Ctrl+Option. Wispr's PTT requires a
// modifier combo (lone keys / bare F-keys are blocked); Ctrl+Option is Wispr's
// own recommended combo and works across all keyboards.
//
// The device can't read Wispr's state, so the firmware OWNS the latch: in
// LOCKED mode it deliberately keeps the combo held down (so Wispr keeps
// recording) and the screen reflects that until you tap to stop.

#include <M5Unified.h>
#include <BleKeyboard.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <math.h>
#include "secrets.h"           // Wi-Fi / OTA credentials (git-ignored)

// Name shown when pairing in macOS Bluetooth settings -- rename freely.
BleKeyboard bleKeyboard("AtomS3 Hotkey", "M5Stack", 100);

// Wi-Fi OTA: lets us flash wirelessly to OTA_HOSTNAME.local. Connection is
// non-blocking so the keyboard works even if Wi-Fi is down.
bool     otaReady    = false;  // true once Wi-Fi is up and OTA is listening
uint32_t lastWifiTry = 0;

// ---- recording / gesture state -------------------------------------------
enum Mode { MODE_IDLE, MODE_PTT, MODE_LOCKED };
Mode mode = MODE_IDLE;
bool wasConnected = false;

// Gesture timing. We run our own timers (M5Unified couples its hold and
// double-click thresholds to one value, which we don't want).
const uint32_t HOLD_MS = 250;   // press longer than this = push-to-talk hold
const uint32_t DTAP_MS = 400;   // max gap between taps to count as a double-tap
uint32_t pressStart = 0;
uint32_t lastTapMs  = 0;
bool     pendingTap = false;

// ---- LCD UI (128x128 IPS), drawn flicker-free via an off-screen canvas -----
M5Canvas canvas(&M5.Display);
uint16_t C_BG, C_BG_REC, C_TITLE, C_SUB, C_GREEN, C_RED, C_BLUE;

static void initColors() {
  C_BG     = canvas.color565( 16,  16,  20);   // near-black charcoal (idle/PTT)
  C_BG_REC = canvas.color565( 46,  10,  10);   // dark-red wash (locked recording)
  C_TITLE  = canvas.color565(244, 244, 248);   // off-white
  C_SUB    = canvas.color565(150, 150, 164);   // grey
  C_GREEN  = canvas.color565( 46, 213, 160);   // mint (ready)
  C_RED    = canvas.color565(255,  69,  58);   // record red
  C_BLUE   = canvas.color565( 90, 162, 255);   // pairing blue
}

// iconMode: 0 = hollow ring + centre dot | 1 = filled dot + ring
//           2 = filled dot + double ring (locked / sustained)
static void renderScreen(const char* title, const char* subtitle, uint16_t accent,
                         uint16_t bg, int iconMode, int iconR) {
  const int cx = canvas.width() / 2;
  const int iconY = 40;
  canvas.fillSprite(bg);

  if (iconMode == 0) {
    canvas.drawCircle(cx, iconY, 19, accent);
    canvas.drawCircle(cx, iconY, 18, accent);
    canvas.fillCircle(cx, iconY, 4, accent);
  } else {
    canvas.fillCircle(cx, iconY, iconR, accent);
    canvas.drawCircle(cx, iconY, iconR + 6, accent);
    if (iconMode == 2) canvas.drawCircle(cx, iconY, iconR + 10, accent);
  }

  // Title -- Orbitron for a modern look, auto-shrunk so it can never overflow.
  canvas.setTextDatum(middle_center);
  canvas.setTextColor(C_TITLE);
  canvas.setFont(&fonts::Orbitron_Light_24);
  if (canvas.textWidth(title) > canvas.width() - 8) {
    canvas.setFont(&fonts::FreeSansBold12pt7b);
  }
  canvas.drawString(title, cx, 86);

  // Subtitle -- small proportional font, comfortably within the screen.
  canvas.setFont(&fonts::FreeSans9pt7b);
  canvas.setTextColor(C_SUB);
  canvas.drawString(subtitle, cx, 110);

  // Tiny corner dot: green when Wi-Fi OTA is ready, dim otherwise.
  canvas.fillCircle(canvas.width() - 9, 9, 3, otaReady ? C_GREEN : C_SUB);

  canvas.pushSprite(0, 0);
}

// Full-screen progress bar shown while an OTA update is downloading.
static void drawUpdating(unsigned int progress, unsigned int total) {
  const int cx = canvas.width() / 2;
  const int pct = total ? (int)((progress * 100UL) / total) : 0;
  canvas.fillSprite(C_BG);
  canvas.setTextDatum(middle_center);
  canvas.setTextColor(C_TITLE);
  canvas.setFont(&fonts::Orbitron_Light_24);
  canvas.drawString("OTA", cx, 36);
  canvas.setFont(&fonts::FreeSans9pt7b);
  canvas.setTextColor(C_SUB);
  canvas.drawString(String(pct) + "%", cx, 64);
  // progress bar
  const int bx = 14, by = 86, bw = canvas.width() - 28, bh = 12;
  canvas.drawRoundRect(bx, by, bw, bh, 4, C_SUB);
  canvas.fillRoundRect(bx + 2, by + 2, (bw - 4) * pct / 100, bh - 4, 2, C_GREEN);
  canvas.pushSprite(0, 0);
}

// Record dot gently "breathes" between r=16 and r=24 (~1.4s cycle).
static int pulseRadius() {
  float phase = (millis() % 1400) / 1400.0f;
  float s = (sinf(phase * TWO_PI) + 1.0f) * 0.5f;
  return 16 + (int)(s * 8.0f);
}

static void drawIdle()         { renderScreen("READY", "hold to talk", C_GREEN, C_BG,     0, 0); }
static void drawDisconnected() { renderScreen("PAIR",  "bluetooth",    C_BLUE,  C_BG,     0, 0); }
static void drawPTT()          { renderScreen("REC",   "listening",    C_RED,   C_BG,     1, pulseRadius()); }
static void drawLocked()       { renderScreen("REC",   "tap to stop",  C_RED,   C_BG_REC, 2, pulseRadius()); }

// ---- key helpers ----------------------------------------------------------
static void holdCombo()    { bleKeyboard.press(KEY_LEFT_CTRL); bleKeyboard.press(KEY_LEFT_ALT); } // Ctrl+Option
static void releaseCombo() { bleKeyboard.releaseAll(); }

static void enterIdle()   { releaseCombo(); mode = MODE_IDLE;   pendingTap = false; drawIdle(); }
static void enterPTT()    { holdCombo();    mode = MODE_PTT;                        drawPTT(); }
static void enterLocked() { holdCombo();    mode = MODE_LOCKED; pendingTap = false; drawLocked(); }

// ---- Wi-Fi OTA ------------------------------------------------------------
// Kept non-blocking: we start Wi-Fi, then bring OTA up lazily once connected,
// and retry quietly if it drops. The keyboard never waits on the network.
static void startWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(true);                 // modem sleep -> friendlier BLE coexistence
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  lastWifiTry = millis();

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  if (strlen(OTA_PASSWORD) > 0) ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onStart([]()    { drawUpdating(0, 100); });
  ArduinoOTA.onProgress([](unsigned int p, unsigned int t) { drawUpdating(p, t); });
  ArduinoOTA.onEnd([]()      { drawUpdating(100, 100); });
  ArduinoOTA.onError([](ota_error_t) { drawDisconnected(); });
}

// Bring OTA up when Wi-Fi connects; retry the connection every 10s if it drops.
static void serviceOta() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!otaReady) { ArduinoOTA.begin(); otaReady = true; }
    ArduinoOTA.handle();
  } else {
    otaReady = false;
    if (millis() - lastWifiTry > 10000) { WiFi.reconnect(); lastWifiTry = millis(); }
  }
}

// ---- lifecycle ------------------------------------------------------------
void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);                 // inits the LCD and BtnA (the front screen)
  M5.Display.setRotation(0);

  canvas.setColorDepth(16);
  canvas.createSprite(M5.Display.width(), M5.Display.height());
  canvas.setTextWrap(false);
  initColors();

  bleKeyboard.begin();           // start BLE HID advertising (primary function)
  startWifi();                   // bring up Wi-Fi + OTA in the background
  drawDisconnected();
}

void loop() {
  M5.update();
  serviceOta();                 // handle Wi-Fi OTA (works even when BLE is unpaired)
  const uint32_t now = millis();
  const bool connected = bleKeyboard.isConnected();

  // Connection edges. Clearing the combo on every (re)connect prevents
  // Ctrl+Option from being left latched on the Mac across a sleep/disconnect.
  if (connected && !wasConnected) {
    enterIdle();
  } else if (!connected && wasConnected) {
    releaseCombo(); mode = MODE_IDLE; pendingTap = false; drawDisconnected();
  }
  wasConnected = connected;

  if (!connected) { delay(20); return; }   // don't send key events while unpaired

  // ---- gestures ----------------------------------------------------------
  if (M5.BtnA.wasPressed()) {
    pressStart = now;
  }

  // Press sustained past the threshold -> push-to-talk (only from idle).
  if (mode == MODE_IDLE && M5.BtnA.isPressed() && (now - pressStart >= HOLD_MS)) {
    enterPTT();
  }

  if (M5.BtnA.wasReleased()) {
    const uint32_t dur = now - pressStart;
    if (mode == MODE_PTT) {
      enterIdle();                          // released the hold -> stop PTT
    } else if (mode == MODE_LOCKED) {
      enterIdle();                          // a tap stops the locked recording
    } else if (dur < HOLD_MS) {             // a quick tap while idle
      if (pendingTap && (now - lastTapMs <= DTAP_MS)) {
        enterLocked();                      // double-tap -> latch recording on
      } else {
        pendingTap = true; lastTapMs = now;
      }
    }
  }

  // A lone tap that never gets a partner just expires (does nothing).
  if (pendingTap && (now - lastTapMs > DTAP_MS)) pendingTap = false;

  // ---- animate active recording -----------------------------------------
  if (mode == MODE_PTT || mode == MODE_LOCKED) {
    static uint32_t lastFrame = 0;
    if (now - lastFrame >= 33) {            // ~30 fps
      lastFrame = now;
      (mode == MODE_PTT) ? drawPTT() : drawLocked();
    }
  }

  delay(5);
}
