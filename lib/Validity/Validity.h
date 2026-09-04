#pragma once
// Validity checks (outline 5.2): per channel, physical range, maximum rate
// of change against the previous snapshot, and stuck detection (N identical
// raw values in a row).
//
// Contract
//   check() downgrades valid and sets fault, never changes value. Channels
//   that were already invalid keep their driver-level fault code.
//   Stuck detection needs a counter per channel, so this is a class with
//   state rather than a free function; construct one per node.
//
// Default limits: T_in 5..45 C, T_out -20..45 C,
// T_water 5..40 C, RH 5..100 %; rate 2 C/min air, 1 C/min water,
// 5 %RH/min; stuck after 30 identical raw readings.
#include <stdint.h>
#include "Reading.h"

class Validity {
public:
    struct ChannelLimits {
        float min;
        float max;
        float maxRatePerMin;
    };
    struct Limits {
        ChannelLimits tIn    = {  5.0f,  45.0f, 2.0f };
        ChannelLimits tOut   = { -20.0f, 45.0f, 2.0f };
        ChannelLimits tWater = {  5.0f,  40.0f, 1.0f };
        ChannelLimits rhIn   = {  5.0f, 100.0f, 5.0f };
        uint16_t stuckCount  = 30;
    };

    Validity() {}
    explicit Validity(const Limits& l) : limits_(l) {}

    void check(Snapshot& s, const Snapshot& prev);

    const Limits& limits() const { return limits_; }
    void setLimits(const Limits& l) { limits_ = l; }

private:
    void checkChannel(Reading& r, const Reading& prev, const ChannelLimits& lim, uint16_t& stuckCounter);

    Limits   limits_;
    uint16_t stuckTIn_    = 0;
    uint16_t stuckRhIn_   = 0;
    uint16_t stuckTOut_   = 0;
    uint16_t stuckTWater_ = 0;
};
