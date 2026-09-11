#include "ui/StatusLed.h"

// A frame is uint32_t[3] (96 bits, one per LED, row-major 12x8), shown
// with matrix_.loadFrame(frame). See examples/DisplaySingleFrame/frames.h
// for the layout and a few ready-made icons. Draw new ones in the Arduino
// LED Matrix Editor (docs.arduino.cc, "LED Matrix Editor") and paste the
// array.

void StatusLed::begin() {
    matrix_.begin();
    render();
}

void StatusLed::show(State s, bool faultPresent) {
    if (s == state_ && faultPresent == fault_) return;
    state_ = s;
    fault_ = faultPresent;
    render();
}

void StatusLed::blinkPublish(uint32_t nowMs) {
    // TODO(led): blank the matrix and set blinkUntilMs_ = nowMs + 150.
    (void)nowMs;
}

void StatusLed::update(uint32_t nowMs) {
    // TODO(led): when blinkUntilMs_ has passed, clear it and render().
    (void)nowMs;
}

void StatusLed::render() {
    // TODO(led): static const uint32_t frames[4][3] indexed by state_
    // (BOOT, WIFI_CONNECTING, MQTT_CONNECTING, ONLINE); if fault_, load
    // a warning frame instead. matrix_.loadFrame(...).
    matrix_.clear();
}
