#include <Arduino.h>
#include "sensors/NtcSensor.h"

NtcSensor::NtcSensor(uint8_t pin, int adcBits, uint8_t samples, const ntc::Params& params)
    : pin_(pin), adcBits_(adcBits), adcMax_((1u << adcBits) - 1), samples_(samples), params_(params) {}

bool NtcSensor::begin() {
    analogReadResolution(adcBits_);
    return true;
}

void NtcSensor::start(uint32_t) {}

bool NtcSensor::ready(uint32_t) const {
    return true;
}

bool NtcSensor::read(Reading& out, uint32_t nowMs) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < samples_; ++i) sum += (uint32_t)analogRead(pin_);
    uint32_t adc = sum / samples_;
    if (!ntc::adcPlausible(adc, adcMax_)) {
        out.fail(FAULT_NO_DEVICE, nowMs);
        out.raw = adc;
        return false;
    }
    out.set(ntc::temperatureFromAdc(adc, adcMax_, params_), adc, nowMs);
    return true;
}
