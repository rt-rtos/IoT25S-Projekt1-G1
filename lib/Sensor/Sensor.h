#pragma once
// Driver contract (outline 5.2). There is no base class: SensorManager
// composes the three concrete drivers statically, and the SHT4x yields two
// channels from one frame, so a single-channel virtual interface did not
// fit. Every driver follows this contract instead:
//
//   bool begin()
//       Initialise the device. false if it does not answer. Emulated
//       sources always return true. Called once from setup().
//   void start(uint32_t nowMs)
//       Trigger one measurement and return immediately.
//   bool ready(uint32_t nowMs) const
//       true when read() will not block. Implementations compare nowMs
//       against the datasheet conversion time.
//   bool read(Reading& out, uint32_t nowMs)          (one channel)
//   bool read(Reading& t, Reading& rh, uint32_t nowMs) (SHT4x, two channels)
//       Fill out; always sets fault and sampledAtMs. Returns valid.
//       On a transport failure value is NaN. Never blocks longer than
//       about 20 ms. Not callable from an interrupt.
//
// Timing: SHT4x 8 ms (emulator: immediate), DS18B20 750 ms at 12 bit,
// NTC immediate (16 ADC reads, under 1 ms).
#include "Reading.h"
