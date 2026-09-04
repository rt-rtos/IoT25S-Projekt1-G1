#include "sensors/sht4x/Sht4xSensor.h"
#include "Sht4xDecoder.h"

bool Sht4xSensor::begin() {
    return source_.begin();
}

void Sht4xSensor::start(uint32_t nowMs) {
    source_.startMeasurement(nowMs);
}

bool Sht4xSensor::ready(uint32_t nowMs) const {
    return source_.frameReady(nowMs);
}

bool Sht4xSensor::read(Reading& t, Reading& rh, uint32_t nowMs) {
    uint8_t frame[6];
    if (!source_.readFrame(frame)) {
        t.fail(FAULT_NO_DEVICE, nowMs);
        rh.fail(FAULT_NO_DEVICE, nowMs);
        return false;
    }
    float tC, rhPct;
    if (!sht4x::decode(frame, tC, rhPct)) {
        t.fail(FAULT_CRC, nowMs);
        rh.fail(FAULT_CRC, nowMs);
        return false;
    }
    t.set(tC, (uint32_t)((frame[0] << 8) | frame[1]), nowMs);
    rh.set(rhPct, (uint32_t)((frame[3] << 8) | frame[4]), nowMs);
    return true;
}
