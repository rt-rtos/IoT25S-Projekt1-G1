#pragma once
// Sensor adapter: frame source + decoder -> two Readings (T_in, RH_in).
// Follows the driver contract in lib/Sensor/Sensor.h.
// Fault mapping: no frame -> NO_DEVICE, bad CRC -> CRC.
#include "Reading.h"
#include "Sht4xFrameSource.h"

class Sht4xSensor {
public:
    explicit Sht4xSensor(Sht4xFrameSource& source) : source_(source) {}

    bool begin();
    void start(uint32_t nowMs);
    bool ready(uint32_t nowMs) const;
    // Both channels from one frame. Returns t.valid && rh.valid.
    bool read(Reading& t, Reading& rh, uint32_t nowMs);

private:
    Sht4xFrameSource& source_;
};
