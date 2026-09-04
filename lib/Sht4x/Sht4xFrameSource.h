#pragma once
// The frame-level seam (outline 5.1): Sht4xSensor sees six bytes whether
// they come from silicon (Sht4xHardware, src/) or from the emulator
// (Sht4xSimulated). A bad CRC is rejected the same way in both cases.
//
// Contract
//   begin()             true if the device answers (emulator: always true).
//   startMeasurement()  Send the measure command. Returns immediately.
//   frameReady()        true when readFrame() will not block.
//   readFrame()         Copy the 6-byte response into out. false on a
//                       transport failure (no ACK, short read); out is
//                       then unspecified. Does not check CRC.
#include <stdint.h>

class Sht4xFrameSource {
public:
    virtual ~Sht4xFrameSource() {}
    virtual bool begin() = 0;
    virtual void startMeasurement(uint32_t nowMs) = 0;
    virtual bool frameReady(uint32_t nowMs) const = 0;
    virtual bool readFrame(uint8_t out[6]) = 0;
};
