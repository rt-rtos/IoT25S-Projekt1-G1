#include "ui/StatusLed.h"

// Frames are three packed uint32_t for the 12x8 matrix; see
// examples/DisplaySingleFrame/frames.h for the format and a few icons.

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
    // TODO(led): one frame per state, fault_ overrides with a warning frame.
    matrix_.clear();
}
