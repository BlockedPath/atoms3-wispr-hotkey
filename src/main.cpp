#include "app/HotkeyController.h"

namespace {
assclets::HotkeyController app;
}

void setup() {
  app.begin();
}

void loop() {
  app.update();
}
