#pragma once
// DS18B20 on 1-Wire (outline 3.2), asynchronous conversion so the 750 ms
// never blocks the loop. Derived from DallasTemperature example
// WaitForConversion2 (examples/WaitForConversion2).
//
// Fault mapping: no device on the bus / disconnected -> NO_DEVICE,
// 85.0 C power-on value (conversion never ran) -> TIMEOUT, scratchpad CRC
// failure is reported by the library as disconnected -> NO_DEVICE.
// A sensor missing at begin() is searched for again on every start(), so
// replugging recovers without a reboot. Follows the driver contract in
// lib/Sensor/Sensor.h.
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Reading.h"

class Ds18b20Sensor {
public:
    Ds18b20Sensor(uint8_t pin, uint8_t resolutionBits);

    bool begin();
    void start(uint32_t nowMs);
    bool ready(uint32_t nowMs) const;
    bool read(Reading& out, uint32_t nowMs);

    bool present() const { return found_; }
    const DeviceAddress& address() const { return addr_; }

private:
    bool detect();

    OneWire           bus_;
    DallasTemperature dallas_;
    DeviceAddress     addr_ = {0};
    uint8_t           resolutionBits_;
    uint16_t          conversionMs_ = 750;
    uint32_t          startedMs_ = 0;
    bool              found_ = false;
    bool              pending_ = false;
};
