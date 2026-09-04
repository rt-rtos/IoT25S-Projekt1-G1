#pragma once
// Sensirion SHT4x datasheet constants the software relies on (outline 3.1).
// Verify against the datasheet revision in use before the numbers go into
// the project documentation.
#include <stdint.h>

namespace sht4x {

constexpr uint8_t I2C_ADDRESS = 0x44;

constexpr uint8_t CMD_MEASURE_HIGH   = 0xFD;  // about 8 ms conversion
constexpr uint8_t CMD_MEASURE_MEDIUM = 0xF6;
constexpr uint8_t CMD_MEASURE_LOW    = 0xE0;
constexpr uint8_t CMD_READ_SERIAL    = 0x89;
constexpr uint8_t CMD_SOFT_RESET     = 0x94;

constexpr uint32_t CONVERSION_MS_HIGH = 9;  // datasheet max 8.2 ms plus margin

// Response frame: T_MSB, T_LSB, CRC_T, RH_MSB, RH_LSB, CRC_RH
constexpr uint8_t FRAME_LEN = 6;

// CRC-8: polynomial 0x31, init 0xFF, no reflection, final XOR 0x00, per word.
constexpr uint8_t CRC_POLY = 0x31;
constexpr uint8_t CRC_INIT = 0xFF;

// Conversion formulas: RH[%] = -6 + 125 * S_RH / 65535, T[C] = -45 + 175 * S_T / 65535
constexpr float RH_OFFSET = -6.0f;
constexpr float RH_SCALE  = 125.0f;
constexpr float T_OFFSET  = -45.0f;
constexpr float T_SCALE   = 175.0f;
constexpr float WORD_MAX  = 65535.0f;

} // namespace sht4x
