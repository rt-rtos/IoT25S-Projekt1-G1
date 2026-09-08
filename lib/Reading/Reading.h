#pragma once
// Reading / Snapshot: the data types every layer above the drivers works on.
// No Arduino dependencies so the validity, aggregation and emulator code
// compiles on a PC (env:native).
#include <stdint.h>
#include <math.h>

enum Fault : uint8_t {
    FAULT_NONE = 0,   // valid reading
    FAULT_NO_DEVICE,  // no presence pulse / no I2C ACK / sensor unplugged
    FAULT_CRC,        // frame or scratchpad CRC mismatch
    FAULT_TIMEOUT,    // conversion never completed (DS18B20 85.0 C power-on value)
    FAULT_RANGE,      // outside physical range (Validity)
    FAULT_RATE,       // rate of change above limit (Validity)
    FAULT_STUCK,      // identical raw value N times in a row (Validity)
    FAULT_NOT_READY,  // read() called before ready(), or never sampled
    FAULT_COUNT
};

const char* faultName(uint8_t fault);

struct Reading {
    float    value       = NAN;             // C or %RH. Only meaningful when valid; fail() sets NaN, skipped check yields NaN, never a plausible number.
    bool     valid       = false;           // fault == FAULT_NONE. Check this, not isnan(value).
    uint8_t  fault       = FAULT_NOT_READY;
    uint32_t sampledAtMs = 0;
    uint32_t raw         = 0;               // raw sensor word, for stuck detection only

    void set(float v, uint32_t rawWord, uint32_t nowMs) {
        value = v; raw = rawWord; sampledAtMs = nowMs; fault = FAULT_NONE; valid = true;
    }
    void fail(uint8_t f, uint32_t nowMs) {
        value = NAN; sampledAtMs = nowMs; fault = f; valid = false;
    }
};

struct Snapshot {
    Reading  tIn;
    Reading  rhIn;
    Reading  tOut;
    Reading  tWater;
    uint32_t seq      = 0;
    uint32_t uptimeMs = 0;
};
