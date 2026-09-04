#include "Sht4xDecoder.h"
#include "Sht4xTypes.h"

namespace sht4x {

uint8_t crc8(const uint8_t* data, size_t len) {
    uint8_t crc = CRC_INIT;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ CRC_POLY) : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

static float clamp(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

float wordToTemperature(uint16_t word) {
    return T_OFFSET + T_SCALE * (float)word / WORD_MAX;
}

float wordToHumidity(uint16_t word) {
    return clamp(RH_OFFSET + RH_SCALE * (float)word / WORD_MAX, 0.0f, 100.0f);
}

uint16_t temperatureToWord(float tC) {
    return (uint16_t)clamp((tC - T_OFFSET) * WORD_MAX / T_SCALE + 0.5f, 0.0f, WORD_MAX);
}

uint16_t humidityToWord(float rh) {
    return (uint16_t)clamp((rh - RH_OFFSET) * WORD_MAX / RH_SCALE + 0.5f, 0.0f, WORD_MAX);
}

bool decode(const uint8_t frame[6], float& tC, float& rh) {
    if (crc8(&frame[0], 2) != frame[2]) return false;
    if (crc8(&frame[3], 2) != frame[5]) return false;
    uint16_t tWord  = (uint16_t)((frame[0] << 8) | frame[1]);
    uint16_t rhWord = (uint16_t)((frame[3] << 8) | frame[4]);
    tC = wordToTemperature(tWord);
    rh = wordToHumidity(rhWord);
    return true;
}

void encode(uint16_t tWord, uint16_t rhWord, uint8_t frame[6]) {
    frame[0] = (uint8_t)(tWord >> 8);
    frame[1] = (uint8_t)(tWord & 0xFF);
    frame[2] = crc8(&frame[0], 2);
    frame[3] = (uint8_t)(rhWord >> 8);
    frame[4] = (uint8_t)(rhWord & 0xFF);
    frame[5] = crc8(&frame[3], 2);
}

} // namespace sht4x
