#pragma once
// Pure functions over the SHT4x 6-byte frame. No Arduino dependencies.
#include <stdint.h>
#include <stddef.h>

namespace sht4x {

// CRC-8 as specified in Sht4xTypes.h. Datasheet test vector: crc8({0xBE,0xEF}) == 0x92.
uint8_t crc8(const uint8_t* data, size_t len);

// Decode a full frame. Returns false if either CRC fails; tC and rh are
// untouched on false. rh is clipped to 0..100.
bool decode(const uint8_t frame[6], float& tC, float& rh);

// Word <-> physical value conversions, exposed for the emulator and tests.
float    wordToTemperature(uint16_t word);
float    wordToHumidity(uint16_t word);      // clipped to 0..100
uint16_t temperatureToWord(float tC);        // clamped to 0..65535
uint16_t humidityToWord(float rh);           // clamped to 0..65535

// Build a valid frame from two words (emulator use).
void encode(uint16_t tWord, uint16_t rhWord, uint8_t frame[6]);

} // namespace sht4x
