#include "Validity.h"

void Validity::check(Snapshot& s, const Snapshot& prev) {
    checkChannel(s.tIn,    prev.tIn,    limits_.tIn,    stuckTIn_);
    checkChannel(s.rhIn,   prev.rhIn,   limits_.rhIn,   stuckRhIn_);
    checkChannel(s.tOut,   prev.tOut,   limits_.tOut,   stuckTOut_);
    checkChannel(s.tWater, prev.tWater, limits_.tWater, stuckTWater_);
}

void Validity::checkChannel(Reading& r, const Reading& prev, const ChannelLimits& lim, uint16_t& stuckCounter) {
    (void)prev; (void)lim; (void)stuckCounter;
    if (!r.valid) return;

    if(r.value < lim.min || r.value > lim.max) {
        r.valid = false;
        r.fault = FAULT_RANGE;

    }
    const uint32_t sampleMsDelta = r.sampledAtMs - prev.sampledAtMs;
    if (prev.valid && sampleMsDelta != 0){
        float delta = r.value - prev.value;
        if(delta<0.0f) delta = -delta;
        if (delta * 60000.0f > lim.maxRatePerMin * sampleMsDelta) {
            r.valid = false;
            r.fault = FAULT_RATE;
        }
    }
    if (r.raw == prev.raw) stuckCounter++;
    else stuckCounter = 1;

    if(stuckCounter >= limits_.stuckCount) {
        r.valid = false;
        r.fault = FAULT_STUCK;
    }

}
