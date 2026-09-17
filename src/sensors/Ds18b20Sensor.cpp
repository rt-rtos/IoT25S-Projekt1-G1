#include "sensors/Ds18b20Sensor.h"

// DS18B20 returns 85.0 C after power-on if a conversion never ran. Kept
// as the raw 1/128 C word so the check is an integer compare.
static constexpr int32_t POWER_ON_RESET_RAW = 85 * 128;

// The example (examples/WaitForConversion2) addresses the device by index.
// This driver keeps the ROM address from detect() and uses the *ByAddress
// calls so one known probe is read directly, and a replug is found again
// by detect() instead of by re-enumerating the bus. Signatures, from
// DallasTemperature.h (4.0.6), since no example uses them:
//   bool      getAddress(uint8_t* addr, uint8_t index);
//   void      setResolution(const uint8_t* addr, uint8_t bits);
//   static uint16_t millisToWaitForConversion(uint8_t bits);
//   void      setWaitForConversion(bool);
//   request_t requestTemperaturesByAddress(const uint8_t* addr);  // .result is bool
//   int32_t   getTemp(const uint8_t* addr);   // DEVICE_DISCONNECTED_RAW (-7040) on failure
//   static float rawToCelsius(int32_t raw);

Ds18b20Sensor::Ds18b20Sensor(uint8_t pin, uint8_t resolutionBits)
    : bus_(pin), dallas_(&bus_), resolutionBits_(resolutionBits) {}

bool Ds18b20Sensor::detect() {
    dallas_.begin();
    found_ = dallas_.getAddress(addr_, 0);
    if (found_) {
        dallas_.setResolution(addr_, resolutionBits_);
        conversionMs_ = DallasTemperature::millisToWaitForConversion(resolutionBits_);
    }
    return found_;
}

bool Ds18b20Sensor::begin() {
    dallas_.setWaitForConversion(false);
    return detect();
}

void Ds18b20Sensor::start(uint32_t nowMs) {
    // A probe missing at begin() or lost in read() is searched for again
    // here, so a replug recovers on the next cycle without a reboot.
    if (!found_ && !detect()) {
        pending_ = false;
        return;
    }
    dallas_.requestTemperaturesByAddress(addr_);
    startedMs_ = nowMs;
    pending_   = true;
}

bool Ds18b20Sensor::ready(uint32_t nowMs) const {
    return !pending_ || (nowMs - startedMs_) >= conversionMs_;
}

bool Ds18b20Sensor::read(Reading& out, uint32_t nowMs) {
    if (!pending_) {
        // Nothing was requested: no probe on the bus, or start() was skipped.
        out.fail(found_ ? FAULT_NOT_READY : FAULT_NO_DEVICE, nowMs);
        return false;
    }
    pending_ = false;

    // Signed local on purpose: the disconnect sentinel is negative and
    // Reading::raw is unsigned.
    int32_t raw = dallas_.getTemp(addr_);
    if (raw == DEVICE_DISCONNECTED_RAW) {
        found_ = false;
        out.fail(FAULT_NO_DEVICE, nowMs);
        return false;
    }
    if (raw == POWER_ON_RESET_RAW) {
        out.fail(FAULT_TIMEOUT, nowMs);
        return false;
    }
    out.set(DallasTemperature::rawToCelsius(raw), (uint32_t)raw, nowMs);
    return true;
}
