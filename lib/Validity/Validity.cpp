#include "Validity.h"

void Validity::check(Snapshot& s, const Snapshot& prev) {
    checkChannel(s.tIn,    prev.tIn,    limits_.tIn,    stuckTIn_);
    checkChannel(s.rhIn,   prev.rhIn,   limits_.rhIn,   stuckRhIn_);
    checkChannel(s.tOut,   prev.tOut,   limits_.tOut,   stuckTOut_);
    checkChannel(s.tWater, prev.tWater, limits_.tWater, stuckTWater_);
}

void Validity::checkChannel(Reading& r, const Reading& prev, const ChannelLimits& lim, uint16_t& stuckCounter) {
    if (!r.valid) return;

    if (r.value < lim.min || r.value > lim.max) {
        r.reject(FAULT_RANGE);
        return;
    }

    const uint32_t elapsedMs = r.sampledAtMs - prev.sampledAtMs;
    if (prev.valid && elapsedMs != 0) {
        float delta = r.value - prev.value;
        if (delta < 0.0f) delta = -delta;
        if (delta * 60000.0f > lim.maxRatePerMin * elapsedMs) {
            r.reject(FAULT_RATE);
            return;
        }
    }

    if (prev.valid && r.raw == prev.raw) {
        if (stuckCounter != UINT16_MAX) ++stuckCounter;
    } else {
        stuckCounter = 1;
    }
    if (stuckCounter >= limits_.stuckCount) {
        r.reject(FAULT_STUCK);
    }
}
