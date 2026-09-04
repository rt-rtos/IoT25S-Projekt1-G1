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
    // TODO(validity): range -> FAULT_RANGE, rate vs prev (use sampledAtMs
    // delta) -> FAULT_RATE, stuck raw counter -> FAULT_STUCK. Set
    // r.valid = false and r.fault, leave r.value untouched.
}
