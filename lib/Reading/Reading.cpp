#include "Reading.h"

const char* faultName(uint8_t fault) {
    static const char* const names[FAULT_COUNT] = {
        "NONE", "NO_DEVICE", "CRC", "TIMEOUT", "RANGE", "RATE", "STUCK", "NOT_READY"
    };
    return fault < FAULT_COUNT ? names[fault] : "UNKNOWN";
}
