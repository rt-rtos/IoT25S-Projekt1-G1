#pragma once
// NTC 10 k in a voltage divider on the ADC (outline 3.3). Averages
// NTC_SAMPLES reads, converts with the beta equation in lib/NtcMath.
// ADC at either rail (short / open) maps to NO_DEVICE, never to a temperature.
// Follows the driver contract in lib/Sensor/Sensor.h.
#include "Reading.h"
#include "NtcMath.h"

class NtcSensor {
public:
    NtcSensor(uint8_t pin, int adcBits, uint8_t samples, const ntc::Params& params);

    bool begin();
    void start(uint32_t nowMs);
    bool ready(uint32_t nowMs) const;
    bool read(Reading& out, uint32_t nowMs);

    void setParams(const ntc::Params& p) { params_ = p; }
    const ntc::Params& params() const { return params_; }

private:
    uint8_t     pin_;
    int         adcBits_;
    uint32_t    adcMax_;
    uint8_t     samples_;
    ntc::Params params_;
};
