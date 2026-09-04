#pragma once
// SHT4x emulator (outline 5.2, Sht4xSimulated).
//
// Model: T_in = T_ref + lampOffset(t) + drift + noise
//        RH_in = base - k * (T_in - T_ref) + noise, clipped 0..100
// T_ref is a constant or, via setReferenceTemperature(), the latest valid
// T_out reading so the demo stays physically coherent.
//
// Scenarios: STEADY, HEATUP (lamp on ramp), COOLDOWN, STUCK (same raw
// words every frame), BADCRC (valid words, corrupted CRC).
//
// Pure C++: compiles on a PC for the component tests.
#include <stdint.h>
#include "Sht4xFrameSource.h"

class Sht4xSimulated : public Sht4xFrameSource {
public:
    enum Scenario : uint8_t { STEADY, HEATUP, COOLDOWN, STUCK, BADCRC };

    struct Params {
        float tRefC       = 22.0f;  // used until setReferenceTemperature() is called
        float lampRiseC   = 6.0f;   // temperature rise at full lamp effect
        float lampTauMs   = 300000; // time constant of the heat-up / cool-down ramp
        float rhBase      = 65.0f;  // %RH at T_ref
        float rhPerDegree = 2.5f;   // %RH drop per degree above T_ref
        float noiseT      = 0.05f;  // peak noise, C
        float noiseRh     = 0.3f;   // peak noise, %RH
    };

    Sht4xSimulated() {}
    explicit Sht4xSimulated(const Params& p) : params_(p) {}

    bool begin() override;
    void startMeasurement(uint32_t nowMs) override;
    bool frameReady(uint32_t nowMs) const override;
    bool readFrame(uint8_t out[6]) override;

    void setScenario(Scenario s, uint32_t nowMs);
    Scenario scenario() const { return scenario_; }
    // Parse "steady" / "heatup" / ... ; false and no change on unknown names.
    bool setScenarioByName(const char* name, uint32_t nowMs);
    void setReferenceTemperature(float tOutC);

private:
    // TODO(emulator): implement the model; currently returns constants.
    void model(uint32_t nowMs, float& tC, float& rh);
    uint32_t noise();

    Params   params_;
    Scenario scenario_        = STEADY;
    uint32_t scenarioStartMs_ = 0;
    uint32_t lastStartMs_     = 0;
    bool     pending_         = false;
    uint8_t  frame_[6]        = {0};
    uint32_t rng_             = 0x12345678;
};
