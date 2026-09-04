#pragma once
// LED matrix status (outline 7): Wi-Fi state, fault present, publish
// blink. Frame format from the Arduino_LED_Matrix DisplaySingleFrame
// example (examples/DisplaySingleFrame).
#include <Arduino_LED_Matrix.h>

class StatusLed {
public:
    enum State : uint8_t { BOOT, WIFI_CONNECTING, MQTT_CONNECTING, ONLINE };

    void begin();
    void show(State s, bool faultPresent);
    // Short flash on publish; call update() every loop pass to end it.
    void blinkPublish(uint32_t nowMs);
    void update(uint32_t nowMs);

private:
    ArduinoLEDMatrix matrix_;
    State    state_ = BOOT;
    bool     fault_ = false;
    uint32_t blinkUntilMs_ = 0;
    void render();
};
