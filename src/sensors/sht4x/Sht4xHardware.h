#pragma once
// Real SHT40 over I2C (outline 3.1). Uses the Qwiic connector, which is
// Wire1 on the Uno R4 WiFi (3.3 V, no level shifting).
#include <Wire.h>
#include "Sht4xFrameSource.h"

class Sht4xHardware : public Sht4xFrameSource {
public:
    explicit Sht4xHardware(TwoWire& bus) : bus_(bus) {}

    bool begin() override;
    void startMeasurement(uint32_t nowMs) override;
    bool frameReady(uint32_t nowMs) const override;
    bool readFrame(uint8_t out[6]) override;

private:
    TwoWire& bus_;
    uint32_t startedMs_ = 0;
    bool     pending_   = false;
};
