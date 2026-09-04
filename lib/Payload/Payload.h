#pragma once
// Telemetry payload (outline 5.4): one flat JSON object per sample.
// Pure C++ (no Arduino, no float printf, which newlib-nano lacks), so it
// is unit-tested on the PC.
//
//   {"seq":123,"uptime_s":4567,
//    "t_in":24.1,"rh_in":61.0,"t_out":20.3,"t_water":null,
//    "faults":{"t_in":0,"rh_in":0,"t_out":0,"t_water":1},
//    "src":{"sht40":"sim","ds18b20":"hw","ntc":"hw"}}
//
// An invalid channel publishes null and its fault code, never a fake
// number and never a missing key. Values carry one decimal.
#include <stddef.h>
#include "Reading.h"

struct SourceInfo {
    const char* sht40;    // "sim" or "hw"
    const char* ds18b20;
    const char* ntc;
};

// snprintf semantics: returns the length the full payload would have, so
// a result >= len means the buffer was too small. buf is always terminated.
int buildTelemetryJson(char* buf, size_t len, const Snapshot& s, const SourceInfo& src);
