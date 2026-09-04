#include "Sht4xSimulated.h"
#include "Sht4xDecoder.h"
#include <string.h>

bool Sht4xSimulated::begin() {
    return true;
}

void Sht4xSimulated::startMeasurement(uint32_t nowMs) {
    lastStartMs_ = nowMs;
    float tC, rh;
    model(nowMs, tC, rh);
    sht4x::encode(sht4x::temperatureToWord(tC), sht4x::humidityToWord(rh), frame_);
    if (scenario_ == BADCRC) {
        frame_[2] ^= 0xFF;
    }
    pending_ = true;
}

bool Sht4xSimulated::frameReady(uint32_t) const {
    return pending_;
}

bool Sht4xSimulated::readFrame(uint8_t out[6]) {
    if (!pending_) return false;
    memcpy(out, frame_, 6);
    pending_ = false;
    return true;
}

void Sht4xSimulated::setScenario(Scenario s, uint32_t nowMs) {
    scenario_ = s;
    scenarioStartMs_ = nowMs;
}

bool Sht4xSimulated::setScenarioByName(const char* name, uint32_t nowMs) {
    struct { const char* n; Scenario s; } table[] = {
        {"steady", STEADY}, {"heatup", HEATUP}, {"cooldown", COOLDOWN},
        {"stuck", STUCK}, {"badcrc", BADCRC},
    };
    for (auto& e : table) {
        if (strcmp(e.n, name) == 0) { setScenario(e.s, nowMs); return true; }
    }
    return false;
}

void Sht4xSimulated::setReferenceTemperature(float tOutC) {
    params_.tRefC = tOutC;
}

uint32_t Sht4xSimulated::noise() {
    // xorshift32, deterministic so tests are repeatable
    rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5;
    return rng_;
}

void Sht4xSimulated::model(uint32_t nowMs, float& tC, float& rh) {
    (void)nowMs;
    // TODO(emulator): lamp ramp (HEATUP/COOLDOWN), drift, noise, STUCK.
    tC = params_.tRefC;
    rh = params_.rhBase;
}
