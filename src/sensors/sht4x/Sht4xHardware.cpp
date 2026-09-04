#include "sensors/sht4x/Sht4xHardware.h"
#include "Sht4xTypes.h"

bool Sht4xHardware::begin() {
    bus_.begin();
    bus_.beginTransmission(sht4x::I2C_ADDRESS);
    return bus_.endTransmission() == 0;
}

void Sht4xHardware::startMeasurement(uint32_t nowMs) {
    bus_.beginTransmission(sht4x::I2C_ADDRESS);
    bus_.write(sht4x::CMD_MEASURE_HIGH);
    pending_   = bus_.endTransmission() == 0;
    startedMs_ = nowMs;
}

bool Sht4xHardware::frameReady(uint32_t nowMs) const {
    return !pending_ || (nowMs - startedMs_) >= sht4x::CONVERSION_MS_HIGH;
}

bool Sht4xHardware::readFrame(uint8_t out[6]) {
    if (!pending_) return false;
    pending_ = false;
    if (bus_.requestFrom(sht4x::I2C_ADDRESS, sht4x::FRAME_LEN) != sht4x::FRAME_LEN) return false;
    for (uint8_t i = 0; i < sht4x::FRAME_LEN; ++i) out[i] = (uint8_t)bus_.read();
    return true;
}
