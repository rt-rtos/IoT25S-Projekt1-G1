#pragma once
// Owns the three sensors and runs one non-blocking sample cycle
// (outline 5.2). Requirement R8 at the acquisition level: a sensor that
// fails keeps its slot with valid = false, the others are unaffected.
//
// Usage from the loop:
//   startCycle(now) once per sample tick, then update(now, snap) every
//   loop pass; it returns true exactly once per cycle, when all sensors
//   are ready or timeoutMs has passed, with snap filled in.
#include "Reading.h"
#include "sensors/sht4x/Sht4xSensor.h"
#include "sensors/Ds18b20Sensor.h"
#include "sensors/NtcSensor.h"

class SensorManager {
public:
    SensorManager(Sht4xSensor& sht, Ds18b20Sensor& ds18b20, NtcSensor& ntc, uint32_t timeoutMs);

    // Calls begin() on every sensor; returns true only if all answered.
    bool begin();
    void startCycle(uint32_t nowMs);
    bool update(uint32_t nowMs, Snapshot& out);
    bool busy() const { return cycleActive_; }

private:
    Sht4xSensor&   sht_;
    Ds18b20Sensor& ds_;
    NtcSensor&     ntc_;
    uint32_t       timeoutMs_;
    uint32_t       cycleStartMs_ = 0;
    uint32_t       seq_          = 0;
    bool           cycleActive_  = false;
};
