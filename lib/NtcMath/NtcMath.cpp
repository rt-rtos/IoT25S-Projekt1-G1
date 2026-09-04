#include "NtcMath.h"
#include <math.h>

namespace ntc {

static constexpr float KELVIN_OFFSET = 273.15f;

bool adcIsShort(uint32_t adc, uint32_t shortMargin) {
    return adc <= shortMargin;
}

bool adcIsOpen(uint32_t adc, uint32_t adcMax, uint32_t openMargin) {
    return adc + openMargin >= adcMax;
}

bool adcPlausible(uint32_t adc, uint32_t adcMax, uint32_t shortMargin, uint32_t openMargin) {
    return !adcIsShort(adc, shortMargin) && !adcIsOpen(adc, adcMax, openMargin);
}

float resistanceFromAdc(uint32_t adc, uint32_t adcMax, float rSeriesOhm) {
    return rSeriesOhm * (float)adc / (float)(adcMax - adc);
}

float temperatureFromResistance(float rNtcOhm, const Params& p) {
    float invT = 1.0f / (p.t0C + KELVIN_OFFSET) + logf(rNtcOhm / p.r0Ohm) / p.betaK;
    return 1.0f / invT - KELVIN_OFFSET + p.offsetC;
}

float temperatureFromAdc(uint32_t adc, uint32_t adcMax, const Params& p) {
    return temperatureFromResistance(resistanceFromAdc(adc, adcMax, p.rSeriesOhm), p);
}

} // namespace ntc
