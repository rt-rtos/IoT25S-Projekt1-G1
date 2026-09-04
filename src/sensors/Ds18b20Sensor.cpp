#include "sensors/Ds18b20Sensor.h"

// DS18B20 returns this after power-on if a conversion never ran.
static constexpr float POWER_ON_RESET_C = 85.0f;

Ds18b20Sensor::Ds18b20Sensor(uint8_t pin, uint8_t resolutionBits)
    : bus_(pin), dallas_(&bus_), resolutionBits_(resolutionBits) {}

bool Ds18b20Sensor::detect() {
    // TODO(ds18b20): dallas_.begin(), getAddress(addr_, 0) -> found_,
    // setResolution(addr_, resolutionBits_), conversionMs_ from
    // DallasTemperature::millisToWaitForConversion(resolutionBits_).
    return found_;
}

bool Ds18b20Sensor::begin() {
    // TODO(ds18b20): setWaitForConversion(false) so requests never block,
    // then detect(). See examples/WaitForConversion2 setup().
    return detect();
}

void Ds18b20Sensor::start(uint32_t nowMs) {
    // TODO(ds18b20): if not found_, try detect() again (replug recovery);
    // requestTemperaturesByAddress(addr_), remember nowMs, set pending_.
    (void)nowMs;
}

bool Ds18b20Sensor::ready(uint32_t nowMs) const {
    // TODO(ds18b20): not pending, or nowMs - startedMs_ >= conversionMs_.
    (void)nowMs;
    return true;
}

bool Ds18b20Sensor::read(Reading& out, uint32_t nowMs) {
    // TODO(ds18b20): raw = dallas_.getTemp(addr_);
    //   DEVICE_DISCONNECTED_RAW -> FAULT_NO_DEVICE (and clear found_),
    //   rawToCelsius(raw) == POWER_ON_RESET_C -> FAULT_TIMEOUT,
    //   otherwise out.set(tC, raw, nowMs).
    (void)POWER_ON_RESET_C;
    out.fail(FAULT_NOT_READY, nowMs);
    return false;
}
