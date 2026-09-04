#pragma once
// NTC thermistor math (outline 3.3). Pure functions, PC-testable.
//
// Circuit: V+ -> R_s -> ADC pin -> NTC -> GND, with the ADC referenced to
// the same V+, so supply variation cancels:
//   R_ntc = R_s * ADC / (ADC_max - ADC)
//   1/T   = 1/T0 + (1/B) * ln(R_ntc / R0)      T in kelvin, T0 = 298.15 K
#include <stdint.h>

namespace ntc {

struct Params {
    float rSeriesOhm = 10000.0f;  // R_s
    float r0Ohm      = 10000.0f;  // NTC resistance at t0C
    float betaK      = 3950.0f;   // B25/85, verify against the part datasheet
    float t0C        = 25.0f;
    float offsetC    = 0.0f;      // single-point calibration against the DS18B20
};

// ADC counts near either rail mean a wiring fault, not a temperature.
// Returns true when adc is inside (shortMargin, adcMax - openMargin).
bool adcPlausible(uint32_t adc, uint32_t adcMax, uint32_t shortMargin = 16, uint32_t openMargin = 16);
bool adcIsShort(uint32_t adc, uint32_t shortMargin = 16);
bool adcIsOpen(uint32_t adc, uint32_t adcMax, uint32_t openMargin = 16);

// Divider midpoint to NTC resistance. adc must be < adcMax.
float resistanceFromAdc(uint32_t adc, uint32_t adcMax, float rSeriesOhm);

// Beta equation, result in C (offsetC applied).
float temperatureFromResistance(float rNtcOhm, const Params& p);

// Convenience: both steps.
float temperatureFromAdc(uint32_t adc, uint32_t adcMax, const Params& p);

} // namespace ntc
